#include "shadow.h"

struct _RemoteQueueElement {
    char* data;
    int dataLen;
    int remoteMachineSocket;

};
typedef struct _RemoteQueueElement RemoteQueueElement;
struct _RemoteQueue {
    GMutex lock;
    GQueue* items;
    GCond cond;

    GMutex producedCntLock;
    int producedCnt;
    int fRoundEnd;
    CountDownLatch* roundLatch;
};
typedef struct _RemoteQueue RemoteQueue;

struct _RemoteEventProcessor {
    int id;
    int slave_count;
    // for BLEEP DDES #1, #2
    RemoteQueue* remoteEventSenderQueue;
    // for BLEEP DDES #2
    pthread_t remoteEventSender;
    int* sendingSocket;
    // for BLEEP DDES #3
    int masterRecvPort;
    int port;
    pthread_t remoteEventReceiver;
    pthread_t* remoteEventReceiveWorker;
    CountDownLatch* startLatch;
};

RemoteQueue* _remoteEvent_remoteQueue_new() {
    RemoteQueue* rq = g_new0(RemoteQueue, 1);

    g_mutex_init(&(rq->lock));
    rq->items = g_queue_new();
    g_cond_init(&(rq->cond));
    g_mutex_init(&(rq->producedCntLock));
    rq->producedCnt = 0;
    rq->fRoundEnd = 0;
    rq->roundLatch = countdownlatch_new(2);

    return rq;
}

// for BLEEP DDES #1
void remoteEvent_produce(RemoteEventProcessor* rep, SimulationTime deliverTime, GQuark srcID, GQuark dstID, Packet* packet) {
    RemoteQueue* rq = rep->remoteEventSenderQueue;
    // process item
//    char* packetStr = packet_serialize(packet);
    // stub: hi!
    char* packetStr = "Hi!";
    int packetStrLength = strlen(packetStr);
    char* str = (char*)malloc(sizeof(char) * (12 + packetStrLength));
    int idx = 0;
    memcpy(&str[idx], &packetStrLength, 4);
    idx += 4;
    memcpy(&str[idx], &deliverTime, 8);
    idx += 8;
    memcpy(&str[idx], &srcID, 4);
    idx += 4;
    memcpy(&str[idx], &dstID, 4);
    idx += 4;
    memcpy(&str[idx], &packetStr, packetStrLength);
    idx += packetStrLength;

    RemoteQueueElement* e = g_new0(RemoteQueueElement, 1);
    e->data = str;
    e->dataLen = idx;
    // TODO: make variation of machine selection rule.
    e->remoteMachineSocket = rep->sendingSocket[dstID % rep->slave_count];

    // for BLEEP DDES #4
    g_mutex_lock(&(rq->producedCntLock));
    rq->producedCnt++;
    g_mutex_unlock(&(rq->producedCntLock));

    // lock mutex
    g_mutex_lock(&(rq->lock));
    // add the new item to the container
    g_queue_push_tail(rq->items, e);
    // signal through cond
    g_cond_signal(&(rq->cond));
    // unlock mutex
    g_mutex_unlock(&(rq->lock));
    /// return
    packet_unref(packet);
    return;
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
    int sentCnt = 0;
    while(TRUE) {
        RemoteQueueElement* e = _remoteEvent_wait_and_get(rq);
        if (!e->dataLen == 0) {
            // break;
            break;
        }
        send(e->remoteMachineSocket, e->data, e->dataLen, 0);
        sentCnt++;
        if (rq->fRoundEnd && rq->producedCnt == sentCnt) {
            rq->fRoundEnd = 0;
            countdownlatch_countDown(rq->roundLatch);
        }
    }
}
// for BLEEP DDES #3
void* _remoteEvent_receiveTaskloop(void* client_socket_ref) {
    int client_socket = *(int*)client_socket_ref;

    while(TRUE) {
        unsigned char packet_size_int[4];
        recv(client_socket, packet_size_int, 4, 0);
        unsigned int packet_size = *(&packet_size_int[0]);
        unsigned char* packet_data = (unsigned char*)malloc(sizeof(unsigned char)*(packet_size + 1));

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
        recv(client_socket, packet_data, packet_size, 0);
        packet_data[packet_size] = 0;

//        Packet* packet = packet_deserialize(packet_data, packet_size);
        //stub: received: Hi!
        printf("%s", packet_data);
        Packet* packet = NULL;

        // make
        worker_processRemotePacket(packet, deliverTime, srcID, dstID);
    }
}
void* _remoteEvent_receiver_run(void* arg) {
    RemoteEventProcessor* rep = (RemoteEventProcessor*)arg;
    int reception_socket;
    int client_socket, c;
    struct sockaddr_in server, client;
    rep->remoteEventReceiveWorker = (pthread_t *)malloc(sizeof(pthread_t) * rep->slave_count - 1);    // remove self

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

    listen(reception_socket, rep->slave_count - 1);
    c = sizeof(struct sockaddr_in);

    countdownlatch_countDown(rep->startLatch);

    int idx = 0;
    while(client_socket = accept(reception_socket, (struct sockaddr *)&client, (socklen_t*)&c)) {
        if (pthread_create(&rep->remoteEventReceiveWorker[idx], NULL, _remoteEvent_receiveTaskloop, (void*)&client_socket) < 0) {
            perror("Cannot create thread\n");
            return NULL;
        }
        idx++;
    }

    while(idx > 0) {
        pthread_join(rep->remoteEventReceiveWorker[idx-1], NULL);
        idx--;
    }

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
    rep->startLatch = countdownlatch_new(1);
    gint ddesRet = pthread_create(&(rep->remoteEventReceiver), NULL, _remoteEvent_receiver_run, rep);
    if(ddesRet != 0) {
        critical("unable to create event_receiver thread");
        return -1;
    }
    countdownlatch_await(rep->startLatch);
    countdownlatch_free(rep->startLatch);
    char simple_response = 1;
    send(client_socket, &simple_response, 1, 0);
    char simple_request = 0;
    recv(client_socket, &simple_request, 1, 0);

    int idx = 0;
    rep->sendingSocket = (int*)malloc(sizeof(int) * rep->slave_count);
    while(idx < rep->slave_count) {
        int sLen = -1;
        char ipbuf[20];
        recv(client_socket, &sLen, 4, 0);
        recv(client_socket, ipbuf, sLen, 0);
        ipbuf[sLen] = 0;
        if (idx != rep->id) {
            _remoteEvent_registerIPSocket(rep, idx, ipbuf, rep->port);
        }
        idx++;
    }
    return 0;
}
RemoteEventProcessor* remoteEvent_new(int masterRecvPort, int slavePort) {
    gint ddesRet;
    RemoteEventProcessor* rep = g_new0(RemoteEventProcessor, 1);
    rep->masterRecvPort = masterRecvPort;
    rep->port = slavePort;
    // for BLEEP DDES #1
    rep->remoteEventSenderQueue = _remoteEvent_remoteQueue_new();
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