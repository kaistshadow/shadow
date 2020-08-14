/*
 * The Shadow Simulator
 * Copyright (c) 2010-2011, Rob Jansen
 * See LICENSE for licensing information
 */

#ifndef SHD_BLEEP_PTRSEND_H_
#define SHD_BLEEP_PTRSEND_H_

#include "shadow.h"

// LEVEL 1 //
//static gssize _process_emu_sendHelper(Process* proc, gint fd, gconstpointer buf, gsize n, gint flags, const struct sockaddr* addr, socklen_t len);
static gssize _process_emu_ptrsendHelper(Process* proc, gint fd, gconstpointer sendtargetptr, gsize n, gint flags, const struct sockaddr* addr, socklen_t len);
//static gssize _process_emu_recvHelper(Process* proc, gint fd, gconstpointer buf, gsize n, gint flags, const struct sockaddr* addr, socklen_t len);
static gssize _process_emu_ptrrecvHelper(Process* proc, gint fd, gconstpointer recvtargetptr, gsize n, gint flags, const struct sockaddr* addr, socklen_t len);

// LEVEL 2 //
//gint host_sendUserData(Host* host, gint handle, gconstpointer buffer, gsize nBytes, in_addr_t ip, in_addr_t port, gsize* bytesCopied);
gint host_ptrsendUserData(Host* host, gint handle, gconstpointer sendtargetptr, gsize nBytes, in_addr_t ip, in_addr_t port, gsize* bytesCopied);
//gint host_receiveUserData(Host* host, gint handle, gpointer buffer, gsize nBytes, in_addr_t* ip, in_port_t* port, gsize* bytesCopied);
gint host_ptrreceiveUserData(Host* host, gint handle, gpointer recvtargetptr, gsize nBytes, in_addr_t* ip, in_port_t* port, gsize* bytesCopied);

// LEVEL 3 //
//gssize transport_sendUserData(Transport* transport, gconstpointer buffer, gsize nBytes,
//                              in_addr_t ip, in_port_t port);
gssize transport_ptrsendUserData(Transport* transport, gconstpointer sendtargetptr, gsize nBytes,
                              in_addr_t ip, in_port_t port);
//gssize transport_receiveUserData(Transport* transport, gpointer buffer, gsize nBytes,
//                                 in_addr_t* ip, in_port_t* port);
gssize transport_ptrreceiveUserData(Transport* transport, gpointer recvtargetptr, gsize nBytes,
                                 in_addr_t* ip, in_port_t* port);
// LEVEL 3 - channel //
//static gssize channel_sendUserData(Channel* channel, gconstpointer buffer, gsize nBytes, in_addr_t ip, in_port_t port);
    //static gssize channel_ptrsendUserData(Channel* channel, gconstpointer sendtargetptr, gsize nBytes, in_addr_t ip, in_port_t port);
//static gssize channel_receiveUserData(Channel* channel, gpointer buffer, gsize nBytes, in_addr_t* ip, in_port_t* port);
    //static gssize channel_ptrreceiveUserData(Channel* channel, gpointer recvtargetptr, gsize nBytes, in_addr_t* ip, in_port_t* port);
// LEVEL 3 - tcp //
//gssize tcp_sendUserData(TCP* tcp, gconstpointer buffer, gsize nBytes, in_addr_t ip, in_port_t port);
gssize tcp_ptrsendUserData(TCP* tcp, gconstpointer sendtargetptr, gsize nBytes, in_addr_t ip, in_port_t port);
//gssize tcp_receiveUserData(TCP* tcp, gpointer buffer, gsize nBytes, in_addr_t* ip, in_port_t* port);
gssize tcp_ptrreceiveUserData(TCP* tcp, gpointer recvtargetptr, gsize nBytes, in_addr_t* ip, in_port_t* port);
// LEVEL 3 - udp //
//gssize udp_sendUserData(UDP* udp, gconstpointer buffer, gsize nBytes, in_addr_t ip, in_port_t port);
    //gssize udp_ptrsendUserData(UDP* udp, gconstpointer sendtargetptr, gsize nBytes, in_addr_t ip, in_port_t port);
//gssize udp_receiveUserData(UDP* udp, gpointer buffer, gsize nBytes, in_addr_t* ip, in_port_t* port);
    //gssize udp_ptrreceiveUserData(UDP* udp, gpointer recvtargetptr, gsize nBytes, in_addr_t* ip, in_port_t* port);

#endif /* SHD_BLEEP_PTRSEND_H_ */