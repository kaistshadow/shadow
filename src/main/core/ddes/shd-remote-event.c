#include "shadow.h"

#define REMOTE_MESSAGE_TYPE_PACKET  1
#define REMOTE_MESSAGE_TYPE_WINDOW  2

struct receiveArgs {
    int socket;
    RemoteEventProcessor* rep;
};
struct _RemoteQueueElement {
    unsigned char* data;
    int dataLen;
    int remoteMachineSocket;

};
typedef struct _RemoteQueueElement RemoteQueueElement;
struct _RemoteQueue {
    GMutex lock;
    GQueue* items;
    GCond cond;
};
typedef struct _RemoteQueue RemoteQueue;

struct _RemoteEventProcessor {
    int id;
    int slave_count;
    Scheduler* scheduler;
    // for BLEEP DDES #1, #2
    RemoteQueue* remoteEventSenderQueue;
    // for BLEEP DDES #2
    pthread_t remoteEventSender;
    int* sendingSocket;
    // for BLEEP DDES #3
    int masterRecvPort;
    int port;
    pthread_t remoteEventReceiver;
    int* receivingSocket;
    pthread_t* remoteEventReceiveWorker;
    CountDownLatch* startLatch;
    CountDownLatch* windowLatch;
    CountDownLatch* windowUpdated;

    GMutex windowLock;
    SimulationTime nextStartWindow;
    SimulationTime nextEndWindow;
};

RemoteQueue* _remoteEvent_remoteQueue_new() {
    RemoteQueue* rq = g_new0(RemoteQueue, 1);

    g_mutex_init(&(rq->lock));
    rq->items = g_queue_new();
    g_cond_init(&(rq->cond));

    return rq;
}

// for BLEEP DDES #1
int remoteEvent_produce(RemoteEventProcessor* rep, SimulationTime deliverTime, GQuark srcID, GQuark dstID, Packet* packet) {
    if (dstID % rep->slave_count == rep->id) {
        return 1;
    }
    RemoteQueue* rq = rep->remoteEventSenderQueue;
    // process item
    unsigned char* packetStr = NULL;
    int packetStrLength = packet_serialize(packet, &packetStr);
    unsigned char* str = (unsigned char*)malloc(sizeof(unsigned char) * (21 + packetStrLength));
    int idx = 0;
    unsigned char fRemoteMessageType = REMOTE_MESSAGE_TYPE_PACKET;
    memcpy(&str[idx], &fRemoteMessageType, 1);
    idx += 1;
    memcpy(&str[idx], &deliverTime, 8);
    idx += 8;
    memcpy(&str[idx], &srcID, 4);
    idx += 4;
    memcpy(&str[idx], &dstID, 4);
    idx += 4;
    memcpy(&str[idx], &packetStrLength, 4);
    idx += 4;
    memcpy(&str[idx], packetStr, packetStrLength);
    idx += packetStrLength;
    free(packetStr);

    RemoteQueueElement* e = g_new0(RemoteQueueElement, 1);
    e->data = str;
    e->dataLen = idx;
    // TODO: make variation of machine selection rule.
    e->remoteMachineSocket = rep->sendingSocket[dstID % rep->slave_count];

    // lock mutex
    g_mutex_lock(&(rq->lock));
    // add the new item to the container
    g_queue_push_tail(rq->items, e);
    // signal through cond
    g_cond_signal(&(rq->cond));
    // unlock mutex
    g_mutex_unlock(&(rq->lock));
    return 0;
}
// for BLEEP DDES #2
RemoteQueueElement* _remoteEvent_wait_and_get(RemoteQueue* rq) {
    // lock mutex
    g_mutex_lock(&(rq->lock));
    // check container empty
    //     cond_wait
    if (g_queue_is_empty(rq->items)) {
        g_cond_wait(&(rq->cond), &(rq->lock));
    }
    // remove oldest
    RemoteQueueElement* e = g_queue_pop_head(rq->items);
    // unlock mutex
    g_mutex_unlock(&(rq->lock));
    // return
    return e;
}
void* _remoteEvent_processEvent(void* arg) {
    RemoteQueue* rq = (RemoteQueue*)arg;
    while(TRUE) {
        RemoteQueueElement* e = _remoteEvent_wait_and_get(rq);
        if (e->dataLen == 0) {
            // break;
            break;
        }
        send(e->remoteMachineSocket, e->data, e->dataLen, 0);
        free(e->data);
        g_free(e);
    }
}
// for BLEEP DDES #3
void* _remoteEvent_receiveTaskloop(void* args) {
    int client_socket = ((struct receiveArgs*)args)->socket;
    RemoteEventProcessor* rep = ((struct receiveArgs*)args)->rep;

    while(TRUE) {
        // message type
        unsigned char fRemoteMessageType;
        recv(client_socket, &fRemoteMessageType, 1, 0);
        if (fRemoteMessageType == REMOTE_MESSAGE_TYPE_PACKET) {
            // deliverTime
            SimulationTime deliverTime;
            recv(client_socket, &deliverTime, 8, 0);
            // srcID
            guint32 srcID;
            recv(client_socket, &srcID, 4, 0);
            // dstID
            guint32 dstID;
            recv(client_socket, &dstID, 4, 0);
            // packet
            int packetStrLength;
            recv(client_socket, &packetStrLength, 4, 0);
            unsigned char* packet_data = (unsigned char*)malloc(sizeof(unsigned char)*(packetStrLength));
            recv(client_socket, packet_data, packetStrLength, 0);
            Packet* packet = packet_deserialize(packet_data);
            worker_processRemotePacket(rep->scheduler, packet, deliverTime, srcID, dstID);
        } else if (fRemoteMessageType == REMOTE_MESSAGE_TYPE_WINDOW) {
            // start
            SimulationTime start;
            recv(client_socket, &start, 8, 0);
            // end
            SimulationTime end;
            recv(client_socket, &end, 8, 0);
            g_mutex_lock(&(rep->windowLock));
            if (rep->nextStartWindow == 0 || rep->nextStartWindow > start) {
                rep->nextStartWindow = start;
            }
            if (rep->nextEndWindow == 0 || rep->nextEndWindow > end) {
                rep->nextEndWindow = end;
            }
            g_mutex_unlock(&(rep->windowLock));
            countdownlatch_countDownAwait(rep->windowLatch);
            countdownlatch_countDownAwait(rep->windowUpdated);
        } else {
            // error
            break;
        }
    }
}
void* _remoteEvent_receiver_run(void* arg) {
    RemoteEventProcessor* rep = (RemoteEventProcessor*)arg;
    int reception_socket;
    int c;
    struct sockaddr_in server, client;
    rep->remoteEventReceiveWorker = (pthread_t *)malloc(sizeof(pthread_t) * rep->slave_count);

    reception_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (reception_socket == -1) {
        printf("Cannot create socket\n");
        return NULL;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(rep->port);

    if (bind(reception_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Cannot bind socket\n");
        close(reception_socket);
        return NULL;
    }

    listen(reception_socket, rep->slave_count);
    c = sizeof(struct sockaddr_in);

    countdownlatch_countDown(rep->startLatch);

    int idx = 0;
    while(idx < rep->slave_count) {
        rep->receivingSocket[idx] = accept(reception_socket, (struct sockaddr *)&client, (socklen_t*)&c);
        struct receiveArgs a = {rep->receivingSocket[idx], rep};
        if (pthread_create(&rep->remoteEventReceiveWorker[idx], NULL, _remoteEvent_receiveTaskloop, (void*)&a) < 0) {
            perror("Cannot create thread\n");
            return NULL;
        }
        idx++;
    }

    while(idx > 0) {
        pthread_join(rep->remoteEventReceiveWorker[idx-1], NULL);
        idx--;
    }

    close(reception_socket);

    return NULL;
}
int _remoteEvent_registerIPSocket(RemoteEventProcessor* rep, int id, char* ip, int port) {
    struct sockaddr_in server;

    rep->sendingSocket[id] = socket(AF_INET, SOCK_STREAM, 0);
    if (rep->sendingSocket[id] == -1) {
        printf("Cannot create socket\n");
        return -1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_port = htons(port);

    // connect the client socket to server socket
    if (connect(rep->sendingSocket[id], (struct sockaddr*)&server, sizeof(server)) != 0) {
        printf("Cannot connect to the socket\n");
        return -1;
    }
    return 0;
}
int _remoteEvent_receiver_setup(RemoteEventProcessor* rep) {
    int reception_socket;
    int client_socket, c;
    struct sockaddr_in server, client;

    reception_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (reception_socket == -1) {
        printf("Cannot create socket\n");
        return -1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(rep->masterRecvPort);

    if (bind(reception_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Cannot bind socket\n");
        close(reception_socket);
        return -1;
    }

    listen(reception_socket, 1);
    c = sizeof(struct sockaddr_in);

    // take action from master
    printf("Listening Master...");
    client_socket = accept(reception_socket, (struct sockaddr *)&client, (socklen_t*)&c);
    printf("Connected to master.\n");

    // own slave id
    recv(client_socket, &rep->id, 4, 0);
    recv(client_socket, &rep->slave_count, 4, 0);

    int idx = 0;
    rep->sendingSocket = (int*)malloc(sizeof(int) * rep->slave_count);
    rep->receivingSocket = (int*)malloc(sizeof(int) * rep->slave_count);
    memset(rep->sendingSocket, 0, sizeof(int) * rep->slave_count);
    memset(rep->receivingSocket, 0, sizeof(int) * rep->slave_count);

    rep->startLatch = countdownlatch_new(1);
    gint ddesRet = pthread_create(&(rep->remoteEventReceiver), NULL, _remoteEvent_receiver_run, rep);
    if(ddesRet != 0) {
        critical("unable to create event_receiver thread");
        return -1;
    }
    countdownlatch_await(rep->startLatch);

    rep->windowLatch = countdownlatch_new(rep->slave_count + 1);
    rep->windowUpdated = countdownlatch_new(rep->slave_count + 1);

    int slave_receiver_ready = 1;
    send(client_socket, &slave_receiver_ready, 4, 0);
    int slave_receiver_ready_resp = 0;
    recv(client_socket, &slave_receiver_ready_resp, 4, 0);

    while(idx < rep->slave_count) {
        int sLen = -1;
        char ipbuf[20];
        recv(client_socket, &sLen, 4, 0);
        recv(client_socket, ipbuf, sLen, 0);
        ipbuf[sLen] = 0;
        _remoteEvent_registerIPSocket(rep, idx, ipbuf, rep->port);
        idx++;
    }

    int simple_response = 1;
    send(client_socket, &simple_response, 4, 0);
    int simple_request = 0;
    recv(client_socket, &simple_request, 4, 0);
    return 0;
}

void remoteEvent_broadcastNextWindow(RemoteEventProcessor* rep, SimulationTime start, SimulationTime end) {
    int i;
    RemoteQueue* rq = rep->remoteEventSenderQueue;
    for(i=0; i<rep->slave_count; i++) {
        // process item
        unsigned char* str = (unsigned char*)malloc(sizeof(unsigned char) + sizeof(SimulationTime) * 2);
        int idx = 0;
        unsigned char fRemoteMessageType = REMOTE_MESSAGE_TYPE_WINDOW;
        memcpy(&str[idx], &fRemoteMessageType, 1);
        idx += 1;
        memcpy(&str[idx], &start, 8);
        idx += 8;
        memcpy(&str[idx], &end, 8);
        idx += 8;

        RemoteQueueElement* e = g_new0(RemoteQueueElement, 1);
        e->data = str;
        e->dataLen = idx;
        e->remoteMachineSocket = rep->sendingSocket[i];

        // lock mutex
        g_mutex_lock(&(rq->lock));
        // add the new item to the container
        g_queue_push_tail(rq->items, e);
        // signal through cond
        g_cond_signal(&(rq->cond));
        // unlock mutex
        g_mutex_unlock(&(rq->lock));
    }
    return;
}

void remoteEvent_makeWindowConsensus(RemoteEventProcessor* rep, SimulationTime* startRef, SimulationTime* endRef) {
    countdownlatch_countDownAwait(rep->windowLatch);
    *startRef = rep->nextStartWindow;
    *endRef = rep->nextEndWindow;
    rep->nextStartWindow = 0;
    rep->nextEndWindow = 0;
    countdownlatch_reset(rep->windowLatch);
    countdownlatch_countDownAwait(rep->windowUpdated);
    countdownlatch_reset(rep->windowUpdated);
}

RemoteEventProcessor* remoteEvent_new(Scheduler* scheduler, int masterRecvPort, int slavePort) {
    gint ddesRet;
    RemoteEventProcessor* rep = g_new0(RemoteEventProcessor, 1);
    rep->scheduler = scheduler;
    rep->masterRecvPort = masterRecvPort;
    rep->port = slavePort;
    // for BLEEP DDES #1
    rep->remoteEventSenderQueue = _remoteEvent_remoteQueue_new();
    rep->nextStartWindow = 0;
    rep->nextEndWindow = 0;
    g_mutex_init(&(rep->windowLock));
    // for BLEEP DDES #2
    ddesRet = pthread_create(&(rep->remoteEventSender), NULL, _remoteEvent_processEvent, rep->remoteEventSenderQueue);
    if(ddesRet != 0) {
        critical("unable to create event_receiver thread");
        return NULL;
    }
    // for BLEEP DDES #3
    _remoteEvent_receiver_setup(rep);
    return rep;
}

void remoteEvent_free(RemoteEventProcessor* rep) {
    pthread_cancel(rep->remoteEventSender);
    for(int i=0; i<rep->slave_count; i++) {
        pthread_cancel(rep->remoteEventReceiveWorker[i]);
    }
    for(int i=0; i<rep->slave_count; i++) {
        close(rep->sendingSocket[i]);
        close(rep->receivingSocket[i]);
    }
    g_mutex_clear(&(rep->remoteEventSenderQueue->lock));
    free(rep->sendingSocket);
    free(rep->receivingSocket);
    free(rep->remoteEventReceiveWorker);
    g_free(rep->remoteEventSenderQueue);
    g_mutex_clear(&(rep->windowLock));
    countdownlatch_free(rep->startLatch);
    countdownlatch_free(rep->windowLatch);
    countdownlatch_free(rep->windowUpdated);
    g_free(rep);
}

int remoteEvent_checkAssigned(RemoteEventProcessor* rep, GQuark id) {
    return id % rep->slave_count == rep->id;
}