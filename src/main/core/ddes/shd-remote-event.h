//
// Created by hjkim17 on 21. 3. 4..
//

#ifndef SHD_DDES_REMOTE_EVENT_H_
#define SHD_DDES_REMOTE_EVENT_H_

typedef struct _RemoteEventProcessor RemoteEventProcessor;

RemoteEventProcessor* remoteEvent_new(Scheduler* scheduler, int masterRecvPort, int slavePort);
int remoteEvent_produce(RemoteEventProcessor* rep, SimulationTime deliverTime, GQuark srcID, GQuark dstID, Packet* packet);
void remoteEvent_free(RemoteEventProcessor* rep);
void remoteEvent_broadcastNextWindow(RemoteEventProcessor* rep, SimulationTime start, SimulationTime end);
void remoteEvent_makeWindowConsensus(RemoteEventProcessor* rep, SimulationTime* startRef, SimulationTime* endRef);
int remoteEvent_checkAssigned(RemoteEventProcessor* rep, GQuark id);
#endif