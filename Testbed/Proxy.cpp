#include "Proxy.h"

void ElaticEC_DataChunk(IdeaMdsUpdateParityCommand& command)
{
	TransmitChunk chunkNeedSend;
	char* dataChunk = (char*)malloc(sizeof(char) * CHUNK_SIZE);

	ReadChunk(dataChunk, command.dataStoreIndex);
	//memset(dataChunk, 0, CHUNK_SIZE);

	memcpy(chunkNeedSend.chunkBuffer, dataChunk, sizeof(char) * CHUNK_SIZE);
	memcpy(chunkNeedSend.destinationIp, command.destinationIp[0], IP_LENGTH);
	chunkNeedSend.sender = DATA_CHUNK;
	chunkNeedSend.destinationPort = command.port;
	chunkNeedSend.dataIndexInNewStripe = command.dataIndexInNewStripe;

	ChunkSender(&chunkNeedSend);

	free(chunkNeedSend.chunkBuffer);
	free(dataChunk);
}

void ElaticEC_ParityChunk(IdeaMdsUpdateParityCommand& command)
{
	int receiveNum = command.receiveNum;
	int port = command.port;
	int socketId;
	int indexOfreceiveChunks = 0;
	char* parityChunk = (char*)malloc(sizeof(char) * CHUNK_SIZE);
	string user = "PARITY_CHUNK";

	//vector<pthread_t> threadIds;
	vector<ReceiveChunk> receiveChunks;

	//threadIds.resize(receiveNum);
	receiveChunks.resize(receiveNum);

	ReadChunk(parityChunk, command.parityStoreIndex);
	//memset(parityChunk, 0, sizeof(char) * CHUNK_SIZE);


	socketId = InitialSocket(user, port);

	listen(socketId, receiveNum);

	while (indexOfreceiveChunks < receiveNum)
	{
		struct sockaddr_in acceptedAddress;
		int acceptedSocket;
		socklen_t accpetedAddrLen = sizeof(acceptedAddress);
		acceptedAddress.sin_family = AF_INET;

		cout << "wait receive parityDelta" << endl;
		acceptedSocket = accept(socketId, (struct sockaddr*)&acceptedAddress, &accpetedAddrLen);
		receiveChunks[indexOfreceiveChunks].socketId = acceptedSocket;
		memcpy(&receiveChunks[indexOfreceiveChunks].address, &acceptedAddress, accpetedAddrLen);

		ChunkReceiver(&receiveChunks[indexOfreceiveChunks]);
		// create a thread
		//pthread_create(&(threadIds[indexOfreceiveChunks]), NULL, ChunkReceiver, &receiveChunks[indexOfreceiveChunks]);

		indexOfreceiveChunks++;
	}
	//for (int i = 0; i < receiveNum; i++)
	//{
	//	pthread_join(threadIds[i], NULL);
	//}

	for (int i = 0; i < receiveNum; i++)
	{
		if (receiveChunks[i].dataIndexInNewStripe != -1)
		{
			cout << "parityChunk: receive dataIndexInNewStripe !=-1" << endl;
			exit(0);
		}
		aggregate_data(parityChunk, 1, receiveChunks[i].chunkBuffer);
	}

	WriteChunk(parityChunk, command.parityStoreIndex);

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

void ElaticEC_CollectNode(IdeaMdsUpdateParityCommand& command)
{
	int receiveNum = command.receiveNum;
	int port = command.port;
	int socketId;
	int indexOfreceiveChunks = 0;
	int dataIndexInStripe;
	int parityIndexInstripe;
	int K;
	
	int totalDataChunkNum = receiveNum + 1;	
	//int threadIndex = 0;
	int storeIndex;
	char* parityChunk = (char*)malloc(sizeof(char) * CHUNK_SIZE);
	int* erasures = (int*)malloc(sizeof(int) * (totalDataChunkNum + M));	//k+m
	char** dataBlocks = (char**)malloc(sizeof(char*) * totalDataChunkNum);	//k
 	char** parityBlocks = (char**)malloc(sizeof(char*) * M);	//m
	for(int i = totalDataChunkNum - M ; i < totalDataChunkNum ; i++){	//k-m -> k
		dataBlocks[i] = (char*)malloc(sizeof(char) * CHUNK_SIZE);
		memset(dataBlocks[i] , 0, sizeof(char) * CHUNK_SIZE);
	}
	K = totalDataChunkNum;
	string destinationIp;
	string myIp;
	string user = "COLLECT_NODE";

	//vector<pthread_t> threadIds;
	vector<ReceiveChunk> receiveChunks;
	vector<TransmitChunk> chunksNeedSend;
	vector<ParityDelta> parityDeltas;

	//threadIds.resize(receiveNum);
	receiveChunks.resize(totalDataChunkNum);
	//chunksNeedSend.resize(M);
	parityDeltas.resize(totalDataChunkNum);


	
	ReadChunk(parityChunk, command.parityStoreIndex);
	parityBlocks[0] =  parityChunk;

	socketId = InitialSocket(user, port);

	listen(socketId, receiveNum);
	
	while (indexOfreceiveChunks < receiveNum)
	{
		struct sockaddr_in acceptedAddress;
		int acceptedSocket;
		socklen_t accpetedAddrLen = sizeof(acceptedAddress);
		acceptedAddress.sin_family = AF_INET;

		cout << "collectNode: wait receive dataChunks and other parityChunks" << endl;
		acceptedSocket = accept(socketId, (struct sockaddr*)&acceptedAddress, &accpetedAddrLen);
		receiveChunks[indexOfreceiveChunks].socketId = acceptedSocket;
		memcpy(&receiveChunks[indexOfreceiveChunks].address, &acceptedAddress, accpetedAddrLen);

		ChunkReceiver(&receiveChunks[indexOfreceiveChunks]);
		// create a thread
		//pthread_create(&(threadIds[indexOfreceiveChunks]), NULL, ChunkReceiver, &receiveChunks[indexOfreceiveChunks]);
		indexOfreceiveChunks++;
	}
	
	for (int i = 0; i < receiveNum; i++)
	{
		if (receiveChunks[i].dataIndexInNewStripe == -1)
		{
			cout << "collectNode: receive dataIndexInNewStripe = -1" << endl;
			exit(0);
		}
		//aggregate_data(parityChunk, 1, receiveChunks[i].chunkBuffer);
	}

	
	for(int i = 0 ; i < totalDataChunkNum - M ; i++){
		dataBlocks[i] = receiveChunks[i].chunkBuffer;
	}
	int parity_index = 1;
	for(int i = totalDataChunkNum - M ; i < receiveNum ; i++){
		parityBlocks[parity_index] = receiveChunks[i].chunkBuffer;
		parity_index++;
	}
	// if(parity_index!=M-1){
	// 	cout<<"parity_index: "<<parity_index<<endl;
	// 	cout<<"parityBlocks error!"<<endl;
	// 	exit(0);
	// }
	for(int i = 0 ; i < M; i++){	//k-m -> k
		erasures[i] = totalDataChunkNum - M + i;
	}
	erasures[M] = -1;
	// decode -> k data chunks 

	decoding(K,M,dataBlocks,parityBlocks,CHUNK_SIZE,erasures);

	vector<vector<int>> eachExtStripeDataIndex;	
	eachExtStripeDataIndex.resize(totalDataChunkNum);//k
	
	for(int i = 0 ; i < totalDataChunkNum ; i ++){
		
		for(int j = 0 ; j < totalDataChunkNum ; j++){
			
			int decDataStrIndex = command.decomStripeDataUpdateToStripeId[j];	
			if(decDataStrIndex == i){	// if data_j -> stripe_i
				eachExtStripeDataIndex[i].push_back(j);
			}
		}
	}

	GetHostAndIp(myIp);
	//for each extstripe
	for(int i = 0 ; i < totalDataChunkNum ; i ++){
		
		if(eachExtStripeDataIndex[i].size() == 0 ){	
			continue;
		}
		//for each parity chunk
		for(int j = 0 ; j < M-1 ; j++){
			TransmitChunk chunkNeedSend;
			memset(chunkNeedSend.chunkBuffer, 0, sizeof(char) * CHUNK_SIZE);
			destinationIp = command.destinationIp[j];
			for(int t = 0 ; t < eachExtStripeDataIndex[i].size() ; t++){
				memset(parityDeltas[t].chunkBuffer, 0, sizeof(char) * CHUNK_SIZE);
			}
			parityIndexInstripe = j; 
			for(int t = 0 ; t < eachExtStripeDataIndex[i].size() ; t++){
				dataIndexInStripe = totalDataChunkNum + t;
				EncodingData(dataBlocks[eachExtStripeDataIndex[i][t]],parityDeltas[t].chunkBuffer,dataIndexInStripe, parityIndexInstripe);
			}
			for(int t = 0 ; t < eachExtStripeDataIndex[i].size() ; t++){
				aggregate_data(chunkNeedSend.chunkBuffer, 1, parityDeltas[j].chunkBuffer);
			}
			strcpy(chunkNeedSend.destinationIp, destinationIp.c_str());
			chunkNeedSend.destinationPort = command.port - command.indexInGroup + i;
			chunkNeedSend.sender = COLLECT_NODE;
			chunksNeedSend.push_back(chunkNeedSend);
		}
	}

	for(int i  = 0 ; i < chunksNeedSend.size() ; i++){
		ChunkSender(&chunksNeedSend[i]);
	}	


	free(parityChunk);
	for (int i = 0; i < totalDataChunkNum; i++)
	{
		free(receiveChunks[i].chunkBuffer);
		free(parityDeltas[i].chunkBuffer);
	}
	for (int i = 0; i < chunksNeedSend.size(); i++)
	{
		free(chunksNeedSend[i].chunkBuffer);
	}

	for(int i = totalDataChunkNum - M ; i < totalDataChunkNum ; i++){	//k-m -> k
		free(dataBlocks[i]);
	}



	if (close(socketId) == -1)
	{
		cout << "COLLECT_NODE: close listen socket error" << endl;
		exit(0);
	}
}

void ElaticEC_ProcessUpdate(IdeaMdsUpdateParityCommand& command)
{
	if (command.role == DATA_CHUNK)
	{
		cout << "I'm dataChunk" << endl;
		ElaticEC_DataChunk(command);
	}
	else if (command.role == PARITY_CHUNK)
	{
		cout << "I'm parityChunk" << endl;
		ElaticEC_ParityChunk(command);
	}
	else if (command.role == COLLECT_NODE)
	{
		cout << "I'm collectNode" << endl;
		ElaticEC_CollectNode(command);
	}
	else
	{
		cout << "error update role" << endl;
	}
}

void ElaticEC_RelocationSender(IdeaMdsRelocationCommand& command)
{
	TransmitChunk chunkNeedSend;
	char* dataChunk = (char*)malloc(sizeof(char) * CHUNK_SIZE);

	ReadChunk(dataChunk, command.dataStoreIndex);
	//memset(dataChunk, 0, sizeof(char) * CHUNK_SIZE);

	memcpy(chunkNeedSend.chunkBuffer, dataChunk, CHUNK_SIZE);
	memcpy(chunkNeedSend.destinationIp, command.destinationIp, IP_LENGTH);
	chunkNeedSend.destinationPort = command.port;
	chunkNeedSend.sender = RELOCATION_SENDER;
	//cout << "destination Ip: " << chunkNeedSend.destinationIp << endl;
	//cout << "destination port: " << chunkNeedSend.destinationPort << endl;

	//pthread_t threadId;
	//pthread_create(&threadId, NULL, ChunkSender, &chunkNeedSend);
	//pthread_join(threadId, NULL);
	ChunkSender(&chunkNeedSend);

	free(chunkNeedSend.chunkBuffer);
	free(dataChunk);
}

void ElaticEC_RelocationReceiver(IdeaMdsRelocationCommand& command)
{
	int receiveNum = 1;
	int port = command.port;
	int socketId;
	int indexOfreceiveChunks = 0;
	int storeIndex = command.writeLocation;
	//char* relocationChunk = (char*)malloc(sizeof(char) * CHUNK_SIZE);
	string user = "RELOCATION_RECEIVER";

	//vector<pthread_t> threadIds;
	vector<ReceiveChunk> receiveChunks;

	//memset(relocationChunk, 0, sizeof(char) * CHUNK_SIZE);
	//threadIds.resize(receiveNum);
	receiveChunks.resize(receiveNum);

	socketId = InitialSocket(user, port);
	listen(socketId, receiveNum);

	while (indexOfreceiveChunks < receiveNum)
	{
		struct sockaddr_in acceptedAddress;
		int acceptedSocket;
		socklen_t accpetedAddrLen = sizeof(acceptedAddress);
		acceptedAddress.sin_family = AF_INET;

		cout << "wait receive relocation chunk" << endl;
		acceptedSocket = accept(socketId, (struct sockaddr*)&acceptedAddress, &accpetedAddrLen);
		receiveChunks[indexOfreceiveChunks].socketId = acceptedSocket;
		memcpy(&receiveChunks[indexOfreceiveChunks].address, &acceptedAddress, accpetedAddrLen);

		ChunkReceiver(&receiveChunks[indexOfreceiveChunks]);
		// create a thread
		//pthread_create(&(threadIds[indexOfreceiveChunks]), NULL, ChunkReceiver, &receiveChunks[indexOfreceiveChunks]);
		indexOfreceiveChunks++;
	}
	//for (int i = 0; i < receiveNum; i++)
	//{
	//	pthread_join(threadIds[i], NULL);
	//}

	WriteChunk(receiveChunks.front().chunkBuffer, storeIndex);

	for (int i = 0; i < receiveNum; i++)
	{
		free(receiveChunks[i].chunkBuffer);
	}

	SendAckToMds(1);

	if (close(socketId) == -1)
	{
		cout << "RELOCATION_RECEIVER: close listen socket error" << endl;
		exit(0);
	}
}

void ElaticEC_ProcessRelocation(IdeaMdsRelocationCommand& command)
{
	if (command.role == RELOCATION_SENDER)
	{
		cout << "I'm relocation sender" << endl;
		ElaticEC_RelocationSender(command);
	}
	else if (command.role == RELOCATION_RECEIVER)
	{
		cout << "I'm relocation receiver" << endl;
		ElaticEC_RelocationReceiver(command);
	}
	else
	{
		cout << "error relocation role" << endl;
	}
}

void* ElasticECWork(void* pointer)
{
	//pthread_detach(pthread_self());
	//cout << "Elastic work" << endl;
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
	if (totalReceiveSize == sizeof(IdeaMdsUpdateParityCommand))
	{
		IdeaMdsUpdateParityCommand command;
		memcpy(&command, receiveBuffer, sizeof(IdeaMdsUpdateParityCommand));
		ElaticEC_ProcessUpdate(command);
	}
	else if (totalReceiveSize == sizeof(IdeaMdsRelocationCommand))
	{
		IdeaMdsRelocationCommand command;
		memcpy(&command, receiveBuffer, sizeof(IdeaMdsRelocationCommand));
		ElaticEC_ProcessRelocation(command);
	}


	if (close(threadInfo->socketId) == -1)
	{
		cout << "ElasticECWork: close socket error" << endl;
		exit(0);
	}
	threadInfo->socketId = -1;
	free(receiveBuffer);
	free(receiveHeadBuffer);
}

void ProxyElasticEC()
{
	cout << "I'm proxy" << endl;
	int socketId;
	int indexOfThreadSet = 0;
	string user = "ProxyElasticEC";

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
			//cout << "Proxy: wait idle thread" << endl;
			indexOfThreadSet = (indexOfThreadSet + 1) % PROXY_THREAD_MAX_NUM;
		}

		threadSet[indexOfThreadSet].socketId = acceptedSocket;
		memcpy(&threadSet[indexOfThreadSet].address, &acceptedAddress, accpetedAddrLen);

		// create a thread
		
		pthread_create(&(threadSet[indexOfThreadSet].threadId), NULL, ElasticECWork, &threadSet[indexOfThreadSet]);
		pthread_detach(threadSet[indexOfThreadSet].threadId);
		//ElasticECWork(&threadSet[indexOfThreadSet]);

		indexOfThreadSet = (indexOfThreadSet + 1) % PROXY_THREAD_MAX_NUM;
	}
}