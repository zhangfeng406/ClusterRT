#include "BaselineProxy.h"

void Base_DataChunk(BaseMdsUpdateParityCommand& command)
{
	int parityIndex;
	int dataIndexInNewStripe = command.dataIndexInNewStripe;
	string ip;
	string myIp;
	vector<BaseTransmitChunk> sendChunks;
	char* dataChunk = (char*)malloc(sizeof(char) * CHUNK_SIZE);

	sendChunks.resize(M);

	ReadChunk(dataChunk, command.storeIndex);
	//memset(dataChunk, 0, CHUNK_SIZE);

	GetHostAndIp(myIp);
	for (int i = 0; i < M; i++)
	{
		parityIndex = i;
		ip = command.destinationIps[i];
		if (ip.compare(myIp) == 0)
		{
			cout << "DATA_CHUNK: destinationIp = myIp, error" << endl;
			exit(0);
		}
		if (ip.empty())
		{
			// if this node is a pairtyNode
			int parityStoreIndex = command.parityStoreIndex;
			if (parityStoreIndex == -1)
			{
				cout << "Base_DataChunk: I'm parityNode and hold dataChunk in new stripe, but parityStoreIndex=-1" << endl;
				exit(0);
			}
			char* parityChunk = (char*)malloc(sizeof(char) * CHUNK_SIZE);

			ReadChunk(parityChunk, parityStoreIndex);
			//memset(parityChunk, 0, CHUNK_SIZE);

			EncodingData(dataChunk, sendChunks[i].chunkBuffer, dataIndexInNewStripe, parityIndex);

			aggregate_data(parityChunk, 1, sendChunks[i].chunkBuffer);

			WriteChunk(parityChunk, parityStoreIndex);
			//memset(parityChunk, 0, CHUNK_SIZE);
		}
		else
		{
			EncodingData(dataChunk, sendChunks[i].chunkBuffer, dataIndexInNewStripe, parityIndex);
			strcpy(sendChunks[i].destinationIp, ip.c_str());
			sendChunks[i].destinationPort = command.port;
		}
	}

	// send parity delta chunks
	for (int i = 0; i < M; i++)
	{
		ip = command.destinationIps[i];
		if (ip.compare(myIp) == 0)
		{
			cout << "DATA_CHUNK: destinationIp = myIp, error" << endl;
			exit(0);
		}
		if (ip.empty())
		{
			continue;
		}
		else
		{
			BaseChunkSender(&sendChunks[i]);
		}
	}

	free(dataChunk);
	for (int i = 0; i < M; i++)
	{
		free(sendChunks[i].chunkBuffer);
	}

	SendAckToMds(1);
}

void Base_ParityChunk(BaseMdsUpdateParityCommand& command)
{
	int receiveNum = command.receiveNum;
	int port = command.port;
	int socketId;
	int received = 0;
	char* parityChunk = (char*)malloc(sizeof(char) * CHUNK_SIZE);
	string user = "Base_ParityChunk";

	vector<BaseTransmitChunk> receiveChunks;
	receiveChunks.resize(receiveNum);

	socketId = InitialSocket(user, port);
	listen(socketId, receiveNum);

	while (received < receiveNum)
	{
		struct sockaddr_in acceptedAddress;
		int acceptedSocket;
		socklen_t accpetedAddrLen = sizeof(acceptedAddress);
		acceptedAddress.sin_family = AF_INET;

		cout << "wait receive parityDelta" << endl;
		acceptedSocket = accept(socketId, (struct sockaddr*)&acceptedAddress, &accpetedAddrLen);
		receiveChunks[received].receiveSocketId = acceptedSocket;
		
		BaseChunkReceiver(&receiveChunks[received]);

		// create a thread
		//pthread_create(&(threadIds[indexOfreceiveChunks]), NULL, ChunkReceiver, &receiveChunks[indexOfreceiveChunks]);

		received++;
	}

	ReadChunk(parityChunk, command.storeIndex);
	//memset(parityChunk, 0, CHUNK_SIZE);

	for (int i = 0; i < receiveNum; i++)
	{
		aggregate_data(parityChunk, 1, receiveChunks[i].chunkBuffer);
	}

	WriteChunk(parityChunk, command.storeIndex);
	//memset(parityChunk, 0, CHUNK_SIZE);
	
	free(parityChunk);
	for (int i = 0; i < receiveNum; i++)
	{
		free(receiveChunks[i].chunkBuffer);
	}

	SendAckToMds(1);

	if (close(socketId) == -1)
	{
		cout << "PARITY_CHUNK: close listen socket error" << endl;
		exit(0);
	}
}

void Base_ProcessUpdate(BaseMdsUpdateParityCommand& command)
{
	if (command.role == DATA_CHUNK)
	{
		cout << "I'm dataChunk" << endl;
		Base_DataChunk(command);
		//SendAckToMds(1);
	}
	else if (command.role == PARITY_CHUNK)
	{
		cout << "I'm parityChunk" << endl;
		Base_ParityChunk(command);
		//SendAckToMds(1);
	}
	else
	{
		cout << "error update role" << endl;
	}
}

void Base_RelocationSender(BaseMdsRelocationCommand& command)
{
	BaseTransmitChunk sendChunk;
	char* dataChunk = (char*)malloc(sizeof(char) * CHUNK_SIZE);
	
	
	ReadChunk(dataChunk, command.dataStoreIndex);
	//memset(dataChunk, 0, CHUNK_SIZE);

	memcpy(sendChunk.chunkBuffer, dataChunk, CHUNK_SIZE);
	memcpy(sendChunk.destinationIp, command.destinationIp, IP_LENGTH);
	sendChunk.destinationPort = command.port;

	BaseChunkSender(&sendChunk);

	free(dataChunk);
	free(sendChunk.chunkBuffer);
}

void Base_RelocationReceiver(BaseMdsRelocationCommand& command)
{
	int receiveNum = command.receiveNum;
	int port = command.port;
	int socketId;
	int received = 0;
	int writeIndex = command.writeIndex;

	string user = "Base_RelocationReceiver";
	BaseTransmitChunk receiveChunk;

	if (receiveNum != 1)
	{
		cout << "Base_RelocationReceiver: receiveNum != 1" << endl;
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

		cout << "wait receive relocation chunk" << endl;
		acceptedSocket = accept(socketId, (struct sockaddr*)&acceptedAddress, &accpetedAddrLen);
		receiveChunk.receiveSocketId = acceptedSocket;

		BaseChunkReceiver(&receiveChunk);

		received++;
	}

	
	WriteChunk(receiveChunk.chunkBuffer, writeIndex);
	//memset(receiveChunk.chunkBuffer, 0, CHUNK_SIZE);

	free(receiveChunk.chunkBuffer);

	SendAckToMds(1);

	if (close(socketId) == -1)
	{
		cout << "Base_RelocationReceiver: close listen socket error" << endl;
		exit(0);
	}
}

void Base_ProcessRelocation(BaseMdsRelocationCommand& command)
{
	if (command.role == RELOCATION_SENDER)
	{
		cout << "I'm relocation sender" << endl;
		Base_RelocationSender(command);
		//SendAckToMds(1);
	}
	else if (command.role == RELOCATION_RECEIVER)
	{
		cout << "I'm relocation receiver" << endl;
		Base_RelocationReceiver(command);
		//SendAckToMds(1);
	}
	else
	{
		cout << "error relocation role" << endl;
	}
}

void* BaselineWork(void* pointer)
{
	//pthread_detach(pthread_self());
	//cout << "Elastic work" << endl;
	ProxyThreadSet* threadInfo = (ProxyThreadSet*)pointer;
	int socketId = threadInfo->socketId;
	char* clientIp;

	BaseMdsUpdateParityCommand updateCommand;
	BaseMdsRelocationCommand relocationCommand;
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
	if (totalReceiveSize == sizeof(BaseMdsUpdateParityCommand))
	{
		memcpy(&updateCommand, receiveBuffer, sizeof(BaseMdsUpdateParityCommand));
		Base_ProcessUpdate(updateCommand);
	}
	else if (totalReceiveSize == sizeof(BaseMdsRelocationCommand))
	{
		memcpy(&relocationCommand, receiveBuffer, sizeof(BaseMdsRelocationCommand));
		Base_ProcessRelocation(relocationCommand);
	}


	if (close(threadInfo->socketId) == -1)
	{
		cout << "BaselineWork: close socket error" << endl;
		exit(0);
	}
	threadInfo->socketId = -1;
	free(receiveBuffer);
	free(receiveHeadBuffer);
}

void ProxyBaseline()
{
	cout << "I'm proxy" << endl;
	int socketId;
	int indexOfThreadSet = 0;
	string user = "ProxyBaseline";

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
		pthread_create(&(threadSet[indexOfThreadSet].threadId), NULL, BaselineWork, &threadSet[indexOfThreadSet]);
		pthread_detach(threadSet[indexOfThreadSet].threadId);

		indexOfThreadSet = (indexOfThreadSet + 1) % PROXY_THREAD_MAX_NUM;
	}
}