#ifndef _COMMON_H
#define _COMMON_H

#include <iostream>
#include <vector>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
using namespace std;

#define M 3
#define W 8
#define IP_LENGTH 20

#define ACK_PORT 5566

#define PROXY_LISTEN_PORT 2000

#define PROXY_START_PORT 2222
#define RELOCATION_START_PORT 3333
#define PARITY_RECV_PORT 4444

#define PROXY_THREAD_MAX_NUM 10

// different method type in testbed
#define ElASTIC_EC 0
#define BASELINE 1
#define ERS 2
#define SRS 3


#define CHUNK_SIZE 64*1024*1024 /*unit: Bytes*/

#define PARTITION_LENGTH 1000
#define MAX_STOREINDEX 5*1024*1024*1024

/* role in idea(idea = ElasticEC) */
/* update Parity */
#define DATA_CHUNK 0
#define PARITY_CHUNK 1
#define COLLECT_NODE 2
/* relocation */
#define RELOCATION_SENDER 3
#define RELOCATION_RECEIVER 4

#define MAX_RELOCATION_NUM 10
/*zf */
#define Max_Data_NUM 15

/* ERS */
#define ERS_MAX_POST 20 // max extension stripe in a group 
#define ERS_MAX_PARITY 10 
#define ERS_MAX_DATA 15
#define ERS_PLACEMENT_SENDER 0
#define ERS_PLACEMENT_RECEIVER 1
#define ERS_REUPDATE_DATA 0
#define ERS_REUPDATE_PARITY 1
#define ERS_PLACEMENT_PER_Round 50

/* SRS */
#define SRS_MAX_POST 20 // max extension stripe in a group 
#define SRS_MAX_PARITY 1
#define SRS_MAX_DATA 20
#define SRS_PLACEMENT_SENDER 0
#define SRS_PLACEMENT_RECEIVER 1
#define SRS_PLACEMENT_PER_Round 50

extern int iStripeNum;
extern int iK;
extern int iN;
extern int nodeNum;
extern int rackNum; //zf
extern int scaleNum;
extern int KMax;

extern int ersKMax;

extern string dataFileName;

extern vector<string> metaDataServerIP;
extern vector<string> proxyIP;

extern vector<vector<int>> chunkIndex_eachNode;
extern vector<vector<int>> encodingMatrix;

extern vector<vector<int>> ersEncodingMatrix;

typedef struct SingleChunk
{
	char* chunkBuffer = (char*)malloc(sizeof(char) * CHUNK_SIZE);

	SingleChunk()
	{
		memset(chunkBuffer, 0, CHUNK_SIZE);
	}
}SingleChunk;


typedef struct SRSTransmitChunk
{
	int receiveSocketId; // for chunk receiver
	char destinationIp[IP_LENGTH];
	int destinationPort;

	int relativeDataIndex; // for scale data chunk

	int newCoefficient; // for reupdate, data fill it and parity use it
	int oldCoefficient; // for reupdate, data fill it and parity use it

	char* chunkBuffer = (char*)malloc(sizeof(char) * CHUNK_SIZE);
	//char chunkBuffer[CHUNK_SIZE];
	SRSTransmitChunk()
	{
		receiveSocketId = -1;
		memset(destinationIp, 0, IP_LENGTH);
		destinationPort = -1;
		memset(chunkBuffer, 0, CHUNK_SIZE);

		relativeDataIndex = -1;

		newCoefficient = -1;
		oldCoefficient = -1;
	}
}SRSTransmitChunk;

typedef struct SRSPlaceMentCommand
{
	int sizeOfCommand;
	char nodeIP_ReceiveThisCommand[IP_LENGTH];
	int port; /* port=RELOCATION_START_PORT+relative stripeIndex in per batch */
	int role; /* SRS_PLACEMENT_SENDER or SRS_PLACEMENT_RECEIVER */

	/* for sender */
	int storeIndex;
	char destinationIp[IP_LENGTH];

	/* for receiver */
	int writeIndex;
	int receiveNum;

	SRSPlaceMentCommand()
	{
		sizeOfCommand = sizeof(SRSPlaceMentCommand);
		memset(nodeIP_ReceiveThisCommand, 0, IP_LENGTH);
		port = -1;

		storeIndex = -1;
		memset(destinationIp, 0, IP_LENGTH);

		writeIndex = -1;
		receiveNum = 0;
	}
}SRSPlaceMentCommand;

typedef struct SRSMdsParityCommand
{
	int sizeOfCommand;
	char nodeIP_ReceiveThisCommand[IP_LENGTH];
	int port; /* port=PROXY_START_PORT+groupId */

	int dataChunkNum_needReceive;
	int parityIndexInStripe;

	/* post=extension */
	int parityChunk_eachPostStripe[SRS_MAX_POST][SRS_MAX_PARITY]; // parity storeIndex
	int oldDataIndexNeed_eachPostStripe[SRS_MAX_POST][SRS_MAX_DATA]; //relative dataIndex in a group
	int newDataIndexNeed_eachPostStripe[SRS_MAX_POST][SRS_MAX_DATA]; //relative dataIndex in a group


	SRSMdsParityCommand()
	{
		sizeOfCommand = sizeof(SRSMdsParityCommand);
		memset(nodeIP_ReceiveThisCommand, 0, IP_LENGTH);
		port = -1;
		dataChunkNum_needReceive = 0;
		parityIndexInStripe = -1;
		for (int i = 0; i < SRS_MAX_POST; i++)
		{
			for (int j = 0; j < SRS_MAX_PARITY; j++)
			{
				parityChunk_eachPostStripe[i][j] = -1;
			}
			for (int j = 0; j < SRS_MAX_DATA; j++)
			{
				oldDataIndexNeed_eachPostStripe[i][j] = -1;
				newDataIndexNeed_eachPostStripe[i][j] = -1;
			}
		}
	}
}SRSMdsParityCommand;

typedef struct SRSMdsDataCommand
{
	int sizeOfCommand;
	char nodeIP_ReceiveThisCommand[IP_LENGTH];
	int port; /* port=PROXY_START_PORT+groupId */
	char destinationIps[M][IP_LENGTH];

	int relativeDataIndex;
	int storeIndex;
	int newEncodingCoe; // send with chunkBuffer
	int oldEncodingCoe; // send with chunkBuffer


	SRSMdsDataCommand()
	{
		sizeOfCommand = sizeof(SRSMdsDataCommand);
		memset(nodeIP_ReceiveThisCommand, 0, IP_LENGTH);
		port = -1;

		for (int i = 0; i < M; i++)
		{
			memset(destinationIps[i], 0, IP_LENGTH);
		}

		relativeDataIndex = -1;
		storeIndex = -1;

		newEncodingCoe = -1;
		oldEncodingCoe = -1;
	}
}SRSMdsDataCommand;



typedef struct ERSTransmitChunk
{
	int receiveSocketId; // for chunk receiver
	char destinationIp[IP_LENGTH];
	int destinationPort;

	int relativeDataIndex; // for scale data chunk

	int newCoefficient; // for reupdate, data fill and parity use
	int oldCoefficient; // for reupdate, data fill and parity use

	char* chunkBuffer = (char*)malloc(sizeof(char) * CHUNK_SIZE);
	//char chunkBuffer[CHUNK_SIZE];
	ERSTransmitChunk()
	{
		receiveSocketId = -1;
		memset(destinationIp, 0, IP_LENGTH);
		destinationPort = -1;
		memset(chunkBuffer, 0, CHUNK_SIZE);

		relativeDataIndex = -1;

		newCoefficient = -1;
		oldCoefficient = -1;
	}
}ERSTransmitChunk;

typedef struct ERSReupdateParityCommand
{
	int sizeOfCommand;
	char nodeIP_ReceiveThisCommand[IP_LENGTH];
	int port; /* port=RELOCATION_START_PORT+relative stripeIndex in per batch */
	int role; /* ERS_REUPDATE_DATA or ERS_REUPDATE_PARITY */

	/* for data */
	int newNodeId;
	int oldNodeId;
	int dataStoreIndex;
	char destinationIps[M][IP_LENGTH];

	/* for parity */
	int receiveNum;
	int parityStoreIndex;

	ERSReupdateParityCommand()
	{
		sizeOfCommand = sizeof(ERSReupdateParityCommand);
		memset(nodeIP_ReceiveThisCommand, 0, IP_LENGTH);
		port = -1;
		role = -1;
		newNodeId = -1;
		oldNodeId = -1;
		dataStoreIndex = -1;
		for (int i = 0; i < M; i++)
		{
			memset(destinationIps[i], 0, IP_LENGTH);
		}
		receiveNum = 0;
		parityStoreIndex = -1;
	}


}ERSReupdateParityCommand;

typedef struct ERSPlaceMentCommand
{
	int sizeOfCommand;
	char nodeIP_ReceiveThisCommand[IP_LENGTH];
	int port; /* port=RELOCATION_START_PORT+relative stripeIndex in per batch */
	int role; /* ERS_PLACEMENT_SENDER or ERS_PLACEMENT_RECEIVER */

	/* for sender */
	int storeIndex;
	char destinationIp[IP_LENGTH];

	/* for receiver */
	int writeIndex;
	int receiveNum;

	ERSPlaceMentCommand()
	{
		sizeOfCommand = sizeof(ERSPlaceMentCommand);
		memset(nodeIP_ReceiveThisCommand, 0, IP_LENGTH);
		port = -1;
		storeIndex = -1;
		memset(destinationIp, 0, IP_LENGTH);
		writeIndex = -1;
		receiveNum = 0;
	}
}ERSPlaceMentCommand;

typedef struct ERSMdsParityCommand
{
	int sizeOfCommand;
	char nodeIP_ReceiveThisCommand[IP_LENGTH];
	int port; /* port=PROXY_START_PORT+groupId */

	//int a;
	//int b;
	int dataChunkNum_needReceive;

	/* post=extension */
	int parityChunk_eachPostStripe[ERS_MAX_POST][ERS_MAX_PARITY]; // parity storeIndex
	int relativeDataIndex_eachPostStripe[ERS_MAX_POST][ERS_MAX_DATA]; //relative dataIndex in a group


	ERSMdsParityCommand()
	{
		sizeOfCommand = sizeof(ERSMdsParityCommand);
		memset(nodeIP_ReceiveThisCommand, 0, IP_LENGTH);
		port = -1;
		dataChunkNum_needReceive = 0;
		for (int i = 0; i < ERS_MAX_POST; i++)
		{
			for (int j = 0; j < ERS_MAX_PARITY; j++)
			{
				parityChunk_eachPostStripe[i][j] = -1;
			}
			for (int j = 0; j < ERS_MAX_DATA; j++)
			{
				relativeDataIndex_eachPostStripe[i][j] = -1;
			}
		}
	}
}ERSMdsParityCommand;

typedef struct ERSMdsDataCommand
{
	int sizeOfCommand;
	char nodeIP_ReceiveThisCommand[IP_LENGTH];
	int port; /* port=PROXY_START_PORT+groupId */
	char destinationIps[M][IP_LENGTH];

	int relativeDataIndex;
	int storeIndex;
	int nodeId;


	ERSMdsDataCommand()
	{
		sizeOfCommand = sizeof(ERSMdsDataCommand);
		memset(nodeIP_ReceiveThisCommand, 0, IP_LENGTH);
		port = -1;

		for (int i = 0; i < M; i++)
		{
			memset(destinationIps[i], 0, IP_LENGTH);
		}

		relativeDataIndex = -1;
		storeIndex = -1;
		nodeId = -1;
	}
}ERSMdsDataCommand;

typedef struct BaseTransmitChunk
{
	int receiveSocketId;
	char destinationIp[IP_LENGTH];
	int destinationPort;
	char* chunkBuffer = (char*)malloc(sizeof(char) * CHUNK_SIZE);
	//char chunkBuffer[CHUNK_SIZE];
	BaseTransmitChunk()
	{
		receiveSocketId = -1;
		memset(destinationIp, 0, IP_LENGTH);
		destinationPort = -1;
		memset(chunkBuffer, 0, CHUNK_SIZE);
	}
}BaseTransmitChunk;

typedef struct BaseMdsUpdateParityCommand
{
	int sizeOfCommand;
	char nodeIP_ReceiveThisCommand[IP_LENGTH];
	int port; /* port=PROXY_START_PORT+stripeIndex(in extensionStripeIdThisRound) */
	int role; /* DATA_CHUNK, PARITY_CHUNK */
	int storeIndex;

	/* for DATA_CHUNK */
	int dataIndexInNewStripe;// [0,KLarge)
	char destinationIps[M][IP_LENGTH];
	int parityStoreIndex; // if the DATA_CHUNK is in a parityNode

	/* for PARITY_CHUNK */
	int parityIndexInNewStripe;// [0,M)
	int receiveNum;

	BaseMdsUpdateParityCommand()
	{
		sizeOfCommand = sizeof(BaseMdsUpdateParityCommand);
		memset(nodeIP_ReceiveThisCommand, 0, IP_LENGTH);
		port = -1;
		role = -1;
		storeIndex = -1;
		dataIndexInNewStripe = -1;
		for (int i = 0; i < M; i++)
		{
			memset(destinationIps[i], 0, IP_LENGTH);
		}
		parityStoreIndex = -1;
		parityIndexInNewStripe = -1;
		receiveNum = 0;
	}
}BaseMdsUpdateParityCommand;

typedef struct BaseMdsRelocationCommand
{
	int sizeOfCommand;
	char nodeIP_ReceiveThisCommand[IP_LENGTH];
	int port; /* port=RELOCATION_START_PORT+stripeIndex(in extensionStripeIdThisRound) */
	int role; /* RELOCATION_SENDER, RELOCATION_RECEIVER */

	/* for RELOCATION_SENDER */
	int dataStoreIndex;
	char destinationIp[IP_LENGTH];

	/* for RELOCATION_RECEIVER */
	int receiveNum;
	int writeIndex;
	BaseMdsRelocationCommand()
	{
		sizeOfCommand = sizeof(BaseMdsRelocationCommand);
		memset(nodeIP_ReceiveThisCommand, 0, IP_LENGTH);
		port = -1;
		role = -1;
		dataStoreIndex = -1;
		memset(destinationIp, 0, IP_LENGTH);
		receiveNum = -1;
		writeIndex = -1;
	}
}BaseMdsRelocationCommand;

typedef struct AckData
{
	int isSuccessful; // 1->successful, 0->not successful, -1->initial
	AckData()
	{
		isSuccessful = -1;
	}
}AckData;

typedef struct ParityDelta
{
	char* chunkBuffer = (char*)malloc(sizeof(char) * CHUNK_SIZE);
	//char chunkBuffer[CHUNK_SIZE];
	ParityDelta()
	{
		chunkBuffer[0] = '\0';
	}
}ParityDelta;

typedef struct TransmitChunk
{
	char destinationIp[IP_LENGTH];
	int destinationPort;
	int dataIndexInNewStripe;
	char* chunkBuffer = (char*)malloc(sizeof(char) * CHUNK_SIZE);
	//char chunkBuffer[CHUNK_SIZE];
	int sender;
	TransmitChunk()
	{
		destinationIp[0] = '\0';
		destinationPort = -1;
		dataIndexInNewStripe = -1;
		memset(chunkBuffer, 0, CHUNK_SIZE);
		sender = -1;
	}
}TransmitChunk;

typedef struct ReceiveChunk
{
	int socketId;
	struct sockaddr_in address; // the source ip that send this chunk
	int dataIndexInNewStripe;
	char* chunkBuffer = (char*)malloc(sizeof(char) * CHUNK_SIZE);
	//char chunkBuffer[CHUNK_SIZE];
	ReceiveChunk()
	{
		socketId = -1;
		dataIndexInNewStripe = -1;
		memset(chunkBuffer, 0, CHUNK_SIZE);
	}
}ReceiveChunk;

typedef struct ProxyThreadSet
{
	int socketId;
	pthread_t threadId;
	struct sockaddr_in address; // the ip connect to proxy

	ProxyThreadSet()
	{
		socketId = -1;
	}
}ProxyThreadSet;

typedef struct
{
	int chunkId;
	int sendNodeId;
	int receiveNodeId;
	int dataIndexInNewStripe;
}SendChunkTask;

typedef struct IdeaMdsUpdateParityCommand
{
	int sizeOfCommand;

	char nodeIP_ReceiveThisCommand[IP_LENGTH];

	/* you listen this port or send to the Ips with the port */
	char destinationIp[M][IP_LENGTH];
	int port;

	int role; /* DATA_CHUNK, PARITY_CHUNK, COLLECT_NODE */
	/* if you are data chunk */
	int dataIndexInNewStripe; // [0,KLarge)
	int dataStoreIndex;
	/* if you are parity chunk(include collect node) */
	int parityIndexInNewStripe;// [0,M)
	int parityStoreIndex;
	int receiveNum;
	int indexInGroup;
	/* if you are collect node */
	// if collect node holds dataChunks that it need
	int dataStoreIndexs_inCollectNode[MAX_RELOCATION_NUM];
	int dataIndexInNewstripe_inCollectNode[MAX_RELOCATION_NUM];
	//zf
	int storedDataChunkNum_inCollectNode;

	int decomStripeDataUpdateToStripeId[Max_Data_NUM];//store decomposed stripe's data stretch stripe Id

	IdeaMdsUpdateParityCommand()
	{
		sizeOfCommand = -1;
		nodeIP_ReceiveThisCommand[0] = '\0';
		for (int i = 0; i < M; i++)
		{
			destinationIp[i][0] = '\0';
		}
		port = -1;
		role = -1;
		dataIndexInNewStripe = -1;
		dataStoreIndex = -1;
		parityIndexInNewStripe = -1;
		parityStoreIndex = -1;
		receiveNum = -1;
		indexInGroup = -1;
		for (int i = 0; i < MAX_RELOCATION_NUM; i++)
		{
			dataStoreIndexs_inCollectNode[i] = -1;
			dataIndexInNewstripe_inCollectNode[i] = -1;
		}
		storedDataChunkNum_inCollectNode = -1;

		for (int i = 0 ; i < Max_Data_NUM ; i++){
			decomStripeDataUpdateToStripeId[i] = -1;
		}
	}

}IdeaMdsUpdateParityCommand;

typedef struct IdeaMdsRelocationCommand
{
	int sizeOfCommand;

	char nodeIP_ReceiveThisCommand[IP_LENGTH];

	/* port=RELOCATION_START_PORT+stripeIndex(in extensionStripeIdThisRound) */
	int port;

	int role; /* RELOCATION_SENDER, RELOCATION_RECEIVER */

	/* for RELOCATION_SENDER */
	int dataStoreIndex;
	/* relocation destinationIp */
	char destinationIp[IP_LENGTH];

	/* for RELOCATION_RECEIVER */
	int writeLocation; // index

	IdeaMdsRelocationCommand()
	{
		sizeOfCommand = -1;
		nodeIP_ReceiveThisCommand[0] = '\0';
		port = -1;
		role = -1;
		dataStoreIndex = -1;
		destinationIp[0] = '\0';
		writeLocation = -1;
	}

}IdeaMdsRelocationCommand;



void StopHere();
void EndHere();

bool compareSecondAscending(pair<int, int>a, pair<int, int>b);
bool compareSecondDescending(pair<int, int>a, pair<int, int>b);
bool compareDescending(int a, int b);
int gcd(int a, int b);

void bitwiseXor(char* result, char* srcA, char* srcB, int length);
void aggregate_data(char* data_delta, int num_recv_chnks, char* ped);
void ReadChunk(char* readBuffer, int storeIndex);
void WriteChunk(char* writeBuffer, int storeIndex);
#endif
