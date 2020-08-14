/*
 * The Shadow Simulator
 * Copyright (c) 2010-2011, Rob Jansen
 * See LICENSE for licensing information
 */
#include "shadow.h"

// LEVEL 1 //
static gssize _process_emu_ptrsendHelper(Process* proc, gint fd, gconstpointer sendtargetptr, gsize n, gint flags,
                                      const struct sockaddr* addr, socklen_t len) {
    /* this function MUST be called after switching in shadow context */
    utility_assert(proc->activeContext == PCTX_SHADOW);

    /* TODO flags are ignored */
    /* make sure this is a socket */
    if(!host_isShadowDescriptor(proc->host, fd)){
        _process_setErrno(proc, EBADF);
        return -1;
    }

    in_addr_t ip = 0;
    in_port_t port = 0;

    /* check if they specified an address to send to */
    if(addr != NULL && len >= sizeof(struct sockaddr_in)) {
        struct sockaddr_in* si = (struct sockaddr_in*) addr;
        ip = si->sin_addr.s_addr;
        port = si->sin_port;
    }

    gsize bytes = 0;
    gint result = host_ptrsendUserData(proc->host, fd, sendtargetptr, n, ip, port, &bytes);      // DIFF

    if(result != 0) {
        _process_setErrno(proc, result);
        return -1;
    }
    return (gssize) bytes;
}
static gssize _process_emu_ptrrecvHelper(Process* proc, gint fd, gpointer recvtargetptr, size_t n, gint flags,
                                      struct sockaddr* addr, socklen_t* len) {
    /* this function MUST be called after switching in shadow context */
    utility_assert(proc->activeContext == PCTX_SHADOW);

    /* TODO flags are ignored */
    /* make sure this is a socket */
    if(!host_isShadowDescriptor(proc->host, fd)){
        _process_setErrno(proc, EBADF);
        return -1;
    }

    in_addr_t ip = 0;
    in_port_t port = 0;

    gsize bytes = 0;
    gint result = host_ptrreceiveUserData(proc->host, fd, recvtargetptr, n, &ip, &port, &bytes);    // DIFF

    if(result != 0) {
        _process_setErrno(proc, result);
        return -1;
    }

    /* check if they wanted to know where we got the data from */
    if(addr != NULL && len != NULL && *len >= sizeof(struct sockaddr_in)) {
        struct sockaddr_in* si = (struct sockaddr_in*) addr;
        si->sin_addr.s_addr = ip;
        si->sin_port = port;
        si->sin_family = AF_INET;
        *len = sizeof(struct sockaddr_in);
    }

    return (gssize) bytes;
}

// LEVEL 2 //
gint host_ptrsendUserData(Host* host, gint handle, gconstpointer sendtargetptr, gsize nBytes,
                       in_addr_t ip, in_addr_t port, gsize* bytesCopied) {
    MAGIC_ASSERT(host);
    utility_assert(bytesCopied);

    Descriptor* descriptor = host_lookupDescriptor(host, handle);
    if(descriptor == NULL) {
        warning("descriptor handle '%i' not found", handle);
        return EBADF;
    }

    DescriptorStatus status = descriptor_getStatus(descriptor);
    if(status & DS_CLOSED) {
        warning("descriptor handle '%i' not a valid open descriptor", handle);
        return EBADF;
    }

    DescriptorType dtype = descriptor_getType(descriptor);
    if(dtype != DT_TCPSOCKET && dtype != DT_UDPSOCKET && dtype != DT_PIPE) {
        return EBADF;
    }

    Transport* transport = (Transport*) descriptor;

    /* we should block if our cpu has been too busy lately */
    if(cpu_isBlocked(host->cpu)) {
        debug("blocked on CPU when trying to send %"G_GSIZE_FORMAT" bytes from socket %i", nBytes, handle);

        /*
         * immediately schedule an event to tell the socket it can write. it will
         * pop out when the CPU delay is absorbed. otherwise we could miss writes.
         */
        descriptor_adjustStatus(descriptor, DS_WRITABLE, TRUE);

        return EAGAIN;
    }

    if(dtype == DT_UDPSOCKET) {
        /* make sure that we have somewhere to send it */
        Socket* socket = (Socket*)transport;
        if(ip == 0 || port == 0) {
            /* its ok as long as they setup a default destination with connect()*/
            if(socket->peerIP == 0 || socket->peerPort == 0) {
                /* we have nowhere to send it */
                return EDESTADDRREQ;
            }
        }

        /* if this socket is not bound, do an implicit bind to a random port */
        if(!socket_isBound(socket)) {
            ProtocolType ptype = socket_getProtocol(socket);
            in_addr_t bindAddress = ip == htonl(INADDR_LOOPBACK) ? htonl(INADDR_LOOPBACK) :
                                    address_toNetworkIP(host->defaultAddress);
            in_port_t bindPort = _host_getRandomFreePort(host, ptype, bindAddress, 0, 0);
            if(!bindPort) {
                return EADDRNOTAVAIL;
            }

            /* bind port and set associations */
            _host_associateInterface(host, socket, bindAddress, bindPort, 0, 0);
        }
    }

    if(dtype == DT_TCPSOCKET) {
        gint error = tcp_getConnectError((TCP*) transport);
        if(error != EISCONN) {
            if(error == EALREADY) {
                /* we should not be writing if the connection is not ready */
                descriptor_adjustStatus(descriptor, DS_WRITABLE, FALSE);
                return EWOULDBLOCK;
            } else {
                return error;
            }
        }
    }

    gssize n = transport_ptrsendUserData(transport, sendtargetptr, nBytes, ip, port);   // DIFF
    if(n > 0) {
        /* user is writing some bytes. */
        *bytesCopied = (gsize)n;
    } else if(n == -2) {
        return ENOTCONN;
    } else if(n == -3) {
        return EPIPE;
    } else if(n < 0) {
        return EWOULDBLOCK;
    }

    return 0;
}
gint host_ptrreceiveUserData(Host* host, gint handle, gpointer recvtargetptr, gsize nBytes,
                          in_addr_t* ip, in_port_t* port, gsize* bytesCopied) {
    MAGIC_ASSERT(host);
    utility_assert(ip && port && bytesCopied);

    Descriptor* descriptor = host_lookupDescriptor(host, handle);
    if(descriptor == NULL) {
        warning("descriptor handle '%i' not found", handle);
        return EBADF;
    }

    /* user can still read even if they already called close (DS_CLOSED).
     * in this case, the descriptor will be unreffed and deleted when it no
     * longer has data, and the above lookup will fail and return EBADF.
     */

    DescriptorType type = descriptor_getType(descriptor);
    if(type != DT_TCPSOCKET && type != DT_UDPSOCKET && type != DT_PIPE) {
        return EBADF;
    }

    Transport* transport = (Transport*) descriptor;

    /* we should block if our cpu has been too busy lately */
    if(cpu_isBlocked(host->cpu)) {
        debug("blocked on CPU when trying to send %"G_GSIZE_FORMAT" bytes from socket %i", nBytes, handle);

        /*
         * immediately schedule an event to tell the socket it can read. it will
         * pop out when the CPU delay is absorbed. otherwise we could miss reads.
         */
        descriptor_adjustStatus(descriptor, DS_READABLE, TRUE);

        return EAGAIN;
    }

    gssize n = transport_ptrreceiveUserData(transport, recvtargetptr, nBytes, ip, port);    // DIFF
    if(n > 0) {
        /* user is reading some bytes. */
        *bytesCopied = (gsize)n;
    } else if(n == -2) {
        return ENOTCONN;
    } else if(n < 0) {
        return EWOULDBLOCK;
    }

    return 0;
}

// LEVEL 3 //
gssize transport_ptrsendUserData(Transport* transport, gconstpointer sendtargetptr, gsize nBytes,
                              in_addr_t ip, in_port_t port) {
    MAGIC_ASSERT(transport);
    MAGIC_ASSERT(transport->vtable);
    return transport->vtable->send(transport, sendtargetptr, nBytes, ip, port);
}
gssize transport_ptrreceiveUserData(Transport* transport, gpointer recvtargetptr, gsize nBytes,
                                 in_addr_t* ip, in_port_t* port) {
    MAGIC_ASSERT(transport);
    MAGIC_ASSERT(transport->vtable);
    return transport->vtable->receive(transport, recvtargetptr, nBytes, ip, port);
}
// LEVEL 3 - channel //
// not implemented yet
// LEVEL 3 - tcp //
gssize tcp_sendUserData(TCP* tcp, gconstpointer buffer, gsize nBytes, in_addr_t ip, in_port_t port) {
    MAGIC_ASSERT(tcp);

    /* return 0 to signal close, if necessary */
    if(tcp->error & TCPE_SEND_EOF)
    {
        if(tcp->flags & TCPF_EOF_WR_SIGNALED) {
            /* we already signaled close, now its an error */
            return -2;
        } else {
            /* we have not signaled close, do that now */
            _tcp_endOfFileSignalled(tcp, TCPF_EOF_WR_SIGNALED);
            return -3;
        }
    }

    /* maximum data we can send network, o/w tcp truncates and only sends 65536*/
    gsize acceptable = MIN(nBytes, 65535);
    gsize space = _tcp_getBufferSpaceOut(tcp);
    gsize remaining = MIN(acceptable, space);

    /* break data into segments and send each in a packet */
    gsize maxPacketLength = CONFIG_MTU - CONFIG_HEADER_SIZE_TCPIPETH;
    gsize bytesCopied = 0;

    /* create as many packets as needed */
    while(remaining > 0) {
        gsize copyLength = MIN(maxPacketLength, remaining);

        /* use helper to create the packet */
        Packet* packet = _tcp_createPacket(tcp, PTCP_ACK, buffer + bytesCopied, copyLength);            // FOCUS
        if(copyLength > 0) {
            /* we are sending more user data */
            tcp->send.end++;
        }

        /* buffer the outgoing packet in TCP */
        _tcp_bufferPacketOut(tcp, packet);

        /* the output buffer holds the packet ref now */
        packet_unref(packet);

        remaining -= copyLength;
        bytesCopied += copyLength;
    }

    debug("%s <-> %s: sending %"G_GSIZE_FORMAT" user bytes", tcp->super.boundString, tcp->super.peerString, bytesCopied);

    /* now flush as much as possible out to socket */
    _tcp_flush(tcp);                                                // FOCUS

    return (gssize) (bytesCopied == 0 ? -1 : bytesCopied);
}
gssize tcp_receiveUserData(TCP* tcp, gpointer buffer, gsize nBytes, in_addr_t* ip, in_port_t* port) {
    MAGIC_ASSERT(tcp);

    /*
     * TODO
     * We call descriptor_adjustStatus too many times here, to handle the readable
     * state of the socket at times when we have a partially read packet. Consider
     * adding a required hook for socket subclasses so the socket layer can
     * query TCP for readability status.
     */

    /* make sure we pull in all readable user data */
    _tcp_flush(tcp);

    gsize remaining = nBytes;
    gsize bytesCopied = 0;
    gsize totalCopied = 0;
    gsize offset = 0;
    gsize copyLength = 0;

    /* check if we have a partial packet waiting to get finished */
    if(remaining > 0 && tcp->partialUserDataPacket) {
        guint partialLength = packet_getPayloadLength(tcp->partialUserDataPacket);
        guint partialBytes = partialLength - tcp->partialOffset;
        utility_assert(partialBytes > 0);

        copyLength = MIN(partialBytes, remaining);
        bytesCopied = packet_copyPayload(tcp->partialUserDataPacket, tcp->partialOffset, buffer, copyLength);
        totalCopied += bytesCopied;
        remaining -= bytesCopied;
        offset += bytesCopied;

        if(bytesCopied >= partialBytes) {
            /* we finished off the partial packet */
            packet_addDeliveryStatus(tcp->partialUserDataPacket, PDS_RCV_SOCKET_DELIVERED);
            packet_unref(tcp->partialUserDataPacket);
            tcp->partialUserDataPacket = NULL;
            tcp->partialOffset = 0;
        } else {
            /* still more partial bytes left */
            tcp->partialOffset += bytesCopied;
            utility_assert(remaining == 0);
        }
    }

    while(remaining > 0) {
        /* if we get here, we should have read the partial packet above, or
         * broken out below */
        utility_assert(tcp->partialUserDataPacket == NULL);
        utility_assert(tcp->partialOffset == 0);

        /* get the next buffered packet - we'll always need it.
         * this could mark the socket as unreadable if this is its last packet.*/
        Packet* packet = socket_removeFromInputBuffer((Socket*)tcp);
        if(!packet) {
            /* no more packets or partial packets */
            break;
        }

        guint packetLength = packet_getPayloadLength(packet);
        copyLength = MIN(packetLength, remaining);
        bytesCopied = packet_copyPayload(packet, 0, buffer + offset, copyLength);
        totalCopied += bytesCopied;
        remaining -= bytesCopied;
        offset += bytesCopied;

        if(bytesCopied < packetLength) {
            /* we were only able to read part of this packet */
            tcp->partialUserDataPacket = packet;
            tcp->partialOffset = bytesCopied;
            break;
        }

        /* we read the entire packet, and are now finished with it */
        packet_addDeliveryStatus(packet, PDS_RCV_SOCKET_DELIVERED);
        packet_unref(packet);
    }

    /* now we update readability of the socket */
    if((socket_getInputBufferLength(&(tcp->super)) > 0) || (tcp->partialUserDataPacket != NULL)) {
        /* we still have readable data */
        descriptor_adjustStatus(&(tcp->super.super.super), DS_READABLE, TRUE);
    } else {
        /* all of our ordered user data has been read */
        if((tcp->unorderedInputLength == 0) && (tcp->error & TCPE_RECEIVE_EOF)) {
            /* there is no more unordered data either, and we need to signal EOF */
            if(totalCopied > 0) {
                /* we just received bytes, so we can't EOF until the next call.
                 * make sure we stay readable so we DO actually EOF the socket */
                descriptor_adjustStatus(&(tcp->super.super.super), DS_READABLE, TRUE);
            } else {
                /* OK, no more data and nothing just received. */
                if(tcp->flags & TCPF_EOF_RD_SIGNALED) {
                    /* we already signaled close, now its an error */
                    return -2;
                } else {
                    /* we have not signaled close, do that now and close out the socket */
                    _tcp_endOfFileSignalled(tcp, TCPF_EOF_RD_SIGNALED);
                    return 0;
                }
            }
        } else {
            /* our socket still has unordered data or is still open, but empty for now */
            descriptor_adjustStatus(&(tcp->super.super.super), DS_READABLE, FALSE);
        }
    }

    /* update the receive buffer size based on new packets received */
    if(tcp->autotune.isEnabled && !tcp->autotune.userDisabledReceive) {
        Host* host = worker_getActiveHost();
        if(host_autotuneReceiveBuffer(host)) {
            _tcp_autotuneReceiveBuffer(tcp, totalCopied);
        }
    }

    /* if we have advertised a 0 window because the application wasn't reading,
     * we now have to update the window and let the sender know */
    _tcp_updateReceiveWindow(tcp);
    if(tcp->receive.window > tcp->send.lastWindow && !tcp->receive.windowUpdatePending) {
        /* our receive window just opened, make sure the sender knows it can
         * send more. otherwise we get into a deadlock situation!
         * make sure we don't send multiple events when read is called many times per instant */
        descriptor_ref(tcp);

        Task* updateWindowTask = task_new((TaskCallbackFunc)_tcp_sendWindowUpdate,
                                          tcp, NULL, descriptor_unref, NULL);
        worker_scheduleTask(updateWindowTask, 1);
        task_unref(updateWindowTask);

        tcp->receive.windowUpdatePending = TRUE;
    }

    debug("%s <-> %s: receiving %"G_GSIZE_FORMAT" user bytes", tcp->super.boundString, tcp->super.peerString, totalCopied);

    return (gssize) (totalCopied == 0 ? -1 : totalCopied);
}
// LEVEL 3 - udp //
// not implemented yet

// LEVEL 4 //
static Packet* _tcp_createPacket(TCP* tcp, enum ProtocolTCPFlags flags, gconstpointer payload, gsize payloadLength) {
    MAGIC_ASSERT(tcp);

    /*
     * packets from children of a server must appear to be coming from the server
     */
    in_addr_t sourceIP = tcp_getIP(tcp);
    in_port_t sourcePort = (tcp->child) ? tcp->child->parent->super.boundPort :
                           tcp->super.boundPort;

    in_addr_t destinationIP = tcp_getPeerIP(tcp);
    in_port_t destinationPort = (tcp->server) ? tcp->server->lastPeerPort : tcp->super.peerPort;

    if(sourceIP == htonl(INADDR_ANY)) {
        /* source interface depends on destination */
        if(destinationIP == htonl(INADDR_LOOPBACK)) {
            sourceIP = htonl(INADDR_LOOPBACK);
        } else {
            sourceIP = host_getDefaultIP(worker_getActiveHost());
        }
    }

    utility_assert(sourceIP && sourcePort && destinationIP && destinationPort);

    /* make sure our receive window is up to date before putting it in the packet */
    _tcp_updateReceiveWindow(tcp);

    /* control packets have no sequence number
     * (except FIN, so we close after sending everything) */
    gboolean isFinNotAck = ((flags & PTCP_FIN) && !(flags & PTCP_ACK));
    guint sequence = payloadLength > 0 || isFinNotAck ? tcp->send.next : 0;

    /* create the TCP packet. the ack, window, and timestamps will be set in _tcp_flush */
    Host* host = worker_getActiveHost();
    Packet* packet = packet_new(payload, payloadLength, (guint)host_getID(host), host_getNewPacketID(host));
    packet_setTCP(packet, flags, sourceIP, sourcePort, destinationIP, destinationPort, sequence);
    packet_addDeliveryStatus(packet, PDS_SND_CREATED);

    /* update sequence number */
    if(sequence > 0) {
        tcp->send.next++;
    }

    return packet;
}