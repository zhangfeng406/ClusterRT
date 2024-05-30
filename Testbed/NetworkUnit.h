#ifndef _NETWORKUNIT_H
#define _NETWORKUNIT_H

#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>/* gethostname */
#include <netdb.h> /* struct hostent */
#include <arpa/inet.h> /* inet_ntop */
#include <sys/socket.h>
#include <pthread.h>

#include "Common.h"

using namespace std;

bool GetHostAndIp(string& ip);
int InitialSocket(string& user, int portNum);

void ReceiveAck(int receiveNum);
void SendAckToMds(int result);

void testNetworkUnit();

void* testSend(void* socketId);

void Print();

void* BaseChunkSender(void* pointer);
void* BaseChunkReceiver(void* pointer);

void* ChunkSender(void* pointer);
void* ChunkReceiver(void* pointer);

void* IdeaSendUpdateCommand(void* pointer);
void* IdeaSendRelocationCommand(void* pointer);

void* BaseSendUpdateCommand(void* pointer);
void* BaseSendRelocationCommand(void* pointer);

void* ERSSendDataCommand(void* pointer);
void* ERSSendParityCommand(void* pointer);

void* ERSSendReupdateCommand(void* pointer);
void* ERSSendPlacementCommand(void* pointer);

void* ERSChunkSender(void* pointer);
void* ERSChunkReceiver(void* pointer);


void* SRSChunkSender(void* pointer);
void* SRSChunkReceiver(void* pointer);
void* SRSSendDataCommand(void* pointer);
void* SRSSendParityCommand(void* pointer);
void* SRSSendPlacementCommand(void* pointer);

#endif // !_NETWORKUNIT_H
