#ifndef _ERSPROXY_H
#define _ERSPROXY_H

#include <iostream>
#include <cstring>
#include <vector>
#include <map>

#include "Common.h"
#include "ec.h"
#include "NetworkUnit.h"

using namespace std;


void ERS_PlacementSender(ERSPlaceMentCommand* command);
void ERS_PlacementReceiver(ERSPlaceMentCommand* command);


void ERS_ReupdateData(ERSReupdateParityCommand* command);
void ERS_ReupdateParity(ERSReupdateParityCommand* command);


void ERS_ProcessReupdate(ERSReupdateParityCommand* command);
void ERS_ProcessPlacement(ERSPlaceMentCommand* command);




void ERS_ProcessScaleData(ERSMdsDataCommand* command);
void ERS_ProcessScaleParity(ERSMdsParityCommand* command);



void* ERSWork(void* pointer);
void ProxyERS();

#endif
