#include "MetaDataServer.h"

Simulation::Simulation(/* args */)
{
	transmissionTime = 1.0;
	KShrinkage = -1;
	minKs = 0;
	scaleNumPerRound = 0;
	initialStripeNum = iStripeNum;
	downScaleStripePerRound = scaleNum;
	K = iK;
	N = iK + M;
	vector<int> chunkindex;
	vector<int> dataIndex;

	transmissionTime = 1;
	ioTime = 1;

	//newChunkIndexs.resize(initialStripeNum);
	nodeSendSeq.resize(nodeNum);
	nodeReceiveNum.resize(nodeNum);

	nodeSendNum.resize(nodeNum);
	totalUnbalanceRatios.clear();

	for (int i = 0 ; i < rackNum ; i++){
        for ( int j = 0 ; j < nodeNum ; j++){
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

int Simulation::Get_K()
{
	return K;
}

void TestMDS()
{
	cout << "hello mds" << endl;
}

double Simulation::GetUnbanlancePerRound()
{
	double unbalanceRatio = 0.0; // maxbalance/avbalance

	int maxUpload = 0;
	int maxDownload = 0;
	int maxLoad = 0;
	double aveLoad = 0.0;
	int traffic = 0;

	maxUpload = *max_element(nodeSendNum.begin(), nodeSendNum.end());
	maxDownload = *max_element(nodeReceiveNum.begin(), nodeReceiveNum.end());
	maxLoad = max(maxUpload, maxDownload);

	for (int i = 0; i < nodeNum; i++)
	{
		traffic += nodeSendNum[i];
	}

	if (traffic == 0)
	{
		return unbalanceRatio;
	}

	aveLoad = (double)traffic / (double)nodeNum;

	unbalanceRatio = (double)maxLoad / aveLoad;

	totalUnbalanceRatios.push_back(unbalanceRatio);

	return unbalanceRatio;
}

double Simulation::IdeaGetUnbanlancePerRound(vector<int>& nodeUpload, vector<int>& nodeDownload)
{
	double unbalanceRatio = 0.0; // maxbalance/avbalance

	int maxUpload = 0;
	int maxDownload = 0;
	int maxLoad = 0;
	double aveLoad = 0.0;
	int traffic = 0;

	maxUpload = *max_element(nodeUpload.begin(), nodeUpload.end());
	maxDownload = *max_element(nodeDownload.begin(), nodeDownload.end());
	maxLoad = max(maxUpload, maxDownload);

	for (int i = 0; i < nodeNum; i++)
	{
		traffic += nodeUpload[i];
	}

	if (traffic == 0)
	{
		return unbalanceRatio;
	}

	aveLoad = (double)traffic / (double)nodeNum;

	unbalanceRatio = (double)maxLoad / aveLoad;

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
	}

	ratio = ratioSum / (double)ratioNum;

	totalUnbalanceRatios.clear();
	return ratio;
}

int Simulation::Get_InitialStripeNum()
{
	return initialStripeNum;
}

void Simulation::GetChunkStoreOrderInEachNode()
{
	int nodeId;
	int globalChunkId;

	chunkIndex_eachNode.clear();
	chunkIndex_eachNode.resize(nodeNum);

	for (int i = 0; i < chunkIndexs.size(); i++)
	{
		for (int j = 0; j < N; j++)
		{
			nodeId = chunkIndexs[i][j];
			globalChunkId = i * N + j;
			chunkIndex_eachNode[nodeId].push_back(globalChunkId);
		}
	}

	return;
}

void Simulation::ideaGetChunkStoreOrderInEachNode(){
	int nodeId;
	int globalChunkId;

	chunkIndex_eachNode.clear();
	chunkIndex_eachNode.resize(nodeNum);

	for (int i = 0; i < initialStripeNum; i++)
	{
		for (int j = 0; j < N; j++)
		{
			nodeId = chunkIndexsNode[i][j];
			globalChunkId = i * N + j;
			chunkIndex_eachNode[nodeId].push_back(globalChunkId);
		}
	}

	return;
}

int LocateStoreIndex(int nodeId, int globalChunkId)
{
	int startIndex;
	int endIndex;
	int midIndex;
	int storedIndex;

	// locate the stored order of given_chunk_id by using binary search
	startIndex = 0;
	storedIndex = 0;
	endIndex = chunkIndex_eachNode[nodeId].size() - 1;
	

	while (1)
	{
		if (chunkIndex_eachNode[nodeId][startIndex] == globalChunkId)
		{
			storedIndex = startIndex;
			break;
		}
		if (chunkIndex_eachNode[nodeId][endIndex] == globalChunkId)
		{
			storedIndex = endIndex;
			break;
		}

		midIndex = startIndex + (endIndex - startIndex) / 2;

		if (chunkIndex_eachNode[nodeId][midIndex] > globalChunkId)
			endIndex = midIndex - 1;

		else if (chunkIndex_eachNode[nodeId][midIndex] < globalChunkId)
			startIndex = midIndex + 1;

		else if (chunkIndex_eachNode[nodeId][midIndex] == globalChunkId)
		{
			storedIndex = midIndex;
			break;
		}

		if (startIndex >= endIndex)
		{
			break;
		}

	}

	return storedIndex;

}

void Simulation::printTraffic(const char input[])
{
	if (strcmp(input, "idea") == 0)
	{
		cout << "idea relocation traffic: " << ideaRelocationNum << endl;
		cout << "idea traffic: " << ideaTraffic << endl;
	}
	else if (strcmp(input, "baseline") == 0)
	{
		cout << "baseline relocation traffic: " << baselineRelocationNum << endl;
		cout << "baseline traffic: " << baselineTraffic << endl;
	}
	else if (strcmp(input, "SRS") == 0)
	{
		cout << "SRS relocation traffic: " << 0 << endl;
		cout << "SRS traffic: " << SRSTraffic << endl;
	}
	else if (strcmp(input, "ERS") == 0)
	{
		cout << "ERS relocation traffic: " << 0 << endl;
		cout << "ERS traffic: " << ERSTraffic << endl;
	}
	else
	{
		cout << "printTraffic error" << endl;
	}
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

double Simulation::noSchedulingTransmission(vector<vector<pair<int, int>>>& transmissionTaskEachStripe,
	vector<vector<pair<int, int>>>& relocationTaskEachStripe, vector<int>& extensionStripeCollectNodeId)
{
	double timeOverhead = 0.0;
	int extentionStripeNum = transmissionTaskEachStripe.size();
	int transmissionNum = 0; // transmissionNum = traffic
	int sendNodeId;
	int receiveNodeId;
	int remain = -1;
	int collectNodeId;
	int collectCanUpdateParityThreshold = M - 1;
	int transmissionTakSize;

	vector<int> isNodeSendOccupied;
	vector<int> isNodeReceiveOccupied;

	isNodeSendOccupied.resize(nodeNum);
	isNodeReceiveOccupied.resize(nodeNum);

	// get transmissionNum
	for (int i = 0; i < extentionStripeNum; i++)
	{
		transmissionNum += transmissionTaskEachStripe[i].size();
		transmissionNum += relocationTaskEachStripe[i].size();
	}

	// execute transmission
	while (transmissionNum > 0)
	{
		for (int i = 0; i < nodeNum; i++)
		{
			isNodeSendOccupied[i] = 0;
			isNodeReceiveOccupied[i] = 0;
		}
		// update
		for (int i = 0; i < extentionStripeNum; i++)
		{
			collectNodeId = extensionStripeCollectNodeId[i];
			transmissionTakSize = transmissionTaskEachStripe[i].size();
			for (int j = 0; j < transmissionTaskEachStripe[i].size(); j++)
			{
				sendNodeId = transmissionTaskEachStripe[i][j].first;
				receiveNodeId = transmissionTaskEachStripe[i][j].second;

				// if collectNode has chunks to receive, it cannot send chunk
				if (sendNodeId == collectNodeId)
				{
					if (transmissionTakSize > collectCanUpdateParityThreshold)
						break;
				}

				// if this node is occupied(send), skip
				if (isNodeSendOccupied[sendNodeId] != 0)
					continue;

				// if this node is occupied(receive), skip
				if (isNodeReceiveOccupied[receiveNodeId] != 0)
					continue;

				// now can transport
				isNodeSendOccupied[sendNodeId] = 1;
				isNodeReceiveOccupied[receiveNodeId] = 1;
				transmissionTaskEachStripe[i].erase(transmissionTaskEachStripe[i].begin() + j);
				//nodeReceiveNumEachStripe[i][receiveNodeId]--;
				transmissionNum--;
			}
		}

		// relocation
		for (int i = 0; i < extentionStripeNum; i++)
		{
			for (int j = 0; j < relocationTaskEachStripe[i].size(); j++)
			{
				sendNodeId = relocationTaskEachStripe[i][j].first;
				receiveNodeId = relocationTaskEachStripe[i][j].second;

				// if this node is occupied(send), skip
				if (isNodeSendOccupied[sendNodeId] != 0)
					continue;

				// if this node is occupied(receive), skip
				if (isNodeReceiveOccupied[receiveNodeId] != 0)
					continue;

				// now can transport
				isNodeSendOccupied[sendNodeId] = 1;
				isNodeReceiveOccupied[receiveNodeId] = 1;
				relocationTaskEachStripe[i].erase(relocationTaskEachStripe[i].begin() + j);
				transmissionNum--;
			}
		}

		//remain = 0;
		if (remain == transmissionNum)
		{
			cout << "transmission error" << endl;
			exit(0);
		}
		remain = 0;
		for (int i = 0; i < extentionStripeNum; i++)
		{
			remain += transmissionTaskEachStripe[i].size();
			remain += relocationTaskEachStripe[i].size();
		}

		timeOverhead += transmissionTime;
	}

	for (int i = 0; i < nodeNum; i++)
	{
		nodeSendSeq[i].clear();
	}

	return timeOverhead;
}

void Simulation::soloveRelocationUpScale(vector<vector<pair<int, int>>>& graph,
	vector<vector<int>>& chunkIndexsAfterExtension,
	vector<int>& deleteStripeIdThisRound, vector<vector<int>>& nodeHoldDeleteChunkId,
	vector<vector<pair<int, int>>>& transmissionTaskEachStripe, vector<vector<pair<int, int>>>& relocationTaskEachStripe,
	vector<int>& extensionStripeCollectNodeId,
	vector<int>& nodeUpload, vector<int>& nodeDownload,
	vector<vector<SendChunkTask>>& updataParityTask_eachExtensionStripe,
	vector<vector<SendChunkTask>>& relocationTask_eachExtensionStripe)
{
	int deleteStripeNum = deleteStripeIdThisRound.size();
	int extensionStripeNum = chunkIndexsAfterExtension.size();
	int nodeNum = nodeDownload.size();
	int vertexNum = deleteStripeNum + nodeNum + extensionStripeNum + 2;
	int currentVertex;
	int deleteStripeStartVertex = 2;
	int nodeStartVertex = deleteStripeStartVertex + deleteStripeNum;
	int extensionStripeStartVertex = nodeStartVertex + nodeNum;
	int sendNodeId;
	int receiveNodeId;
	int nodeIdStillHaveDataChunk;
	int tempInt;
	int collectNodeId;
	int dataNodeId;
	int parityNodeId;

	// testbed modification
	int chunkId;
	int dataIndexInNewStripe;
	SendChunkTask tempSendChunkTask;

	vector<pair<int, int>> sortedNodeDownload;
	tempInt = nodeDownload[0];
	for (int i = 0; i < nodeNum; i++)
	{
		sortedNodeDownload.push_back(make_pair(i, nodeDownload[i]));
	}

	currentVertex = extensionStripeStartVertex;
	nodeIdStillHaveDataChunk = 0;
	for (int i = 0; i < extensionStripeNum; i++)
	{
		// collect dataChunks(no relocation ones)
		collectNodeId = extensionStripeCollectNodeId[i];
		for (int j = K; j < chunkIndexsAfterExtension[i].size(); j++)
		{
			dataNodeId = chunkIndexsAfterExtension[i][j];
			transmissionTaskEachStripe[i].push_back(make_pair(dataNodeId, collectNodeId));
		}

		while (chunkIndexsAfterExtension[i].size() < KLarge)
		{
			// choose sendNodeId
			for (; nodeIdStillHaveDataChunk < nodeNum; nodeIdStillHaveDataChunk++)
			{
				if (nodeHoldDeleteChunkId[nodeIdStillHaveDataChunk].size() > 0)
				{
					break;
				}
			}
			sendNodeId = nodeIdStillHaveDataChunk;
			if (sendNodeId >= nodeNum)
			{
				cout << "error in soloveRelocationUpScale 2" << endl;
				cout << "Simulation stop!" << endl;
				exit(0);
			}

			// choose receiveNodeId
			sort(sortedNodeDownload.begin(), sortedNodeDownload.end(), compareSecondAscending);
			receiveNodeId = -1;
			for (int j = 0; j < nodeNum; j++)
			{
				receiveNodeId = sortedNodeDownload[j].first;
				if (receiveNodeId == sendNodeId)
					continue;
				// if this extensionStripe has chunk in this node, skip and choose another one
				if (graph[nodeStartVertex + receiveNodeId][currentVertex].first == 1
					&& graph[nodeStartVertex + receiveNodeId][currentVertex].second == 0)
				{
					break;
				}
				else
				{
					receiveNodeId = -1;
				}
			}
			if (receiveNodeId == -1)
			{
				cout << "error in soloveRelocationUpScale 1" << endl;
				cout << "Simulation stop!" << endl;
				exit(0);
			}
			if (sendNodeId == receiveNodeId)
			{
				cout << "error in soloveRelocationUpScale 3" << endl;
				cout << "Simulation stop!" << endl;
				exit(0);
			}

			// testbed modification
			dataIndexInNewStripe = chunkIndexsAfterExtension[i].size();

			chunkIndexsAfterExtension[i].push_back(receiveNodeId);
			nodeUpload[sendNodeId] += 1;
			nodeDownload[receiveNodeId] += 1;
			// update graph
			graph[nodeStartVertex + receiveNodeId][currentVertex].second = 1;
			// relocation
			relocationTaskEachStripe[i].push_back(make_pair(sendNodeId, receiveNodeId));
			// collect this dataChunk
			if (sendNodeId != collectNodeId)
				transmissionTaskEachStripe[i].push_back(make_pair(sendNodeId, collectNodeId));
			ideaRelocationNum++;
			for (int j = 0; j < nodeNum; j++)
			{
				sortedNodeDownload[j].first = j;
				sortedNodeDownload[j].second = nodeDownload[j];
			}

			// testbed modification
			// fill relocationTask for each extensionStripe
			chunkId = nodeHoldDeleteChunkId[sendNodeId].back();
			tempSendChunkTask.chunkId = chunkId;
			tempSendChunkTask.sendNodeId = sendNodeId;
			tempSendChunkTask.receiveNodeId = receiveNodeId;
			tempSendChunkTask.dataIndexInNewStripe = dataIndexInNewStripe; // -1 -> this is a relocation task, <--- no
			relocationTask_eachExtensionStripe[i].push_back(tempSendChunkTask);
			//fill updateParityTask for each extensionStripe
			tempSendChunkTask.receiveNodeId = collectNodeId;
			tempSendChunkTask.dataIndexInNewStripe = dataIndexInNewStripe;
			updataParityTask_eachExtensionStripe[i].push_back(tempSendChunkTask);

			nodeHoldDeleteChunkId[sendNodeId].pop_back();

		}
		currentVertex++;
	}
}

double Simulation::ideaUpScalePerRound(vector<int>& extensionStripeIdThisRound,
	vector<int>& deleteStripeIdThisRound, vector<int>& extensionStripeCollectNodeId,
	vector<int>& nodeUpload, vector<int>& nodeDownload,
	vector<vector<SendChunkTask>>& updataParityTask_eachExtensionStripe,
	vector<vector<SendChunkTask>>& relocationTask_eachExtensionStripe)
{
	double timeOverhead = 0.0;
	/* set up flow graph */
	int deleteStripeNum = deleteStripeIdThisRound.size();
	int extensionStripeNum = extensionStripeIdThisRound.size();
	int nodeNum = nodeDownload.size();
	int vertexNum = deleteStripeNum + nodeNum + extensionStripeNum + 2;
	int currentVertex;
	int deleteStripeStartVertex = 2;
	int nodeStartVertex = deleteStripeStartVertex + deleteStripeNum;
	int extensionStripeStartVertex = nodeStartVertex + nodeNum;
	int chunkId;
	int absoluteStripeId;
	int dataNodeId;
	int nodeId;
	int parityNodeId;
	int kGap = KLarge - K;
	int collectNodeId;

	// testbed modification
	int dataIndexInNewStripe;
	SendChunkTask tempSendChunkTask;

	vector<vector<pair<int, int>>> graph;
	vector<vector<pair<int, int>>> transmissionTaskEachStripe;
	vector<vector<pair<int, int>>> relocationTaskEachStripe;
	vector<vector<int>> nodeHoldDeleteDataChunkId;
	vector<vector<int>> oldStripeRemainDataNode;
	vector<vector<int>> chunkIndexsAfterExtension;

	transmissionTaskEachStripe.resize(extensionStripeNum);
	relocationTaskEachStripe.resize(extensionStripeNum);
	chunkIndexsAfterExtension.resize(extensionStripeNum);
	graph.resize(vertexNum + 1);
	for (int i = 0; i <= vertexNum; i++)
	{
		graph[i].resize(vertexNum + 1);
		for (int j = 0; j <= vertexNum; j++)
		{
			graph[i][j].first = graph[i][j].second = 0;
		}
	}

	currentVertex = 1;
	for (int i = deleteStripeStartVertex; i < nodeStartVertex; i++)
	{
		graph[currentVertex][i].first = K;
	}
	currentVertex = deleteStripeStartVertex;
	for (int i = 0; i < deleteStripeNum; i++)
	{
		absoluteStripeId = deleteStripeIdThisRound[i];
		for (int j = 0; j < K; j++)
		{
			dataNodeId = chunkIndexs[absoluteStripeId][j];
			graph[currentVertex][nodeStartVertex + dataNodeId].first = 1;
		}
		currentVertex++;
	}
	for (int i = 0; i < extensionStripeNum; i++)
	{
		for (int j = 0; j < nodeNum; j++)
		{
			graph[nodeStartVertex + j][extensionStripeStartVertex + i].first = 1;
		}
		absoluteStripeId = extensionStripeIdThisRound[i];
		for (int j = 0; j < N; j++)
		{
			nodeId = chunkIndexs[absoluteStripeId][j];
			graph[nodeStartVertex + nodeId][extensionStripeStartVertex + i].first = 0;
		}
	}
	for (int i = 0; i < extensionStripeNum; i++)
	{
		graph[extensionStripeStartVertex + i][vertexNum].first = kGap;
	}

	/* dinic */
	dinic(graph, vertexNum);

	nodeHoldDeleteDataChunkId.resize(nodeNum);
	/* fill nodeHoldAbsoluteDataIndex */
	currentVertex = deleteStripeStartVertex;
	for (int i = 0; i < deleteStripeNum; i++)
	{
		absoluteStripeId = deleteStripeIdThisRound[i];
		for (int j = 0; j < K; j++)
		{
			dataNodeId = chunkIndexs[absoluteStripeId][j];
			if (graph[currentVertex][nodeStartVertex + dataNodeId].first == 1)
			{
				chunkId = absoluteStripeId * N + j; // testbed modification
				nodeHoldDeleteDataChunkId[dataNodeId].push_back(chunkId);
			}
		}
		currentVertex++;
	}

	/* fill chunkIndexAfterExtension */
	for (int i = 0; i < extensionStripeNum; i++)
	{
		absoluteStripeId = extensionStripeIdThisRound[i];
		for (int j = 0; j < K; j++)
		{
			dataNodeId = chunkIndexs[absoluteStripeId][j];
			chunkIndexsAfterExtension[i].push_back(dataNodeId);
		}
	}
	/* add data node for each extension stripe in chunkIndexAfterExtension */
	for (int i = 0; i < extensionStripeNum; i++)
	{
		collectNodeId = extensionStripeCollectNodeId[i];
		for (int j = 0; j < nodeNum; j++)
		{
			if (graph[nodeStartVertex + j][extensionStripeStartVertex + i].second == 1)
			{
				if (graph[nodeStartVertex + j][extensionStripeStartVertex + i].first != 1)
				{
					cout << "error in ideaUpScalePerRound" << endl;
					cout << "Simulation stop!" << endl;
					exit(0);
				}

				// testbed modification
				dataIndexInNewStripe = chunkIndexsAfterExtension[i].size();

				chunkIndexsAfterExtension[i].push_back(j);

				// testbed modification
				chunkId = nodeHoldDeleteDataChunkId[j].back();
				tempSendChunkTask.chunkId = chunkId;
				tempSendChunkTask.sendNodeId = j;
				tempSendChunkTask.receiveNodeId = collectNodeId;
				tempSendChunkTask.dataIndexInNewStripe = dataIndexInNewStripe;
				updataParityTask_eachExtensionStripe[i].push_back(tempSendChunkTask);

				nodeHoldDeleteDataChunkId[j].pop_back();

			}
		}
	}

	/* solve data relocation, fill dataChunk to all newStripe, finish collect dataChunks*/
	soloveRelocationUpScale(graph, chunkIndexsAfterExtension, deleteStripeIdThisRound,
		nodeHoldDeleteDataChunkId, transmissionTaskEachStripe, relocationTaskEachStripe,
		extensionStripeCollectNodeId, nodeUpload, nodeDownload,
		updataParityTask_eachExtensionStripe, relocationTask_eachExtensionStripe);

	/* update newChunkIndexs for new stripes here */

	/* finish fill chunkIndexAfterExtension: fill parityChunk */
	for (int i = 0; i < extensionStripeNum; i++)
	{
		absoluteStripeId = extensionStripeIdThisRound[i];
		for (int j = 0; j < M; j++)
		{
			parityNodeId = chunkIndexs[absoluteStripeId][K + j];
			chunkIndexsAfterExtension[i].push_back(parityNodeId);
		}
	}
	/* finish transmissionTaskEachStripe */
	for (int i = 0; i < extensionStripeNum; i++)
	{
		collectNodeId = extensionStripeCollectNodeId[i];
		for (int j = 0; j < M; j++)
		{
			parityNodeId = chunkIndexsAfterExtension[i][KLarge + j];
			if (parityNodeId == collectNodeId)
				continue;
			transmissionTaskEachStripe[i].push_back(make_pair(collectNodeId, parityNodeId));
		}
	}

	for (int i = 0; i < extensionStripeNum; i++)
	{
		newChunkIndexs.push_back(chunkIndexsAfterExtension[i]);
	}

	//timeOverhead = schedulingTransmission();
	//timeOverhead = noSchedulingTransmission();
	timeOverhead = noSchedulingTransmission(transmissionTaskEachStripe, relocationTaskEachStripe,
		extensionStripeCollectNodeId);

	return timeOverhead;
}

int Simulation::tryPickDeleteStripeAndGetTarget(int stripeId, vector<int>& nodeUpload)
{
	int target;
	int dataNodeId;
	int kGap = KLarge - K;
	int minUpload;
	int maxUpload;

	vector<int> tempNodeUpload = nodeUpload;

	// try to pick, update temp load
	for (int i = 0; i < K; i++)
	{
		dataNodeId = chunkIndexs[stripeId][i];
		tempNodeUpload[dataNodeId] += 1;
	}

	// get target
	minUpload = *min_element(tempNodeUpload.begin(), tempNodeUpload.end());
	maxUpload = *max_element(tempNodeUpload.begin(), tempNodeUpload.end());

	target = maxUpload - minUpload;
	return target;
}

vector<int> Simulation::pickDeleteStripeAccordingLoad(vector<int>& remainStripeId,
	int pickNumber, vector<int>& nodeUpload)
{
	int remainStripeIdSize = remainStripeId.size();
	int remainStripeNum = remainStripeIdSize;
	int stripeId;
	int stripeIndex;
	int target;
	int dataNodeId;
	int minTargetNow;
	int indexWithMinTarget;

	vector<int> pickedStripeId;
	// pair<stripeIndex, target>, target: (maxDownload-minDownload) + (maxUpload-minUpload)
	//vector<pair<int, int>> stripeAttachTarget;

	while (pickedStripeId.size() < pickNumber)
	{
		if (remainStripeNum == 0)
			break;
		indexWithMinTarget = -1;
		minTargetNow = INT_MAX;
		//stripeAttachTarget.clear();
		for (int i = 0; i < remainStripeIdSize; i++)
		{
			if (remainStripeId[i] == -1)
				continue;
			stripeId = remainStripeId[i];
			target = tryPickDeleteStripeAndGetTarget(stripeId, nodeUpload);
			//stripeAttachTarget.push_back(make_pair(i, target));
			if (target < minTargetNow)
			{
				minTargetNow = target;
				indexWithMinTarget = i;
			}
		}

		// sort and get the stripe of min target
		//sort(stripeAttachTarget.begin(), stripeAttachTarget.end(), compareSecondAscending);
		stripeIndex = indexWithMinTarget;
		//stripeIndex = stripeAttachTarget.front().first;
		stripeId = remainStripeId[stripeIndex];
		pickedStripeId.push_back(stripeId);
		remainStripeId[stripeIndex] = -1;
		remainStripeNum--;

		// update upload
		for (int i = 0; i < K; i++)
		{
			dataNodeId = chunkIndexs[stripeId][i];
			nodeUpload[dataNodeId] += 1;
		}

	}

	return pickedStripeId;
}

int Simulation::tryPickExtensionStripeAndGetTarget(int stripeId, int& collectNodeId,
	vector<int>& nodeUpload, vector<int>& nodeDownload)
{
	int target;
	int parityNodeId;
	int kGap = KLarge - K;
	int minDownload;
	int minUpload;
	int maxDownload;
	int maxUpload;
	int tryingCollectNodeId;
	int minTarget = INT_MAX;
	int maxLoad;
	int minLoad;

	vector<int> tempNodeDownload;
	vector<int> tempNodeUpload;

	//get collectNodeId with the min target
	for (int i = 0; i < M; i++)
	{
		tryingCollectNodeId = chunkIndexs[stripeId][K + i];
		tempNodeDownload = nodeDownload;
		tempNodeUpload = nodeUpload;

		// try to pick, update temp load
		for (int j = 0; j < M; j++)
		{
			parityNodeId = chunkIndexs[stripeId][K + j];
			if (parityNodeId == tryingCollectNodeId)
			{
				tempNodeDownload[parityNodeId] += kGap;
				tempNodeUpload[parityNodeId] += M - 1;
			}
			else
			{
				tempNodeDownload[parityNodeId] += 1;
			}
		}

		// get target
		minDownload = *min_element(tempNodeDownload.begin(), tempNodeDownload.end());
		maxDownload = *max_element(tempNodeDownload.begin(), tempNodeDownload.end());
		minUpload = *min_element(tempNodeUpload.begin(), tempNodeUpload.end());
		maxUpload = *max_element(tempNodeUpload.begin(), tempNodeUpload.end());

		maxLoad = max(maxDownload, maxUpload);
		minLoad = min(minDownload, minUpload);

		target = maxLoad - minLoad;

		// judge the smaller one
		if (target < minTarget)
		{
			minTarget = target;
			collectNodeId = tryingCollectNodeId;
		}

		// choose the first parity as the collectNode
		break;
	}

	return minTarget;
}

vector<int> Simulation::pickExtensionStripeAccordingLoad(vector<int>& remainStripeId,
	int pickNumber, vector<int>& pickedStripeCollectNodeId,
	vector<int>& nodeUpload, vector<int>& nodeDownload)
{
	int remainStripeIdSize = remainStripeId.size();
	int remainStripeNum = remainStripeIdSize;
	int collectNodeId;
	int stripeId;
	int stripeIndex;
	int target;
	int parityNodeId;
	int kGap = KLarge - K;
	int minTargetNow;
	int indexWithMinTarget;

	vector<int> pickedStripeId;
	vector<int> remainStripeCollectNodeId;
	// pair<stripeIndex, target>, target: (maxDownload-minDownload) + (maxUpload-minUpload)
	//vector<pair<int, int>> stripeAttachTarget;

	pickedStripeCollectNodeId.clear();
	remainStripeCollectNodeId.resize(remainStripeIdSize);

	while (pickedStripeId.size() < pickNumber)
	{
		if (remainStripeNum == 0)
			break;
		//stripeAttachTarget.clear();
		indexWithMinTarget = -1;
		minTargetNow = INT_MAX;
		for (int i = 0; i < remainStripeIdSize; i++)
		{
			if (remainStripeId[i] == -1)
				continue;
			stripeId = remainStripeId[i];
			target = tryPickExtensionStripeAndGetTarget(stripeId, collectNodeId, nodeUpload, nodeDownload);
			remainStripeCollectNodeId[i] = collectNodeId;
			//stripeAttachTarget.push_back(make_pair(i, target));
			if (target < minTargetNow)
			{
				minTargetNow = target;
				indexWithMinTarget = i;
			}
		}

		// sort and get the stripe of min target
		//sort(stripeAttachTarget.begin(), stripeAttachTarget.end(), compareSecondAscending);
		stripeIndex = indexWithMinTarget;
		//stripeIndex = stripeAttachTarget.front().first;
		stripeId = remainStripeId[stripeIndex];
		pickedStripeId.push_back(stripeId);
		collectNodeId = remainStripeCollectNodeId[stripeIndex];
		pickedStripeCollectNodeId.push_back(collectNodeId);
		remainStripeId[stripeIndex] = -1;
		remainStripeNum--;

		// update download and upload
		for (int i = 0; i < M; i++)
		{
			parityNodeId = chunkIndexs[stripeId][K + i];
			if (parityNodeId == collectNodeId)
			{
				nodeDownload[parityNodeId] += kGap;
				nodeUpload[parityNodeId] += M - 1;
			}
			else
			{
				nodeDownload[parityNodeId] += 1;
			}
		}

	}

	return pickedStripeId;
}

void Simulation::IdeaExecuteScale(vector<int>& extensionStripeIdThisRound,
	vector<int>& extensionStripeCollectNodeId,
	vector<vector<SendChunkTask>>& updataParityTask_eachExtensionStripe,
	vector<vector<SendChunkTask>>& relocationTask_eachExtensionStripe)
{
	int extensionStripeNum = extensionStripeIdThisRound.size();

	// (KLarge-K)(data chunk) + M(parity chunk, one of them is collectNode)
	int commandNumEachExtensionStripe = KLarge - K + M;
	int port;// (PROXY_START_PORT+index) in extensionStripeIdThisRound
	int kGap = KLarge - K;
	int sendNodeId;
	int receiceNodeId;
	int chunkId;
	int storeIndex;
	int collectNodeId;
	int index;
	int stripeId;
	int tempInt;
	int parityNodeId;
	int ackNumNeedReceive = 0;

	string ip;

	vector<int> indexOfCollectNode_eachExtensionStripe;
	vector<vector<IdeaMdsUpdateParityCommand>> commandEachExensionStripe; // update
	vector<vector<IdeaMdsRelocationCommand>> relocationCommandEachExtensionStripe;

	commandEachExensionStripe.resize(extensionStripeNum);
	relocationCommandEachExtensionStripe.resize(extensionStripeNum);
	indexOfCollectNode_eachExtensionStripe.resize(extensionStripeNum);
	for (int i = 0; i < extensionStripeNum; i++)
	{
		commandEachExensionStripe[i].resize(commandNumEachExtensionStripe);
	}

	/* add updateParityTask into commandEachExtensionStripe: i.e collect dataChunk,
	   we first fill command for dataChunk here
	*/
	for (int i = 0; i < extensionStripeNum; i++)
	{
		port = PROXY_START_PORT + i;
		for (int j = 0; j < updataParityTask_eachExtensionStripe[i].size(); j++)
		{
			sendNodeId = updataParityTask_eachExtensionStripe[i][j].sendNodeId;
			receiceNodeId = updataParityTask_eachExtensionStripe[i][j].receiveNodeId;
			if (sendNodeId == receiceNodeId)
			{
				// collectNode holds dataChunk, this will be dealt with in a relocation section later
				continue;
			}
			// size of command
			commandEachExensionStripe[i][j].sizeOfCommand = sizeof(IdeaMdsUpdateParityCommand);
			// receive command node ip
			ip = proxyIP[sendNodeId];
			strcpy(commandEachExensionStripe[i][j].nodeIP_ReceiveThisCommand, ip.c_str());
			// destinationIp
			ip = proxyIP[receiceNodeId];
			strcpy(commandEachExensionStripe[i][j].destinationIp[0], ip.c_str());
			// destination node's port to send
			commandEachExensionStripe[i][j].port = port;
			// role
			commandEachExensionStripe[i][j].role = DATA_CHUNK;
			// for DATA_CHUNK
			commandEachExensionStripe[i][j].dataIndexInNewStripe =
				updataParityTask_eachExtensionStripe[i][j].dataIndexInNewStripe;
			chunkId = updataParityTask_eachExtensionStripe[i][j].chunkId;
			storeIndex = LocateStoreIndex(sendNodeId, chunkId);
			commandEachExensionStripe[i][j].dataStoreIndex = storeIndex;
		}
	}

	/* we the fill command for parity node(not include collectNode) here */
	for (int i = 0; i < extensionStripeNum; i++)
	{
		stripeId = extensionStripeIdThisRound[i];
		collectNodeId = extensionStripeCollectNodeId[i];
		port = PROXY_START_PORT + i;
		//command for parityNode start from kGap
		index = kGap - 1;
		for (int j = 0; j < M; j++)
		{
			index++;
			parityNodeId = chunkIndexs[stripeId][K + j];
			if (parityNodeId == collectNodeId)
			{
				continue;
			}
			chunkId = stripeId * N + K + j;
			// size of command
			commandEachExensionStripe[i][index].sizeOfCommand = sizeof(IdeaMdsUpdateParityCommand);
			// receive command node ip
			ip = proxyIP[parityNodeId];
			strcpy(commandEachExensionStripe[i][index].nodeIP_ReceiveThisCommand, ip.c_str());
			// listening in this port, and wait for a parity delta chunk
			commandEachExensionStripe[i][index].port = port;
			// role
			commandEachExensionStripe[i][index].role = PARITY_CHUNK;
			// for PARITY_CHUNK(not include collectNode)
			commandEachExensionStripe[i][index].parityIndexInNewStripe = j;
			storeIndex = LocateStoreIndex(parityNodeId, chunkId);
			commandEachExensionStripe[i][index].parityStoreIndex = storeIndex;
			commandEachExensionStripe[i][index].receiveNum = 1;
		}
	}

	/* we then fill command for collectNode here */
	for (int i = 0; i < extensionStripeNum; i++)
	{
		stripeId = extensionStripeIdThisRound[i];
		collectNodeId = extensionStripeCollectNodeId[i];
		port = PROXY_START_PORT + i;
		// get the collectNode's index in commands for each extensionStripe
		index = kGap - 1;
		for (int j = 0; j < M; j++)
		{
			index++;
			parityNodeId = chunkIndexs[stripeId][K + j];
			chunkId = stripeId * N + K + j;
			if (parityNodeId == collectNodeId)
			{
				commandEachExensionStripe[i][index].parityIndexInNewStripe = j;
				break;
			}
		}
		// record index of collect node
		indexOfCollectNode_eachExtensionStripe[i] = index;
		// size of command
		commandEachExensionStripe[i][index].sizeOfCommand = sizeof(IdeaMdsUpdateParityCommand);
		// receive command node ip
		ip = proxyIP[collectNodeId];
		strcpy(commandEachExensionStripe[i][index].nodeIP_ReceiveThisCommand, ip.c_str());
		// destinationIp
		for (int j = 0; j < M; j++)
		{
			receiceNodeId = chunkIndexs[stripeId][K + j];
			if (receiceNodeId == collectNodeId)
			{
				continue;
			}
			ip = proxyIP[receiceNodeId];
			strcpy(commandEachExensionStripe[i][index].destinationIp[j], ip.c_str());
		}

		// destination node's port to send
		port = PROXY_START_PORT + i;
		commandEachExensionStripe[i][index].port = port;
		// role
		commandEachExensionStripe[i][index].role = COLLECT_NODE;

		// collectNode is also a parityNode
		storeIndex = LocateStoreIndex(collectNodeId, chunkId);
		commandEachExensionStripe[i][index].parityStoreIndex = storeIndex;
		// default, maybe less(maybe change in relocation later)
		commandEachExensionStripe[i][index].receiveNum = kGap;
		commandEachExensionStripe[i][index].storedDataChunkNum_inCollectNode = 0;

	}

	/* we the fill command for relocation here */
	for (int i = 0; i < extensionStripeNum; i++)
	{
		if (relocationTask_eachExtensionStripe[i].size() == 0)
		{
			continue;
		}
		stripeId = extensionStripeIdThisRound[i];
		collectNodeId = extensionStripeCollectNodeId[i];
		port = RELOCATION_START_PORT + i;
		for (int j = 0; j < relocationTask_eachExtensionStripe[i].size(); j++)
		{
			sendNodeId = relocationTask_eachExtensionStripe[i][j].sendNodeId;
			receiceNodeId = relocationTask_eachExtensionStripe[i][j].receiveNodeId;
			chunkId = relocationTask_eachExtensionStripe[i][j].chunkId;
			IdeaMdsRelocationCommand senderIdeaMdsRelocationCommand;
			IdeaMdsRelocationCommand receiverIdeaMdsRelocationCommand;

			// for sender
			senderIdeaMdsRelocationCommand.sizeOfCommand = sizeof(IdeaMdsRelocationCommand);
			ip = proxyIP[sendNodeId];
			strcpy(senderIdeaMdsRelocationCommand.nodeIP_ReceiveThisCommand, ip.c_str());
			senderIdeaMdsRelocationCommand.port = port;
			senderIdeaMdsRelocationCommand.role = RELOCATION_SENDER;
			storeIndex = LocateStoreIndex(sendNodeId, chunkId);
			senderIdeaMdsRelocationCommand.dataStoreIndex = storeIndex;
			ip = proxyIP[receiceNodeId];
			strcpy(senderIdeaMdsRelocationCommand.destinationIp, ip.c_str());

			// for receiver
			receiverIdeaMdsRelocationCommand.sizeOfCommand = sizeof(IdeaMdsRelocationCommand);
			strcpy(receiverIdeaMdsRelocationCommand.nodeIP_ReceiveThisCommand, ip.c_str());
			receiverIdeaMdsRelocationCommand.port = port;
			receiverIdeaMdsRelocationCommand.role = RELOCATION_RECEIVER;
			storeIndex = chunkIndex_eachNode[receiceNodeId].size();
			receiverIdeaMdsRelocationCommand.writeLocation = storeIndex;


			// if the dataChunk in collectNode
			if (sendNodeId == collectNodeId)
			{
				index = indexOfCollectNode_eachExtensionStripe[i];
				storeIndex = LocateStoreIndex(collectNodeId, chunkId);
				tempInt = commandEachExensionStripe[i][index].storedDataChunkNum_inCollectNode;
				commandEachExensionStripe[i][index].dataStoreIndexs_inCollectNode[tempInt] = storeIndex;
				commandEachExensionStripe[i][index].dataIndexInNewstripe_inCollectNode[tempInt]
					= relocationTask_eachExtensionStripe[i][j].dataIndexInNewStripe;
				commandEachExensionStripe[i][index].storedDataChunkNum_inCollectNode += 1;
				commandEachExensionStripe[i][index].receiveNum--;
				if (commandEachExensionStripe[i][index].storedDataChunkNum_inCollectNode > MAX_RELOCATION_NUM)
				{
					cout << "relocation too much in collect node" << endl;
					exit(0);
				}
			}

			relocationCommandEachExtensionStripe[i].push_back(receiverIdeaMdsRelocationCommand);
			relocationCommandEachExtensionStripe[i].push_back(senderIdeaMdsRelocationCommand);
		}

	}

	ackNumNeedReceive = extensionStripeNum * M;
	for (int i = 0; i < extensionStripeNum; i++)
	{
		ackNumNeedReceive += relocationTask_eachExtensionStripe[i].size();
	}

	cout << "idea mds send command" << endl;
	for (int i = 0; i < extensionStripeNum; i++)
	{
		IdeaSendUpdateCommandEachStripe(commandEachExensionStripe[i]);
		//StopHere();
	}
	for (int i = 0; i < extensionStripeNum; i++)
	{
		if (relocationCommandEachExtensionStripe[i].size() == 0)
		{
			continue;
		}
		IdeaSendRelocationCommandEachStripe(relocationCommandEachExtensionStripe[i]);
		//StopHere();
	}
	cout << "idea mds send command finish" << endl;

	ReceiveAck(ackNumNeedReceive);
	//StopHere();

}

double Simulation::ideaUpScale(int KLarge)
{
	cout << this->K << " to " << KLarge << endl;
	this->KLarge = KLarge;
	this->minKl = KLarge / gcd(K, KLarge);
	// stripeNumber: total scale stripe number actually
	this->stripeNumber = initialStripeNum - (initialStripeNum % minKl);
	this->scaleNumPerRound = downScaleStripePerRound;
	this->ideaTraffic = 0;
	this->ideaRelocationNum = 0;
	while (this->scaleNumPerRound % minKl != 0)
	{
		this->scaleNumPerRound--;
	}
	ideaScaleNumberPerRound = scaleNumPerRound;
	ideaStripeNumber = stripeNumber;
	double timeOverhead = 0.0;
	int remainStripeNum;
	int minK = K / gcd(K, KLarge);
	int p1 = scaleNumPerRound * minK / minKl;
	int p2 = scaleNumPerRound - p1;
	int stripeId;
	int partitionId;
	int actulPartitionLength = PARTITION_LENGTH;
	int stripePartitionSize;
	vector<int> remainStripeId;
	vector<int> extensionStripeIdThisRound;
	vector<int> deleteStripeIdThisRound;
	vector<int> nodeDownload;
	vector<int> nodeUpload;
	vector<int> extensionStripeCollectNodeId;

	// testbed modification
	vector<vector<int>> stripePartition;
	vector<vector<SendChunkTask>> updataParityTask_eachExtensionStripe;
	vector<vector<SendChunkTask>> relocationTask_eachExtensionStripe;
	actulPartitionLength = actulPartitionLength - (PARTITION_LENGTH % minKl);
	stripePartitionSize = stripeNumber / actulPartitionLength;
	if (stripeNumber % actulPartitionLength != 0)
	{
		stripePartitionSize += 1;
	}
	stripePartition.resize(stripePartitionSize);

	newChunkIndexs.clear();

	if (p1 % minK != 0 || p2 % (minKl - minK) != 0)
	{
		cout << "error in ideaUpScale 1" << endl;
		cout << "Simulation stop" << endl;
		exit(0);
	}

	// testbed modification
	//for (int i = 0; i < stripeNumber; i++)
	//{
	//	remainStripeId.push_back(i);
	//}
	stripeId = 0;
	for (int i = 0; i < stripePartition.size(); i++)
	{
		for (int j = 0; j < actulPartitionLength; j++)
		{
			if (stripeId >= stripeNumber)
			{
				break;
			}
			stripePartition[i].push_back(stripeId);
			stripeId++;
		}
	}
	remainStripeId = stripePartition.front();

	for (int i = 0; i < nodeNum; i++)
	{
		nodeSendSeq[i].clear();
		nodeDownload.push_back(0);
		nodeUpload.push_back(0);
	}

	// testbed modification
	partitionId = 1;
	while ((remainStripeNum = remainStripeId.size()) > 0)
	{
		//cout << "remain stripe number: " << remainStripeNum << endl;

		if (p1 + p2 <= remainStripeNum)
		{
			extensionStripeIdThisRound = pickExtensionStripeAccordingLoad(remainStripeId,
				p1, extensionStripeCollectNodeId, nodeUpload, nodeDownload);

			// testbed modification
			updataParityTask_eachExtensionStripe.clear();
			relocationTask_eachExtensionStripe.clear();
			updataParityTask_eachExtensionStripe.resize(p1);
			relocationTask_eachExtensionStripe.resize(p1);

			for (int i = 0; i < remainStripeNum; i++)
			{
				if (remainStripeId[i] == -1)
				{
					remainStripeId.erase(remainStripeId.begin() + i);
					remainStripeNum--;
					i -= 1;
				}
			}

			deleteStripeIdThisRound = pickDeleteStripeAccordingLoad(remainStripeId,
				p2, nodeUpload);
			for (int i = 0; i < remainStripeNum; i++)
			{
				if (remainStripeId[i] == -1)
				{
					remainStripeId.erase(remainStripeId.begin() + i);
					remainStripeNum--;
					i -= 1;
				}
			}

			timeOverhead += ideaUpScalePerRound(extensionStripeIdThisRound,
				deleteStripeIdThisRound, extensionStripeCollectNodeId, nodeUpload, nodeDownload,
				updataParityTask_eachExtensionStripe, relocationTask_eachExtensionStripe);

			IdeaGetUnbanlancePerRound(nodeUpload, nodeDownload);


			IdeaExecuteScale(extensionStripeIdThisRound, extensionStripeCollectNodeId,
				updataParityTask_eachExtensionStripe, relocationTask_eachExtensionStripe);


			for (int i = 0; i < nodeNum; i++)
			{
				nodeSendSeq[i].clear();
				ideaTraffic += nodeUpload[i];
				nodeUpload[i] = 0;
				nodeDownload[i] = 0;
			}
		}

		if (remainStripeNum < scaleNumPerRound && remainStripeNum>0)
		{
			p1 = remainStripeNum * minK / minKl;
			p2 = remainStripeNum - p1;
			if (p1 % minK != 0 || p2 % (minKl - minK) != 0)
			{
				cout << "error in ideaUpScale 2" << endl;
				cout << "Simulation stop" << endl;
				exit(0);
			}
		}

		//testbed modification
		if (remainStripeId.size() <= 0)
		{
			if (partitionId < stripePartition.size())
			{
				cout << "partition++" << endl;
				remainStripeId = stripePartition[partitionId];
				partitionId++;
				p1 = scaleNumPerRound * minK / minKl;
				p2 = scaleNumPerRound - p1;
			}
		}
	}

	chunkIndexs = newChunkIndexs;
	K = KLarge;
	N = K + M;
	initialStripeNum = chunkIndexs.size();
	return timeOverhead;
}

void ElasticEC_MDS(vector<int>& kPool)
{
	if (kPool.size() == 0)
	{
		cout << "no k in kPool" << endl;
	}

	iK = kPool[0];
	Simulation elasticEC;

	struct timeval t_start;
	struct timeval t_end;
	double total_time = 0.0;

	double timeOverhead = 0.0;
	int kl = iK;

	for (int i = 1; i < kPool.size(); i++)
	{
		total_time = 0.0;
		gettimeofday(&t_start, NULL);

		elasticEC.GetChunkStoreOrderInEachNode();

		// multiScale
		cout << "----------------------------" << endl;
		cout << elasticEC.Get_K() << " -> " << kPool[i] << ": " << endl;
		kl = kPool[i];
		cout << "initialStripeNum: " << elasticEC.Get_InitialStripeNum() << endl;

		timeOverhead = elasticEC.ideaUpScale(kl);

		cout << "idea time overhead: " << timeOverhead << endl;
		cout << "unbalance ratio: " << elasticEC.GetUnbalanceRatio() << endl;
		elasticEC.printTraffic("idea");

		gettimeofday(&t_end, NULL);
		total_time = total_time + (t_end.tv_sec - t_start.tv_sec) * 1000000 + (t_end.tv_usec - t_start.tv_usec);
		total_time = total_time / 1000000;
		cout << "time: " << total_time << " s" << endl;

		cout << "----------------------------" << endl;


	}

}

double Simulation::noSchedulingTransmission()
{
	double timeOverhead = 0.0;
	int nodeId;
	int sendNodeId;
	int receiveNodeId;

	vector<int> nodeIndex;
	vector<int> downloadBandwidthOccupiedForEachNode;
	vector<pair<int, int>> sorteddDataNumberNeedSendForEachNode;

	nodeIndex.resize(nodeNum);
	downloadBandwidthOccupiedForEachNode.resize(nodeNum);

	for (int i = 0; i < nodeNum; i++)
	{
		nodeReceiveNum[i] = 0;
		nodeIndex[i] = i;
		nodeSendNum[i] = 0;
	}
	for (int i = 0; i < nodeNum; i++)
	{
		for (int j = 0; j < nodeSendSeq[i].size(); j++)
		{
			receiveNodeId = nodeSendSeq[i][j];
			nodeReceiveNum[receiveNodeId]++;
		}
		nodeSendNum[i] = nodeSendSeq[i].size();
	}

	GetUnbanlancePerRound();

	for (nodeId = 0; nodeId < nodeNum; nodeId++)
	{
		reverse(nodeSendSeq[nodeId].begin(),
			nodeSendSeq[nodeId].end());
		sorteddDataNumberNeedSendForEachNode.push_back(
			make_pair(nodeId, nodeSendSeq[nodeId].size()));
	}
	sort(sorteddDataNumberNeedSendForEachNode.begin(),
		sorteddDataNumberNeedSendForEachNode.end(), compareSecondAscending);

	/*update parity chunks*/
	while (sorteddDataNumberNeedSendForEachNode.back().second > 0)
	{
		timeOverhead += transmissionTime;
		for (nodeId = 0; nodeId < nodeNum; nodeId++)
		{
			downloadBandwidthOccupiedForEachNode[nodeId] = 0;
		}
		random_shuffle(nodeIndex.begin(), nodeIndex.end());
		for (int i = 0; i < nodeNum; i++)
		{
			sendNodeId = nodeIndex[i];
			//sendNodeId = i;
			if (nodeSendSeq[sendNodeId].size() == 0)
				continue;
			receiveNodeId = nodeSendSeq[sendNodeId].back();
			if (downloadBandwidthOccupiedForEachNode[receiveNodeId] == 1)
				continue;
			else
			{
				downloadBandwidthOccupiedForEachNode[receiveNodeId] = 1;
				nodeSendSeq[sendNodeId].pop_back();
			}
		}
		for (nodeId = 0; nodeId < nodeNum; nodeId++)
		{
			sorteddDataNumberNeedSendForEachNode[nodeId].first = nodeId;
			sorteddDataNumberNeedSendForEachNode[nodeId].second =
				nodeSendSeq[nodeId].size();
		}
		sort(sorteddDataNumberNeedSendForEachNode.begin(),
			sorteddDataNumberNeedSendForEachNode.end(), compareSecondAscending);
	}

	return timeOverhead;
}

void Simulation::BaselineExecuteScale(int p1, vector<vector<BaseMdsUpdateParityCommand>>& updataParityTask,
	vector<vector<BaseMdsRelocationCommand>>& relocationTask)
{
	int ackNumNeedReceive = 0;
	int step = 5;
	for (int i = 0; i < p1; i++)
	{
		//if ((i != 0) && (i % step == 0))
		//{
		//	ReceiveAck(ackNumNeedReceive);
		//	ackNumNeedReceive = 0;
		//}
		for (int j = 0; j < updataParityTask[i].size(); j++)
		{
			BaseSendUpdateCommand(&updataParityTask[i][j]);
			ackNumNeedReceive++;
		}
		//StopHere();
	}

	for (int i = 0; i < p1; i++)
	{
		for (int j = 0; j < relocationTask[i].size(); j++)
		{
			BaseSendRelocationCommand(&relocationTask[i][j]);
			ackNumNeedReceive++;
		}
	}
	//ackNumNeedReceive = ackNumNeedReceive / 2;
	//ReceiveAck(ackNumNeedReceive);
}

double Simulation::baselineUpScalePerRound(int startStripeId, int endStripeId, int p1, int p2)
{
	double timeOverhead = 0.0;

	if (endStripeId > stripeNumber)
	{
		endStripeId = stripeNumber;
	}
	if ((endStripeId - startStripeId) % minKl != 0)
	{
		cout << "error in baselineDownScalePerRound" << endl;
		exit(0);
	}
	if (endStripeId <= startStripeId)
		return timeOverhead;

	int absoluteStripeId;
	int nodeId;
	int upScaleStripeNumber = endStripeId - startStripeId;
	int dataNodeId;
	int parityNodeId;
	int sendNodeId;
	int receiveNodeId;
	int extensionStripePointer;
	int destinationPort;
	int dataIndexInNewStripe;
	int chunkId;
	int kGap = KLarge - K;
	int index;
	int extensionStripeId;
	int ackNumNeedReceive = 0;
	string ip;
	vector<int> downloadBandwidthOccupiedForEachNode;
	vector<int> nodeUsed;
	vector<int> indexs;
	vector<vector<int>> destinationOfEachNode;
	vector<vector<int>> chunkIndexsAfterScale;
	vector<pair<int, int>> sorteddDataNumberNeedSendForEachNode;
	// for each extension stripe: KLarge-K dataChunkCommand, and M parityChunkCommand
	vector<vector<BaseMdsUpdateParityCommand>> updataParityTask;
	vector<vector<BaseMdsRelocationCommand>> relocationTask;

	destinationOfEachNode.resize(nodeNum);
	chunkIndexsAfterScale.resize(p1);
	nodeUsed.resize(nodeNum);
	updataParityTask.resize(p1);
	relocationTask.resize(p1);
	for (nodeId = 0; nodeId < nodeNum; nodeId++)
	{
		downloadBandwidthOccupiedForEachNode.push_back(0);
		indexs.push_back(nodeId);
	}

	for (int i = 0; i < p1; i++)
	{
		absoluteStripeId = startStripeId + i;
		for (int j = 0; j < K; j++)
		{
			dataNodeId = chunkIndexs[absoluteStripeId][j];
			chunkIndexsAfterScale[i].push_back(dataNodeId);
		}
	}

	for (int i = 0; i < p1; i++)
	{
		updataParityTask[i].resize(kGap + M);
	}

	extensionStripePointer = 0;
	for (int i = 0; i < nodeNum; i++)
	{
		nodeUsed[i] = 0;
	}
	for (int i = 0; i < N; i++)
	{
		nodeId = chunkIndexs[startStripeId + extensionStripePointer][i];
		nodeUsed[nodeId] = 1;
	}
	for (int i = p1; i < upScaleStripeNumber; i++)
	{
		absoluteStripeId = startStripeId + i;
		for (int j = 0; j < K; j++)
		{
			dataNodeId = chunkIndexs[absoluteStripeId][j];
			chunkId = absoluteStripeId * N + j;
			if (nodeUsed[dataNodeId] == 0)
			{
				dataIndexInNewStripe = chunkIndexsAfterScale[extensionStripePointer].size();
				index = dataIndexInNewStripe - K;
				nodeUsed[dataNodeId] = 1;
				chunkIndexsAfterScale[extensionStripePointer].push_back(dataNodeId);

				/* for update parity(DATA_CHUNK) */
				ip = proxyIP[dataNodeId];
				strcpy(updataParityTask[extensionStripePointer][index].nodeIP_ReceiveThisCommand, ip.c_str());
				updataParityTask[extensionStripePointer][index].port = PROXY_START_PORT + extensionStripePointer;
				updataParityTask[extensionStripePointer][index].role = DATA_CHUNK;
				updataParityTask[extensionStripePointer][index].storeIndex = LocateStoreIndex(dataNodeId, chunkId);
				updataParityTask[extensionStripePointer][index].dataIndexInNewStripe = dataIndexInNewStripe;
				extensionStripeId = startStripeId + extensionStripePointer;
				for (int t = 0; t < M; t++)
				{
					parityNodeId = chunkIndexs[extensionStripeId][K + t];
					if (parityNodeId == dataNodeId)
					{
						cout << "baselineUpScalePerRound: parityNodeId == dataNodeId in update parity(DATA_CHUNK)" << endl;
						exit(0);
					}
					ip = proxyIP[parityNodeId];
					strcpy(updataParityTask[extensionStripePointer][index].destinationIps[t], ip.c_str());

					// add receiveNum in Command for parityChunk
					updataParityTask[extensionStripePointer][kGap + t].receiveNum++;
				}

			}
			else
			{
				//solove relocation
				sendNodeId = dataNodeId;
				receiveNodeId = dataNodeId;
				random_shuffle(indexs.begin(), indexs.end());
				for (int t = 0; t < nodeNum; t++)
				{
					receiveNodeId = indexs[t];
					if (nodeUsed[receiveNodeId] == 0)
					{
						break;
					}
				}
				if (receiveNodeId == sendNodeId || nodeUsed[receiveNodeId] != 0)
				{
					cout << "error in baselineUpScalePerRound 1" << endl;
					cout << "Simulation stop" << endl;
					exit(0);
				}
				destinationOfEachNode[sendNodeId].push_back(receiveNodeId);
				nodeUsed[receiveNodeId] = 1;
				dataIndexInNewStripe = chunkIndexsAfterScale[extensionStripePointer].size();
				chunkIndexsAfterScale[extensionStripePointer].push_back(receiveNodeId);
				baselineRelocationNum++;

				ackNumNeedReceive += 1;

				// testbed
				/* for relocation */
				BaseMdsRelocationCommand tempRelocationTask_sender;
				BaseMdsRelocationCommand tempRelocationTask_receiver;
				// for relocation sender
				ip = proxyIP[sendNodeId];
				strcpy(tempRelocationTask_sender.nodeIP_ReceiveThisCommand, ip.c_str());
				tempRelocationTask_sender.port = RELOCATION_START_PORT + extensionStripePointer;
				tempRelocationTask_sender.role = RELOCATION_SENDER;
				tempRelocationTask_sender.dataStoreIndex = LocateStoreIndex(sendNodeId, chunkId);
				ip = proxyIP[receiveNodeId];
				strcpy(tempRelocationTask_sender.destinationIp, ip.c_str());
				// for relocation receiver
				strcpy(tempRelocationTask_receiver.nodeIP_ReceiveThisCommand, ip.c_str());
				tempRelocationTask_receiver.port = RELOCATION_START_PORT + extensionStripePointer;;
				tempRelocationTask_receiver.role = RELOCATION_RECEIVER;
				tempRelocationTask_receiver.receiveNum = 1;
				tempRelocationTask_receiver.writeIndex = chunkIndex_eachNode[receiveNodeId].size();
				// push task in relocationTask
				relocationTask[extensionStripePointer].push_back(tempRelocationTask_sender);
				relocationTask[extensionStripePointer].push_back(tempRelocationTask_receiver);

				/* for update parity */
				extensionStripeId = startStripeId + extensionStripePointer;
				index = dataIndexInNewStripe - K;
				// for update parity(DATA_CHUNK)
				ip = proxyIP[sendNodeId];
				strcpy(updataParityTask[extensionStripePointer][index].nodeIP_ReceiveThisCommand, ip.c_str());
				updataParityTask[extensionStripePointer][index].port = PROXY_START_PORT + extensionStripePointer;
				updataParityTask[extensionStripePointer][index].role = DATA_CHUNK;
				updataParityTask[extensionStripePointer][index].storeIndex = LocateStoreIndex(sendNodeId, chunkId);
				updataParityTask[extensionStripePointer][index].dataIndexInNewStripe = dataIndexInNewStripe;
				extensionStripeId = startStripeId + extensionStripePointer;
				for (int t = 0; t < M; t++)
				{
					parityNodeId = chunkIndexs[extensionStripeId][K + t];
					if (parityNodeId == sendNodeId)
					{
						// if this chunk is in a parityNode
						memset(updataParityTask[extensionStripePointer][index].destinationIps[t], 0, IP_LENGTH);
						int tempParityChunkId = extensionStripeId * N + K + t;
						updataParityTask[extensionStripePointer][index].parityStoreIndex = LocateStoreIndex(sendNodeId, tempParityChunkId);
					}
					else
					{
						// if this chunk is not in a parityNode
						ip = proxyIP[parityNodeId];
						strcpy(updataParityTask[extensionStripePointer][index].destinationIps[t], ip.c_str());

						// add receiveNum in Command for parityChunk
						updataParityTask[extensionStripePointer][kGap + t].receiveNum++;
					}

				}
			}
			if (chunkIndexsAfterScale[extensionStripePointer].size() == KLarge)
			{
				for (int t = 0; t < nodeNum; t++)
				{
					nodeUsed[t] = 0;
				}
				extensionStripePointer++;
				for (int t = 0; t < N; t++)
				{
					nodeId = chunkIndexs[startStripeId + extensionStripePointer][t];
					nodeUsed[nodeId] = 1;
				}
			}
		}
	}

	/* fill parity node */
	for (int i = 0; i < p1; i++)
	{
		absoluteStripeId = startStripeId + i;
		for (int j = 0; j < M; j++)
		{
			parityNodeId = chunkIndexs[absoluteStripeId][K + j];
			chunkIndexsAfterScale[i].push_back(parityNodeId);
		}

		// testbed
		/* for update parity: for PARITY_CHUNK */
		for (int j = 0; j < M; j++)
		{
			index = kGap + j;
			parityNodeId = chunkIndexs[absoluteStripeId][K + j];
			ip = proxyIP[parityNodeId];
			strcpy(updataParityTask[i][kGap + j].nodeIP_ReceiveThisCommand, ip.c_str());
			updataParityTask[i][kGap + j].port = PROXY_START_PORT + i;
			updataParityTask[i][kGap + j].role = PARITY_CHUNK;
			chunkId = absoluteStripeId * N + K + j;
			updataParityTask[i][kGap + j].storeIndex = LocateStoreIndex(parityNodeId, chunkId);
			updataParityTask[i][kGap + j].parityIndexInNewStripe = j;

		}
	}
	for (int i = 0; i < p1; i++)
	{
		for (int j = K; j < KLarge; j++)
		{
			sendNodeId = chunkIndexsAfterScale[i][j];
			for (int t = 0; t < M; t++)
			{
				receiveNodeId = chunkIndexsAfterScale[i][KLarge + t];
				if (receiveNodeId == sendNodeId)
				{
					cout << "error in baselineUpScalePerRound 2" << endl;
					cout << "Simulation stop" << endl;
					exit(0);
				}
				destinationOfEachNode[sendNodeId].push_back(receiveNodeId);
			}
		}
	}

	for (int i = 0; i < p1; i++)
	{
		newChunkIndexs.push_back(chunkIndexsAfterScale[i]);
	}

	for (nodeId = 0; nodeId < nodeNum; nodeId++)
	{
		reverse(destinationOfEachNode[nodeId].begin(),
			destinationOfEachNode[nodeId].end());
		baselineTraffic += destinationOfEachNode[nodeId].size();
		//sorteddDataNumberNeedSendForEachNode.push_back(
		//	make_pair(nodeId, destinationOfEachNode[nodeId].size()));
		reverse(destinationOfEachNode[nodeId].begin(),
			destinationOfEachNode[nodeId].end());
		nodeSendSeq[nodeId] = destinationOfEachNode[nodeId];
	}
	//sort(sorteddDataNumberNeedSendForEachNode.begin(),
	//	sorteddDataNumberNeedSendForEachNode.end(), compareSecondAscending);
	timeOverhead += noSchedulingTransmission();

	ackNumNeedReceive += (kGap + M) * p1;
	BaselineExecuteScale(p1, updataParityTask, relocationTask);

	ReceiveAck(ackNumNeedReceive);


	return timeOverhead;
}

double Simulation::baselineUpScale(int KLarge)
{
	cout << this->K << " to " << KLarge << endl;
	this->KLarge = KLarge;
	this->minKl = KLarge / gcd(K, KLarge);
	this->stripeNumber = initialStripeNum - (initialStripeNum % minKl);
	this->scaleNumPerRound = downScaleStripePerRound;
	this->baselineTraffic = 0;
	this->baselineRelocationNum = 0;
	while (this->scaleNumPerRound % minKl != 0)
	{
		this->scaleNumPerRound--;
	}
	baselineScaleNumberPerRound = scaleNumPerRound;
	baselineStripeNumber = stripeNumber;
	newChunkIndexs.clear();
	double timeOverhead = 0.0;
	int minK = K / gcd(K, KLarge);
	int p1 = scaleNumPerRound * minK / minKl;
	int p2 = scaleNumPerRound - p1;
	if (p1 % minK != 0 || p2 % (minKl - minK) != 0)
	{
		cout << "error in ideaUpScale 1" << endl;
		cout << "Simulation stop" << endl;
		exit(0);
	}
	for (int i = 0; i < stripeNumber; i += scaleNumPerRound)
	{
		if (i + scaleNumPerRound > stripeNumber)
		{
			int remainStripeNum = stripeNumber - i;
			p1 = remainStripeNum * minK / minKl;
			p2 = remainStripeNum - p1;
			if (p1 % minK != 0 || p2 % (minKl - minK) != 0 || p2 == 0)
			{
				cout << "error in baselineUpScale 1" << endl;
				cout << "Simulation stop" << endl;
				exit(0);
			}
		}
		timeOverhead += baselineUpScalePerRound(i, i + scaleNumPerRound, p1, p2);
	}
	chunkIndexs = newChunkIndexs;
	initialStripeNum = chunkIndexs.size();
	K = KLarge;
	N = K + M;
	return timeOverhead;
}

void Baseline_MDS(vector<int>& kPool)
{
	if (kPool.size() == 0)
	{
		cout << "no k in kPool" << endl;
	}

	iK = kPool[0];
	Simulation baseline;

	struct timeval t_start;
	struct timeval t_end;
	double total_time = 0.0;

	double timeOverhead = 0.0;
	int kl = iK;

	for (int i = 1; i < kPool.size(); i++)
	{
		total_time = 0.0;
		gettimeofday(&t_start, NULL);

		baseline.GetChunkStoreOrderInEachNode();

		// multiScale
		cout << "----------------------------" << endl;
		cout << baseline.Get_K() << " -> " << kPool[i] << ": " << endl;
		kl = kPool[i];
		cout << "initialStripeNum: " << baseline.Get_InitialStripeNum() << endl;

		timeOverhead = baseline.baselineUpScale(kl);

		cout << "baseline time overhead: " << timeOverhead << endl;
		cout << "unbalance ratio: " << baseline.GetUnbalanceRatio() << endl;
		baseline.printTraffic("baseline");

		gettimeofday(&t_end, NULL);
		total_time = total_time + (t_end.tv_sec - t_start.tv_sec) * 1000000 + (t_end.tv_usec - t_start.tv_usec);
		total_time = total_time / 1000000;
		cout << "time: " << total_time << " s" << endl;

		cout << "----------------------------" << endl;


	}
}

int Simulation::getTwoDataIndex(vector<vector<int>>& smallStripeDataIndex,
	vector<vector<int>>& largeStripeDataIndex, int KLarge)
{
	int groupTraffic = 0;
	int largerK = KLarge;
	int smallerK = K;
	int minLargerK = largerK / gcd(smallerK, largerK);
	int minSmallerK = smallerK / gcd(smallerK, largerK);
	int lcm = largerK * smallerK / gcd(smallerK, largerK);
	int a = (largerK / smallerK) * (lcm / largerK); // largerK / smallerK, floor(rounding down)
	int b = lcm / smallerK - a;
	int r = largerK % smallerK;
	int startNodeId;
	int countSmallerK;
	int tempInt;
	int smallerStripePoint;
	int largerstripePoint;

	vector<int> relativeDataNodeSendNum;

	// testbed
	int dataNum = (largerK / smallerK) * smallerK;
	int nodeId;
	int remainChunkInLarge;

	relativeDataNodeSendNum.resize(largerK);

	for (int i = 0; i < largerK; i++)
	{
		relativeDataNodeSendNum[i] = 0;
	}

	tempInt = 0;//tempInt => dataIndex
	smallerStripePoint = 0;

	for (int i = 0; i < minSmallerK; i++)
	{
		if (i < b)
		{
			startNodeId = (i * (smallerK - r)) % largerK;
		}
		else
		{
			startNodeId = ((i - b + 1) * r) % largerK;
		}
		countSmallerK = 0;

		for (int j = 0; j < dataNum; j++)
		{
			largeStripeDataIndex[i][(startNodeId + j) % largerK] = tempInt;
			smallStripeDataIndex[smallerStripePoint][(startNodeId + j) % largerK] = tempInt;
			tempInt++;
			countSmallerK++;
			if (countSmallerK == smallerK)
			{
				countSmallerK = 0;
				smallerStripePoint++;
			}
		}
	}

	if (smallerStripePoint != a)
	{
		cout << "error in getTwoDataIndex 1" << endl;
		cout << "Simulation Stop" << endl;
		exit(0);
	}
	largerstripePoint = b;
	remainChunkInLarge = r;
	for (int i = 0; i < b; i++)
	{
		smallerStripePoint = a + i;
		countSmallerK = 0;

		//testbed
		startNodeId = (i * (smallerK - r)) % largerK;
		startNodeId = (startNodeId + dataNum) % largerK;

		for (int j = 0; j < r; j++)
		{
			nodeId = (startNodeId + j) % largerK;
			if (largeStripeDataIndex[i][nodeId] == -1)
			{
				largeStripeDataIndex[i][nodeId] = tempInt;
				smallStripeDataIndex[smallerStripePoint][nodeId] = tempInt;
				tempInt++;
				countSmallerK++;
			}
			else
			{
				cout << "error in getTwoIndex get StripeDataIndex 1" << endl;
				exit(0);
			}
		}

		while (countSmallerK < smallerK)
		{
			for (int j = 0; j < smallerK - r; j++)
			{
				nodeId = (nodeId + 1) % largerK;
				if (largeStripeDataIndex[largerstripePoint][nodeId] == -1)
				{
					largeStripeDataIndex[largerstripePoint][nodeId] = tempInt;
					smallStripeDataIndex[smallerStripePoint][nodeId] = tempInt;
					relativeDataNodeSendNum[nodeId] += M;
					groupTraffic += M;

					tempInt++;
					countSmallerK++;

					remainChunkInLarge--;
					if (remainChunkInLarge == 0)
					{
						largerstripePoint++;
						remainChunkInLarge = r;
					}
				}
				else
				{
					cout << "error in ers get StripeDataIndex 2" << endl;
					exit(0);
				}
			}
		}
	}

	return groupTraffic;
}

int Simulation::ERSGetTargetPlacement(int KLarge, vector<vector<int>>& newChunkIndex_beforeScale)
{
	int largerK = KLarge;
	int smallerK = K;
	int minLargerK = largerK / gcd(smallerK, largerK);
	int minSmallerK = smallerK / gcd(smallerK, largerK);
	int NLarge = largerK + M;
	int traffic = 0;
	int groupTraffic = 0;

	//place chunk
	int startNodeId;
	int stripeNum_afterScale = initialStripeNum * K / largerK;
	int chunkId;
	int parityNodeId;

	vector<vector<int>> smallStripeDataIndex;
	vector<vector<int>> largeStripeDataIndex;
	//place chunk
	vector<int> tempChunkIndex;
	vector<vector<int>> tempNewChunkIndex;

	smallStripeDataIndex.resize(minLargerK);
	largeStripeDataIndex.resize(minSmallerK);

	tempChunkIndex.resize(NLarge);
	tempNewChunkIndex.resize(minLargerK);

	if (NLarge > nodeNum)
	{
		cout << "error in ERSFirstScale" << endl;
		cout << "Simulation Stop" << endl;
		exit(0);
	}


	for (int i = 0; i < minLargerK; i++)
	{
		smallStripeDataIndex[i].resize(largerK);
		tempNewChunkIndex[i].resize(K + M);
		for (int j = 0; j < largerK; j++)
		{
			smallStripeDataIndex[i][j] = -1;
		}
	}
	for (int i = 0; i < minSmallerK; i++)
	{
		largeStripeDataIndex[i].resize(largerK);
		for (int j = 0; j < largerK; j++)
		{
			largeStripeDataIndex[i][j] = -1;
		}
	}

	groupTraffic = getTwoDataIndex(smallStripeDataIndex, largeStripeDataIndex, KLarge);

	
	startNodeId = 0;
	for (int i = 0; (i + minSmallerK) <= stripeNum_afterScale; i += minSmallerK)
	{
		for (int j = 0; j < NLarge; j++)
		{
			tempChunkIndex[j] = (startNodeId + j) % nodeNum;
		}

		for (int j = 0; j < minSmallerK; j++)
		{
			newChunkIndexs.push_back(tempChunkIndex);
		}

		//place newChunkIndex_beforeScale
		//initial
		for (int j = 0; j < minLargerK; j++)
		{
			for (int t = 0; t < K + M; t++)
			{
				tempNewChunkIndex[j][t] = -1;
			}
		}

		//place data chunk
		for (int j = 0; j < minLargerK; j++)
		{
			for (int t = 0; t < KLarge; t++)
			{
				if (smallStripeDataIndex[j][t] == -1)
					continue;
				else
				{
					chunkId = smallStripeDataIndex[j][t];
					if (tempNewChunkIndex[chunkId / K][chunkId % K] != -1)
					{
						cout << "error: tempNewChunkIndex" << endl;
						exit(0);
					}
					else
					{
						tempNewChunkIndex[chunkId / K][chunkId % K] = (startNodeId + t) % nodeNum;
					}
				}
			}
		}
		//place parity chunk
		for (int t = 0; t < M; t++)
		{
			parityNodeId = (startNodeId + largerK + t) % nodeNum;
			for (int j = 0; j < minLargerK; j++)
			{
				tempNewChunkIndex[j][K + t] = parityNodeId;
			}
		}
		newChunkIndex_beforeScale.insert(newChunkIndex_beforeScale.end(),
			tempNewChunkIndex.begin(), tempNewChunkIndex.end());

		startNodeId = (startNodeId + 1) % nodeNum;
		traffic += groupTraffic;
	}

	return traffic;
}

int Simulation::ERSPlacement(int KTarget, int scaleTime)
{
	int traffic = 0;

	// testbed
	int totalPlaceMentStripeNum;
	int placementNumPerRound;
	int indexInPerRound;
	int receiveNum;
	int newNodeId;
	int oldNodeId;
	int chunkId;
	int parityNodeId;
	int ackNumNeedReceive;
	int writeIndex;
	string ip;

	vector<vector<ERSReupdateParityCommand>> reupdateCommandPerRound;
	vector<ERSReupdateParityCommand> reupdateAStripe;

	vector<vector<ERSPlaceMentCommand>> placementCommandPerRound;
	vector<ERSPlaceMentCommand> placementAStripe;


	vector<vector<int>> newChunkIndex_beforeScale;

	traffic = ERSGetTargetPlacement(KTarget, newChunkIndex_beforeScale);
	traffic = 0;



	if (scaleTime == 0)
	{
		traffic = 0;
		chunkIndexs.clear();
		chunkIndexs = newChunkIndex_beforeScale;

		initialStripeNum = chunkIndexs.size();
		return traffic;
	}

	//for (int i = 0; i < newChunkIndex_beforeScale.size(); i++)
	//{
	//	//compare data chunk

	//	for (int j = 0; j < K; j++)
	//	{
	//		if (chunkIndexs[i][j] != newChunkIndex_beforeScale[i][j])
	//		{
	//			traffic += 1; // move data chunk
	//			traffic += M; // recaculate parity
	//		}
	//	}
	//	for (int j = 0; j < M; j++)
	//	{
	//		if (chunkIndexs[i][K + j] != newChunkIndex_beforeScale[i][K + j])
	//		{
	//			traffic += 1; // move parity chunk
	//		}
	//	}
	//}

	// testbed
	GetChunkStoreOrderInEachNode();
	totalPlaceMentStripeNum = newChunkIndex_beforeScale.size();
	placementNumPerRound = ERS_PLACEMENT_PER_Round;
	/* for Reupdate */
	indexInPerRound = 0;
	ackNumNeedReceive = 0;

	vector<int> oldIndex;
	vector<int> newIndex;
	vector<int> intersectionIndex;
	vector<int> differentInOld;
	vector<int> differentInNew;

	for (int i = 0; i < newChunkIndex_beforeScale.size(); i++)
	{
		reupdateAStripe.clear();
		receiveNum = 0; // receive dataChunkNum for parityChunk
		oldIndex.clear();
		newIndex.clear();
		intersectionIndex.clear();
		differentInOld.clear();
		differentInNew.clear();

		//compare data chunk
		for (int j = 0; j < K; j++)
		{
			//if (chunkIndexs[i][j] != newChunkIndex_beforeScale[i][j])
			//{
			//	traffic += 1; // move data chunk
			//	traffic += M; // recaculate parity
			//}
			oldIndex.push_back(chunkIndexs[i][j]);
			newIndex.push_back(newChunkIndex_beforeScale[i][j]);

		}
		sort(oldIndex.begin(), oldIndex.end());
		sort(newIndex.begin(), newIndex.end());
		set_intersection(oldIndex.begin(),
			oldIndex.end(),
			newIndex.begin(),
			newIndex.end(),
			inserter(intersectionIndex, intersectionIndex.begin()));
		set_difference(oldIndex.begin(),
			oldIndex.end(),
			intersectionIndex.begin(),
			intersectionIndex.end(),
			inserter(differentInOld, differentInOld.begin()));
		set_difference(newIndex.begin(),
			newIndex.end(),
			intersectionIndex.begin(),
			intersectionIndex.end(),
			inserter(differentInNew, differentInNew.begin()));

		if (differentInOld.size() != differentInNew.size())
		{
			cout << "ERSPlacement: differentInOld.size() != differentInNew.size() in dataChunk" << endl;
			exit(0);
		}

		// for data
		for (int j = 0; j < differentInOld.size(); j++)
		{
			traffic += M; // recaculate parity
			receiveNum += 1;
			// fill command for dataChunk
			oldNodeId = differentInOld[j];
			newNodeId = differentInNew[j];
			ERSReupdateParityCommand tempReCommand;
			ip = proxyIP[oldNodeId];
			strcpy(tempReCommand.nodeIP_ReceiveThisCommand, ip.c_str());
			tempReCommand.port = RELOCATION_START_PORT + indexInPerRound;
			tempReCommand.role = ERS_REUPDATE_DATA;
			tempReCommand.newNodeId = newNodeId;
			tempReCommand.oldNodeId = oldNodeId;
			chunkId = -1;
			for (int t = 0; t < K; t++)
			{
				if (chunkIndexs[i][t] == oldNodeId)
				{
					chunkId = i * N + t;
					break;
				}
			}
			if (chunkId == -1)
			{
				cout << "ERSPlacement: Reupdate not find dataChunk" << endl;
				exit(0);
			}
			tempReCommand.dataStoreIndex = LocateStoreIndex(oldNodeId, chunkId);
			for (int t = 0; t < M; t++)
			{
				parityNodeId = chunkIndexs[i][K + t];
				ip = proxyIP[parityNodeId];
				strcpy(tempReCommand.destinationIps[t], ip.c_str());
			}

			// push in AStripe
			reupdateAStripe.push_back(tempReCommand);

		}

		// for parity
		if (receiveNum > 0)
		{
			for (int j = 0; j < M; j++)
			{
				ERSReupdateParityCommand tempReCommand;
				parityNodeId = chunkIndexs[i][K + j];
				ip = proxyIP[parityNodeId];
				strcpy(tempReCommand.nodeIP_ReceiveThisCommand, ip.c_str());
				tempReCommand.port = RELOCATION_START_PORT + indexInPerRound;
				tempReCommand.role = ERS_REUPDATE_PARITY;
				tempReCommand.receiveNum = receiveNum;
				chunkId = i * N + K + j;
				tempReCommand.parityStoreIndex = LocateStoreIndex(parityNodeId, chunkId);

				// push in AStripe
				reupdateAStripe.push_back(tempReCommand);

				ackNumNeedReceive++;
			}
		}


		// push reupdateAStripe in reupdateCommandPerRound
		reupdateCommandPerRound.push_back(reupdateAStripe);

		indexInPerRound++;
		if (indexInPerRound == placementNumPerRound)
		{
			// send command
			for (int j = 0; j < reupdateCommandPerRound.size(); j++)
			{
				for (int t = 0; t < reupdateCommandPerRound[j].size(); t++)
				{
					ERSSendReupdateCommand(&reupdateCommandPerRound[j][t]);
				}
				//if (reupdateCommandPerRound[j].size() > 0)
				//	StopHere();
			}
			// wait ack
			ReceiveAck(ackNumNeedReceive);

			indexInPerRound = 0;
			ackNumNeedReceive = 0;
			reupdateCommandPerRound.clear();
		}
	}
	if (indexInPerRound != 0)
	{
		// send command
		for (int j = 0; j < reupdateCommandPerRound.size(); j++)
		{
			for (int t = 0; t < reupdateCommandPerRound[j].size(); t++)
			{
				ERSSendReupdateCommand(&reupdateCommandPerRound[j][t]);
			}
		}
		// wait ack
		ReceiveAck(ackNumNeedReceive);

		indexInPerRound = 0;
		ackNumNeedReceive = 0;
		reupdateCommandPerRound.clear();
	}


	/* for placement */
	indexInPerRound = 0;
	ackNumNeedReceive = 0;
	for (int i = 0; i < newChunkIndex_beforeScale.size(); i++)
	{
		placementAStripe.clear();

		oldIndex.clear();
		newIndex.clear();
		intersectionIndex.clear();
		differentInOld.clear();
		differentInNew.clear();
		//compare data chunk
		for (int j = 0; j < K; j++)
		{
			//if (chunkIndexs[i][j] != newChunkIndex_beforeScale[i][j])
			//{
			//	traffic += 1; // move data chunk
			//	traffic += M; // recaculate parity
			//}
			oldIndex.push_back(chunkIndexs[i][j]);
			newIndex.push_back(newChunkIndex_beforeScale[i][j]);

		}
		sort(oldIndex.begin(), oldIndex.end());
		sort(newIndex.begin(), newIndex.end());
		set_intersection(oldIndex.begin(),
			oldIndex.end(),
			newIndex.begin(),
			newIndex.end(),
			inserter(intersectionIndex, intersectionIndex.begin()));
		set_difference(oldIndex.begin(),
			oldIndex.end(),
			intersectionIndex.begin(),
			intersectionIndex.end(),
			inserter(differentInOld, differentInOld.begin()));
		set_difference(newIndex.begin(),
			newIndex.end(),
			intersectionIndex.begin(),
			intersectionIndex.end(),
			inserter(differentInNew, differentInNew.begin()));

		if (differentInOld.size() != differentInNew.size())
		{
			cout << "ERSPlacement: differentInOld.size() != differentInNew.size() in dataChunk" << endl;
			exit(0);
		}

		//compare data chunk
		for (int j = 0; j < differentInOld.size(); j++)
		{
			traffic += 1; // move data chunk
			ERSPlaceMentCommand tempSender;
			ERSPlaceMentCommand tempReceiver;

			// for tempSender
			oldNodeId = differentInOld[j];
			newNodeId = differentInNew[j];
			ip = proxyIP[oldNodeId];
			strcpy(tempSender.nodeIP_ReceiveThisCommand, ip.c_str());
			tempSender.port = RELOCATION_START_PORT + indexInPerRound;
			tempSender.role = ERS_PLACEMENT_SENDER;
			chunkId = -1;
			for (int t = 0; t < K; t++)
			{
				if (chunkIndexs[i][t] == oldNodeId)
				{
					chunkId = i * N + t;
					break;
				}
			}
			if (chunkId == -1)
			{
				cout << "ERSPlacement: Placement not find dataChunk" << endl;
				exit(0);
			}
			tempSender.storeIndex = LocateStoreIndex(oldNodeId, chunkId);
			ip = proxyIP[newNodeId];
			strcpy(tempSender.destinationIp, ip.c_str());

			// for tempReceiver
			ip = proxyIP[newNodeId];
			strcpy(tempReceiver.nodeIP_ReceiveThisCommand, ip.c_str());
			tempReceiver.port = RELOCATION_START_PORT + indexInPerRound;
			tempReceiver.role = ERS_PLACEMENT_RECEIVER;
			writeIndex = chunkIndex_eachNode[newNodeId].size();
			tempReceiver.writeIndex = writeIndex;
			tempReceiver.receiveNum = 1;

			// push tempSender and tempReceiver in placementAStripe
			placementAStripe.push_back(tempSender);
			placementAStripe.push_back(tempReceiver);

			ackNumNeedReceive++;
		}

		oldIndex.clear();
		newIndex.clear();
		intersectionIndex.clear();
		differentInOld.clear();
		differentInNew.clear();
		for (int j = 0; j < M; j++)
		{
			//if (chunkIndexs[i][K + j] != newChunkIndex_beforeScale[i][K + j])
			//{
			//	traffic += 1; // move parity chunk
			//}
			oldIndex.push_back(chunkIndexs[i][K + j]);
			newIndex.push_back(newChunkIndex_beforeScale[i][K + j]);
		}
		sort(oldIndex.begin(), oldIndex.end());
		sort(newIndex.begin(), newIndex.end());
		set_intersection(oldIndex.begin(),
			oldIndex.end(),
			newIndex.begin(),
			newIndex.end(),
			inserter(intersectionIndex, intersectionIndex.begin()));
		set_difference(oldIndex.begin(),
			oldIndex.end(),
			intersectionIndex.begin(),
			intersectionIndex.end(),
			inserter(differentInOld, differentInOld.begin()));
		set_difference(newIndex.begin(),
			newIndex.end(),
			intersectionIndex.begin(),
			intersectionIndex.end(),
			inserter(differentInNew, differentInNew.begin()));
		if (differentInOld.size() != differentInNew.size())
		{
			cout << "ERSPlacement: differentInOld.size() != differentInNew.size() in parityChunk" << endl;
			exit(0);
		}

		for (int j = 0; j < differentInOld.size(); j++)
		{

			traffic += 1; // move parity chunk

			ERSPlaceMentCommand tempSender;
			ERSPlaceMentCommand tempReceiver;

			// for tempSender
			oldNodeId = differentInOld[j];
			newNodeId = differentInNew[j];
			ip = proxyIP[oldNodeId];
			strcpy(tempSender.nodeIP_ReceiveThisCommand, ip.c_str());
			tempSender.port = RELOCATION_START_PORT + indexInPerRound;
			tempSender.role = ERS_PLACEMENT_SENDER;
			chunkId = -1;
			for (int t = 0; t < M; t++)
			{
				if (chunkIndexs[i][K + t] == oldNodeId)
				{
					chunkId = i * N + K + t;
					break;
				}
			}
			if (chunkId == -1)
			{
				cout << "ERSPlacement: Placement not find parityChunk" << endl;
				exit(0);
			}
			tempSender.storeIndex = LocateStoreIndex(oldNodeId, chunkId);
			ip = proxyIP[newNodeId];
			strcpy(tempSender.destinationIp, ip.c_str());

			// for tempReceiver
			ip = proxyIP[newNodeId];
			strcpy(tempReceiver.nodeIP_ReceiveThisCommand, ip.c_str());
			tempReceiver.port = RELOCATION_START_PORT + indexInPerRound;
			tempReceiver.role = ERS_PLACEMENT_RECEIVER;
			writeIndex = chunkIndex_eachNode[newNodeId].size();
			tempReceiver.writeIndex = writeIndex;
			tempReceiver.receiveNum = 1;

			// push tempSender and tempReceiver in placementAStripe
			placementAStripe.push_back(tempSender);
			placementAStripe.push_back(tempReceiver);

			ackNumNeedReceive++;

		}

		// push placementAStripe in placementCommandPerRound
		placementCommandPerRound.push_back(placementAStripe);


		indexInPerRound++;
		if (indexInPerRound == placementNumPerRound)
		{
			// send command
			for (int j = 0; j < placementCommandPerRound.size(); j++)
			{
				for (int t = 0; t < placementCommandPerRound[j].size(); t++)
				{
					ERSSendPlacementCommand(&placementCommandPerRound[j][t]);
				}

			}
			// wait ack
			ReceiveAck(ackNumNeedReceive);

			indexInPerRound = 0;
			ackNumNeedReceive = 0;
			placementCommandPerRound.clear();
		}
	}
	if (indexInPerRound != 0)
	{
		// send command
		for (int j = 0; j < placementCommandPerRound.size(); j++)
		{
			for (int t = 0; t < placementCommandPerRound[j].size(); t++)
			{
				ERSSendPlacementCommand(&placementCommandPerRound[j][t]);
			}
		}
		// wait ack
		ReceiveAck(ackNumNeedReceive);

		indexInPerRound = 0;
		ackNumNeedReceive = 0;
		placementCommandPerRound.clear();
	}


	// 4. process scale(K -> K+delta): have done in 2.
	//5. update something
	chunkIndexs.clear();
	chunkIndexs = newChunkIndex_beforeScale;
	//stripeNum = chunkIndexs.size();// 3750-?
	initialStripeNum = chunkIndexs.size();
	//K += deltaK;

	if (scaleTime == 0)
	{
		traffic = 0;
	}

	return traffic;
}

double Simulation::ERSUpScale(int KTarget)
{
	this->KLarge = KTarget;
	this->minKl = KLarge / gcd(K, KLarge);
	this->minK = K / gcd(K, KLarge);
	this->ERSTraffic = 0;
	int largerK = KTarget;
	int smallerK = K;
	int minLargerK = largerK / gcd(smallerK, largerK);
	int minSmallerK = smallerK / gcd(smallerK, largerK);
	int lcm = largerK * smallerK / gcd(smallerK, largerK);
	int rotation;
	int s = largerK;
	int NLarge = s + M;
	int parityStartIndex;
	int dataStartIndex;
	int sendNodeId;
	int receiveNodeId;
	int tempInt;
	int a = (largerK / smallerK) * (lcm / largerK);
	int b = lcm / smallerK - a;
	int r = largerK % smallerK;
	int countSmallerK;
	int smallerStripePoint;
	int largerstripePoint;
	int startNodeId;

	double timeOverhead = 0.0;

	vector<int> allNodeId;
	vector<int> selectNodeId;
	vector<int> dataIndex;
	vector<int> parityIndex;
	vector<int> relativeDataNodeSendNum;
	vector<int> tempChunkIndex;
	vector<vector<int>> smallStripeDataIndex;
	vector<vector<int>> largeStripeDataIndex;

	// testbed
	int dataNum = (largerK / smallerK) * smallerK;
	int nodeId;
	int remainChunkInLarge;

	// testbed
	int groupId;
	int receiveDataChunkNum;
	int absoluteStripeId;
	int relativeStripeId;
	int relativeChunkId; // relative
	int relativeNodeId;
	int chunkId;
	int parityNodeId;
	int startStripeId;
	int dataIndexIn_chunkIndex;
	int relativePostStripeId;
	int storeIndex;
	int ackNumNeedReceive;
	string ip;
	vector<vector<ERSMdsDataCommand>> dataTasksTotal; // for groups execute together
	vector<vector<ERSMdsParityCommand>> parityTasksTotal; // for groups execute together


	vector<vector<int>> requiredParityEachPostStripe; // store the parity's relative stripeId
	vector<vector<int>> xoredDataIndexEachPostStripe; // relative data indexS

	allNodeId.resize(nodeNum);
	selectNodeId.resize(NLarge);
	dataIndex.resize(largerK);
	parityIndex.resize(M);
	smallStripeDataIndex.resize(minLargerK);
	largeStripeDataIndex.resize(minSmallerK);
	relativeDataNodeSendNum.resize(largerK);
	tempChunkIndex.resize(largerK + M);
	newChunkIndexs.clear();

	requiredParityEachPostStripe.resize(minSmallerK);
	xoredDataIndexEachPostStripe.resize(minSmallerK);

	if (NLarge > nodeNum)
	{
		cout << "error in ERSUpScale" << endl;
		cout << "Simulation Stop" << endl;
		exit(0);
	}

	for (int i = 0; i < largerK; i++)
	{
		relativeDataNodeSendNum[i] = 0;
	}

	for (int i = 0; i < minLargerK; i++)
	{
		smallStripeDataIndex[i].resize(largerK);
		for (int j = 0; j < largerK; j++)
		{
			smallStripeDataIndex[i][j] = -1;
		}
	}
	for (int i = 0; i < minSmallerK; i++)
	{
		largeStripeDataIndex[i].resize(largerK);
		for (int j = 0; j < largerK; j++)
		{
			largeStripeDataIndex[i][j] = -1;
		}
	}

	tempInt = 0;//tempInt => dataIndex
	smallerStripePoint = 0;
	for (int i = 0; i < minSmallerK; i++)
	{
		if (i < b)
		{
			startNodeId = (i * (smallerK - r)) % largerK;
		}
		else
		{
			startNodeId = ((i - b + 1) * r) % largerK;
		}
		countSmallerK = 0;

		for (int j = 0; j < dataNum; j++)
		{
			largeStripeDataIndex[i][(startNodeId + j) % largerK] = tempInt;
			smallStripeDataIndex[smallerStripePoint][(startNodeId + j) % largerK] = tempInt;
			tempInt++;
			countSmallerK++;
			if (countSmallerK == smallerK)
			{
				// testbed
				requiredParityEachPostStripe[i].push_back(smallerStripePoint);

				countSmallerK = 0;
				smallerStripePoint++;
			}
		}
	}

	if (smallerStripePoint != a)
	{
		cout << "error in ERSUpScale 1" << endl;
		cout << "Simulation Stop" << endl;
		exit(0);
	}
	largerstripePoint = b;
	remainChunkInLarge = r;
	for (int i = 0; i < b; i++)
	{
		smallerStripePoint = a + i;

		countSmallerK = 0;

		//testbed
		startNodeId = (i * (smallerK - r)) % largerK;
		startNodeId = (startNodeId + dataNum) % largerK;

		for (int j = 0; j < r; j++)
		{
			nodeId = (startNodeId + j) % largerK;
			if (largeStripeDataIndex[i][nodeId] == -1)
			{
				largeStripeDataIndex[i][nodeId] = tempInt;
				smallStripeDataIndex[smallerStripePoint][nodeId] = tempInt;
				tempInt++;
				countSmallerK++;
			}
			else
			{
				cout << "error in ers get StripeDataIndex 1" << endl;
				exit(0);
			}
		}
		requiredParityEachPostStripe[i].push_back(smallerStripePoint);
		while (countSmallerK < smallerK)
		{
			for (int j = 0; j < smallerK - r; j++)
			{
				nodeId = (nodeId + 1) % largerK;
				if (largeStripeDataIndex[largerstripePoint][nodeId] == -1)
				{
					largeStripeDataIndex[largerstripePoint][nodeId] = tempInt;
					smallStripeDataIndex[smallerStripePoint][nodeId] = tempInt;
					relativeDataNodeSendNum[nodeId] += M;

					// tesbed
					xoredDataIndexEachPostStripe[i].push_back(tempInt);
					xoredDataIndexEachPostStripe[largerstripePoint].push_back(tempInt);

					tempInt++;
					countSmallerK++;

					remainChunkInLarge--;
					if (remainChunkInLarge == 0)
					{
						largerstripePoint++;
						remainChunkInLarge = r;
					}
				}
				else
				{
					cout << "error in ers get StripeDataIndex 2" << endl;
					exit(0);
				}
			}
		}
	}

	receiveDataChunkNum = 0;
	for (int i = 0; i < b; i++)
	{
		receiveDataChunkNum += xoredDataIndexEachPostStripe[i].size();
	}

	for (int i = 0; i < nodeNum; i++)
	{
		allNodeId[i] = i;
		nodeSendSeq[i].clear();
	}
	stripeNumber = 0;
	ERSScaleNumberPerRound = 0;
	while (ERSScaleNumberPerRound + minLargerK <= downScaleStripePerRound)
	{
		ERSScaleNumberPerRound += minLargerK;
	}
	startNodeId = 0;

	// testbed
	if (minSmallerK > ERS_MAX_POST)
	{
		cout << "ERSUpscale: minSmallerK > ERS_MAX_POST" << endl;
		exit(0);
	}
	for (int i = 0; i < minSmallerK; i++)
	{
		if (requiredParityEachPostStripe[i].size() > ERS_MAX_PARITY)
		{
			cout << "ERSUpscale: requiredParityEachPostStripe[i].size() > ERS_MAX_PARITY" << endl;
			exit(0);
		}
		if (xoredDataIndexEachPostStripe[i].size() > ERS_MAX_DATA)
		{
			cout << "ERSUpscale: xoredDataIndexEachPostStripe[i].size() > ERS_MAX_DATA" << endl;
			exit(0);
		}
	}

	groupId = 0;
	startStripeId = -minLargerK;
	while (stripeNumber + minLargerK <= initialStripeNum)
	{
		//random_shuffle(allNodeId.begin(), allNodeId.end());
		for (int i = 0; i < NLarge; i++)
		{
			//selectNodeId[i] = allNodeId[i];
			selectNodeId[i] = (startNodeId + i) % nodeNum;
		}
		//sort(selectNodeId.begin(), selectNodeId.end());
		rotation = 0;
		startNodeId = (startNodeId + 1) % nodeNum;

		// testbed
		vector<ERSMdsParityCommand> tempParityCommand_aGroup;
		vector<ERSMdsDataCommand> tempDataCommand_aGroup;
		tempParityCommand_aGroup.resize(M);
		for (int i = 0; i < M; i++)
		{
			tempParityCommand_aGroup[i].dataChunkNum_needReceive = receiveDataChunkNum;
		}
		startStripeId += minLargerK;

		while (stripeNumber + minLargerK <= initialStripeNum && rotation < 1)
		{
			parityStartIndex = (largerK + rotation) % NLarge;
			dataStartIndex = (parityStartIndex + M) % NLarge;
			for (int i = 0; i < largerK; i++)
			{
				tempInt = (dataStartIndex + i) % NLarge;
				dataIndex[i] = selectNodeId[tempInt];
			}
			for (int i = 0; i < M; i++)
			{
				tempInt = (parityStartIndex + i) % NLarge;
				parityIndex[i] = selectNodeId[tempInt];
			}

			for (int i = 0; i < largerK; i++)
			{
				tempChunkIndex[i] = dataIndex[i];
			}
			for (int i = 0; i < M; i++)
			{
				tempChunkIndex[largerK + i] = parityIndex[i];
			}

			for (int i = 0; i < largerK; i++)
			{
				sendNodeId = dataIndex[i];
				tempInt = 0;
				for (int j = 0; j < relativeDataNodeSendNum[i]; j++)
				{
					receiveNodeId = parityIndex[tempInt];
					nodeSendSeq[sendNodeId].push_back(receiveNodeId);
					tempInt++;
					if (tempInt == M)
					{
						tempInt = 0;
					}
				}
			}

			// testbed
			// fill command for data
			for (int i = 0; i < b; i++)
			{
				for (int j = 0; j < xoredDataIndexEachPostStripe[i].size(); j++)
				{
					ERSMdsDataCommand tempDataCommand;
					relativeChunkId = xoredDataIndexEachPostStripe[i][j];
					relativeStripeId = relativeChunkId / smallerK;
					relativeNodeId = -1;

					for (int t = 0; t < smallStripeDataIndex[relativeStripeId].size(); t++)
					{
						if (smallStripeDataIndex[relativeStripeId][t] == relativeChunkId)
						{
							relativeNodeId = t;
							break;
						}
					}
					if (relativeNodeId == -1)
					{
						cout << "error in ERSUpscale: relativeNodeId == -1" << endl;
						exit(0);
					}
					sendNodeId = dataIndex[relativeNodeId];
					ip = proxyIP[sendNodeId];
					strcpy(tempDataCommand.nodeIP_ReceiveThisCommand, ip.c_str());
					tempDataCommand.port = PROXY_START_PORT + groupId;
					for (int t = 0; t < M; t++)
					{
						parityNodeId = parityIndex[t];
						ip = proxyIP[parityNodeId];
						strcpy(tempDataCommand.destinationIps[t], ip.c_str());
					}
					tempDataCommand.relativeDataIndex = relativeChunkId;
					absoluteStripeId = startStripeId + relativeStripeId;
					dataIndexIn_chunkIndex = -1;
					for (int t = 0; t < K; t++)
					{
						if (chunkIndexs[absoluteStripeId][t] == sendNodeId)
						{
							dataIndexIn_chunkIndex = t;
							break;
						}
					}
					if (dataIndexIn_chunkIndex == -1)
					{
						cout << "error in ERSUPScale: dataIndexIn_chunkIndex == -1" << endl;
						exit(0);
					}
					chunkId = absoluteStripeId * N + dataIndexIn_chunkIndex;
					tempDataCommand.storeIndex = LocateStoreIndex(sendNodeId, chunkId);
					tempDataCommand.nodeId = sendNodeId;

					tempDataCommand_aGroup.push_back(tempDataCommand);
				}


			}
			if (tempDataCommand_aGroup.size() > 0)
			{
				dataTasksTotal.push_back(tempDataCommand_aGroup);
			}

			// fill command for parity
			for (int i = 0; i < M; i++)
			{
				parityNodeId = parityIndex[i];
				ip = proxyIP[parityNodeId];
				strcpy(tempParityCommand_aGroup[i].nodeIP_ReceiveThisCommand, ip.c_str());
				tempParityCommand_aGroup[i].port = PROXY_START_PORT + groupId;

				// fill parityChunk_eachPostStripe
				for (int j = 0; j < minSmallerK; j++)
				{
					// relativePostStripeId = j; j->relativePostStripeId
					for (int t = 0; t < requiredParityEachPostStripe[j].size(); t++)
					{
						relativeStripeId = requiredParityEachPostStripe[j][t];
						absoluteStripeId = startStripeId + relativeStripeId;
						chunkId = absoluteStripeId * N + K + i;
						storeIndex = LocateStoreIndex(parityNodeId, chunkId);
						tempParityCommand_aGroup[i].parityChunk_eachPostStripe[j][t] = storeIndex;
					}
				}

				// fill relativeDataIndex_eachPostStripe
				for (int j = 0; j < minSmallerK; j++)
				{
					// j->relativePostStripeId
					for (int t = 0; t < xoredDataIndexEachPostStripe[j].size(); t++)
					{
						relativeChunkId = xoredDataIndexEachPostStripe[j][t];
						tempParityCommand_aGroup[i].relativeDataIndex_eachPostStripe[j][t] = relativeChunkId;
					}
				}
			}
			parityTasksTotal.push_back(tempParityCommand_aGroup);

			groupId++;

			for (int i = 0; i < minSmallerK; i++)
			{
				newChunkIndexs.push_back(tempChunkIndex);
			}

			rotation++;
			stripeNumber += minLargerK;
			if (stripeNumber % ERSScaleNumberPerRound == 0)
			{
				for (int i = 0; i < nodeNum; i++)
				{
					ERSTraffic += nodeSendSeq[i].size();
				}
				timeOverhead += noSchedulingTransmission();

				// testbed
				groupId = 0;

				// execute scale: send command
				ackNumNeedReceive = 0;
				for (int i = 0; i < parityTasksTotal.size(); i++)
				{
					for (int j = 0; j < parityTasksTotal[i].size(); j++)
					{
						ERSSendParityCommand(&parityTasksTotal[i][j]);
						ackNumNeedReceive++;
					}

					if (dataTasksTotal.size() > 0)
					{
						for (int j = 0; j < dataTasksTotal[i].size(); j++)
						{
							ERSSendDataCommand(&dataTasksTotal[i][j]);
						}
					}
					//StopHere();
				}
				ReceiveAck(ackNumNeedReceive);

				parityTasksTotal.clear();
				dataTasksTotal.clear();
			}
		}
	}

	// testbed
	// execute scale: send command
	ackNumNeedReceive = 0;
	for (int i = 0; i < parityTasksTotal.size(); i++)
	{
		for (int j = 0; j < parityTasksTotal[i].size(); j++)
		{
			ERSSendParityCommand(&parityTasksTotal[i][j]);
			ackNumNeedReceive++;
		}

		if (dataTasksTotal.size() > 0)
		{
			for (int j = 0; j < dataTasksTotal[i].size(); j++)
			{
				ERSSendDataCommand(&dataTasksTotal[i][j]);
			}
		}
	}
	ReceiveAck(ackNumNeedReceive);

	parityTasksTotal.clear();
	dataTasksTotal.clear();

	for (int i = 0; i < nodeNum; i++)
	{
		ERSTraffic += nodeSendSeq[i].size();
	}
	chunkIndexs = newChunkIndexs;
	ERSStripeNumber = stripeNumber;
	initialStripeNum = chunkIndexs.size();
	K = KTarget;
	N = K + M;
	timeOverhead += noSchedulingTransmission();
	//timeOverhead += schedulingTransmission();
	return timeOverhead;
}

void ERS_MDS(vector<int>& kPool)
{
	if (kPool.size() == 0)
	{
		cout << "no k in kPool" << endl;
	}

	iK = kPool[0];
	Simulation ers;

	struct timeval t_start;
	struct timeval t_end;
	double total_time = 0.0;

	double timeOverhead = 0.0;
	int placementTraffic;
	int kl = iK;

	for (int i = 1; i < kPool.size(); i++)
	{
		total_time = 0.0;
		gettimeofday(&t_start, NULL);
		// execute placement
		//cout << "placement traffic: " << endl;
		placementTraffic = 0;
		placementTraffic = ers.ERSPlacement(kPool[i], i - 1);
		cout << "placement traffic: " << placementTraffic << endl;

		ers.GetChunkStoreOrderInEachNode();

		// multiScale
		cout << "----------------------------" << endl;
		cout << ers.Get_K() << " -> " << kPool[i] << ": " << endl;
		kl = kPool[i];
		cout << "initialStripeNum: " << ers.Get_InitialStripeNum() << endl;

		timeOverhead = ers.ERSUpScale(kl);

		cout << "ers time overhead: " << timeOverhead << endl;
		cout << "unbalance ratio: " << ers.GetUnbalanceRatio() << endl;
		ers.printTraffic("ERS");

		gettimeofday(&t_end, NULL);
		total_time = total_time + (t_end.tv_sec - t_start.tv_sec) * 1000000 + (t_end.tv_usec - t_start.tv_usec);
		total_time = total_time / 1000000;
		cout << "time: " << total_time << " s" << endl;

		cout << "----------------------------" << endl;
	}
}

int Simulation::SRSGetThreeDataIndex(vector<int>& dataChunkId_map_newEncodingIndex,
	vector<vector<int>>& logical_smallerK_StripeDataIndex,
	vector<vector<int>>& smallerK_StripeDataIndex,
	vector<vector<int>>& largerK_StripeDataIndex,
	vector<vector<int>>& oldDataIndexNeed_eachExtensionStripeInGroup, //subtraction
	vector<vector<int>>& newDataIndexNeed_eachExtensionStripeInGroup, //add
	vector<vector<pair<int, pair<int, int>>>>& sendDataIndex_eachStripeInGroup, //pair<relativeDataIndex,pair<old,new>>
	int KTarget)
{
	int groupTraffic = 0;
	int largerK = KTarget;
	int smallerK = K;
	int minLargerK = largerK / gcd(smallerK, largerK);
	int minSmallerK = smallerK / gcd(smallerK, largerK);
	int lcm = largerK * smallerK / gcd(smallerK, largerK);
	int totalGroupStripeNum = minLargerK;
	int extentionStripeNum = minSmallerK;
	int dataChunkId;
	int newIndex;
	int oldIndex;

	vector<int> dataChunkId_map_oldEncodingIndex;

	logical_smallerK_StripeDataIndex.clear();
	smallerK_StripeDataIndex.clear();
	largerK_StripeDataIndex.clear();
	oldDataIndexNeed_eachExtensionStripeInGroup.clear();
	newDataIndexNeed_eachExtensionStripeInGroup.clear();
	sendDataIndex_eachStripeInGroup.clear();

	dataChunkId_map_newEncodingIndex.clear();
	dataChunkId_map_newEncodingIndex.resize(lcm);
	dataChunkId_map_oldEncodingIndex.resize(lcm);

	// fill logical_smallerK_StripeDataIndex
	logical_smallerK_StripeDataIndex.resize(totalGroupStripeNum);
	for (int i = 0; i < totalGroupStripeNum; i++)
	{
		logical_smallerK_StripeDataIndex[i].resize(smallerK);
	}
	dataChunkId = 0;
	for (int i = 0; i < smallerK; i++)
	{
		for (int j = 0; j < totalGroupStripeNum; j++)
		{
			logical_smallerK_StripeDataIndex[j][i] = dataChunkId;
			dataChunkId_map_oldEncodingIndex[dataChunkId] = i;
			dataChunkId++;
		}
	}


	
	largerK_StripeDataIndex.resize(extentionStripeNum);
	for (int i = 0; i < extentionStripeNum; i++)
	{
		largerK_StripeDataIndex[i].resize(largerK);
	}
	dataChunkId = 0;
	for (int i = 0; i < largerK; i++)
	{
		for (int j = 0; j < extentionStripeNum; j++)
		{
			largerK_StripeDataIndex[j][i] = dataChunkId;
			dataChunkId_map_newEncodingIndex[dataChunkId] = i;
			dataChunkId++;
		}
	}

	
	smallerK_StripeDataIndex.resize(totalGroupStripeNum);
	for (int i = 0; i < totalGroupStripeNum; i++)
	{
		smallerK_StripeDataIndex[i].resize(largerK);
	}
	for (int i = 0; i < totalGroupStripeNum; i++)
	{
		for (int j = 0; j < largerK; j++)
		{
			smallerK_StripeDataIndex[i][j] = -1;
		}
	}
	for (int i = 0; i < totalGroupStripeNum; i++)
	{
		for (int j = 0; j < smallerK; j++)
		{
			dataChunkId = logical_smallerK_StripeDataIndex[i][j];
			newIndex = dataChunkId_map_newEncodingIndex[dataChunkId];
			smallerK_StripeDataIndex[i][newIndex] = dataChunkId;
		}
	}


	
	oldDataIndexNeed_eachExtensionStripeInGroup.resize(extentionStripeNum);
	newDataIndexNeed_eachExtensionStripeInGroup.resize(extentionStripeNum);
	sendDataIndex_eachStripeInGroup.resize(totalGroupStripeNum);

	
	for (int i = 0; i < extentionStripeNum; i++)
	{
		for (int j = 0; j < smallerK; j++)
		{
			dataChunkId = logical_smallerK_StripeDataIndex[i][j];
			if (largerK_StripeDataIndex[i][j] != dataChunkId)
			{
				pair<int, pair<int, int>> tempPair; // //pair<relativeDataIndex,pair<old,new>>
				tempPair.first = dataChunkId;
				tempPair.second.first = dataChunkId_map_oldEncodingIndex[dataChunkId];
				tempPair.second.second = dataChunkId_map_newEncodingIndex[dataChunkId];
				sendDataIndex_eachStripeInGroup[i].push_back(tempPair);
				oldDataIndexNeed_eachExtensionStripeInGroup[i].push_back(dataChunkId);
				groupTraffic++;
			}
		}

	}

	
	for (int i = extentionStripeNum; i < totalGroupStripeNum; i++)
	{
		for (int j = 0; j < smallerK; j++)
		{
			dataChunkId = logical_smallerK_StripeDataIndex[i][j];
			pair<int, pair<int, int>> tempPair; // //pair<relativeDataIndex,pair<old,new>>
			tempPair.first = dataChunkId;
			tempPair.second.first = dataChunkId_map_oldEncodingIndex[dataChunkId];
			tempPair.second.second = dataChunkId_map_newEncodingIndex[dataChunkId];
			sendDataIndex_eachStripeInGroup[i].push_back(tempPair);
			groupTraffic++;
		}
	}

	
	for (int i = 0; i < extentionStripeNum; i++)
	{
		for (int j = 0; j < smallerK; j++)
		{
			dataChunkId = largerK_StripeDataIndex[i][j];
			if (logical_smallerK_StripeDataIndex[i][j] != dataChunkId)
			{
				int isFindChunk = 0;
				
				if (isFindChunk == 0)
				{
					newDataIndexNeed_eachExtensionStripeInGroup[i].push_back(dataChunkId);
				}
			}
		}
		for (int j = smallerK; j < largerK; j++)
		{
			dataChunkId = largerK_StripeDataIndex[i][j];
			newDataIndexNeed_eachExtensionStripeInGroup[i].push_back(dataChunkId);
		}
	}

	groupTraffic = groupTraffic * M;
	return groupTraffic;
}

int Simulation::SRSGetTargetPlacement(int KTarget, vector<vector<int>>& newChunkIndex_beforeScale)
{
	int groupTraffic = 0;
	int largerK = KTarget;
	int smallerK = K;
	int minLargerK = largerK / gcd(smallerK, largerK);
	int minSmallerK = smallerK / gcd(smallerK, largerK);
	int NLarge = largerK + M;
	vector<int> dataChunkId_map_newEncodingIndex;
	vector<vector<int>> logical_smallerK_StripeDataIndex;
	vector<vector<int>> smallerK_StripeDataIndex;
	vector<vector<int>> largerK_StripeDataIndex;
	vector<vector<int>> oldDataIndexNeed_eachExtensionStripeInGroup; //subtraction
	vector<vector<int>> newDataIndexNeed_eachExtensionStripeInGroup; //add
	vector<vector<pair<int, pair<int, int>>>> sendDataIndex_eachStripeInGroup;

	groupTraffic = SRSGetThreeDataIndex(dataChunkId_map_newEncodingIndex,
		logical_smallerK_StripeDataIndex, smallerK_StripeDataIndex, largerK_StripeDataIndex,
		oldDataIndexNeed_eachExtensionStripeInGroup, newDataIndexNeed_eachExtensionStripeInGroup,
		sendDataIndex_eachStripeInGroup, KTarget);

	//place chunk
	int startNodeId;
	int stripeNum_afterScale = initialStripeNum * K / largerK;
	int dataChunkIdInGroup;
	int encodingIndex;
	int chunkId;
	int parityNodeId;

	vector<vector<int>> tempNewChunkIndex;

	newChunkIndex_beforeScale.clear();
	tempNewChunkIndex.resize(minLargerK);

	for (int i = 0; i < minLargerK; i++)
	{
		tempNewChunkIndex[i].resize(K + M);
	}

	startNodeId = 0;
	for (int i = 0; (i + minSmallerK) <= stripeNum_afterScale; i += minSmallerK)
	{
		//place newChunkIndex_beforeScale
		//initial
		for (int j = 0; j < minLargerK; j++)
		{
			for (int t = 0; t < K + M; t++)
			{
				tempNewChunkIndex[j][t] = -1;
			}
		}

		//place data chunk
		for (int j = 0; j < minLargerK; j++)
		{
			for (int t = 0; t < smallerK; t++)
			{
				dataChunkIdInGroup = logical_smallerK_StripeDataIndex[j][t];
				encodingIndex = dataChunkId_map_newEncodingIndex[dataChunkIdInGroup];
				tempNewChunkIndex[j][t] = (startNodeId + encodingIndex) % nodeNum;
			}
		}
		//place parity chunk
		for (int t = 0; t < M; t++)
		{
			parityNodeId = (startNodeId + largerK + t) % nodeNum;
			for (int j = 0; j < minLargerK; j++)
			{
				tempNewChunkIndex[j][K + t] = parityNodeId;
			}
		}

		newChunkIndex_beforeScale.insert(newChunkIndex_beforeScale.end(),
			tempNewChunkIndex.begin(), tempNewChunkIndex.end());

		startNodeId = (startNodeId + 1) % nodeNum;
	}

	return groupTraffic;
}

int Simulation::SRSPlacement(int KTarget, int scaleTime)
{
	int traffic = 0;

	vector<vector<int>> newChunkIndex_beforeScale;

	traffic = SRSGetTargetPlacement(KTarget, newChunkIndex_beforeScale);
	traffic = 0;

	if (scaleTime == 0)
	{
		chunkIndexs.clear();
		chunkIndexs = newChunkIndex_beforeScale;
		initialStripeNum = chunkIndexs.size();
		return traffic;
	}


	// testbed
	int totalPlaceMentStripeNum;
	int placementNumPerRound;
	int indexInPerRound;
	int receiveNum;
	int newNodeId;
	int oldNodeId;
	int chunkId;
	int parityNodeId;
	int ackNumNeedReceive;
	int writeIndex;
	string ip;

	vector<vector<SRSPlaceMentCommand>> placementCommandPerRound;
	vector<SRSPlaceMentCommand> placementAStripe;

	GetChunkStoreOrderInEachNode();


	//for (int i = 0; i < newChunkIndex_beforeScale.size(); i++)
	//{
	//	//compare data chunk
	//	for (int j = 0; j < K; j++)
	//	{
	//		if (chunkIndexs[i][j] != newChunkIndex_beforeScale[i][j])
	//		{
	//			traffic += 1; // move data chunk
	//		}
	//	}
	//	for (int j = 0; j < M; j++)
	//	{
	//		if (chunkIndexs[i][K + j] != newChunkIndex_beforeScale[i][K + j])
	//		{
	//			traffic += 1; // move parity chunk
	//		}
	//	}
	//}



	/* for placement */

	vector<int> oldIndex;
	vector<int> newIndex;
	vector<int> intersectionIndex;
	vector<int> differentInOld;
	vector<int> differentInNew;

	totalPlaceMentStripeNum = newChunkIndex_beforeScale.size();
	placementNumPerRound = ERS_PLACEMENT_PER_Round;
	indexInPerRound = 0;
	ackNumNeedReceive = 0;
	for (int i = 0; i < totalPlaceMentStripeNum; i++)
	{
		placementAStripe.clear();


		oldIndex.clear();
		newIndex.clear();
		intersectionIndex.clear();
		differentInOld.clear();
		differentInNew.clear();
		//compare data chunk
		for (int j = 0; j < K; j++)
		{
			//if (chunkIndexs[i][j] != newChunkIndex_beforeScale[i][j])
			//{
			//	traffic += 1; // move data chunk
			//	traffic += M; // recaculate parity
			//}
			oldIndex.push_back(chunkIndexs[i][j]);
			newIndex.push_back(newChunkIndex_beforeScale[i][j]);

		}
		sort(oldIndex.begin(), oldIndex.end());
		sort(newIndex.begin(), newIndex.end());
		set_intersection(oldIndex.begin(),
			oldIndex.end(),
			newIndex.begin(),
			newIndex.end(),
			inserter(intersectionIndex, intersectionIndex.begin()));
		set_difference(oldIndex.begin(),
			oldIndex.end(),
			intersectionIndex.begin(),
			intersectionIndex.end(),
			inserter(differentInOld, differentInOld.begin()));
		set_difference(newIndex.begin(),
			newIndex.end(),
			intersectionIndex.begin(),
			intersectionIndex.end(),
			inserter(differentInNew, differentInNew.begin()));

		if (differentInOld.size() != differentInNew.size())
		{
			cout << "SRSPlacement: differentInOld.size() != differentInNew.size() in dataChunk" << endl;
			exit(0);
		}

		//compare data chunk
		for (int j = 0; j < differentInOld.size(); j++)
		{

			traffic += 1; // move data chunk
			SRSPlaceMentCommand tempSender;
			SRSPlaceMentCommand tempReceiver;

			// for tempSender
			oldNodeId = differentInOld[j];
			newNodeId = differentInNew[j];
			ip = proxyIP[oldNodeId];
			chunkId = -1;
			for (int t = 0; t < K; t++)
			{
				if (chunkIndexs[i][t] == oldNodeId)
				{
					chunkId = i * N + t;
					break;
				}
			}
			if (chunkId == -1)
			{
				cout << "SRSPlacement: Placement not find dataChunk" << endl;
				exit(0);
			}

			strcpy(tempSender.nodeIP_ReceiveThisCommand, ip.c_str());
			tempSender.port = RELOCATION_START_PORT + indexInPerRound;
			tempSender.role = SRS_PLACEMENT_SENDER;
			tempSender.storeIndex = LocateStoreIndex(oldNodeId, chunkId);
			ip = proxyIP[newNodeId];
			strcpy(tempSender.destinationIp, ip.c_str());

			// for tempReceiver
			ip = proxyIP[newNodeId];

			strcpy(tempReceiver.nodeIP_ReceiveThisCommand, ip.c_str());
			tempReceiver.port = RELOCATION_START_PORT + indexInPerRound;
			tempReceiver.role = SRS_PLACEMENT_RECEIVER;
			writeIndex = chunkIndex_eachNode[newNodeId].size();
			tempReceiver.writeIndex = writeIndex;
			tempReceiver.receiveNum = 1;

			// push tempSender and tempReceiver in placementAStripe
			placementAStripe.push_back(tempSender);
			placementAStripe.push_back(tempReceiver);

			ackNumNeedReceive++;

		}


		oldIndex.clear();
		newIndex.clear();
		intersectionIndex.clear();
		differentInOld.clear();
		differentInNew.clear();
		for (int j = 0; j < M; j++)
		{
			//if (chunkIndexs[i][K + j] != newChunkIndex_beforeScale[i][K + j])
			//{
			//	traffic += 1; // move parity chunk
			//}
			oldIndex.push_back(chunkIndexs[i][K + j]);
			newIndex.push_back(newChunkIndex_beforeScale[i][K + j]);
		}
		sort(oldIndex.begin(), oldIndex.end());
		sort(newIndex.begin(), newIndex.end());
		set_intersection(oldIndex.begin(),
			oldIndex.end(),
			newIndex.begin(),
			newIndex.end(),
			inserter(intersectionIndex, intersectionIndex.begin()));
		set_difference(oldIndex.begin(),
			oldIndex.end(),
			intersectionIndex.begin(),
			intersectionIndex.end(),
			inserter(differentInOld, differentInOld.begin()));
		set_difference(newIndex.begin(),
			newIndex.end(),
			intersectionIndex.begin(),
			intersectionIndex.end(),
			inserter(differentInNew, differentInNew.begin()));
		if (differentInOld.size() != differentInNew.size())
		{
			cout << "SRSPlacement: differentInOld.size() != differentInNew.size() in parityChunk" << endl;
			exit(0);
		}

		for (int j = 0; j < differentInOld.size(); j++)
		{

			traffic += 1; // move parity chunk

			SRSPlaceMentCommand tempSender;
			SRSPlaceMentCommand tempReceiver;

			// for tempSender
			oldNodeId = chunkIndexs[i][K + j];
			newNodeId = newChunkIndex_beforeScale[i][K + j];
			ip = proxyIP[oldNodeId];
			chunkId = -1;
			for (int t = 0; t < M; t++)
			{
				if (chunkIndexs[i][K + t] == oldNodeId)
				{
					chunkId = i * N + K + t;
					break;
				}
			}
			if (chunkId == -1)
			{
				cout << "SRSPlacement: Placement not find parityChunk" << endl;
				exit(0);
			}

			strcpy(tempSender.nodeIP_ReceiveThisCommand, ip.c_str());
			tempSender.port = RELOCATION_START_PORT + indexInPerRound;
			tempSender.role = SRS_PLACEMENT_SENDER;
			tempSender.storeIndex = LocateStoreIndex(oldNodeId, chunkId);
			ip = proxyIP[newNodeId];
			strcpy(tempSender.destinationIp, ip.c_str());

			// for tempReceiver
			ip = proxyIP[newNodeId];

			strcpy(tempReceiver.nodeIP_ReceiveThisCommand, ip.c_str());
			tempReceiver.port = RELOCATION_START_PORT + indexInPerRound;
			tempReceiver.role = SRS_PLACEMENT_RECEIVER;
			writeIndex = chunkIndex_eachNode[newNodeId].size();
			tempReceiver.writeIndex = writeIndex;
			tempReceiver.receiveNum = 1;

			// push tempSender and tempReceiver in placementAStripe
			placementAStripe.push_back(tempSender);
			placementAStripe.push_back(tempReceiver);

			ackNumNeedReceive++;

		}

		// push placementAStripe in placementCommandPerRound
		placementCommandPerRound.push_back(placementAStripe);


		indexInPerRound++;
		if (indexInPerRound == placementNumPerRound)
		{
			// send command
			for (int j = 0; j < placementCommandPerRound.size(); j++)
			{
				for (int t = 0; t < placementCommandPerRound[j].size(); t++)
				{
					SRSSendPlacementCommand(&placementCommandPerRound[j][t]);

				}

			}

			// wait ack
			ReceiveAck(ackNumNeedReceive);

			indexInPerRound = 0;
			ackNumNeedReceive = 0;
			placementCommandPerRound.clear();
		}
	}

	if (indexInPerRound != 0)
	{
		// send command
		for (int j = 0; j < placementCommandPerRound.size(); j++)
		{
			for (int t = 0; t < placementCommandPerRound[j].size(); t++)
			{
				SRSSendPlacementCommand(&placementCommandPerRound[j][t]);
			}
		}
		// wait ack
		ReceiveAck(ackNumNeedReceive);

		indexInPerRound = 0;
		ackNumNeedReceive = 0;
		placementCommandPerRound.clear();
	}


	// 4. process scale(K -> K+delta): have done in 2.
	//5. update something
	chunkIndexs.clear();
	chunkIndexs = newChunkIndex_beforeScale;
	//stripeNum = chunkIndexs.size();// 3750-?
	initialStripeNum = chunkIndexs.size();
	//K += deltaK;

	return traffic;
}

double Simulation::SRSUpScale(int KLarge)
{
	this->KLarge = KLarge;
	this->minKl = KLarge / gcd(K, KLarge);
	this->minK = K / gcd(K, KLarge);
	this->SRSTraffic = 0;
	int lcm = K * KLarge / gcd(K, KLarge);
	int rotation;
	int s = KLarge;
	int NLarge = s + M;
	int parityStartIndex;
	int dataStartIndex;
	int sendNodeId;
	int receiveNodeId;
	int tempInt;
	int startNodeId;
	double timeOverhead = 0.0;

	vector<int> allNodeId;
	vector<int> selectNodeId;
	vector<int> dataIndex;
	vector<int> parityIndex;
	vector<int> tempChunkIndex;

	// testbed
	int groupTraffic = 0;
	int groupId;
	int startStripeId;
	int relativeStripeId;
	int absoluteStripeId;
	int chunkId;
	int newEncodingCoe;
	int oldEncodingCoe;
	int locationInOldStripe;
	int nodeId;
	int parityNodeId;
	int dataChunkNumNeedReceive;
	int storeIndex;
	int relativeDataIndex;
	int ackNumNeedReceive;
	string ip;
	vector<int> dataChunkId_map_newEncodingIndex;
	vector<vector<int>> logical_smallerK_StripeDataIndex;
	vector<vector<int>> smallerK_StripeDataIndex;
	vector<vector<int>> largerK_StripeDataIndex;
	vector<vector<int>> oldDataIndexNeed_eachExtensionStripeInGroup; //subtraction
	vector<vector<int>> newDataIndexNeed_eachExtensionStripeInGroup; //add
	vector<vector<pair<int, pair<int, int>>>> sendDataIndex_eachStripeInGroup;

	vector<vector<SRSMdsDataCommand>> dataTasksTotal; // for groups execute together
	vector<vector<SRSMdsParityCommand>> parityTasksTotal; // for groups execute together

	groupTraffic = SRSGetThreeDataIndex(dataChunkId_map_newEncodingIndex,
		logical_smallerK_StripeDataIndex, smallerK_StripeDataIndex, largerK_StripeDataIndex,
		oldDataIndexNeed_eachExtensionStripeInGroup, newDataIndexNeed_eachExtensionStripeInGroup,
		sendDataIndex_eachStripeInGroup, KLarge);

	dataChunkNumNeedReceive = 0;
	for (int i = 0; i < sendDataIndex_eachStripeInGroup.size(); i++)
	{
		dataChunkNumNeedReceive += sendDataIndex_eachStripeInGroup[i].size();
	}
	// check is out of bounds
	if (minKl > SRS_MAX_POST)
	{
		cout << "SRSUpScale: minKl > SRS_MAX_POST" << endl;
		exit(0);
	}
	for (int i = 0; i < oldDataIndexNeed_eachExtensionStripeInGroup.size(); i++)
	{
		if (oldDataIndexNeed_eachExtensionStripeInGroup[i].size() > SRS_MAX_DATA)
		{
			cout << "SRSUpScale: oldDataIndexNeed_eachExtensionStripeInGroup[i].size() > SRS_MAX_DATA" << endl;
			exit(0);
		}
	}
	for (int i = 0; i < newDataIndexNeed_eachExtensionStripeInGroup.size(); i++)
	{
		if (newDataIndexNeed_eachExtensionStripeInGroup[i].size() > SRS_MAX_DATA)
		{
			cout << "SRSUpScale: newDataIndexNeed_eachExtensionStripeInGroup[i].size() > SRS_MAX_DATA" << endl;
			exit(0);
		}
	}

	allNodeId.resize(nodeNum);
	selectNodeId.resize(NLarge);
	dataIndex.resize(KLarge);
	parityIndex.resize(M);
	tempChunkIndex.resize(NLarge);
	newChunkIndexs.clear();

	if (NLarge > nodeNum)
	{
		cout << "error in SRSUpScale" << endl;
		cout << "Simulation Stop" << endl;
		exit(0);
	}

	for (int i = 0; i < nodeNum; i++)
	{
		allNodeId[i] = i;
		nodeSendSeq[i].clear();
	}
	stripeNumber = 0;
	SRSScaleNumberPerRound = 0;
	while (SRSScaleNumberPerRound + minKl <= downScaleStripePerRound)
	{
		SRSScaleNumberPerRound += minKl;
	}

	startNodeId = 0;
	groupId = 0;
	dataTasksTotal.clear();
	parityTasksTotal.clear();
	startStripeId = 0;
	while (stripeNumber + minKl <= initialStripeNum)
	{
		//random_shuffle(allNodeId.begin(), allNodeId.end());
		for (int i = 0; i < NLarge; i++)
		{
			//selectNodeId[i] = allNodeId[i];
			selectNodeId[i] = (startNodeId + i) % nodeNum;
		}
		//sort(selectNodeId.begin(), selectNodeId.end());
		rotation = 0;
		startNodeId = (startNodeId + 1) % nodeNum;
		while (stripeNumber + minKl <= initialStripeNum && rotation < 1)
		{
			parityStartIndex = (KLarge + rotation) % NLarge;
			dataStartIndex = (parityStartIndex + M) % NLarge;
			for (int i = 0; i < KLarge; i++)
			{
				tempInt = (dataStartIndex + i) % NLarge;
				dataIndex[i] = selectNodeId[tempInt];
			}
			for (int i = 0; i < M; i++)
			{
				tempInt = (parityStartIndex + i) % NLarge;
				parityIndex[i] = selectNodeId[tempInt];
			}
			for (int i = 0; i < KLarge; i++)
			{
				tempChunkIndex[i] = dataIndex[i];
			}
			for (int i = 0; i < M; i++)
			{
				tempChunkIndex[KLarge + i] = parityIndex[i];
			}
			for (int i = 0; i < minK; i++)
			{
				for (int j = 1; j < KLarge; j++)
				{
					sendNodeId = dataIndex[j];
					for (int t = 0; t < M; t++)
					{
						receiveNodeId = parityIndex[t];
						nodeSendSeq[sendNodeId].push_back(receiveNodeId);
					}
				}
				newChunkIndexs.push_back(tempChunkIndex);
			}
			rotation++;
			stripeNumber += minKl;

			// testbed
			vector<SRSMdsParityCommand> tempParityCommand_aGroup;
			vector<SRSMdsDataCommand> tempDataCommand_aGroup;
			tempParityCommand_aGroup.resize(M);

			// fill tempDataCommand_aGroup
			for (int i = 0; i < sendDataIndex_eachStripeInGroup.size(); i++)
			{
				for (int j = 0; j < sendDataIndex_eachStripeInGroup[i].size(); j++)
				{
					SRSMdsDataCommand tempDataCommand;
					locationInOldStripe = sendDataIndex_eachStripeInGroup[i][j].second.first;
					absoluteStripeId = startStripeId + i;
					nodeId = chunkIndexs[absoluteStripeId][locationInOldStripe];
					ip = proxyIP[nodeId];
					chunkId = absoluteStripeId * N + locationInOldStripe;

					strcpy(tempDataCommand.nodeIP_ReceiveThisCommand, ip.c_str());
					tempDataCommand.port = PROXY_START_PORT + groupId;
					for (int t = 0; t < M; t++)
					{
						parityNodeId = chunkIndexs[absoluteStripeId][K + t];
						ip = proxyIP[parityNodeId];
						strcpy(tempDataCommand.destinationIps[t], ip.c_str());
					}

					tempDataCommand.relativeDataIndex = sendDataIndex_eachStripeInGroup[i][j].first;
					tempDataCommand.storeIndex = LocateStoreIndex(nodeId, chunkId);
					tempDataCommand.oldEncodingCoe = sendDataIndex_eachStripeInGroup[i][j].second.first;
					tempDataCommand.newEncodingCoe = sendDataIndex_eachStripeInGroup[i][j].second.second;

					tempDataCommand_aGroup.push_back(tempDataCommand);

				}
			}
			// push in dataTasksTotal
			dataTasksTotal.push_back(tempDataCommand_aGroup);

			// fill tempParityCommand_aGroup
			for (int i = 0; i < M; i++)
			{
				parityNodeId = parityIndex[i];
				ip = proxyIP[parityNodeId];
				strcpy(tempParityCommand_aGroup[i].nodeIP_ReceiveThisCommand, ip.c_str());
				tempParityCommand_aGroup[i].port = PROXY_START_PORT + groupId;
				tempParityCommand_aGroup[i].dataChunkNum_needReceive = dataChunkNumNeedReceive;
				tempParityCommand_aGroup[i].parityIndexInStripe = i;

				// fill parityChunk_eachPostStripe
				for (int j = 0; j < minK; j++)
				{
					absoluteStripeId = startStripeId + j;
					chunkId = absoluteStripeId * N + K + i;
					storeIndex = LocateStoreIndex(parityNodeId, chunkId);
					tempParityCommand_aGroup[i].parityChunk_eachPostStripe[j][0] = storeIndex;
				}
				// fill oldDataIndexNeed_eachPostStripe
				for (int j = 0; j < minK; j++)
				{
					for (int t = 0; t < oldDataIndexNeed_eachExtensionStripeInGroup[j].size(); t++)
					{
						relativeDataIndex = oldDataIndexNeed_eachExtensionStripeInGroup[j][t];
						tempParityCommand_aGroup[i].oldDataIndexNeed_eachPostStripe[j][t] = relativeDataIndex;
					}
				}
				// fill newDataIndexNeed_eachPostStripe
				for (int j = 0; j < minK; j++)
				{
					for (int t = 0; t < newDataIndexNeed_eachExtensionStripeInGroup[j].size(); t++)
					{
						relativeDataIndex = newDataIndexNeed_eachExtensionStripeInGroup[j][t];
						tempParityCommand_aGroup[i].newDataIndexNeed_eachPostStripe[j][t] = relativeDataIndex;
					}
				}
			}
			// push in parityTasksTotal
			parityTasksTotal.push_back(tempParityCommand_aGroup);


			SRSTraffic += groupTraffic;
			groupId++;

			startStripeId += minKl;

			if (stripeNumber % SRSScaleNumberPerRound == 0)
			{
				int temptemp = 0;
				for (int i = 0; i < nodeNum; i++)
				{
					temptemp += nodeSendSeq[i].size();
					//SRSTraffic += nodeSendSeq[i].size();
				}
				timeOverhead += noSchedulingTransmission();
				groupId = 0;

				// execute scale: send command
				ackNumNeedReceive = 0;
				for (int i = 0; i < parityTasksTotal.size(); i++)
				{
					for (int j = 0; j < parityTasksTotal[i].size(); j++)
					{
						SRSSendParityCommand(&parityTasksTotal[i][j]);
						ackNumNeedReceive++;
					}

					if (dataTasksTotal.size() > 0)
					{
						for (int j = 0; j < dataTasksTotal[i].size(); j++)
						{
							SRSSendDataCommand(&dataTasksTotal[i][j]);
						}
					}
					//StopHere();
				}
				ReceiveAck(ackNumNeedReceive);

				dataTasksTotal.clear();
				parityTasksTotal.clear();
			}
		}
	}


	ackNumNeedReceive = 0;
	for (int i = 0; i < parityTasksTotal.size(); i++)
	{
		for (int j = 0; j < parityTasksTotal[i].size(); j++)
		{
			SRSSendParityCommand(&parityTasksTotal[i][j]);
			ackNumNeedReceive++;
		}

		if (dataTasksTotal.size() > 0)
		{
			for (int j = 0; j < dataTasksTotal[i].size(); j++)
			{
				SRSSendDataCommand(&dataTasksTotal[i][j]);
			}
		}
	}
	if (ackNumNeedReceive > 0)
		ReceiveAck(ackNumNeedReceive);

	parityTasksTotal.clear();
	dataTasksTotal.clear();

	for (int i = 0; i < nodeNum; i++)
	{
		//SRSTraffic += nodeSendSeq[i].size();
	}
	SRSStripeNumber = stripeNumber;
	timeOverhead += noSchedulingTransmission();
	//timeOverhead += schedulingTransmission();
	chunkIndexs = newChunkIndexs;
	initialStripeNum = chunkIndexs.size();
	K = KLarge;
	N = K + M;
	return timeOverhead;
}

void SRS_MDS(vector<int>& kPool)
{
	if (kPool.size() == 0)
	{
		cout << "no k in kPool" << endl;
	}

	iK = kPool[0];
	Simulation srs;

	struct timeval t_start;
	struct timeval t_end;
	double total_time = 0.0;

	double timeOverhead = 0.0;
	int placementTraffic;
	int kl = iK;

	for (int i = 1; i < kPool.size(); i++)
	{
		total_time = 0.0;
		gettimeofday(&t_start, NULL);
		// execute placement
		//cout << "placement traffic: " << endl;
		placementTraffic = 0;
		placementTraffic = srs.SRSPlacement(kPool[i], i - 1);
		cout << "placement traffic: " << placementTraffic << endl;

		srs.GetChunkStoreOrderInEachNode();

		// multiScale
		cout << "----------------------------" << endl;
		cout << srs.Get_K() << " -> " << kPool[i] << ": " << endl;
		kl = kPool[i];
		cout << "initialStripeNum: " << srs.Get_InitialStripeNum() << endl;

		timeOverhead = srs.SRSUpScale(kl);

		cout << "srs time overhead: " << timeOverhead << endl;
		cout << "unbalance ratio: " << srs.GetUnbalanceRatio() << endl;
		srs.printTraffic("SRS");

		gettimeofday(&t_end, NULL);
		total_time = total_time + (t_end.tv_sec - t_start.tv_sec) * 1000000 + (t_end.tv_usec - t_start.tv_usec);
		total_time = total_time / 1000000;
		cout << "time: " << total_time << " s" << endl;

		cout << "----------------------------" << endl;
	}
}

//zf
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

	//testbed
	int port;
	string ip;
	int chunkid;
	int sendnodeid;
	int receivenodeid;
	int storeindex;
	int ackNum = 0;
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
		ackNum = 0;
		vector<vector<IdeaMdsUpdateParityCommand>> parityUpdateCommandEachExtensionStripe;
		vector<vector<IdeaMdsRelocationCommand>> relocationCommandEachExtensionStripe;
		

		
        for(int i = 0 ; i < groupNum ; i++){
            updateTraffic += (minLargerK - minSmallerK)*( K - M );
            currentStripeId = i*eachGroupStripeNum + currentRound*minLargerK;

            
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

			//testbed:relocation command
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

				//get stretched node index
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

			//old -> new
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

				
					int tb_send_nodeid = -1 , tb_receive_nodeid = -1, tb_chunkId = -1;
					tb_receive_nodeid = stretChunkNodeId;
					

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

			vector<vector<int>> numOfExtStripe; 
			numOfExtStripe.resize(K);
			for(int j = 0 ; j < K ; j++){
				numOfExtStripe[j].resize(M);
				int numofstripe = 0;
				for(int t = 0 ; t < largerK - K ; t++){
					for(int q = 0 ; q < K ; q++){
						if(decomDatatoStretStripeId[t][q] == j){
							numofstripe++;
							numOfExtStripe[j][t%M] --;
							break;
						}
					}
				}
				for(int t = 0 ; t < M ; t++){
					numOfExtStripe[j][t] += numofstripe;
				}
			}


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


			
			vector<IdeaMdsRelocationCommand> groupRelocationCommand;
			
			for(int j = 0 ; j < K ; j++){
				port = RELOCATION_START_PORT + currentStripeId + j;
				for(int t = 0 ; t < RelocationSendChunkTasks[j].size() ; t++){
					if(RelocationSendChunkTasks[j][t].sendNodeId == RelocationSendChunkTasks[j][t].receiveNodeId){
						continue;
					}
					sendnodeid = RelocationSendChunkTasks[j][t].sendNodeId;
					receivenodeid = RelocationSendChunkTasks[j][t].receiveNodeId;
					chunkid = RelocationSendChunkTasks[j][t].chunkId;
					IdeaMdsRelocationCommand senderIdeaMdsRelocationCommand;
					IdeaMdsRelocationCommand receiverIdeaMdsRelocationCommand;

					senderIdeaMdsRelocationCommand.sizeOfCommand = sizeof(IdeaMdsRelocationCommand);
					ip = proxyIP[sendnodeid];
					strcpy(senderIdeaMdsRelocationCommand.nodeIP_ReceiveThisCommand, ip.c_str());
					senderIdeaMdsRelocationCommand.port = port;
					senderIdeaMdsRelocationCommand.role = RELOCATION_SENDER;
					storeindex = LocateStoreIndex(sendnodeid, chunkid);
					senderIdeaMdsRelocationCommand.dataStoreIndex = storeindex;
					ip = proxyIP[receivenodeid];
					strcpy(senderIdeaMdsRelocationCommand.destinationIp, ip.c_str());

					receiverIdeaMdsRelocationCommand.sizeOfCommand = sizeof(IdeaMdsRelocationCommand);
					strcpy(receiverIdeaMdsRelocationCommand.nodeIP_ReceiveThisCommand, ip.c_str());
					receiverIdeaMdsRelocationCommand.port = port;
					receiverIdeaMdsRelocationCommand.role = RELOCATION_RECEIVER;
					storeindex = chunkIndex_eachNode[receivenodeid].size();
					receiverIdeaMdsRelocationCommand.writeLocation = storeindex;

					groupRelocationCommand.push_back(senderIdeaMdsRelocationCommand);
					groupRelocationCommand.push_back(receiverIdeaMdsRelocationCommand);
					ackNum++;
				}
			}

			
			vector<IdeaMdsUpdateParityCommand> groupUpdateCommand;


			
			for(int j = 0 ; j < largerK - K ; j ++){
				port = PROXY_START_PORT + currentStripeId + K + j;
				receivenodeid = chunkIndexs[currentStripeId][K+(j)%M];
				for(int t = 0 ; t < ParityUpdateSendChunkTasks[j].size() ;  t++){
					sendnodeid = ParityUpdateSendChunkTasks[j][t].sendNodeId;
					chunkid = ParityUpdateSendChunkTasks[j][t].chunkId;

					IdeaMdsUpdateParityCommand tempUpdateCommand;
					tempUpdateCommand.sizeOfCommand = sizeof(IdeaMdsUpdateParityCommand);
					// receive command node ip
					ip = proxyIP[sendnodeid];
					strcpy(tempUpdateCommand.nodeIP_ReceiveThisCommand, ip.c_str());
					// destinationIp
					ip = proxyIP[receivenodeid];
					strcpy(tempUpdateCommand.destinationIp[0], ip.c_str());
					tempUpdateCommand.port = port;
					tempUpdateCommand.role = DATA_CHUNK;
					tempUpdateCommand.dataIndexInNewStripe = K+t;
					storeindex = LocateStoreIndex(sendnodeid, chunkid);
					tempUpdateCommand.dataStoreIndex = storeindex;

					groupUpdateCommand.push_back(tempUpdateCommand);
				}


				port = PROXY_START_PORT + currentStripeId + K + j;	
				int paritynodeid;
				paritynodeid = receivenodeid;
				chunkid = (currentStripeId+K+j)*N + K;

				IdeaMdsUpdateParityCommand tempUpdateCommand;
				tempUpdateCommand.sizeOfCommand = sizeof(IdeaMdsUpdateParityCommand);
				ip = proxyIP[paritynodeid];
				strcpy(tempUpdateCommand.nodeIP_ReceiveThisCommand, ip.c_str());
				tempUpdateCommand.port = port;
				tempUpdateCommand.role = COLLECT_NODE;
				storeindex = LocateStoreIndex(paritynodeid,chunkid);
				tempUpdateCommand.parityStoreIndex = storeindex;
				tempUpdateCommand.receiveNum = K - 1;
				tempUpdateCommand.indexInGroup = K + j ;
				for(int t = 0 ; t < K ; t++){
					tempUpdateCommand.decomStripeDataUpdateToStripeId[t] = decomDatatoStretStripeId[j][t];
				}
				int tempIndex = 0;
				for(int t = 0 ; t < M ; t++){
					if(paritynodeid == chunkIndexs[currentStripeId][K+t]){
						continue;
					}
					ip = proxyIP[chunkIndexs[currentStripeId][K+t]];
					strcpy(tempUpdateCommand.destinationIp[tempIndex], ip.c_str());
					tempIndex++;
				}

				groupUpdateCommand.push_back(tempUpdateCommand);

			}
			

			
			for(int j  = 0 ; j < K ; j++){
				port = PROXY_START_PORT + currentStripeId + j;

				for(int t = 0 ; t < M ; t++){
					int paritynodeid;
					paritynodeid = chunkIndexs[currentStripeId+j][K+t];
					chunkid = (currentStripeId+j)*N + K + t;

					IdeaMdsUpdateParityCommand tempUpdateCommand;
					tempUpdateCommand.sizeOfCommand = sizeof(IdeaMdsUpdateParityCommand);
					// receive command node ip
					ip = proxyIP[paritynodeid];
					strcpy(tempUpdateCommand.nodeIP_ReceiveThisCommand, ip.c_str());
					tempUpdateCommand.port = port;
					tempUpdateCommand.role = PARITY_CHUNK;
					tempUpdateCommand.parityIndexInNewStripe = j;
					storeindex = LocateStoreIndex(paritynodeid, chunkid);
					tempUpdateCommand.parityStoreIndex = storeindex;
					tempUpdateCommand.receiveNum = numOfExtStripe[j][t];
					
					groupUpdateCommand.push_back(tempUpdateCommand);
					ackNum++;
				}
			}

			
        //each group

		relocationCommandEachExtensionStripe.push_back(groupRelocationCommand);
		parityUpdateCommandEachExtensionStripe.push_back(groupUpdateCommand);


        }

		
		cout << "idea mds send command" << endl;
		
		for(int i = 0 ; i < relocationCommandEachExtensionStripe.size() ; i++){
			IdeaSendRelocationCommandEachStripe(relocationCommandEachExtensionStripe[i]);
		}

		for(int i = 0 ; i < parityUpdateCommandEachExtensionStripe.size() ; i++){
			IdeaSendUpdateCommandEachStripe(parityUpdateCommandEachExtensionStripe[i]);
		}

		
		cout << "idea mds send command finish" << endl;
		ReceiveAck(ackNum);
        currentRound ++;
    }    

	K = largerK;
	chunkIndexs = newchunkIndexs;
	initialStripeNum = chunkIndexs.size();
	N = K + M ;
}

void idea_MDS(vector<int>& kPool){
	if (kPool.size() == 0)
	{
		cout << "no k in kPool" << endl;
	}
	iK = kPool[0];
	Simulation idea;
	struct timeval t_start;
	struct timeval t_end;
	double total_time = 0.0;

	double timeOverhead = 0.0;
	int placementTraffic;
	int kl = iK;
	for (int i = 1; i < kPool.size(); i++)
	{
		total_time = 0.0;
		gettimeofday(&t_start, NULL);
		idea.GetChunkStoreOrderInEachNode();
		idea.ideaScale(kPool[i],i-1);

		gettimeofday(&t_end, NULL);
		total_time = total_time + (t_end.tv_sec - t_start.tv_sec) * 1000000 + (t_end.tv_usec - t_start.tv_usec);
		total_time = total_time / 1000000;
		cout << "time: " << total_time << " s" << endl;

		cout << "----------------------------" << endl;
	}
}
//zf

void MDS(int methodType, vector<int>& kPool)
{
	cout << "it's mds" << endl;

	int tempInt;
	string myIp;

	GetHostAndIp(myIp);
	cout << "my ip = " << myIp << endl;
	tempInt = 0;
	for (int i = 0; i < metaDataServerIP.size(); i++)
	{
		if (myIp.compare(metaDataServerIP[i]) == 0)
		{
			tempInt = 1;
			break;
		}
	}
	if (tempInt == 0)
	{
		cout << "I'm not MDS" << endl;
		//exit(0);
	}

	if (methodType == ElASTIC_EC)
	{
		ElasticEC_MDS(kPool);
	}
	else if (methodType == BASELINE)
	{
		Baseline_MDS(kPool);
	}
	else if (methodType == ERS)
	{
		ERS_MDS(kPool);
	}
	else if (methodType == SRS)
	{
		SRS_MDS(kPool);
	}
	else{
		idea_MDS(kPool);
	}
}

void IdeaSendUpdateCommandEachStripe(vector<IdeaMdsUpdateParityCommand>& commandsEachStripe)
{
	int commandNum = commandsEachStripe.size();
	pthread_t sendThread[commandNum];
	vector<int> isThreadUsed;
	for (int i = 0; i < commandNum; i++)
	{
		isThreadUsed.push_back(1);
	}
	for (int i = 0; i < commandNum; i++)
	{
		//cout << "commandsEachStripe[" << i << "].port=" << commandsEachStripe[i].port << endl;
		if (commandsEachStripe[i].port == -1)
		{
			isThreadUsed[i] = 0;
			continue;
		}
		IdeaSendUpdateCommand(&commandsEachStripe[i]);
		//pthread_create(&sendThread[i], NULL, IdeaSendUpdateCommand, &commandsEachStripe[i]);
	}
	
}

void IdeaSendRelocationCommandEachStripe(vector<IdeaMdsRelocationCommand>& commandsEachStripe)
{
	int commandNum = commandsEachStripe.size();
	pthread_t sendThread[commandNum];
	for (int i = 0; i < commandNum; i++)
	{
		//pthread_create(&sendThread[i], NULL, IdeaSendRelocationCommand, &commandsEachStripe[i]);
		IdeaSendRelocationCommand(&commandsEachStripe[i]);
	}
	
}

