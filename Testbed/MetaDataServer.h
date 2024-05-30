#ifndef _METADATASERVER_H
#define _METADATASERVER_H

#include <iostream>
#include <algorithm>
#include <climits>
#include <queue>
#include <cstring>
#include <sys/time.h>

#include "Common.h"
#include "NetworkUnit.h"
#include "ec.h"

using namespace std;


class Simulation
{
private:
	/* data */
	/* chunkIndexs: STRIPENUM*N */
	int initialStripeNum;
	int downScaleStripePerRound;
	int K;
	int N;
	int KShrinkage;
	int scaleNumPerRound;
	int minKs;
	int stripeNumber;
	int ideaScaleNumberPerRound;
	int baselineScaleNumberPerRound;
	int SRSScaleNumberPerRound;
	int ERSScaleNumberPerRound;
	int ideaStripeNumber;
	int baselineStripeNumber;
	int SRSStripeNumber;
	int ERSStripeNumber;
	int KLarge;
	int minKl;
	int minK;
	int ideaTraffic;
	int baselineTraffic;
	int SRSTraffic;
	int ERSTraffic;
	int ideaRelocationNum;
	int baselineRelocationNum;
	double transmissionTime;
	double ioTime;

	//zf
	int scaleStripeNum;
	vector<vector<int>> chunkIndexsNode;
	vector<vector<int>> StripeInEachRackNums;
	//zf

	vector<int> nodeReceiveNum;
	vector<vector<int>> nodeSendSeq;
	vector<vector<int>> chunkIndexs;
	vector<vector<int>> newChunkIndexs;
	vector<int> nodeSendNum;
	vector<double> totalUnbalanceRatios;

	vector<int> maximalMatching(vector<vector<int>>& bipartiteGraph,
		int  verticesNumberOnTheLeft, int verticesNumberOnTheRight);
	bool findWaysToAugment(int currentVertex,
		vector<vector<int>>& bipartiteGraph,
		vector<int>& result,
		vector<int>& whetherVerticesSearched,
		int verticesNumberOnTheRight);
	bool dinicBFS(vector<vector<pair<int, int>>>& graph, vector<int>& level, int vertexNum);
	int dinicDFS(vector<vector<pair<int, int>>>& graph, vector<int>& level,
		int vertexNum, int currentVertex, int cp);
	int dinic(vector<vector<pair<int, int>>>& graph, int vertexNum);

	void soloveRelocationUpScale(vector<vector<pair<int, int>>>& graph,
		vector<vector<int>>& chunkIndexsAfterExtension,
		vector<int>& deleteStripeIdThisRound, vector<vector<int>>& nodeHoldDeleteChunkId,
		vector<vector<pair<int, int>>>& transmissionTaskEachStripe,
		vector<vector<pair<int, int>>>& relocationTaskEachStripe,
		vector<int>& extensionStripeCollectNodeId,
		vector<int>& nodeUpload, vector<int>& nodeDownload,
		vector<vector<SendChunkTask>>& updataParityTask_eachExtensionStripe,
		vector<vector<SendChunkTask>>& relocationTask_eachExtensionStripe);

	vector<int> pickExtensionStripeAccordingLoad(vector<int>& remainStripeId,
		int pickNumber, vector<int>& pickedStripeCollectNodeId,
		vector<int>& nodeUpload, vector<int>& nodeDownload);
	vector<int> pickDeleteStripeAccordingLoad(vector<int>& remainStripeId,
		int pickNumber, vector<int>& nodeUpload);

	int tryPickExtensionStripeAndGetTarget(int stripeId, int& collectNodeId,
		vector<int>& nodeUpload, vector<int>& nodeDownload);
	int tryPickDeleteStripeAndGetTarget(int stripeId, vector<int>& nodeUpload);

	double ideaUpScalePerRound(vector<int>& extensionStripeIdThisRound,
		vector<int>& deleteStripeIdThisRound, vector<int>& extensionStripeCollectNodeId,
		vector<int>& nodeUpload, vector<int>& nodeDownload,
		vector<vector<SendChunkTask>>& updataParityTask_eachExtensionStripe,
		vector<vector<SendChunkTask>>& relocationTask_eachExtensionStripe);
	double baselineUpScalePerRound(int startStripeId, int endStripeId, int p1, int p2);

	double noSchedulingTransmission();

	double noSchedulingTransmission(vector<vector<pair<int, int>>>& transmissionTaskEachStripe,
		vector<vector<pair<int, int>>>& relocationTaskEachStripe,
		vector<int>& extensionStripeCollectNodeId);

	void IdeaExecuteScale(vector<int>& extensionStripeIdThisRound,
		vector<int>& extensionStripeCollectNodeId,
		vector<vector<SendChunkTask>>& updataParityTask_eachExtensionStripe,
		vector<vector<SendChunkTask>>& relocationTask_eachExtensionStripe);

	void BaselineExecuteScale(int p1, vector<vector<BaseMdsUpdateParityCommand>>& updataParityTask,
		vector<vector<BaseMdsRelocationCommand>>& relocationTask);

	int getTwoDataIndex(vector<vector<int>>& smallStripeDataIndex,
		vector<vector<int>>& largeStripeDataIndex, int KLarge);
	int ERSGetTargetPlacement(int KLarge, vector<vector<int>>& newChunkIndex_beforeScale);


	int SRSGetThreeDataIndex(vector<int>& dataChunkId_map_newEncodingIndex,
		vector<vector<int>>& logical_smallerK_StripeDataIndex,
		vector<vector<int>>& smallerK_StripeDataIndex,
		vector<vector<int>>& largerK_StripeDataIndex,
		vector<vector<int>>& oldDataIndexNeed_eachExtensionStripeInGroup, //subtraction
		vector<vector<int>>& newDataIndexNeed_eachExtensionStripeInGroup, //add
		vector<vector<pair<int,pair<int, int>>>>& sendDataIndex_eachStripeInGroup, //pair<relativeDataIndex,pair<old,new>>
		int KTarget);
	int SRSGetTargetPlacement(int KTarget, vector<vector<int>>& newChunkIndex_beforeScale);

	double GetUnbanlancePerRound();
	double IdeaGetUnbanlancePerRound(vector<int>& nodeUpload, vector<int>& nodeDownload);
	//idea-zf
    int getTheChunkId(int stripeId,int nodeId);
	void getRackPriority(int stripeId,int kTarget,vector<int>& priorityRack);
	int scaleForSmallerK(int kTarget,
								int nowStripeId,
								vector<vector<int>>& chunkIndexsAfterExtension,
                                vector<vector<int>>& requiredParityEachPostStripe,
                                vector<vector<int>>& xoredDataRackIndexEachPostStripe,
                                vector<vector<pair<int,int>>>& relocationStripeIndexEachPostStripe);

	int scaleForLargerK(int kTarget,
								int nowStripeId,
								vector<vector<int>>& chunkIndexsAfterExtension,
                                vector<vector<int>>& requiredParityEachPostStripe,
                                vector<vector<int>>& xoredDataRackIndexEachPostStripe,
                                vector<vector<pair<int,int>>>& relocationStripeIndexEachPostStripe);

    void ideaGetNodeSendCommand(int kTarget,
                                int nowStripeId,
                                vector<vector<int>>& requiredParityEachPostStripe,
                                vector<vector<int>>& xoredDataRackIndexEachPostStripe,
                                vector<vector<pair<int,int>>>& relocationStripeIndexEachPostStripe,
                                vector<vector<int>>& numInEachRack,
								vector<ERSMdsParityCommand>& tempParityCommand_aGroup,
								vector<ERSMdsDataCommand>& tempDataCommand_aGroup,
								vector<vector<ERSPlaceMentCommand>>& placementCommandPerRound,
								int gId,
								int sId);

	void eachStripeNeedInRack(int stripeId,int targetK,vector<int>& stripeNeedInRack);

	
	void ideaRackInitialPlacement(int kTarget);
	double ideaGetLoadbalanceScheme(int kTarget,vector<int>& schemeStartStripeId,vector<int>& groupHaveDone,vector<vector<int>>& uploadEachGroup);
    void getRackPlacement(int kTarget,
                      vector<vector<int>> &dataRackPlacementIndex,
                      vector<vector<int>> &parityRackPlacementIndex);
    void getNodePlacement(int kTarget,
                      vector<vector<int>> &dataRackPlacementIndex,
                      vector<vector<int>> &parityRackPlacementIndex,
                      vector<vector<int>> &dataNodePlacementIndex,
                      vector<vector<int>> &parityNodePlacementIndex);
    void getDataNumEachRack(int kTarget,
                      vector<vector<int>> &oldDataRackPlacementIndex,
                      vector<vector<int>> &newDataRackPlacementIndex,
					  vector<vector<int>> dataNeedInRack);
    void ideaGetNodePlacement(int kTarget);

	void ideaGetChunkStoreOrderInEachNode();
    //idea-zf
public:
	Simulation(/* args */);
	~Simulation();
	int Get_K();

	void printTraffic(const char input[]);

	void GetChunkStoreOrderInEachNode();

	double ideaUpScale(int KLarge);
	double baselineUpScale(int KLarge);
	double SRSUpScale(int KLarge);
	double ERSUpScale(int KTarget);

	int ERSPlacement(int KTarget,int scaleTime);

	int SRSPlacement(int KTarget, int scaleTime);


	int Get_InitialStripeNum();

	double GetUnbalanceRatio();

	//zf
	void ideaScale(int kTarget,int scaleTime);
	//zf
};

void TestMDS();

void IdeaSendUpdateCommandEachStripe(vector<IdeaMdsUpdateParityCommand>& commandsEachStripe);
void IdeaSendRelocationCommandEachStripe(vector<IdeaMdsRelocationCommand>& commandsEachStripe);


int LocateStoreIndex(int nodeId, int globalChunkId);

int gcd(int a, int b);

void ElasticEC_MDS(vector<int>& kPool);
void Baseline_MDS(vector<int>& kPool);
void ERS_MDS(vector<int>& kPool);
void SRS_MDS(vector<int>& kPool);
void idea_MDS(vector<int>& kPool);
void MDS(int methodType, vector<int>& kPool);

#endif // !_METADATASERVER_H


