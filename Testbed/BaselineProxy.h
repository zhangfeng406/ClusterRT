#ifndef _BASELINEPROXY_H
#define _BASELINEPROXY_H

#include <iostream>
#include <cstring>
#include <vector>

#include "Common.h"
#include "ec.h"
#include "NetworkUnit.h"

using namespace std;

void Base_ProcessUpdate(BaseMdsUpdateParityCommand& command);
void Base_ProcessRelocation(BaseMdsRelocationCommand& command);

void Base_DataChunk(BaseMdsUpdateParityCommand& command);
void Base_ParityChunk(BaseMdsUpdateParityCommand& command);

void Base_RelocationSender(BaseMdsRelocationCommand& command);
void Base_RelocationReceiver(BaseMdsRelocationCommand& command);


void* BaselineWork(void* pointer);
void ProxyBaseline();

#endif
