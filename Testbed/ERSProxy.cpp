#include "ERSProxy.h"

void ERS_ReupdateData(ERSReupdateParityCommand* command)
{
	int port = command->port;
	int newNodeId = command->newNodeId;
	int oldNodeId = command->oldNodeId;
	int dataStoreIndex = command->dataStoreIndex;
	string ip;
	vector<ERSTransmitChunk> sendChunks;
	char* dataChunk = (char*)malloc(sizeof(char) * CHUNK_SIZE);

	if (port == -1)
	{
		cout << "ERS_ReupdateData: error port" << endl;
		exit(0);
	}

	sendChunks.resize(M);

	ReadChunk(dataChunk, dataStoreIndex);
	//memset(dataChunk, 0, CHUNK_SIZE);

	// fill sendChunks' message
	for (int i = 0; i < M; i++)
	{
		ip = command->destinationIps[i];
		strcpy(sendChunks[i].destinationIp, ip.c_str());
		sendChunks[i].destinationPort = port;

		// for reupdate
		sendChunks[i].newCoefficient = ERS_GetCoefficient(newNodeId, i);
		sendChunks[i].oldCoefficient = ERS_GetCoefficient(oldNodeId, i);

		memcpy(sendChunks[i].chunkBuffer, dataChunk, CHUNK_SIZE);
	}

	// send chunks
	for (int i = 0; i < M; i++)
	{
		ERSChunkSender(&sendChunks[i]);
	}

	free(dataChunk);
	for (int i = 0; i < M; i++)
	{
		free(sendChunks[i].chunkBuffer);
	}

}

void ERS_ReupdateParity(ERSReupdateParityCommand* command)// need send ack
{
	int receiveNum = command->receiveNum;
	int port = command->port;
	int socketId;
	int received = 0;

	int parityStoreIndex = command->parityStoreIndex;
	int newCoefficient;
	int oldCoefficient;
	char* parityChunk = (char*)malloc(sizeof(char) * CHUNK_SIZE);
	char* encodedChunk = (char*)malloc(sizeof(char) * CHUNK_SIZE);

	string user = "ERS_ReupdateParity";

	ERSTransmitChunk receiveChunk;

	//vector<ERSTransmitChunk> receiveChunks;
	//receiveChunks.resize(receiveNum);

	ReadChunk(parityChunk, parityStoreIndex);
	//memset(parityChunk, 0, CHUNK_SIZE);

	socketId = InitialSocket(user, port);
	listen(socketId, receiveNum);

	while (received < receiveNum)
	{
		struct sockaddr_in acceptedAddress;
		int acceptedSocket;
		socklen_t accpetedAddrLen = sizeof(acceptedAddress);
		acceptedAddress.sin_family = AF_INET;

		cout << "ERS_ReupdateParity: wait receive dataChunks" << endl;
		acceptedSocket = accept(socketId, (struct sockaddr*)&acceptedAddress, &accpetedAddrLen);
		receiveChunk.receiveSocketId = acceptedSocket;

		ERSChunkReceiver(&receiveChunk);

		received++;

		if (receiveChunk.newCoefficient == -1)
		{
			cout << "ERS_ReupdateParity: receiveChunk.newCoefficient = -1, error" << endl;
			exit(0);
		}
		if (receiveChunk.oldCoefficient == -1)
		{
			cout << "ERS_ReupdateParity: receiveChunk.oldCoefficient = -1, error" << endl;
			exit(0);
		}

		newCoefficient = receiveChunk.newCoefficient;
		oldCoefficient = receiveChunk.oldCoefficient;

		// delete the old
		ERS_EncodingData(receiveChunk.chunkBuffer, encodedChunk, oldCoefficient);
		aggregate_data(parityChunk, 1, encodedChunk);
		// add the new
		ERS_EncodingData(receiveChunk.chunkBuffer, encodedChunk, newCoefficient);
		aggregate_data(parityChunk, 1, encodedChunk);

	}

	if (close(socketId) == -1)
	{
		cout << "ERS_ReupdateParity: close listen socket error" << endl;
		exit(0);
	}


	// write the new parity chunk
	WriteChunk(parityChunk, parityStoreIndex);

	free(receiveChunk.chunkBuffer);

	free(parityChunk);
	free(encodedChunk);

	SendAckToMds(1);
}

void ERS_ProcessReupdate(ERSReupdateParityCommand* command)
{
	if (command->role == ERS_REUPDATE_DATA)
	{
		cout << "ERS Reupdate Data" << endl;
		ERS_ReupdateData(command);
	}
	else if (command->role == ERS_REUPDATE_PARITY)
	{
		cout << "ERS Reupdate Parity" << endl;
		ERS_ReupdateParity(command);
		//SendAckToMds(1);
	}
	else
	{
		cout << "ERS_ProcessReupdate: error role" << endl;
		exit(0);
	}
}

void ERS_PlacementSender(ERSPlaceMentCommand* command)
{
	int port = command->port;
	int storeIndex = command->storeIndex;
	string ip;
	ERSTransmitChunk sendChunk;
	char* storedChunk = (char*)malloc(sizeof(char) * CHUNK_SIZE);

	if (port == -1)
	{
		cout << "ERS_PlacementSender: error port" << endl;
		exit(0);
	}

	ReadChunk(storedChunk, storeIndex);
	//memset(storedChunk, 0, CHUNK_SIZE);

	// fill sendChunks' message
	ip = command->destinationIp;
	strcpy(sendChunk.destinationIp, ip.c_str());
	sendChunk.destinationPort = port;
	memcpy(sendChunk.chunkBuffer, storedChunk, CHUNK_SIZE);

	// send chunk
	ERSChunkSender(&sendChunk);

	free(storedChunk);

	free(sendChunk.chunkBuffer);

}

void ERS_PlacementReceiver(ERSPlaceMentCommand* command) // need send ack
{
	int receiveNum = command->receiveNum;
	int port = command->port;
	int socketId;
	int received = 0;
	string user = "ERS_PlacementReceiver";

	vector<ERSTransmitChunk> receiveChunks;
	receiveChunks.resize(receiveNum);
	if (receiveNum != 1)
	{
		cout << "ERS_PlacementReceiver: receiveNum != 1, error" << endl;
		exit(0);
	}

	socketId = InitialSocket(user, port);
	listen(socketId, receiveNum);

	while (received < receiveNum)
	{
		struct sockaddr_in acceptedAddress;
		int acceptedSocket;
		socklen_t accpetedAddrLen = sizeof(acceptedAddress);
		acceptedAddress.sin_family = AF_INET;

		cout << "ERS_PlacementReceiver: wait receive dataChunks" << endl;
		acceptedSocket = accept(socketId, (struct sockaddr*)&acceptedAddress, &accpetedAddrLen);
		receiveChunks[received].receiveSocketId = acceptedSocket;

		ERSChunkReceiver(&receiveChunks[received]);

		received++;
	}

	if (close(socketId) == -1)
	{
		cout << "ERS_PlacementReceiver: close listen socket error" << endl;
		exit(0);
	}

	int writeIndex = command->writeIndex;
	
	WriteChunk(receiveChunks.front().chunkBuffer, writeIndex);
	free(receiveChunks.front().chunkBuffer);

	SendAckToMds(1);
}

void ERS_ProcessPlacement(ERSPlaceMentCommand* command)
{
	if (command->role == ERS_PLACEMENT_SENDER)
	{
		cout << "ERS Placement Sender" << endl;
		ERS_PlacementSender(command);
	}
	else if (command->role == ERS_PLACEMENT_RECEIVER)
	{
		cout << "ERS Placement Receiver" << endl;
		ERS_PlacementReceiver(command);
		//SendAckToMds(1);
	}
	else
	{
		cout << "ERS_ProcessPlacement: error role" << endl;
		exit(0);
	}
}

void ERS_ProcessScaleData(ERSMdsDataCommand* command)
{
	cout << "ERSScaleData" << endl;

	int port = command->port;
	int relativeDataIndex = command->relativeDataIndex;
	int nodeId = command->nodeId;
	string ip;
	vector<ERSTransmitChunk> sendChunks;
	char* dataChunk = (char*)malloc(sizeof(char) * CHUNK_SIZE);
	char* encodedChunk = (char*)malloc(sizeof(char) * CHUNK_SIZE);

	if (port == -1)
	{
		cout << "ERS_ProcessScaleData: error port" << endl;
		exit(0);
	}

	sendChunks.resize(M);

	ReadChunk(dataChunk, command->storeIndex);
	//memset(dataChunk, 0, CHUNK_SIZE);

	// fill sendChunks' message
	for (int i = 0; i < M; i++)
	{
		ip = command->destinationIps[i];
		strcpy(sendChunks[i].destinationIp, ip.c_str());
		sendChunks[i].destinationPort = port;
		sendChunks[i].relativeDataIndex = relativeDataIndex;

		// encoding dataChunk
		ERS_EncodingData(dataChunk, encodedChunk, nodeId, i);

		memcpy(sendChunks[i].chunkBuffer, encodedChunk, CHUNK_SIZE);
	}

	// send chunks
	for (int i = 0; i < M; i++)
	{
		ERSChunkSender(&sendChunks[i]);
	}

	free(dataChunk);
	free(encodedChunk);
	for (int i = 0; i < M; i++)
	{
		free(sendChunks[i].chunkBuffer);
	}

}

void ERS_ProcessScaleParity(ERSMdsParityCommand* command) // need send ack
{
	cout << "ERSScaleParity" << endl;

	int receiveNum = command->dataChunkNum_needReceive;
	int port = command->port;
	int socketId;
	int received = 0;
	string user = "ERS_ProcessScaleParity";

	vector<ERSTransmitChunk> receiveChunks;
	receiveChunks.resize(receiveNum);

	socketId = InitialSocket(user, port);
	listen(socketId, receiveNum);

	while (received < receiveNum)
	{
		struct sockaddr_in acceptedAddress;
		int acceptedSocket;
		socklen_t accpetedAddrLen = sizeof(acceptedAddress);
		acceptedAddress.sin_family = AF_INET;

		cout << "ERS_ProcessScaleParity: wait receive dataChunks" << endl;
		acceptedSocket = accept(socketId, (struct sockaddr*)&acceptedAddress, &accpetedAddrLen);
		receiveChunks[received].receiveSocketId = acceptedSocket;

		ERSChunkReceiver(&receiveChunks[received]);

		received++;
	}

	if (close(socketId) == -1)
	{
		cout << "ERS_ProcessScaleParity: close listen socket error" << endl;
		exit(0);
	}

	// check is the receiveChunks correct
	for (int i = 0; i < receiveNum; i++)
	{
		if (receiveChunks[i].relativeDataIndex == -1)
		{
			cout << "ERS_ProcessScaleParity: receiveChunks[i].relativeDataIndex=-1, error" << endl;
			exit(0);
		}
	}

	int extensionStripeNum = 0;
	int storeIndex;
	int relativeDataIndex;
	int receiveChunksId;
	char* tempChunk = (char*)malloc(sizeof(char) * CHUNK_SIZE);
	map<int, int> relativeDataIndex_map_indexInReceive;
	vector<SingleChunk> parityChunks;

	for (int i = 0; i < receiveNum; i++)
	{
		relativeDataIndex = receiveChunks[i].relativeDataIndex;
		relativeDataIndex_map_indexInReceive[relativeDataIndex] = i;
	}

	for (int i = 0; i < ERS_MAX_POST; i++)
	{
		if (command->parityChunk_eachPostStripe[i][0] == -1)
		{
			break;
		}
		else
		{
			extensionStripeNum++;
		}
	}

	parityChunks.resize(extensionStripeNum);

	// add parities for each extension stripes
	for (int i = 0; i < extensionStripeNum; i++)
	{
		for (int j = 0; j < ERS_MAX_PARITY; j++)
		{
			if (command->parityChunk_eachPostStripe[i][j] == -1)
			{
				break;
			}
			storeIndex = command->parityChunk_eachPostStripe[i][j];

			ReadChunk(tempChunk, storeIndex);
			//memset(tempChunk, 0, CHUNK_SIZE);
			aggregate_data(parityChunks[i].chunkBuffer, 1, tempChunk);
		}
	}

	// update parities for each extension stripes with dataChunks
	for (int i = 0; i < extensionStripeNum; i++)
	{
		for (int j = 0; j < ERS_MAX_DATA; j++)
		{
			if (command->relativeDataIndex_eachPostStripe[i][j] == -1)
			{
				break;
			}
			else
			{
				relativeDataIndex = command->relativeDataIndex_eachPostStripe[i][j];

				// find the index of the relativeDataIndex in received dataChunks 
				if (relativeDataIndex_map_indexInReceive.find(relativeDataIndex) == relativeDataIndex_map_indexInReceive.end())
				{
					cout << "ERS_ProcessScaleParity: not find relativeDataIndex, error" << endl;
					exit(0);
				}
				else
				{
					receiveChunksId = relativeDataIndex_map_indexInReceive[relativeDataIndex];
				}

				if (receiveChunks[receiveChunksId].relativeDataIndex != relativeDataIndex)
				{
					cout << "ERS_ProcessScaleParity: receiveChunks[receiveChunksId].relativeDataIndex != relativeDataIndex, error" << endl;
					exit(0);
				}

				// aggregate chunk
				aggregate_data(parityChunks[i].chunkBuffer, 1, receiveChunks[receiveChunksId].chunkBuffer);
			}
		}
	}

	// write the new parity to the first pairty's location
	for (int i = 0; i < extensionStripeNum; i++)
	{
		storeIndex = command->parityChunk_eachPostStripe[i][0];

		WriteChunk(parityChunks[i].chunkBuffer, storeIndex);
	}

	for (int i = 0; i < receiveNum; i++)
	{
		free(receiveChunks[i].chunkBuffer);
	}
	free(tempChunk);
	for (int i = 0; i < extensionStripeNum; i++)
	{
		free(parityChunks[i].chunkBuffer);
	}

	SendAckToMds(1);

}

void* ERSWork(void* pointer)
{
	//pthread_detach(pthread_self());
	//cout << "ERSWork" << endl;
	ProxyThreadSet* threadInfo = (ProxyThreadSet*)pointer;
	int socketId = threadInfo->socketId;
	char* clientIp;

	//BaseMdsUpdateParityCommand updateCommand;
	//BaseMdsRelocationCommand relocationCommand;

	clientIp = inet_ntoa(threadInfo->address.sin_addr);
	//cout << "connect IP = " << clientIp << endl;

	int headSize = sizeof(int);
	int totalReceiveSize = -1;
	int readReturn;
	int receivedLength = 0;
	char* receiveHeadBuffer = (char*)malloc(headSize);

	// read head(head = the totalSize need read)
	readReturn = read(socketId, receiveHeadBuffer, headSize);
	memcpy(&totalReceiveSize, receiveHeadBuffer, sizeof(int));

	char* receiveBuffer = (char*)malloc(totalReceiveSize);
	memcpy(receiveBuffer, receiveHeadBuffer, readReturn);
	receivedLength += readReturn;

	while (receivedLength < totalReceiveSize)
	{
		readReturn = read(socketId, receiveBuffer + receivedLength, totalReceiveSize - receivedLength);
		receivedLength += readReturn;
	}

	// deal with command
	if (totalReceiveSize == sizeof(ERSMdsDataCommand))
	{
		ERSMdsDataCommand* command = (ERSMdsDataCommand*)malloc(totalReceiveSize);
		memcpy(command, receiveBuffer, totalReceiveSize);
		ERS_ProcessScaleData(command);
		free(command);
	}
	else if (totalReceiveSize == sizeof(ERSMdsParityCommand))
	{
		ERSMdsParityCommand* command = (ERSMdsParityCommand*)malloc(totalReceiveSize);
		memcpy(command, receiveBuffer, totalReceiveSize);
		ERS_ProcessScaleParity(command);
		free(command);
	}
	else if (totalReceiveSize == sizeof(ERSReupdateParityCommand))
	{
		ERSReupdateParityCommand* command = (ERSReupdateParityCommand*)malloc(totalReceiveSize);
		memcpy(command, receiveBuffer, totalReceiveSize);
		ERS_ProcessReupdate(command);
		free(command);
	}
	else if (totalReceiveSize == sizeof(ERSPlaceMentCommand))
	{
		ERSPlaceMentCommand* command = (ERSPlaceMentCommand*)malloc(totalReceiveSize);
		memcpy(command, receiveBuffer, totalReceiveSize);
		ERS_ProcessPlacement(command);
		free(command);
	}
	else
	{
		cout << "ERSWork: error totalReceiveSize" << endl;
		exit(0);
	}



	if (close(threadInfo->socketId) == -1)
	{
		cout << "ERSWork: close socket error" << endl;
		exit(0);
	}
	threadInfo->socketId = -1;
	free(receiveBuffer);
	free(receiveHeadBuffer);
}

void ProxyERS()
{
	cout << "I'm proxy" << endl;
	int socketId;
	int indexOfThreadSet = 0;
	string user = "ProxyERS";

	ersEncodingMatrix = GetEncodingMatrix(ersKMax, M, W);

	vector<ProxyThreadSet> threadSet;
	threadSet.resize(PROXY_THREAD_MAX_NUM);
	socketId = InitialSocket(user, PROXY_LISTEN_PORT);
	listen(socketId, 100);

	while (true)
	{
		struct sockaddr_in acceptedAddress;
		int acceptedSocket;
		socklen_t accpetedAddrLen = sizeof(acceptedAddress);
		acceptedAddress.sin_family = AF_INET;

		cout << "wait accept" << endl;
		acceptedSocket = accept(socketId, (struct sockaddr*)&acceptedAddress, &accpetedAddrLen);

		// find an idle thread
		while (threadSet[indexOfThreadSet].socketId != -1)
		{
			indexOfThreadSet = (indexOfThreadSet + 1) % PROXY_THREAD_MAX_NUM;
		}

		threadSet[indexOfThreadSet].socketId = acceptedSocket;
		memcpy(&threadSet[indexOfThreadSet].address, &acceptedAddress, accpetedAddrLen);

		// create a thread
		pthread_create(&(threadSet[indexOfThreadSet].threadId), NULL, ERSWork, &threadSet[indexOfThreadSet]);
		pthread_detach(threadSet[indexOfThreadSet].threadId);

		indexOfThreadSet = (indexOfThreadSet + 1) % PROXY_THREAD_MAX_NUM;
	}
}
