#ifndef _PROXY_H
#define _PROXY_H

#include <iostream>
#include <cstring>
#include <vector>
#include <fcntl.h>

#include "Common.h"
#include "ec.h"
#include "NetworkUnit.h"

using namespace std;

void* ElasticECWork(void* pointer);
void ElaticEC_ProcessUpdate(IdeaMdsUpdateParityCommand& command);
void ElaticEC_DataChunk(IdeaMdsUpdateParityCommand& command);
void ElaticEC_ParityChunk(IdeaMdsUpdateParityCommand& command);
void ElaticEC_CollectNode(IdeaMdsUpdateParityCommand& command);

void ElaticEC_ProcessRelocation(IdeaMdsRelocationCommand& command);
void ElaticEC_RelocationSender(IdeaMdsRelocationCommand& command);
void ElaticEC_RelocationReceiver(IdeaMdsRelocationCommand& command);
void ProxyElasticEC();



#endif // !_PROXY_H
