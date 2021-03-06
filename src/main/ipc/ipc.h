//
// Created by ilios on 20. 8. 3..
//

#ifndef BLEEP_IPC_H
#define BLEEP_IPC_H


#include "shadow.h"

typedef struct _IPC_CONF IPC_Conf;

struct _IPC_CONF {
     gboolean IPCenabled;
     void *zmq_context;
     void *zmq_data_socket;
};

gboolean check_ipc_server();
gboolean connect_ipc();
void enable_ipc();
void disable_ipc();

gboolean is_ipc_enabled();

void sendIPC_tcp_connect(int fd, const struct sockaddr* addr, socklen_t len);
void sendIPC_tcp_send(Socket* socket, int fd, const void *buf, size_t n, int flags);
void sendIPC_tcp_recv(Socket* socket, int fd, void *buf, size_t n);


#endif //BLEEP_IPC_H
