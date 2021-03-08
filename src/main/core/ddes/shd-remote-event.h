//
// Created by hjkim17 on 21. 3. 4..
//

#ifndef SHD_DDES_REMOTE_EVENT_H_
#define SHD_DDES_REMOTE_EVENT_H_

typedef struct _RemoteEventProcessor RemoteEventProcessor;

RemoteEventProcessor* remoteEvent_new(int masterRecvPort, int slavePort);
void remoteEvent_produce(RemoteEventProcessor* rep, SimulationTime deliverTime, GQuark srcID, GQuark dstID, Packet* packet);

#endif