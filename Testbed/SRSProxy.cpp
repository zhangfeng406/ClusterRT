#include "SRSProxy.h"

void SRS_ProcessScaleData(SRSMdsDataCommand* command)
{
	cout << "SRSScaleData" << endl;

	int port = command->port;
	int relativeDataIndex = command->relativeDataIndex;
	int storeIndex = command->storeIndex;
	int newEncodingCoe = command->newEncodingCoe;
	int oldEncodingCoe = command->oldEncodingCoe;

	string ip;
	vector<SRSTransmitChunk> sendChunks;
	char* dataChunk = (char*)malloc(sizeof(char) * CHUNK_SIZE);

	if (port == -1)
	{
		cout << "SRS_ProcessScaleData: error port" << endl;
		exit(0);
	}

	sendChunks.resize(M);

	ReadChunk(dataChunk, storeIndex);
	//memset(dataChunk, 0, CHUNK_SIZE);

	// fill sendChunks' message
	for (int i = 0; i < M; i++)
	{
		ip = command->destinationIps[i];

		strcpy(sendChunks[i].destinationIp, ip.c_str());
		sendChunks[i].destinationPort = port;
		sendChunks[i].relativeDataIndex = relativeDataIndex;
		sendChunks[i].newCoefficient = newEncodingCoe;
		sendChunks[i].oldCoefficient = oldEncodingCoe;

		memcpy(sendChunks[i].chunkBuffer, dataChunk, CHUNK_SIZE);
	}

	// send chunks
	for (int i = 0; i < M; i++)
	{
		SRSChunkSender(&sendChunks[i]);
	}

	free(dataChunk);
	for (int i = 0; i < M; i++)
	{
		free(sendChunks[i].chunkBuffer);
	}

}

void SRS_ProcessScaleParity(SRSMdsParityCommand* command) // need send ack
{
	cout << "SRSScaleParity" << endl;

	int receiveNum = command->dataChunkNum_needReceive;
	int port = command->port;
	int socketId;
	int received = 0;

	int extensionStripeNum = 0;
	int storeIndex;
	int relativeDataIndex;
	int oldStripeIndex;
	int newStripeIndex;
	int newEncodingCoe;
	int oldEncodingCoe;
	int parityIndexInStripe = command->parityIndexInStripe;
	map<int, int> relativeDataIndex_map_oldStripe;
	map<int, int> relativeDataIndex_map_newStripe;
	vector<SingleChunk> parityChunks;

	char* tempChunk = (char*)malloc(sizeof(char) * CHUNK_SIZE);

	string user = "SRS_ProcessScaleParity";

	//vector<SRSTransmitChunk> receiveChunks;
	//receiveChunks.resize(receiveNum);

	SRSTransmitChunk receiveChunk;

	for (int i = 0; i < SRS_MAX_POST; i++)
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
		for (int j = 0; j < SRS_MAX_PARITY; j++)
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

	for (int i = 0; i < extensionStripeNum; i++)
	{
		for (int j = 0; j < SRS_MAX_DATA; j++)
		{
			if (command->oldDataIndexNeed_eachPostStripe[i][j] == -1)
			{
				break;
			}
			else
			{
				relativeDataIndex = command->oldDataIndexNeed_eachPostStripe[i][j];

				// find the index of the relativeDataIndex in received dataChunks 
				if (relativeDataIndex_map_oldStripe.find(relativeDataIndex) == relativeDataIndex_map_oldStripe.end())
				{
					relativeDataIndex_map_oldStripe[relativeDataIndex] = i;
				}
				else
				{
					cout << "SRS_ProcessScaleParity: a dataChunk appear in two old stripe" << endl;
					exit(0);
				}

			}
		}
	}

	for (int i = 0; i < extensionStripeNum; i++)
	{
		for (int j = 0; j < SRS_MAX_DATA; j++)
		{
			if (command->newDataIndexNeed_eachPostStripe[i][j] == -1)
			{
				break;
			}
			else
			{
				relativeDataIndex = command->newDataIndexNeed_eachPostStripe[i][j];

				// find the index of the relativeDataIndex in received dataChunks 
				if (relativeDataIndex_map_newStripe.find(relativeDataIndex) == relativeDataIndex_map_newStripe.end())
				{
					relativeDataIndex_map_newStripe[relativeDataIndex] = i;
				}
				else
				{
					cout << "SRS_ProcessScaleParity: a dataChunk appear in two new stripe" << endl;
					exit(0);
				}
			}
		}
	}

	socketId = InitialSocket(user, port);
	listen(socketId, receiveNum);

	while (received < receiveNum)
	{
		struct sockaddr_in acceptedAddress;
		int acceptedSocket;
		socklen_t accpetedAddrLen = sizeof(acceptedAddress);
		acceptedAddress.sin_family = AF_INET;

		cout << "SRS_ProcessScaleParity: wait receive dataChunks" << endl;
		acceptedSocket = accept(socketId, (struct sockaddr*)&acceptedAddress, &accpetedAddrLen);
		//receiveChunks[received].receiveSocketId = acceptedSocket;

		//SRSChunkReceiver(&receiveChunks[received]);

		receiveChunk.receiveSocketId = acceptedSocket;
		SRSChunkReceiver(&receiveChunk);

		received++;

		if (receiveChunk.relativeDataIndex < 0)
		{
			cout << "SRS_ProcessScaleParity: receiveChunk.relativeDataIndex < 0), error" << endl;
			exit(0);
		}

		relativeDataIndex = receiveChunk.relativeDataIndex;
		if (relativeDataIndex_map_oldStripe.find(relativeDataIndex) == relativeDataIndex_map_oldStripe.end())
		{

		}
		else
		{
			oldStripeIndex = relativeDataIndex_map_oldStripe[relativeDataIndex];
			oldEncodingCoe = receiveChunk.oldCoefficient;
			EncodingData(receiveChunk.chunkBuffer, tempChunk, oldEncodingCoe, parityIndexInStripe);
			aggregate_data(parityChunks[oldStripeIndex].chunkBuffer, 1, tempChunk);
		}
		if (relativeDataIndex_map_newStripe.find(relativeDataIndex) == relativeDataIndex_map_newStripe.end())
		{
			cout << "SRS_ProcessScaleParity: not find relativeDataIndex in new stripe" << endl;
			exit(0);
		}
		else
		{
			newStripeIndex = relativeDataIndex_map_newStripe[relativeDataIndex];
			newEncodingCoe = receiveChunk.newCoefficient;
			EncodingData(receiveChunk.chunkBuffer, tempChunk, newEncodingCoe, parityIndexInStripe);
			aggregate_data(parityChunks[newStripeIndex].chunkBuffer, 1, tempChunk);
		}

	}

	if (close(socketId) == -1)
	{
		cout << "SRS_ProcessScaleParity: close listen socket error" << endl;
		exit(0);
	}

	// write the new parity to the first pairty's location
	for (int i = 0; i < extensionStripeNum; i++)
	{
		storeIndex = command->parityChunk_eachPostStripe[i][0];

		WriteChunk(parityChunks[i].chunkBuffer, storeIndex);
	}

	free(receiveChunk.chunkBuffer);

	free(tempChunk);
	for (int i = 0; i < extensionStripeNum; i++)
	{
		free(parityChunks[i].chunkBuffer);
	}

	SendAckToMds(1);
}

void SRS_PlacementSender(SRSPlaceMentCommand* command)
{
	int port = command->port;
	int storeIndex = command->storeIndex;
	string ip;
	SRSTransmitChunk sendChunk;
	char* storedChunk = (char*)malloc(sizeof(char) * CHUNK_SIZE);

	if (port == -1)
	{
		cout << "SRS_PlacementSender: error port" << endl;
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
	SRSChunkSender(&sendChunk);

	free(storedChunk);

	free(sendChunk.chunkBuffer);
}

void SRS_PlacementReceiver(SRSPlaceMentCommand* command)
{
	int receiveNum = command->receiveNum;
	int port = command->port;
	int socketId;
	int received = 0;
	string user = "SRS_PlacementReceiver";

	vector<SRSTransmitChunk> receiveChunks;
	receiveChunks.resize(receiveNum);
	if (receiveNum != 1)
	{
		cout << "SRS_PlacementReceiver: receiveNum != 1, error" << endl;
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

		cout << "SRS_PlacementReceiver: wait receive dataChunks" << endl;
		acceptedSocket = accept(socketId, (struct sockaddr*)&acceptedAddress, &accpetedAddrLen);
		receiveChunks[received].receiveSocketId = acceptedSocket;

		SRSChunkReceiver(&receiveChunks[received]);

		received++;
	}

	if (close(socketId) == -1)
	{
		cout << "SRS_PlacementReceiver: close listen socket error" << endl;
		exit(0);
	}

	int writeIndex = command->writeIndex;

	WriteChunk(receiveChunks.front().chunkBuffer, writeIndex);
	free(receiveChunks.front().chunkBuffer);

	SendAckToMds(1);
}

void SRS_ProcessPlacement(SRSPlaceMentCommand* command)
{
	if (command->role == SRS_PLACEMENT_SENDER)
	{
		cout << "SRS Placement Sender" << endl;
		SRS_PlacementSender(command);
	}
	else if (command->role == SRS_PLACEMENT_RECEIVER)
	{
		cout << "SRS Placement Receiver" << endl;
		SRS_PlacementReceiver(command);
		//SendAckToMds(1);
	}
	else
	{
		cout << "SRS_ProcessPlacement: error role" << endl;
		exit(0);
	}
}

void* SRSWork(void* pointer)
{
	//pthread_detach(pthread_self());
	//cout << "SRSWork" << endl;
	ProxyThreadSet* threadInfo = (ProxyThreadSet*)pointer;
	int socketId = threadInfo->socketId;
	char* clientIp;

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
	if (totalReceiveSize == sizeof(SRSMdsDataCommand))
	{
		SRSMdsDataCommand* command = (SRSMdsDataCommand*)malloc(totalReceiveSize);
		memcpy(command, receiveBuffer, totalReceiveSize);
		SRS_ProcessScaleData(command);
		free(command);
	}
	else if (totalReceiveSize == sizeof(SRSMdsParityCommand))
	{
		SRSMdsParityCommand* command = (SRSMdsParityCommand*)malloc(totalReceiveSize);
		memcpy(command, receiveBuffer, totalReceiveSize);
		SRS_ProcessScaleParity(command);
		free(command);
	}
	else if (totalReceiveSize == sizeof(SRSPlaceMentCommand))
	{
		SRSPlaceMentCommand* command = (SRSPlaceMentCommand*)malloc(totalReceiveSize);
		memcpy(command, receiveBuffer, totalReceiveSize);
		SRS_ProcessPlacement(command);
		free(command);
	}
	else
	{
		cout << "SRSWork: error totalReceiveSize" << endl;
		exit(0);
	}



	if (close(threadInfo->socketId) == -1)
	{
		cout << "SRSWork: close socket error" << endl;
		exit(0);
	}
	threadInfo->socketId = -1;
	free(receiveBuffer);
	free(receiveHeadBuffer);
}

void ProxySRS()
{
	cout << "I'm proxy" << endl;
	int socketId;
	int indexOfThreadSet = 0;
	string user = "ProxySRS";

	encodingMatrix = GetEncodingMatrix(KMax, M, W);

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
		pthread_create(&(threadSet[indexOfThreadSet].threadId), NULL, SRSWork, &threadSet[indexOfThreadSet]);
		pthread_detach(threadSet[indexOfThreadSet].threadId);

		indexOfThreadSet = (indexOfThreadSet + 1) % PROXY_THREAD_MAX_NUM;
	}
}