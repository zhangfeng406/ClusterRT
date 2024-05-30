#include "NetworkUnit.h"
int mmmm = 0;
/* get local ip */

bool GetHostAndIp(string& ip)
{
	char name[256];
	gethostname(name, sizeof(name));

	struct hostent* host = gethostbyname(name);
	char ipStr[32];
	const char* ret = inet_ntop(host->h_addrtype, host->h_addr_list[0], ipStr, sizeof(ipStr));
	if (NULL == ret) {
		cout << "hostname transform to ip failed";
		return false;
	}
	ip = ipStr;
	return true;
}


/* initial server socket: apply a socket and bind */
int InitialSocket(string& user, int portNum)
{
	int socketId = -1;
	int bindReturn = -1;

	sockaddr_in add;

	add.sin_family = AF_INET;
	add.sin_addr.s_addr = INADDR_ANY;
	add.sin_port = htons(portNum);

	socketId = socket(AF_INET, SOCK_STREAM, 0);
	if (socketId == -1)
	{
		cout << "InitialSocket: socket create failed!" << endl;
		exit(0);
	}

	int enable = 1;
	setsockopt(socketId, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

	bindReturn = bind(socketId, (struct sockaddr*)&add, sizeof(add));
	if (bindReturn == -1)
	{
		perror("InitialSocket: bind error\n");
		cout << "InitialSocket: user=" << user << endl;
		cout << "InitialSocket: bind failed!" << endl;
		cout << "InitialSocket: port=" << portNum << endl;
		exit(0);
	}

	return socketId;
}

void ReceiveAck(int receiveNum)
{
	int socketId;
	int received = 0;
	int receivedLength = 0;
	int readReturn;
	int totalReceiveSize = sizeof(AckData);
	char* receiveBuffer = (char*)malloc(totalReceiveSize);
	AckData receivedAck;
	string user = "ReceiveAck";


	socketId = InitialSocket(user, ACK_PORT);


	listen(socketId, receiveNum);

	cout << "ReceiveAck: need receive ack num: " << receiveNum << endl;
	while (received < receiveNum)
	{
		struct sockaddr_in acceptedAddress;
		int acceptedSocket;
		socklen_t accpetedAddrLen = sizeof(acceptedAddress);
		acceptedAddress.sin_family = AF_INET;

		receivedLength = 0;
		//cout << "ReceiveAck: received num=" << received << endl;
		acceptedSocket = accept(socketId, (struct sockaddr*)&acceptedAddress, &accpetedAddrLen);

		while (receivedLength < totalReceiveSize)
		{
			readReturn = read(acceptedSocket, receiveBuffer + receivedLength, totalReceiveSize - receivedLength);
			receivedLength += readReturn;
		}
		if (close(acceptedSocket) == -1)
		{
			cout << "ReceiveAck: close accept socket error" << endl;
			exit(0);
		}
		memcpy(&receivedAck, receiveBuffer, sizeof(totalReceiveSize));
		if (receivedAck.isSuccessful == 1)
		{
			received++;
		}
		else if (receivedAck.isSuccessful == 0)
		{
			cout << "ReceiveAck: this ack said somthing is not successful" << endl;
		}
		else if (receivedAck.isSuccessful == -1)
		{
			cout << "ReceiveAck: this ack is initial, find bug" << endl;
		}
		else
		{
			cout << "ReceiveAck: error ack data" << endl;
			exit(0);
		}

	}
	cout << "ReceiveAck: receive finished" << endl;

	free(receiveBuffer);
	if (close(socketId) == -1)
	{
		cout << "ReceiveAck: close listen socket error" << endl;
		exit(0);
	}


}

void SendAckToMds(int result)
{
	AckData ack;
	int connectSocketId = socket(AF_INET, SOCK_STREAM, 0);
	int totalSendSize = sizeof(AckData);
	int sentLength = 0;
	int ret;
	char* sendBuffer = (char*)malloc(totalSendSize);
	sockaddr_in connectAddr;
	ack.isSuccessful = result;

	connectAddr.sin_family = AF_INET;
	connectAddr.sin_port = htons(ACK_PORT);
	inet_aton(metaDataServerIP.front().c_str(), &connectAddr.sin_addr);

	memcpy(sendBuffer, &ack, totalSendSize);
	while (connect(connectSocketId, (struct sockaddr*)&connectAddr, sizeof(connectAddr)) < 0);

	while (sentLength < totalSendSize)
	{
		ret = write(connectSocketId, sendBuffer + sentLength, totalSendSize - sentLength);
		sentLength += ret;
	}

	free(sendBuffer);
	ret = close(connectSocketId);
	if (ret == -1)
	{
		cout << "ChunkSender: close send socket error!" << endl;
		exit(0);
	}

}

void* SRSChunkSender(void* pointer)
{
	SRSTransmitChunk* sendChunk = (SRSTransmitChunk*)pointer;
	size_t totalSendSize = sizeof(SRSTransmitChunk) + sizeof(char) * CHUNK_SIZE;

	int sentLength = 0;
	int ret;
	int port = sendChunk->destinationPort;
	char* sendBuffer = (char*)malloc(totalSendSize);


	int connectSocketId = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in connectAddr;
	connectAddr.sin_family = AF_INET;
	connectAddr.sin_port = htons(port);

	if (inet_aton(sendChunk->destinationIp, &connectAddr.sin_addr) == 0)
	{
		cout << "SRSChunkSender: Proxy IP Address Error" << endl;
		cout << "SRSChunkSender: Proxy IP is " << sendChunk->destinationIp << endl;
		cout << "SRSChunkSender: Proxy port is " << sendChunk->destinationPort << endl;
		exit(0);
	}

	memcpy(sendBuffer, sendChunk, sizeof(SRSTransmitChunk));
	memcpy(sendBuffer + sizeof(SRSTransmitChunk), sendChunk->chunkBuffer, CHUNK_SIZE);
	while (connect(connectSocketId, (struct sockaddr*)&connectAddr, sizeof(connectAddr)) < 0);

	while (sentLength < totalSendSize)
	{
		ret = write(connectSocketId, sendBuffer + sentLength, totalSendSize - sentLength);
		sentLength += ret;
	}

	free(sendBuffer);
	ret = close(connectSocketId);
	if (ret == -1)
	{
		cout << "SRSChunkSender: close send socket error!" << endl;
		exit(0);
	}
}

void* SRSChunkReceiver(void* pointer)
{
	SRSTransmitChunk* receiveChunk = (SRSTransmitChunk*)pointer;
	int totalReceiveSize = sizeof(SRSTransmitChunk) + CHUNK_SIZE;
	int readReturn;
	int receivedLength = 0;
	int socketId = receiveChunk->receiveSocketId;
	char* receiveBuffer = (char*)malloc(totalReceiveSize);
	char* tempChunkBuffer = receiveChunk->chunkBuffer;

	while (receivedLength < totalReceiveSize)
	{
		readReturn = read(socketId, receiveBuffer + receivedLength, totalReceiveSize - receivedLength);
		receivedLength += readReturn;
	}


	memcpy(receiveChunk, receiveBuffer, sizeof(SRSTransmitChunk));
	receiveChunk->chunkBuffer = tempChunkBuffer;
	memcpy(receiveChunk->chunkBuffer, receiveBuffer + sizeof(SRSTransmitChunk), CHUNK_SIZE);

	if (close(socketId) == -1)
	{
		cout << "SRSChunkReceiver: close socket error" << endl;
		exit(0);
	}

	free(receiveBuffer);
}

void* ERSChunkSender(void* pointer)
{
	ERSTransmitChunk* sendChunk = (ERSTransmitChunk*)pointer;
	size_t totalSendSize = sizeof(ERSTransmitChunk) + sizeof(char) * CHUNK_SIZE;

	int sentLength = 0;
	int ret;
	int port = sendChunk->destinationPort;
	char* sendBuffer = (char*)malloc(totalSendSize);


	int connectSocketId = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in connectAddr;
	connectAddr.sin_family = AF_INET;
	connectAddr.sin_port = htons(port);

	if (inet_aton(sendChunk->destinationIp, &connectAddr.sin_addr) == 0)
	{
		cout << "ERSChunkSender: Proxy IP Address Error" << endl;
		cout << "ERSChunkSender: Proxy IP is " << sendChunk->destinationIp << endl;
		cout << "ERSChunkSender: Proxy port is " << sendChunk->destinationPort << endl;
		exit(0);
	}

	memcpy(sendBuffer, sendChunk, sizeof(ERSTransmitChunk));
	memcpy(sendBuffer + sizeof(ERSTransmitChunk), sendChunk->chunkBuffer, CHUNK_SIZE);
	while (connect(connectSocketId, (struct sockaddr*)&connectAddr, sizeof(connectAddr)) < 0);

	while (sentLength < totalSendSize)
	{
		ret = write(connectSocketId, sendBuffer + sentLength, totalSendSize - sentLength);
		sentLength += ret;
	}

	free(sendBuffer);
	ret = close(connectSocketId);
	if (ret == -1)
	{
		cout << "ERSChunkSender: close send socket error!" << endl;
		exit(0);
	}
}

void* ERSChunkReceiver(void* pointer)
{
	ERSTransmitChunk* receiveChunk = (ERSTransmitChunk*)pointer;
	int totalReceiveSize = sizeof(ERSTransmitChunk) + CHUNK_SIZE;
	int readReturn;
	int receivedLength = 0;
	int socketId = receiveChunk->receiveSocketId;
	char* receiveBuffer = (char*)malloc(totalReceiveSize);
	char* tempChunkBuffer = receiveChunk->chunkBuffer;

	while (receivedLength < totalReceiveSize)
	{
		readReturn = read(socketId, receiveBuffer + receivedLength, totalReceiveSize - receivedLength);
		receivedLength += readReturn;
	}


	memcpy(receiveChunk, receiveBuffer, sizeof(ERSTransmitChunk));
	receiveChunk->chunkBuffer = tempChunkBuffer;
	memcpy(receiveChunk->chunkBuffer, receiveBuffer + sizeof(ERSTransmitChunk), CHUNK_SIZE);

	if (close(socketId) == -1)
	{
		cout << "ERSChunkReceiver: close socket error" << endl;
		exit(0);
	}

	free(receiveBuffer);
}

void* BaseChunkSender(void* pointer)
{
	BaseTransmitChunk* sendChunk = (BaseTransmitChunk*)pointer;
	size_t totalSendSize = sizeof(BaseTransmitChunk) + sizeof(char) * CHUNK_SIZE;

	int sentLength = 0;
	int ret;
	int port = sendChunk->destinationPort;
	char* sendBuffer = (char*)malloc(totalSendSize);


	int connectSocketId = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in connectAddr;
	connectAddr.sin_family = AF_INET;
	connectAddr.sin_port = htons(port);

	if (inet_aton(sendChunk->destinationIp, &connectAddr.sin_addr) == 0)
	{
		cout << "BaseChunkSender: Proxy IP Address Error" << endl;
		cout << "BaseChunkSender: Proxy IP is " << sendChunk->destinationIp << endl;
		cout << "BaseChunkSender: Proxy port is " << sendChunk->destinationPort << endl;
		exit(0);
	}

	memcpy(sendBuffer, sendChunk, sizeof(BaseTransmitChunk));
	memcpy(sendBuffer + sizeof(BaseTransmitChunk), sendChunk->chunkBuffer, CHUNK_SIZE);
	while (connect(connectSocketId, (struct sockaddr*)&connectAddr, sizeof(connectAddr)) < 0);

	while (sentLength < totalSendSize)
	{
		ret = write(connectSocketId, sendBuffer + sentLength, totalSendSize - sentLength);
		sentLength += ret;
	}

	free(sendBuffer);
	ret = close(connectSocketId);
	if (ret == -1)
	{
		cout << "BaseChunkSender: close send socket error!" << endl;
		exit(0);
	}
}

void* BaseChunkReceiver(void* pointer)
{
	BaseTransmitChunk* receiveChunk = (BaseTransmitChunk*)pointer;
	int totalReceiveSize = sizeof(BaseTransmitChunk) + CHUNK_SIZE;
	int readReturn;
	int receivedLength = 0;
	int socketId = receiveChunk->receiveSocketId;
	char* receiveBuffer = (char*)malloc(totalReceiveSize);
	char* tempChunkBuffer = receiveChunk->chunkBuffer;

	while (receivedLength < totalReceiveSize)
	{
		readReturn = read(socketId, receiveBuffer + receivedLength, totalReceiveSize - receivedLength);
		receivedLength += readReturn;
	}


	memcpy(receiveChunk, receiveBuffer, sizeof(BaseTransmitChunk));
	receiveChunk->chunkBuffer = tempChunkBuffer;
	memcpy(receiveChunk->chunkBuffer, receiveBuffer + sizeof(BaseTransmitChunk), CHUNK_SIZE);

	if (close(socketId) == -1)
	{
		cout << "BaseChunkReceiver: close socket error" << endl;
		exit(0);
	}

	free(receiveBuffer);
}

void* ChunkSender(void* pointer)
{
	//pthread_detach(pthread_self());
	//cout << "1" << endl;
	TransmitChunk* chunkNeedSend = (TransmitChunk*)pointer;
	//cout << "2" << endl;
	//cout << "ChunkSender: chunkNeedSend.dataIndexInNewStripe=" << chunkNeedSend.dataIndexInNewStripe << endl;
	size_t totalSendSize = sizeof(TransmitChunk) + sizeof(char) * CHUNK_SIZE;
	//printf("totalSendSize=%u\n", totalSendSize);
	//printf("sizeof=%u\n", sizeof(TransmitChunk));
	int sentLength = 0;
	int ret;
	int port = chunkNeedSend->destinationPort;
	char* destinationIp;
	char* sendBuffer = (char*)malloc(totalSendSize);

	int connectSocketId = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in connectAddr;
	connectAddr.sin_family = AF_INET;
	connectAddr.sin_port = htons(port);

	if (inet_aton(chunkNeedSend->destinationIp, &connectAddr.sin_addr) == 0)
	{
		cout << "ChunkSender: user=" << chunkNeedSend->sender << endl;
		cout << "ChunkSender: Proxy IP Address Error" << endl;
		cout << "ChunkSender: Proxy IP is " << chunkNeedSend->destinationIp << endl;
		cout << "ChunkSender: Proxy port is " << chunkNeedSend->destinationPort << endl;
		exit(0);
	}

	//cout << "ChunkSender: user" << chunkNeedSend->sender << endl;
	memcpy(sendBuffer, chunkNeedSend, sizeof(TransmitChunk));
	memcpy(sendBuffer + sizeof(TransmitChunk), chunkNeedSend->chunkBuffer, CHUNK_SIZE);
	while (connect(connectSocketId, (struct sockaddr*)&connectAddr, sizeof(connectAddr)) < 0);

	destinationIp = inet_ntoa(connectAddr.sin_addr);
	//cout << "ChunkSender: destination IP = " << destinationIp << endl;

	while (sentLength < totalSendSize)
	{
		ret = write(connectSocketId, sendBuffer + sentLength, totalSendSize - sentLength);
		sentLength += ret;
	}

	free(sendBuffer);
	ret = close(connectSocketId);
	if (ret == -1)
	{
		cout << "ChunkSender: close send socket error!" << endl;
		exit(0);
	}
}

void* ChunkReceiver(void* pointer)
{
	ReceiveChunk* chunkNeedReceive = (ReceiveChunk*)pointer;
	TransmitChunk tempTransmitChunk;

	int totalReceiveSize = sizeof(TransmitChunk) + CHUNK_SIZE;
	int readReturn;
	int receivedLength = 0;
	int socketId = chunkNeedReceive->socketId;
	char* clientIp;
	char* receiveBuffer = (char*)malloc(totalReceiveSize);
	char* tempChunkBuffer = tempTransmitChunk.chunkBuffer;

	free(tempTransmitChunk.chunkBuffer);
	//char* buffer = (char*)malloc(sizeof(char) * CHUNK_SIZE);
	clientIp = inet_ntoa(chunkNeedReceive->address.sin_addr);
	//cout << "ChunkReceiver: connect IP = " << clientIp << endl;

	while (receivedLength < totalReceiveSize)
	{
		readReturn = read(socketId, receiveBuffer + receivedLength, totalReceiveSize - receivedLength);
		receivedLength += readReturn;
	}

	//cout << "1" << endl;
	memcpy(&tempTransmitChunk, receiveBuffer, sizeof(TransmitChunk));
	//memcpy(tempTransmitChunk.chunkBuffer, receiveBuffer + sizeof(TransmitChunk), CHUNK_SIZE);
	//free(tempTransmitChunk.chunkBuffer);
	//tempTransmitChunk.chunkBuffer = receiveBuffer + sizeof(TransmitChunk);
	//cout << "2" << endl;

	//cout << "tempTransmitChunk.dataIndexInNewStripe=" << tempTransmitChunk.dataIndexInNewStripe << endl;
	chunkNeedReceive->dataIndexInNewStripe = tempTransmitChunk.dataIndexInNewStripe;
	//cout << "3" << endl;
	//memcpy(chunkNeedReceive->chunkBuffer, tempTransmitChunk.chunkBuffer, sizeof(char) * CHUNK_SIZE);
	memcpy(chunkNeedReceive->chunkBuffer, receiveBuffer + sizeof(TransmitChunk), CHUNK_SIZE);
	//cout << "4" << endl;


	if (close(socketId) == -1)
	{
		cout << "ChunkReceiver: close socket error" << endl;
		exit(0);
	}
	free(receiveBuffer);

}

void* IdeaSendUpdateCommand(void* pointer)
{
	//pthread_detach(pthread_self());
	IdeaMdsUpdateParityCommand sendMessage = *(IdeaMdsUpdateParityCommand*)pointer;
	//cout << "sendMessage.port=" << sendMessage.port << endl;

	int totalSendSize = sendMessage.sizeOfCommand;
	int sentLength = 0;
	int ret;
	char* sendBuff = (char*)malloc(totalSendSize);
	char* destinationIp;

	int connectSocketId = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in connectAddr;
	connectAddr.sin_family = AF_INET;
	connectAddr.sin_port = htons(PROXY_LISTEN_PORT);

	if (inet_aton(sendMessage.nodeIP_ReceiveThisCommand, &connectAddr.sin_addr) == 0)
	{
		cout << "IdeaSendUpdateCommand: Proxy IP Address Error" << endl;
		cout << "IdeaSendUpdateCommand: Proxy IP is" << sendMessage.nodeIP_ReceiveThisCommand << endl;
		cout << "sendMessage.port = " << sendMessage.port << endl;
		exit(0);
	}

	int temp = 0;
	for (int i = 0; i < M; i++)
	{
		if (sendMessage.role == PARITY_CHUNK || sendMessage.role == RELOCATION_RECEIVER)
		{
			temp = 1;
			break;
		}
		if (sendMessage.destinationIp[i][0] != '\0')
		{
			temp = 1;
			break;
		}
	}
	if (temp == 0)
	{
		cout << "IdeaSendUpdateCommand: error temp" << endl;
		exit(0);
	}

	memcpy(sendBuff, pointer, totalSendSize);
	while (connect(connectSocketId, (struct sockaddr*)&connectAddr, sizeof(connectAddr)) < 0);

	destinationIp = inet_ntoa(connectAddr.sin_addr);
	//cout << "destination IP = " << destinationIp << endl;

	while (sentLength < totalSendSize)
	{
		ret = write(connectSocketId, sendBuff + sentLength, totalSendSize - sentLength);
		sentLength += ret;
	}

	free(sendBuff);
	ret = close(connectSocketId);
	if (ret == -1)
	{
		cout << "IdeaSendUpdateCommand: close send socket error!" << endl;
		exit(0);
	}

}

void* IdeaSendRelocationCommand(void* pointer)
{
	//pthread_detach(pthread_self());
	IdeaMdsRelocationCommand sendMessage = *(IdeaMdsRelocationCommand*)pointer;

	int totalSendSize = sendMessage.sizeOfCommand;
	int sentLength = 0;
	int ret;
	char* sendBuff = (char*)malloc(totalSendSize);

	int connectSocketId = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in connectAddr;
	connectAddr.sin_family = AF_INET;
	connectAddr.sin_port = htons(PROXY_LISTEN_PORT);

	if (inet_aton(sendMessage.nodeIP_ReceiveThisCommand, &connectAddr.sin_addr) == 0)
	{
		cout << "IdeaSendRelocationCommand: Proxy IP Address Error" << endl;
		cout << "IdeaSendRelocationCommand: Proxy IP is" << sendMessage.nodeIP_ReceiveThisCommand << endl;
		exit(0);
	}

	memcpy(sendBuff, pointer, totalSendSize);
	while (connect(connectSocketId, (struct sockaddr*)&connectAddr, sizeof(connectAddr)) < 0);

	while (sentLength < totalSendSize)
	{
		ret = write(connectSocketId, sendBuff + sentLength, totalSendSize - sentLength);
		sentLength += ret;
	}

	free(sendBuff);
	ret = close(connectSocketId);
	if (ret == -1)
	{
		cout << "IdeaSendRelocationCommand: close send socket error!" << endl;
		exit(0);
	}

}

void* BaseSendUpdateCommand(void* pointer)
{
	BaseMdsUpdateParityCommand* command = (BaseMdsUpdateParityCommand*)pointer;
	//cout << "sendMessage.port=" << sendMessage.port << endl;

	int totalSendSize = command->sizeOfCommand;
	int sentLength = 0;
	int ret;
	char* sendBuffer = (char*)malloc(totalSendSize);
	char* destinationIp;

	int connectSocketId = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in connectAddr;
	connectAddr.sin_family = AF_INET;
	connectAddr.sin_port = htons(PROXY_LISTEN_PORT);

	if (inet_aton(command->nodeIP_ReceiveThisCommand, &connectAddr.sin_addr) == 0)
	{
		cout << "BaseSendUpdateCommand: Proxy IP Address Error" << endl;
		cout << "BaseSendUpdateCommand: Proxy IP is" << command->nodeIP_ReceiveThisCommand << endl;
		cout << "BaseSendUpdateCommand: sendMessage.port = " << command->port << endl;
		exit(0);
	}

	memcpy(sendBuffer, command, totalSendSize);
	while (connect(connectSocketId, (struct sockaddr*)&connectAddr, sizeof(connectAddr)) < 0);

	destinationIp = inet_ntoa(connectAddr.sin_addr);
	//cout << "destination IP = " << destinationIp << endl;

	while (sentLength < totalSendSize)
	{
		ret = write(connectSocketId, sendBuffer + sentLength, totalSendSize - sentLength);
		sentLength += ret;
	}

	free(sendBuffer);
	ret = close(connectSocketId);
	if (ret == -1)
	{
		cout << "BaseSendUpdateCommand: close send socket error!" << endl;
		exit(0);
	}
}

void* BaseSendRelocationCommand(void* pointer)
{
	BaseMdsRelocationCommand* command = (BaseMdsRelocationCommand*)pointer;

	int totalSendSize = command->sizeOfCommand;
	int sentLength = 0;
	int ret;
	char* sendBuffer = (char*)malloc(totalSendSize);

	int connectSocketId = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in connectAddr;
	connectAddr.sin_family = AF_INET;
	connectAddr.sin_port = htons(PROXY_LISTEN_PORT);

	if (inet_aton(command->nodeIP_ReceiveThisCommand, &connectAddr.sin_addr) == 0)
	{
		cout << "BaseSendRelocationCommand: Proxy IP Address Error" << endl;
		cout << "BaseSendRelocationCommand: Proxy IP is" << command->nodeIP_ReceiveThisCommand << endl;
		exit(0);
	}

	memcpy(sendBuffer, command, totalSendSize);
	while (connect(connectSocketId, (struct sockaddr*)&connectAddr, sizeof(connectAddr)) < 0);

	while (sentLength < totalSendSize)
	{
		ret = write(connectSocketId, sendBuffer + sentLength, totalSendSize - sentLength);
		sentLength += ret;
	}

	free(sendBuffer);
	ret = close(connectSocketId);
	if (ret == -1)
	{
		cout << "BaseSendRelocationCommand: close send socket error!" << endl;
		exit(0);
	}
}

void* ERSSendDataCommand(void* pointer)
{
	ERSMdsDataCommand* command = (ERSMdsDataCommand*)pointer;
	//cout << "sendMessage.port=" << sendMessage.port << endl;

	int totalSendSize = command->sizeOfCommand;
	int sentLength = 0;
	int ret;
	char* sendBuffer = (char*)malloc(totalSendSize);
	char* destinationIp;

	int connectSocketId = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in connectAddr;
	connectAddr.sin_family = AF_INET;
	connectAddr.sin_port = htons(PROXY_LISTEN_PORT);

	if (inet_aton(command->nodeIP_ReceiveThisCommand, &connectAddr.sin_addr) == 0)
	{
		cout << "ERSSendDataCommand: Proxy IP Address Error" << endl;
		cout << "ERSSendDataCommand: Proxy IP is" << command->nodeIP_ReceiveThisCommand << endl;
		cout << "ERSSendDataCommand: command.port = " << command->port << endl;
		exit(0);
	}

	memcpy(sendBuffer, command, totalSendSize);
	while (connect(connectSocketId, (struct sockaddr*)&connectAddr, sizeof(connectAddr)) < 0);

	destinationIp = inet_ntoa(connectAddr.sin_addr);
	//cout << "destination IP = " << destinationIp << endl;

	while (sentLength < totalSendSize)
	{
		ret = write(connectSocketId, sendBuffer + sentLength, totalSendSize - sentLength);
		sentLength += ret;
	}

	free(sendBuffer);
	ret = close(connectSocketId);
	if (ret == -1)
	{
		cout << "ERSSendDataCommand: close send socket error!" << endl;
		exit(0);
	}
}

void* ERSSendParityCommand(void* pointer)
{
	ERSMdsParityCommand* command = (ERSMdsParityCommand*)pointer;
	//cout << "sendMessage.port=" << sendMessage.port << endl;

	int totalSendSize = command->sizeOfCommand;
	int sentLength = 0;
	int ret;
	char* sendBuffer = (char*)malloc(totalSendSize);
	char* destinationIp;

	int connectSocketId = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in connectAddr;
	connectAddr.sin_family = AF_INET;
	connectAddr.sin_port = htons(PROXY_LISTEN_PORT);

	if (inet_aton(command->nodeIP_ReceiveThisCommand, &connectAddr.sin_addr) == 0)
	{
		cout << "ERSSendParityCommand: Proxy IP Address Error" << endl;
		cout << "ERSSendParityCommand: Proxy IP is" << command->nodeIP_ReceiveThisCommand << endl;
		cout << "ERSSendParityCommand: command.port = " << command->port << endl;
		exit(0);
	}

	memcpy(sendBuffer, command, totalSendSize);
	while (connect(connectSocketId, (struct sockaddr*)&connectAddr, sizeof(connectAddr)) < 0);

	destinationIp = inet_ntoa(connectAddr.sin_addr);
	//cout << "destination IP = " << destinationIp << endl;

	while (sentLength < totalSendSize)
	{
		ret = write(connectSocketId, sendBuffer + sentLength, totalSendSize - sentLength);
		sentLength += ret;
	}

	free(sendBuffer);
	ret = close(connectSocketId);
	if (ret == -1)
	{
		cout << "ERSSendParityCommand: close send socket error!" << endl;
		exit(0);
	}
}

void* ERSSendReupdateCommand(void* pointer)
{
	ERSReupdateParityCommand* command = (ERSReupdateParityCommand*)pointer;
	//cout << "sendMessage.port=" << sendMessage.port << endl;

	int totalSendSize = command->sizeOfCommand;
	int sentLength = 0;
	int ret;
	char* sendBuffer = (char*)malloc(totalSendSize);
	char* destinationIp;

	int connectSocketId = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in connectAddr;
	connectAddr.sin_family = AF_INET;
	connectAddr.sin_port = htons(PROXY_LISTEN_PORT);

	if (inet_aton(command->nodeIP_ReceiveThisCommand, &connectAddr.sin_addr) == 0)
	{
		cout << "ERSSendReupdateCommand: Proxy IP Address Error" << endl;
		cout << "ERSSendReupdateCommand: Proxy IP is" << command->nodeIP_ReceiveThisCommand << endl;
		cout << "ERSSendReupdateCommand: command.port = " << command->port << endl;
		exit(0);
	}

	memcpy(sendBuffer, command, totalSendSize);
	while (connect(connectSocketId, (struct sockaddr*)&connectAddr, sizeof(connectAddr)) < 0);

	destinationIp = inet_ntoa(connectAddr.sin_addr);
	//cout << "destination IP = " << destinationIp << endl;

	while (sentLength < totalSendSize)
	{
		ret = write(connectSocketId, sendBuffer + sentLength, totalSendSize - sentLength);
		sentLength += ret;
	}

	free(sendBuffer);
	ret = close(connectSocketId);
	if (ret == -1)
	{
		cout << "ERSSendReupdateCommand: close send socket error!" << endl;
		exit(0);
	}
}

void* ERSSendPlacementCommand(void* pointer)
{
	ERSPlaceMentCommand* command = (ERSPlaceMentCommand*)pointer;
	//cout << "sendMessage.port=" << sendMessage.port << endl;

	int totalSendSize = command->sizeOfCommand;
	int sentLength = 0;
	int ret;
	char* sendBuffer = (char*)malloc(totalSendSize);
	char* destinationIp;

	int connectSocketId = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in connectAddr;
	connectAddr.sin_family = AF_INET;
	connectAddr.sin_port = htons(PROXY_LISTEN_PORT);

	if (inet_aton(command->nodeIP_ReceiveThisCommand, &connectAddr.sin_addr) == 0)
	{
		cout << "ERSSendPlacementCommand: Proxy IP Address Error" << endl;
		cout << "ERSSendPlacementCommand: Proxy IP is" << command->nodeIP_ReceiveThisCommand << endl;
		cout << "ERSSendPlacementCommand: command.port = " << command->port << endl;
		exit(0);
	}

	memcpy(sendBuffer, command, totalSendSize);
	while (connect(connectSocketId, (struct sockaddr*)&connectAddr, sizeof(connectAddr)) < 0);

	destinationIp = inet_ntoa(connectAddr.sin_addr);
	//cout << "destination IP = " << destinationIp << endl;

	while (sentLength < totalSendSize)
	{
		ret = write(connectSocketId, sendBuffer + sentLength, totalSendSize - sentLength);
		sentLength += ret;
	}

	free(sendBuffer);
	ret = close(connectSocketId);
	if (ret == -1)
	{
		cout << "ERSSendPlacementCommand: close send socket error!" << endl;
		exit(0);
	}
}

void* SRSSendDataCommand(void* pointer)
{
	SRSMdsDataCommand* command = (SRSMdsDataCommand*)pointer;
	//cout << "sendMessage.port=" << sendMessage.port << endl;

	int totalSendSize = command->sizeOfCommand;
	int sentLength = 0;
	int ret;
	char* sendBuffer = (char*)malloc(totalSendSize);
	char* destinationIp;

	int connectSocketId = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in connectAddr;
	connectAddr.sin_family = AF_INET;
	connectAddr.sin_port = htons(PROXY_LISTEN_PORT);

	if (inet_aton(command->nodeIP_ReceiveThisCommand, &connectAddr.sin_addr) == 0)
	{
		cout << "SRSSendDataCommand: Proxy IP Address Error" << endl;
		cout << "SRSSendDataCommand: Proxy IP is" << command->nodeIP_ReceiveThisCommand << endl;
		cout << "SRSSendDataCommand: command.port = " << command->port << endl;
		exit(0);
	}

	memcpy(sendBuffer, command, totalSendSize);
	while (connect(connectSocketId, (struct sockaddr*)&connectAddr, sizeof(connectAddr)) < 0);

	destinationIp = inet_ntoa(connectAddr.sin_addr);
	//cout << "destination IP = " << destinationIp << endl;

	while (sentLength < totalSendSize)
	{
		ret = write(connectSocketId, sendBuffer + sentLength, totalSendSize - sentLength);
		sentLength += ret;
	}

	free(sendBuffer);
	ret = close(connectSocketId);
	if (ret == -1)
	{
		cout << "SRSSendDataCommand: close send socket error!" << endl;
		exit(0);
	}
}

void* SRSSendParityCommand(void* pointer)
{
	SRSMdsParityCommand* command = (SRSMdsParityCommand*)pointer;
	//cout << "sendMessage.port=" << sendMessage.port << endl;

	int totalSendSize = command->sizeOfCommand;
	int sentLength = 0;
	int ret;
	char* sendBuffer = (char*)malloc(totalSendSize);
	char* destinationIp;

	int connectSocketId = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in connectAddr;
	connectAddr.sin_family = AF_INET;
	connectAddr.sin_port = htons(PROXY_LISTEN_PORT);

	if (inet_aton(command->nodeIP_ReceiveThisCommand, &connectAddr.sin_addr) == 0)
	{
		cout << "SRSSendParityCommand: Proxy IP Address Error" << endl;
		cout << "SRSSendParityCommand: Proxy IP is" << command->nodeIP_ReceiveThisCommand << endl;
		cout << "SRSSendParityCommand: command.port = " << command->port << endl;
		exit(0);
	}

	memcpy(sendBuffer, command, totalSendSize);
	while (connect(connectSocketId, (struct sockaddr*)&connectAddr, sizeof(connectAddr)) < 0);

	destinationIp = inet_ntoa(connectAddr.sin_addr);
	//cout << "destination IP = " << destinationIp << endl;

	while (sentLength < totalSendSize)
	{
		ret = write(connectSocketId, sendBuffer + sentLength, totalSendSize - sentLength);
		sentLength += ret;
	}

	free(sendBuffer);
	ret = close(connectSocketId);
	if (ret == -1)
	{
		cout << "SRSSendParityCommand: close send socket error!" << endl;
		exit(0);
	}
}

void* SRSSendPlacementCommand(void* pointer)
{
	SRSPlaceMentCommand* command = (SRSPlaceMentCommand*)pointer;
	//cout << "sendMessage.port=" << sendMessage.port << endl;

	int totalSendSize = command->sizeOfCommand;
	int sentLength = 0;
	int ret;
	char* sendBuffer = (char*)malloc(totalSendSize);
	char* destinationIp;

	int connectSocketId = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in connectAddr;
	connectAddr.sin_family = AF_INET;
	connectAddr.sin_port = htons(PROXY_LISTEN_PORT);

	if (inet_aton(command->nodeIP_ReceiveThisCommand, &connectAddr.sin_addr) == 0)
	{
		cout << "SRSSendPlacementCommand: Proxy IP Address Error" << endl;
		cout << "SRSSendPlacementCommand: Proxy IP is" << command->nodeIP_ReceiveThisCommand << endl;
		cout << "SRSSendPlacementCommand: command.port = " << command->port << endl;
		exit(0);
	}

	memcpy(sendBuffer, command, totalSendSize);
	while (connect(connectSocketId, (struct sockaddr*)&connectAddr, sizeof(connectAddr)) < 0);

	destinationIp = inet_ntoa(connectAddr.sin_addr);
	//cout << "destination IP = " << destinationIp << endl;

	while (sentLength < totalSendSize)
	{
		ret = write(connectSocketId, sendBuffer + sentLength, totalSendSize - sentLength);
		sentLength += ret;
	}

	free(sendBuffer);
	ret = close(connectSocketId);
	if (ret == -1)
	{
		cout << "SRSSendPlacementCommand: close send socket error!" << endl;
		exit(0);
	}
}

void* testSend(void* socketId)
{
	int sendSocketId = *(int*)(socketId);
	//int* sendSocketId = (int*)socketId;
	cout << "sendSocketId" << sendSocketId << endl;
	write(sendSocketId, "hello", strlen("hello") + 1);

	mmmm += 1;
	cout << "client mmmm = " << mmmm << endl;
	sleep(20);


	if (close(sendSocketId) == -1)
	{
		cout << "close pid socket error" << endl;
		exit(0);
	}
}

void* TestPrint(void* id)
{
	int ID = *(int*)(id);
	cout << "hello " << ID << endl;
}

void testNetworkUnit()
{
	string ip;
	string user = "testNetworkUnit";

	GetHostAndIp(ip);
	cout << ip << endl;

	for (int i = 0; i < metaDataServerIP.size(); i++)
	{
		if (ip.compare(metaDataServerIP[i]) == 0)
		{
			int connectSocketId = socket(AF_INET, SOCK_STREAM, 0);
			sockaddr_in connectAddr;
			connectAddr.sin_family = AF_INET;
			connectAddr.sin_port = htons(PROXY_LISTEN_PORT);
			inet_aton(proxyIP.front().c_str(), &connectAddr.sin_addr);

			while (connect(connectSocketId, (struct sockaddr*)&connectAddr, sizeof(connectAddr)) < 0);
			char receiveBuf[256];
			read(connectSocketId, receiveBuf, 256);
			cout << receiveBuf << endl;
			close(connectSocketId);
			return;
		}
	}

	for (int i = 0; i < proxyIP.size(); i++)
	{
		if (ip.compare(proxyIP[i]) == 0)
		{
			int socketId;
			socketId = InitialSocket(user, PROXY_LISTEN_PORT);
			listen(socketId, 100);

			pthread_t server_pthread[10];
			//for (int x = 0; x < 10; x++)
			//{
			//	pthread_create(&server_pthread[x], NULL, TestPrint, &x);
			//}
			//break;

			close(socketId);
			socketId = InitialSocket(user, PROXY_LISTEN_PORT);
			close(socketId);
			socketId = InitialSocket(user, PROXY_LISTEN_PORT);

			int index = 0;

			int temp = 0;
			while (1)
			{
				struct sockaddr_in pidAddr;
				int pidSocket;
				socklen_t pidAddrLen = sizeof(pidAddr);
				pidAddr.sin_family = AF_INET;

				char* clientIp;

				cout << "wait accept" << endl;
				pidSocket = accept(socketId, (struct sockaddr*)&pidAddr, &pidAddrLen);
				temp += 1;


				clientIp = inet_ntoa(pidAddr.sin_addr);
				cout << "connect IP = " << clientIp << endl;
				//unsigned short port = ntohs(pidAddr.sin_port);
				//unsigned short port2 = htons(pidAddr.sin_port);
				//cout << "connet port = " << port << endl;
				//cout << "port2 = " << port2 << endl;
				//mmmm += 1;

				pthread_create(&server_pthread[index], NULL, testSend, &pidSocket);
				cout << "pidSocket=" << pidSocket << endl;
				pidSocket = -1;
				cout << "pidSocket=" << pidSocket << endl;
				index++;

				mmmm += 1;
				cout << "server mmmm = " << mmmm << endl;

			}
			if (close(socketId) == -1)
			{
				cout << "close listen socket error" << endl;
				exit(0);
			}


			return;
		}
	}
}

void Print()
{
	cout << proxyIP.back() << endl;
}