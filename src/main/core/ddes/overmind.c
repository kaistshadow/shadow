#include "shadow.h"

#include <pthread.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct _slave_info slave_info;
typedef struct _overmind overmind;

struct _slave_info {
    overmind* o;
    char* ip;
    int port;
    int slave_id;
};
struct _overmind {
    slave_info** slaves;
    int slave_count;
    CountDownLatch* slave_receiver_ready_latch;
    CountDownLatch* latch;
};

void* advent(void* arg) {
    int fd;
    struct sockaddr_in server;

    slave_info* si = (slave_info*)arg;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        printf("Cannot create socket\n");
        return NULL;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(si->ip);
    server.sin_port = htons(si->port);

    // connect the client socket to server socket
    if (connect(fd, (struct sockaddr*)&server, sizeof(server)) != 0) {
        printf("Cannot connect to the socket\n");
        close(fd);
        return NULL;
    }

    // pass message
    /*
     * send below:
     * 1. (int)slave id
     * 2. (int)slave count
     * 3. (var)(length+ip) set
     */
    send(fd, &(si->slave_id), 4, 0);
    int slave_count = si->o->slave_count;
    send(fd, &slave_count, 4, 0);

    // slave receiver ready
    // receive response
    int res = 0;
    recv(fd, &res, 4, 0);
    if (res != 1) {
        printf("error occured in response\n");
        close(fd);
        return NULL;
    }
    // good to go
    countdownlatch_countDownAwait(si->o->slave_receiver_ready_latch);
    // all threads are good to go, then send good to go message
    int req = 1;
    send(fd, &req, 4, 0);

    for (int i=0; i<slave_count; i++) {
        int sLen = strlen(si->o->slaves[i]->ip);
        send(fd, &sLen, 4, 0);
        send(fd, si->o->slaves[i]->ip, sLen, 0);
    }

    // receive response
    res = 0;
    recv(fd, &res, 4, 0);
    if (res != 1) {
        printf("error occured in response\n");
        close(fd);
        return NULL;
    }

    // good to go
    countdownlatch_countDownAwait(si->o->latch);

    // all threads are good to go, then send good to go message
    req = 1;
    send(fd, &req, 4, 0);

    close(fd);
    return NULL;
}

void overmind_add_slave(overmind* o, const char* ip, int port) {
    slave_info* si = (slave_info*)malloc(sizeof(slave_info));
    si->o = o;
    si->slave_id = (o->slave_count)++;
    si->ip = (char*)malloc(sizeof(char) * (strlen(ip) + 1));
    memcpy(si->ip, ip, strlen(ip));
    si->ip[strlen(ip)] = 0;
    si->port = port;
    o->slaves[si->slave_id] = si;
}
void overmind_free(overmind* o) {
    int i;
    for (i=0; i<o->slave_count; i++) {
        free(o->slaves[i]);
    }
    free(o->slaves);
    free(o);
}
void overmind_start(overmind* o) {
    int i;
    // set countdown latch
    o->slave_receiver_ready_latch = countdownlatch_new(o->slave_count);
    o->latch = countdownlatch_new(o->slave_count);

    pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * o->slave_count);
    for (i=0; i<o->slave_count; i++) {
        if (pthread_create(&threads[i], NULL, advent, (void*)o->slaves[i]) < 0) {
            printf("Cannot create thread\n");
            return;
        }
    }
    for (i = 0; i<o->slave_count; i++) {
        pthread_join(threads[i], NULL);
    }
    countdownlatch_free(o->slave_receiver_ready_latch);
    countdownlatch_free(o->latch);
}
int main(int argc, char* argv[]) {
    int slavePort = 8879;

    overmind* o = (overmind*)malloc(sizeof(overmind));

    o->slaves = (slave_info**)malloc(sizeof(slave_info*) * 1);
    overmind_add_slave(o, "127.0.0.1", 8879);

    overmind_start(o);

    overmind_free(o);
}