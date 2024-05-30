#ifndef _SRSPROXY_H
#define _SRSPROXY_H

#include <iostream>
#include <cstring>
#include <vector>
#include <map>

#include "Common.h"
#include "ec.h"
#include "NetworkUnit.h"

using namespace std;

void SRS_PlacementSender(SRSPlaceMentCommand* command);
void SRS_PlacementReceiver(SRSPlaceMentCommand* command);
void SRS_ProcessPlacement(SRSPlaceMentCommand* command);


void SRS_ProcessScaleData(SRSMdsDataCommand* command);
void SRS_ProcessScaleParity(SRSMdsParityCommand* command);


void* SRSWork(void* pointer);
void ProxySRS();

#endif
