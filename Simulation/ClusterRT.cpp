#include <iostream>
#include <ctime>
#include <cstring>
#include <vector>
#include <algorithm>
#include <numeric>
#include <map>
#include <queue>
#include <climits>
#include <cfloat>
#include <cmath>
using namespace std;

#define CHUNKSIZE 1024 * 1024 /*unit: Bytes*/
#define BANDWIDTH 1024 * 1024 * 1024 /*unit: Bytes/sec*/
#define DISKIO 200 * 1024 * 1024 /*unit: Bytes/sec*/

int rackNum = 6;
int M = 3;
int nodeNum = rackNum*M;


vector<vector<int>> oldChunkIndex;
vector<vector<int>> newChunkIndex;
int istripeNum = 300;
int iK = 6;
int iN = iK + M;
int NODENUM = rackNum*M;
int scaleNum = 200;
int PARTITION_LENGTH = 5000;




typedef struct
{
	int chunkId;
	int sendNodeId;
	int receiveNodeId;
	int dataIndexInNewStripe;
}SendChunkTask;

int gcd(int a, int b){
	return b ? gcd(b, a % b) : a;
}
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

    
    int scaleStripeNum;
    
	double transmissionTime;
	double ioTime;
	vector<int> nodeReceiveNum;
	vector<vector<int>> nodeSendSeq;
	vector<vector<int>> chunkIndexs;
	vector<vector<int>> newChunkIndexs;
	vector<vector<int>> StripeInEachRackNums;

	
	vector<vector<int>> chunkIndexsNode;
	
	vector<int> nodeSendNum;
	vector<double> totalUnbalanceRatios;

	// testbed
	int nodeNum;

	int placementTraffic;
	int relocationStripeNum;

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
		vector<int>& deleteStripeIdThisRound, vector<vector<int>>& nodeHoldAbsoluteDataIndex,
		vector<int>& nodeUpload, vector<int>& nodeDownload);

	void soloveRelocationUpScale(vector<vector<pair<int, int>>>& graph,
		vector<vector<int>>& chunkIndexsAfterExtension,
		vector<int>& deleteStripeIdThisRound, vector<vector<int>>& nodeHoldDeleteChunkId,
		vector<vector<pair<int, int>>>& transmissionTaskEachStripe,
		vector<vector<pair<int, int>>>& relocationTaskEachStripe,
		vector<int>& extensionStripeCollectNodeId,
		vector<int>& nodeUpload, vector<int>& nodeDownload,
		vector<vector<SendChunkTask>>& updataParityTask_eachExtensionStripe,
		vector<vector<SendChunkTask>>& relocationTask_eachExtensionStripe);

	vector<int> upScalePickStripeAccordingDownload(vector<int>& remainStripeId,
		int pickNumber, vector<int>& nodeDownload);
	vector<int> upScalePickStripeAccordingUpload(vector<int>& remainStripeId,
		int pickNumber, vector<int>& nodeDownload);

	vector<int> pickExtensionStripeAccordingLoad(vector<int>& remainStripeId,
		int pickNumber, vector<int>& pickedStripeCollectNodeId,
		vector<int>& nodeUpload, vector<int>& nodeDownload);
	vector<int> pickDeleteStripeAccordingLoad(vector<int>& remainStripeId,
		int pickNumber, vector<int>& nodeUpload);

	double tryPickExtensionStripeAndGetTarget(int stripeId, int& collectNodeId,
		vector<int>& nodeUpload, vector<int>& nodeDownload);
	double tryPickDeleteStripeAndGetTarget(int stripeId, vector<int>& nodeUpload);

	double calculateVariance(vector<int> load);
	double calculateMean(vector<int> load);
	double schedulingTransmission();
	double noSchedulingTransmission();
	double noSchedulingTransmission(vector<vector<vector<int>>>& transmissionDependencyEachStripe,
		vector<vector<int>>& vertexIdMapNodeIdEachStripe); // need give transport dependency
	double noSchedulingTransmission(vector<vector<pair<int, int>>>& transmissionTaskEachStripe,
		vector<vector<pair<int, int>>>& relocationTaskEachStripe,
		vector<int>& extensionStripeCollectNodeId);

	double ideaUpScalePerRound(vector<int>& extensionStripeIdThisRound,
		vector<int>& deleteStripeIdThisRound, vector<int>& extensionStripeCollectNodeId,
		vector<int>& nodeUpload, vector<int>& nodeDownload,
		vector<vector<SendChunkTask>>& updataParityTask_eachExtensionStripe,
		vector<vector<SendChunkTask>>& relocationTask_eachExtensionStripe);
	double baselineUpScalePerRound(int startStripeId, int endStripeId, int p1, int p2);

	int getTwoDataIndex(vector<vector<int>>& smallStripeDataIndex,
		vector<vector<int>>& largeStripeDataIndex, int KLarge);
	int ERSGetTargetPlacement(int KLarge, vector<vector<int>>& newChunkIndex_beforeScale);


	int SRSGetThreeDataIndex(vector<int>& dataChunkId_map_newEncodingIndex,
		vector<vector<int>>& logical_smallerK_StripeDataIndex,
		vector<vector<int>>& smallerK_StripeDataIndex,
		vector<vector<int>>& largerK_StripeDataIndex,
		vector<vector<int>>& oldDataIndexNeed_eachExtensionStripeInGroup, //subtraction
		vector<vector<int>>& newDataIndexNeed_eachExtensionStripeInGroup, //add
		vector<vector<pair<int, int>>>& sendDataIndex_eachStripeInGroup, //pair<old,new>
		int KTarget);
	int SRSGetTargetPlacement(int KTarget, vector<vector<int>>& newChunkIndex_beforeScale);

	double GetUnbanlancePerRound();
	double IdeaGetUnbanlancePerRound(vector<int>& nodeUpload, vector<int>& nodeDownload);
    //idea-zf
	void getRackPriority(int stripeId,int kTarget,vector<int>& priorityRack);
	int scaleForSmallerK(int kTarget,
								int nowStripeId,
								vector<vector<int>>& chunkIndexsAfterExtension);

	int scaleForLargerK(int kTarget,
								int nowStripeId,
								vector<vector<int>>& chunkIndexsAfterExtension);

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
    
public:
	Simulation(/* args */);
	~Simulation();
	int Get_K();
	void print();
	void print(int stripeId);
	void printTraffic(const char input[]);

	void checkLoad(int startStripeId, int endStripeId, int Ks);

	double ideaUpScale(int KLarge);
	double baselineUpScale(int KLarge);
	double SRSUpScale(int KLarge);
	double ERSUpScale(int KTarget);

	int ERSPlacement(int KTarget, int scaleTime);

	int SRSPlacement(int KTarget, int scaleTime);


	int Get_InitialStripeNum();
	int Get_ChunkIndexsSize();
	double GetUnbalanceRatio();
    int Get_ScaleStripeNum();
    
    int ideaScale(int kTarget);
	void ideaScale(int kTarget,int scaleTime);
    
};

Simulation::Simulation(/* args */)
{
	transmissionTime = (double)CHUNKSIZE;
	transmissionTime /= (double)BANDWIDTH;
	ioTime = (double)CHUNKSIZE;
	ioTime /= (double)DISKIO;
	KShrinkage = -1;
	minKs = 0;
	scaleNumPerRound = 0;
	initialStripeNum = istripeNum;
	downScaleStripePerRound = scaleNum;
	K = iK;
	N = iK + M;
	this->nodeNum = NODENUM;
	vector<int> chunkindex;
	vector<int> dataIndex;

	transmissionTime = 1;
	ioTime = 1;

	//0720
	placementTraffic = 0;
	relocationStripeNum = 0;
    
	//newChunkIndexs.resize(initialStripeNum);
	nodeSendSeq.resize(NODENUM);
	nodeReceiveNum.resize(NODENUM);

	nodeSendNum.resize(nodeNum);
	totalUnbalanceRatios.clear();
    
    scaleStripeNum = 0;
    
	
    
    for (int i = 0 ; i < rackNum ; i++){
        for ( int j = 0 ; j < NODENUM ; j++){
            if( i*M<=j && j<(i+1)*M ){
                continue;
            }
            dataIndex.push_back(j);
        }
        for(int t = 0 ; t < initialStripeNum/rackNum ; t++){
            random_shuffle(dataIndex.begin(), dataIndex.end());
            chunkindex.clear();
            for (int j = 0 ; j < K ; j++){
                chunkindex.push_back(dataIndex[j]);
            }
            for (int j = 0 ; j < M ; j++){
                chunkindex.push_back(i*M+j);
            }
            chunkIndexs.push_back(chunkindex);
        }
		dataIndex.clear();
    }
}

Simulation::~Simulation()
{
}

double Simulation::noSchedulingTransmission(){
	vector<int> uploadEachRack(rackNum,0);
	vector<int> downloadEachRack(rackNum,0);
	int traffic = 0;
	double unbalanceRatio = 0.0;
	for(int i = 0 ; i < NODENUM ;i++){
		traffic+=nodeSendSeq[i].size();
	}
	for(int i = 0 ; i < rackNum;i++){
		for(int j = 0 ; j < M ;j++){
			uploadEachRack[i]+=nodeSendSeq[i*M+j].size();
		}
	}
	for(int i = 0 ; i < NODENUM ; i++){
		for(int j = 0 ; j < nodeSendSeq[i].size();j++){
			downloadEachRack[nodeSendSeq[i][j]/M]++;
		}
	}
	if (traffic == 0)
	{
		return unbalanceRatio;
	}
	int maxUpload=0,maxDownload=0,maxLoad=0;
	double aveLoad = 0.0;
	maxUpload = *max_element(uploadEachRack.begin(), uploadEachRack.end());
	maxDownload = *max_element(downloadEachRack.begin(), downloadEachRack.end());
	maxLoad = max(maxUpload, maxDownload);

	aveLoad = (double)traffic/(double)rackNum;
	unbalanceRatio = (double)maxLoad / aveLoad;
	// cout<<maxLoad<<" "<<aveLoad<<endl;
	totalUnbalanceRatios.push_back(unbalanceRatio);

	return unbalanceRatio;
}

double Simulation::GetUnbalanceRatio()
{
	double ratio = 0.0;

	int ratioNum = totalUnbalanceRatios.size();
	double ratioSum = 0.0;

	for (int i = 0; i < ratioNum; i++)
	{
		ratioSum += totalUnbalanceRatios[i];
		//cout << "ratio-" << i << ": " << totalUnbalanceRatios[i] << endl;
	}

	// if (ratioNum > 1)
	// {
	// 	ratioSum -= totalUnbalanceRatios.back();
	// 	ratioNum--;
	// }

	ratio = ratioSum / (double)ratioNum;

	totalUnbalanceRatios.clear();
	return ratio;
}

int Simulation::Get_K()
{
	return K;
}

int Simulation::Get_InitialStripeNum()
{
    for(int i = 0 ; i < initialStripeNum ; i++){
        for(int j = 0 ; j < N ; j++){
            cout<<chunkIndexs[i][j]<<" ";
        }
        cout<<endl;
    }
	return initialStripeNum;
}

int Simulation::Get_ChunkIndexsSize()
{
	int size = chunkIndexs.size();

	return size;
}
int Simulation::Get_ScaleStripeNum(){
    return scaleStripeNum;
}
void Simulation::print()
{
	for (int i = 0; i < initialStripeNum; i++)
	{
		cout << i << ": ";
		for (int j = 0; j < N; j++)
		{
			cout << chunkIndexs[i][j] << " ";
		}
		cout << endl;
	}
}

void Simulation::print(int stripeId)
{
	for (int i = 0; i < N; i++)
	{
		cout << chunkIndexs[stripeId][i] << " ";
	}
	cout << endl;
}

void Simulation::printTraffic(const char input[])
{
	int size = chunkIndexs.size();
	double aveTraffic = 0.0;

	if (strcmp(input, "idea") == 0)
	{
		double relocationRatio = 0.0;
		relocationRatio = relocationStripeNum;
		relocationRatio = relocationRatio / (double)size;

		cout << "idea relocation traffic: " << ideaRelocationNum << endl;
		cout << "idea traffic: " << ideaTraffic << endl;
		aveTraffic = ideaTraffic;
		aveTraffic = aveTraffic / (double)size;
		cout << "idea ave traffic: " << aveTraffic << endl;
		cout << "idea relocation ratio: " << relocationRatio << endl;
		relocationStripeNum = 0;
	}
	else if (strcmp(input, "baseline") == 0)
	{
		cout << "baseline relocation traffic: " << baselineRelocationNum << endl;
		cout << "baseline traffic: " << baselineTraffic << endl;
		aveTraffic = baselineTraffic;
		aveTraffic = aveTraffic / (double)size;
		cout << "baseline ave traffic: " << aveTraffic << endl;
	}
	else if (strcmp(input, "SRS") == 0)
	{
		// cout << "SRS ave relocation chunks num: " << placementTraffic/ (double)size<< endl;
		cout << "SRS ave relocation traffic: " << (placementTraffic/ (double)size)*(0.0625) << " GB"<< endl;
		// cout << "SRS ave update chunks num: " << SRSTraffic/ (double)size << endl;
		cout << "SRS ave update traffc: " << (SRSTraffic/ (double)size)*(0.0625) <<" GB" << endl;
		aveTraffic = SRSTraffic;
		aveTraffic += placementTraffic;
		aveTraffic = aveTraffic / (double)size;
		// cout << "SRS ave chunks num: " << aveTraffic << endl;
		cout << "SRS ave traffic: " << aveTraffic*(0.0625)<<" GB"<<endl; 
		placementTraffic = 0;
	}
	else if (strcmp(input, "ERS") == 0)
	{
		// cout << "ERS ave relocation chunks num: " << placementTraffic/ (double)size << endl;
		cout << "ERS ave relocation traffic: " << (placementTraffic/ (double)size)*(0.0625) << " GB"<< endl;
		// cout << "ERS ave update chunks num: " << ERSTraffic/ (double)size << endl;
		cout << "ERS ave update traffc: " << (ERSTraffic/ (double)size)*(0.0625) <<" GB" << endl;
		aveTraffic = ERSTraffic;
		aveTraffic += placementTraffic;
		aveTraffic = aveTraffic / (double)size;
		// cout << "ERS ave traffic: " << aveTraffic << endl;
		cout << "ERS ave traffic: " << aveTraffic*(0.0625)<<" GB"<<endl; 
		placementTraffic = 0;
	}
	else
	{
		cout << "printTraffic error" << endl;
	}
}



bool Simulation::findWaysToAugment(int currentVertex,
	vector<vector<int>>& bipartiteGraph,
	vector<int>& result,
	vector<int>& whetherVerticesSearched,
	int verticesNumberOnTheRight)
{
	for (int vertexIdOnTheRight = 0; vertexIdOnTheRight < verticesNumberOnTheRight;
		vertexIdOnTheRight++)
	{
		if (bipartiteGraph[currentVertex][vertexIdOnTheRight] == 1
			&& whetherVerticesSearched[vertexIdOnTheRight] == 0)
		{
			whetherVerticesSearched[vertexIdOnTheRight] = 1;
			if (result[vertexIdOnTheRight] == -1
				|| findWaysToAugment(result[vertexIdOnTheRight],
					bipartiteGraph, result, whetherVerticesSearched,
					verticesNumberOnTheRight))
			{
				result[vertexIdOnTheRight] = currentVertex;
				return true;
			}
		}
	}
	return false;
}

vector<int> Simulation::maximalMatching(vector<vector<int>>& bipartiteGraph,
	int  verticesNumberOnTheLeft, int verticesNumberOnTheRight)
{
	vector<int> result;
	vector<int> whetherVerticesSearched;

	for (int i = 0; i < verticesNumberOnTheRight; i++)
	{
		result.push_back(-1);
		whetherVerticesSearched.push_back(0);
	}

	for (int vertexIdOnTheLeft = 0; vertexIdOnTheLeft < verticesNumberOnTheLeft;
		vertexIdOnTheLeft++)
	{
		findWaysToAugment(vertexIdOnTheLeft, bipartiteGraph, result,
			whetherVerticesSearched, verticesNumberOnTheRight);
		for (int i = 0; i < verticesNumberOnTheRight; i++)
		{
			whetherVerticesSearched[i] = 0;
		}
	}

	return result;
}

bool Simulation::dinicBFS(vector<vector<pair<int, int>>>& graph, vector<int>& level,
	int vertexNum)
{
	queue<int> q;
	for (int i = 0; i <= vertexNum; i++)
		level[i] = 0;

	q.push(1);
	level[1] = 1;
	int u, v;
	while (!q.empty())
	{
		u = q.front();
		q.pop();
		for (v = 1; v <= vertexNum; v++)
		{
			if (!level[v] && graph[u][v].first > graph[u][v].second)
			{
				level[v] = level[u] + 1;
				q.push(v);
			}
		}
	}
	return level[vertexNum] != 0;
}
int Simulation::dinicDFS(vector<vector<pair<int, int>>>& graph, vector<int>& level,
	int vertexNum, int currentVertex, int cp)
{
	int tmp = cp;
	int v, t;
	if (currentVertex == vertexNum)
		return cp;
	for (v = 1; v <= vertexNum && tmp; v++)
	{
		if (level[currentVertex] + 1 == level[v])
		{
			if (graph[currentVertex][v].first > graph[currentVertex][v].second)
			{
				t = dinicDFS(graph, level, vertexNum, v,
					min(tmp, graph[currentVertex][v].first - graph[currentVertex][v].second));
				graph[currentVertex][v].second += t;
				graph[v][currentVertex].second -= t;
				tmp -= t;
			}
		}
	}
	return cp - tmp;
}

int Simulation::dinic(vector<vector<pair<int, int>>>& graph, int vertexNum)
{
	vector<int> level;
	int sum = 0, tf = 0;
	level.resize(vertexNum + 1);
	while (dinicBFS(graph, level, vertexNum))
	{
		while (tf = dinicDFS(graph, level, vertexNum, 1, INT_MAX))
			sum += tf;
	}
	return sum;
}

void Simulation::ideaScale(int kTarget,int scaleTime){
    this->KLarge = kTarget;
	this->minKl = KLarge / gcd(K, KLarge);
	this->minK = K / gcd(K, KLarge);
    int largerK = kTarget;
	int smallerK = K;
	int minLargerK = largerK / gcd(smallerK, largerK);
	int minSmallerK = smallerK / gcd(smallerK, largerK);
	minLargerK = largerK;
	minSmallerK = K;
	int lcm = largerK * smallerK / gcd(smallerK, largerK);
    int groupNum = rackNum;
    int eachGroupStripeNum = (initialStripeNum/groupNum);
    eachGroupStripeNum = (eachGroupStripeNum/minLargerK)*minLargerK;
	int eachGroupStripeNumAfterExt = (eachGroupStripeNum/minLargerK)*minSmallerK;
    int actualStripeNum = eachGroupStripeNum*groupNum;
    int currentRound = 0;
    int maxRound = eachGroupStripeNum/minLargerK;
    int updateTraffic = 0;
	int relocationTraffic = 0;
	int stripeNumAfterExtension = (actualStripeNum/minLargerK)*minSmallerK;

	//
	if(kTarget > (rackNum-1)*M){
		cout<<"ERROR! k' <= (rack num -1)*m "<<endl;
		exit(0);
	}
	//
    vector<int> stripeRackIndex;
    vector<vector<int>> rackIndex;
    vector<vector<int>> rackIndexAfterExtension;
	vector<vector<int>> nodeIndex;
	nodeIndex.resize(actualStripeNum);
	for(int i = 0 ; i < actualStripeNum ; i++){
		nodeIndex[i].resize(nodeNum);
		for(int j = 0 ; j < nodeNum ; j++){
			nodeIndex[i][j] = 0;
		}
	}

	vector<vector<int>> chunkIndexAfterExtension;
	chunkIndexAfterExtension.resize(stripeNumAfterExtension);

    vector<vector<int>> newchunkIndexs;
	int chunkIndexssize = chunkIndexs.size();
	int parSize = chunkIndexssize / groupNum;
	for(int i = 0 ; i < groupNum ; i ++){
		for(int j = 0 ; j < eachGroupStripeNum ; j ++){
			newchunkIndexs.push_back(chunkIndexs[i*parSize + j]);
		}
	}
	chunkIndexs = newchunkIndexs;

	newchunkIndexs.clear();
	newchunkIndexs.resize(stripeNumAfterExtension);
	for(int i = 0 ; i < stripeNumAfterExtension ; i ++){
		newchunkIndexs[i].resize(largerK+M);
		for(int j = 0 ; j < largerK + M ; j++){
			newchunkIndexs[i][j]=0;
		}
	}

	for(int i = 0 ; i < actualStripeNum ; i++){
		for(int j = 0 ; j < K ; j++){
			nodeIndex[i][chunkIndexs[i][j]] = 1;	//data node
		}
		for(int j = K; j < N ; j++){
			nodeIndex[i][chunkIndexs[i][j]] = 1; 	//parity node
		}
	}

    //get the rackIndex;
    for (int i = 0 ; i < actualStripeNum ; i++){
        for(int j = 0 ; j < chunkIndexs[i].size() ; j++){
            stripeRackIndex.push_back(chunkIndexs[i][j]/M);
        }
        rackIndex.push_back(stripeRackIndex);
        stripeRackIndex.clear();
    }

    // for (int i = 0 ; i < actualStripeNum ; i++){
    //     for(int j = 0 ; j < N ; j++){
    //         cout<<rackIndex[i][j]<<" ";
    //     }
    //     cout<<endl;
    // }

    while(currentRound < maxRound){
        int currentStripeId = 0 ;

		// different group => different parity cluster
        for(int i = 0 ; i < groupNum ; i++){
            updateTraffic += (minLargerK - minSmallerK)*( K - M );
            currentStripeId = i*eachGroupStripeNum + currentRound*minLargerK;

            //maxFlow
            int deleteStripeNum = minLargerK - minSmallerK;
			int extensionStripeNum = minSmallerK;
			int rackNumm = rackNum;
			int vertexNum = deleteStripeNum + rackNumm + extensionStripeNum + 2;
			int currentVertex;
			int deleteStripeStartVertex = 2;
			int rackStartVertex = deleteStripeStartVertex + deleteStripeNum;
			int extensionStripeStartVertex = rackStartVertex + rackNumm;
			int absoluteStripeId = 0;
			int dataRackId = 0;
            int delStripeId = 0;
            int extStripeId = 0;


            delStripeId = i*eachGroupStripeNum + currentRound*minLargerK + minSmallerK; //decomposed stripe index;
            extStripeId = i*eachGroupStripeNum + currentRound*minLargerK ; //

			vector<vector<pair<int, int>>> graph;
			graph.resize(vertexNum + 1);
            for (int j = 0; j <= vertexNum; j++){
				graph[j].resize(vertexNum + 1);
				for (int t = 0; t <= vertexNum; t++){
					graph[j][t].first = graph[j][t].second = 0;
				}
			}
            
            currentVertex = 1;
            for (int j = deleteStripeStartVertex; j < rackStartVertex; j++){
				graph[currentVertex][j].first = K;
			}
			currentVertex = deleteStripeStartVertex;

            for(int j = 0 ; j < deleteStripeNum ; j++){
                for(int t = 0 ; t < K ; t++){
                    dataRackId = rackIndex[delStripeId][t];
                    graph[currentVertex][rackStartVertex + dataRackId].first += 1;
                }
                currentVertex++;
                delStripeId++;
            }

            for(int j = 0 ; j < extensionStripeNum ; j++){
                for(int t = 0 ; t < rackNum ; t++){
                    graph[rackStartVertex + t][extensionStripeStartVertex + j].first = M;	//each rack max-chunk-num = M
                }
                for (int t = 0 ; t < K+M ; t++){
                    dataRackId = rackIndex[extStripeId][t];
                    graph[rackStartVertex + dataRackId][extensionStripeStartVertex + j].first -= 1;
                }
				extStripeId++;
            }

            for(int j = 0 ; j < extensionStripeNum ; j++){
                graph[extensionStripeStartVertex + j][vertexNum].first = KLarge - K ;
            }

            /* dinic */
            int dinicvl = 0;
	        dinicvl = dinic(graph, vertexNum);
            relocationTraffic += (largerK-K)*(K) - dinicvl;

			
			vector<int> decomNodeIndex;
			vector<int> stretchNodeIndex;
			vector<vector<int>> nodeHoldDeleteDataChunkId;
			nodeHoldDeleteDataChunkId.resize(nodeNum);

			
			currentStripeId = i*eachGroupStripeNum + currentRound*minLargerK;
			int newCurrentStripeId = i*eachGroupStripeNumAfterExt + currentRound*minSmallerK;

			for(int j = 0 ; j < largerK - K ; j++){
				for(int t = 0 ; t < K ; t++){
					decomNodeIndex.push_back(chunkIndexs[ currentStripeId + K + j ][t]);
				}
			}

			for(int j = 0 ; j < decomNodeIndex.size() ; j++){
				int nowDecStripeId = j / (K);
				int nowDecChunkId = j % (K);
				int chunkid = (currentStripeId + K + nowDecStripeId)*(K+M) + nowDecChunkId;
				nodeHoldDeleteDataChunkId[decomNodeIndex[j]].push_back(chunkid);
			}

			for(int j = 0 ; j < extensionStripeNum ; j ++){	
				for(int t = 0 ; t < rackNum ; t++){
					if(graph[rackStartVertex + t][extensionStripeStartVertex + j].second > 0){
						if(graph[rackStartVertex + t][extensionStripeStartVertex + j].first < graph[rackStartVertex + t][extensionStripeStartVertex + j].second){
							cout<<"error in max_match!"<<endl;
							cout<<"simulation stop!"<<endl;
							exit(0);
						}
						// fill chunkindexsAfterExtension
						for(int k = 0 ; k < graph[rackStartVertex + t][extensionStripeStartVertex + j].second ; k++){
							chunkIndexAfterExtension[newCurrentStripeId+j].push_back(t);
						}
					}
				}

				if(chunkIndexAfterExtension[ newCurrentStripeId + j ].size() < largerK - K){
					

					vector<int> eachRackNum(rackNum,0);

					for(int t = 0 ; t < K+M ; t ++){
						eachRackNum[rackIndex[currentStripeId+j][t]]+=1;
					}
					for(int t = 0 ; t < chunkIndexAfterExtension[ newCurrentStripeId + j ].size() ; t++){
						eachRackNum[chunkIndexAfterExtension[ newCurrentStripeId + j ][t]]+=1;
					}
					int nowGetNum = chunkIndexAfterExtension[ newCurrentStripeId + j ].size();
					for(int t = nowGetNum ; t < largerK - K ; t++){
						for(int k = 0 ; k < rackNum ; k ++){
							if(eachRackNum[k] < M){
								chunkIndexAfterExtension[ newCurrentStripeId + j ].push_back(k);
								eachRackNum[k]++;
								break;
							}
						}
					}
				}

				if(chunkIndexAfterExtension[ newCurrentStripeId + j].size() != largerK - K){
					cout<<"relocation error!"<<endl;
					exit(0);
				}

				
				vector<int> aftNodeIndex(largerK - K , 0);
				for(int t = 0 ; t < largerK - K ; t++){
					int nowRackId = chunkIndexAfterExtension[ newCurrentStripeId + j][t];
					
					for(int q = 0 ; q < M ; q++){
						if(nodeIndex[currentStripeId + j][nowRackId*M + q] != 0){
							continue;
						}
						nodeIndex[currentStripeId + j][nowRackId*M + q] ++;
						aftNodeIndex[t] = nowRackId*M + q;
						break;
					}
				}
				

				
				for(int t = 0 ; t < K ; t++){
					newchunkIndexs[newCurrentStripeId + j][t] = (chunkIndexs[currentStripeId + j][t]);
				}
				for(int t = 0 ; t < largerK - K ; t ++ ){
					newchunkIndexs[newCurrentStripeId + j][t+K] = (aftNodeIndex[t]);
				}
				for(int t = 0 ; t < M ; t ++ ){
					newchunkIndexs[newCurrentStripeId + j][t+largerK] = (chunkIndexs[currentStripeId + j][K+t]);
				}

				
				for(int t = K ; t < largerK ; t++){
					stretchNodeIndex.push_back(newchunkIndexs[newCurrentStripeId + j][t]);
				}


			}

			
			if(decomNodeIndex.size()!=stretchNodeIndex.size()){
				cout<<"relocation node index error!"<<endl;
				cout<<"decom-size: "<<decomNodeIndex.size()<<endl;
				cout<<"stretch-size: "<<stretchNodeIndex.size()<<endl;
				exit(0);
			}
			sort(decomNodeIndex.begin(),decomNodeIndex.end());
			sort(stretchNodeIndex.begin(),stretchNodeIndex.end());
			vector<int> intersectionIndex;
			vector<int> differentInOld;
			vector<int> differentInNew;

			set_intersection(decomNodeIndex.begin(),
				decomNodeIndex.end(),
				stretchNodeIndex.begin(),
				stretchNodeIndex.end(),
				inserter(intersectionIndex, intersectionIndex.begin()));
			set_difference(decomNodeIndex.begin(),
				decomNodeIndex.end(),
				intersectionIndex.begin(),
				intersectionIndex.end(),
				inserter(differentInOld, differentInOld.begin()));
			set_difference(stretchNodeIndex.begin(),
				stretchNodeIndex.end(),
				intersectionIndex.begin(),
				intersectionIndex.end(),
				inserter(differentInNew, differentInNew.begin()));

			
			if (differentInOld.size() != differentInNew.size())
			{
				cout << "FastRT: differentInOld.size() != differentInNew.size() in dataChunk" << endl;
				exit(0);
			}
		
			vector<pair<int,int>> relocationNodeToNode;
			
			for(int j = 0 ; j < decomNodeIndex.size() ; j++){
				relocationNodeToNode.push_back(make_pair(decomNodeIndex[j],stretchNodeIndex[j]));
			}
			
			vector<vector<SendChunkTask>> RelocationSendChunkTasks;
			vector<vector<SendChunkTask>> ParityUpdateSendChunkTasks;
			RelocationSendChunkTasks.resize(extensionStripeNum);
			ParityUpdateSendChunkTasks.resize(largerK - K);

			vector<vector<int>> stretChunkId; 
			vector<vector<int>> decomDatatoStretStripeId; 
			decomDatatoStretStripeId.resize(largerK - K);
			for(int j = 0 ; j < largerK - K ; j ++){
				decomDatatoStretStripeId[j].resize(K);
			}
			stretChunkId.resize(extensionStripeNum);
			//for data_relocation sendchunktask
			for(int j = 0 ; j < extensionStripeNum ; j ++ ){
				for(int t = 0 ; t < largerK - K ; t++ ){
					int stretChunkNodeId = newchunkIndexs[newCurrentStripeId + j][t+K];

					//test_bed
					int tb_send_nodeid = -1 , tb_receive_nodeid = -1, tb_chunkId = -1;
					tb_receive_nodeid = stretChunkNodeId;
					//test_bed

					for(int q = 0 ; q < relocationNodeToNode.size() ; q++){
						if(relocationNodeToNode[q].second == stretChunkNodeId){
							if(nodeHoldDeleteDataChunkId[relocationNodeToNode[q].first].size() == 0) {
								cout<<"error size"<<endl;
								exit(0);
							}
							tb_send_nodeid = relocationNodeToNode[q].first;
							int nowStretChunkId = nodeHoldDeleteDataChunkId[relocationNodeToNode[q].first].back();
							
							tb_chunkId = nowStretChunkId;

							int nowStripe_Id = 0,nowData_Id = 0 ;
							//get the data index
							nowStripe_Id = nowStretChunkId/(N);
							nowData_Id = nowStretChunkId%(N);
							decomDatatoStretStripeId[nowStripe_Id - K - currentStripeId][nowData_Id] = j;
							//
							stretChunkId[j].push_back(nowStretChunkId);
							nodeHoldDeleteDataChunkId[relocationNodeToNode[q].first].pop_back();
							relocationNodeToNode.erase(relocationNodeToNode.begin() + q);
							q--;
							break;
						}
					}

					SendChunkTask tempSCT;
					tempSCT.sendNodeId = tb_send_nodeid;
					tempSCT.receiveNodeId = tb_receive_nodeid;
					tempSCT.chunkId = tb_chunkId;
					tempSCT.dataIndexInNewStripe = K + t;
					RelocationSendChunkTasks[j].push_back(tempSCT);

				}
				if(stretChunkId[j].size()!=largerK-K){
					cout<<"error in stretchunkId!"<<endl;
					exit(0);
				}
			}

			vector<int> numOfExtStripe; 
			numOfExtStripe.resize(K);
			for(int j = 0 ; j < K ; j++){
				int numofstripe = 0;
				for(int t = 0 ; t < largerK - K ; t++){
					for(int q = 0 ; q < K ; q++){
						if(decomDatatoStretStripeId[t][q] == j){
							numofstripe++;
							break;
						}
					}
				}
				numOfExtStripe[j] = numofstripe;
			}

			//for Parity_update sendchunktask
			for(int j = 0 ; j < largerK - K ; j ++){
				int tb_send_nodeid = -1 , tb_receive_nodeid = -1, tb_chunkId = -1;
				int tb_nowStripeId = currentStripeId + K + j;
				tb_receive_nodeid = chunkIndexs[tb_nowStripeId][K];
				for(int t = 0 ; t < K - M ; t++){
					tb_send_nodeid = chunkIndexs[tb_nowStripeId][t];
					tb_chunkId = tb_nowStripeId*(K+M)+t;

					SendChunkTask tempSendChunkTask;
					tempSendChunkTask.sendNodeId = tb_send_nodeid;
					tempSendChunkTask.receiveNodeId = tb_receive_nodeid;
					tempSendChunkTask.chunkId = tb_chunkId;
					tempSendChunkTask.dataIndexInNewStripe = K+t;
					ParityUpdateSendChunkTasks[j].push_back(tempSendChunkTask);
				}
				for(int t = K+1 ; t < K+M ; t++){
					tb_send_nodeid = chunkIndexs[tb_nowStripeId][t];
					tb_chunkId = tb_nowStripeId*(K+M)+t;

					SendChunkTask tempSendChunkTask;
					tempSendChunkTask.sendNodeId = tb_send_nodeid;
					tempSendChunkTask.receiveNodeId = tb_receive_nodeid;
					tempSendChunkTask.chunkId = tb_chunkId;
					tempSendChunkTask.dataIndexInNewStripe = K+t;
					ParityUpdateSendChunkTasks[j].push_back(tempSendChunkTask);
				}
			}


			// for a group command:



			if(currentStripeId == 0 ){
				for(int j = 0 ; j <extensionStripeNum ; j ++){
					for(int t = 0 ; t < largerK - K ; t++){
						cout<<stretChunkId[j][t]<<" ";
					}
				}
				cout<<endl<<"========"<<endl;

				for(int j  =0 ; j <largerK - K ; j++){
					for(int t = 0;  t < K ; t++){
						cout<<decomDatatoStretStripeId[j][t]<<" ";
					}
				}
				cout<<endl<<"========"<<endl;

				for(int j = 0 ; j < K ; j++){
					cout<<numOfExtStripe[j]<<" ";
				}
				cout<<endl<<"========"<<endl;
				for(int j = 0 ; j < extensionStripeNum ; j ++){
					for( int  t = 0 ; t < RelocationSendChunkTasks[j].size() ; t++){
						cout<<RelocationSendChunkTasks[j][t].sendNodeId<<" "<<RelocationSendChunkTasks[j][t].receiveNodeId<<" "<<RelocationSendChunkTasks[j][t].chunkId<<" "<<RelocationSendChunkTasks[j][t].dataIndexInNewStripe<<endl;
					}
				}

				cout<<endl<<"========"<<endl;

				for(int j = 0 ; j < largerK - K ; j ++){
					for(int t = 0 ; t < ParityUpdateSendChunkTasks[j].size() ; t++){
						cout<<ParityUpdateSendChunkTasks[j][t].sendNodeId<<" "<<ParityUpdateSendChunkTasks[j][t].receiveNodeId<<" "<<ParityUpdateSendChunkTasks[j][t].chunkId<<" "<<ParityUpdateSendChunkTasks[j][t].dataIndexInNewStripe<<endl;
					}
				}
			}

			//select parity update node
			vector<int> parityUpdateNodeIndex;
			for(int j = 0 ; j < largerK - K ; j++){
				for(int t = 0 ; t < K - M ; t++){
					parityUpdateNodeIndex.push_back(chunkIndexs[currentStripeId+K+j][t]);
				}
			}
			//select parity update node
        //each group
        }

		

        currentRound ++;
    }    
    
	// for(int i = 0 ; i < K+M ; i ++){
	// 	cout<<chunkIndexs[0][i]<<" ";
	// }
	// cout<<endl<<"------------"<<endl;
	// for(int i = 0 ; i < largerK + M ; i ++){
	// 	cout<<newchunkIndexs[0][i]<<" ";
	// }
	// cout<<endl;
	// cout<<"updateTraffic:"<<updateTraffic<<endl;
	// cout<<"relocationTraffic:"<<relocationTraffic<<endl;

	cout<<"idea---------------------------------------"<<endl;
	cout<<"k = "<<K<<"----->"<<"k' = "<<largerK<<endl;
	cout<<"Relocation traffic:"<<relocationTraffic*(0.0625)<<" GB"<<endl;
	cout<<"ave Relocation traffic:"<<(double)((double)relocationTraffic/(double)stripeNumAfterExtension)*(0.0625)<<" GB"<<endl;
	cout<<"update traffic:"<<updateTraffic*(0.0625)<<" GB"<<endl;
	cout<<"ave update traffic:"<<(double)((double)updateTraffic/(double)stripeNumAfterExtension)*(0.0625)<<" GB"<<endl;
	cout<<"ave total traffic:"<<(double)((double)(updateTraffic+relocationTraffic)/(double)stripeNumAfterExtension)*(0.0625)<< " GB"<<endl;
	// cout<<"Balance ratio:"<<totalBalanceratio/((double)transGroupNum)<<endl;
	cout<<"current Stripe num:"<<stripeNumAfterExtension<<endl;
	cout<<"--------------"<<endl;


	K = largerK;
	chunkIndexs = newchunkIndexs;
	initialStripeNum = chunkIndexs.size();
	N = K + M ;
}


void test(){
    Simulation idea;
    vector<int> kPoll = {8,10,12,15};
	// vector<int> kPoll = {8,10,12,15,17,20,23,26,29,32,35,38,48,64,96};
	for ( int i = 0 ; i < kPoll.size() ; i++){
		idea.ideaScale(kPoll[i],i);
	}
}

int main(){
    test();
    return 0;
}
