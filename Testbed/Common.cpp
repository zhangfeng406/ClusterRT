#include "Common.h"

int iStripeNum = 300;
int iK = 6;
int iN = iK + M;
int nodeNum = 18;
int scaleNum = 20;
int KMax = nodeNum - M;
int rackNum = 6;
// for ers
//int iStripeNum = 3000;
//int iK = 6;
//int iN = iK + M;
//int nodeNum = 30;
//int scaleNum = 200;
//int KMax = nodeNum - M;

int ersKMax = nodeNum;

string dataFileName = "data_file";

// MDS
vector<string> metaDataServerIP = { "172.31.105.76" };
vector<string> proxyIP = { "172.31.105.77", "172.31.105.78", "172.31.105.79", "172.31.105.80", "172.31.105.81", "172.31.105.82", "172.31.105.83", "172.31.105.84", "172.31.105.85"
, "172.31.105.86", "172.31.105.87", "172.31.105.88", "172.31.105.89", "172.31.105.90", "172.31.105.91", "172.31.105.92", "172.31.105.93", "172.31.105.94" };

// for srs
//vector<string> metaDataServerIP = { "192.168.0.202" };
//vector<string> proxyIP = { "192.168.0.203","192.168.0.204","192.168.0.205","192.168.0.201","192.168.0.207" };

// for srs
//vector<string> metaDataServerIP = { "192.168.0.200" };
//vector<string> proxyIP = { "192.168.0.201","192.168.0.202","192.168.0.203","192.168.0.204","192.168.0.205",
//"192.168.0.206","192.168.0.207","192.168.0.208","192.168.0.209","192.168.0.210",
//"192.168.0.211","192.168.0.212","192.168.0.213","192.168.0.214","192.168.0.215",
//"192.168.0.216","192.168.0.217","192.168.0.218","192.168.0.219","192.168.0.220",
//"192.168.0.221","192.168.0.222","192.168.0.223","192.168.0.224","192.168.0.225",
//"192.168.0.226","192.168.0.227","192.168.0.228","192.168.0.229","192.168.0.230" };

// for ers
//vector<string> metaDataServerIP = { "192.168.0.202" };
//vector<string> proxyIP = { "192.168.0.203","192.168.0.204","192.168.0.205","192.168.0.201","192.168.0.207" };

// for ers
//vector<string> metaDataServerIP = { "192.168.0.200" };
//vector<string> proxyIP = { "192.168.0.201","192.168.0.202","192.168.0.203","192.168.0.204","192.168.0.205",
//"192.168.0.206","192.168.0.207","192.168.0.208","192.168.0.209","192.168.0.210",
//"192.168.0.211","192.168.0.212","192.168.0.213","192.168.0.214","192.168.0.215",
//"192.168.0.216","192.168.0.217","192.168.0.218","192.168.0.219","192.168.0.220",
//"192.168.0.221","192.168.0.222","192.168.0.223","192.168.0.224","192.168.0.225",
//"192.168.0.226","192.168.0.227","192.168.0.228","192.168.0.229","192.168.0.230" };

//vector<string> metaDataServerIP = { "192.168.0.202" };
//vector<string> proxyIP = { "192.168.0.203","192.168.0.201","192.168.0.204","192.168.0.205","192.168.0.207" };


// for idea
//vector<string> metaDataServerIP = { "192.168.0.202" };
//vector<string> proxyIP = { "192.168.0.201","192.168.0.203","192.168.0.204","192.168.0.205","192.168.0.207" };



vector<vector<int>> chunkIndex_eachNode;
vector<vector<int>> encodingMatrix;

vector<vector<int>> ersEncodingMatrix;

void StopHere()
{
	int a;
	cout << "Stop Here" << endl;
	cin >> a;
}

void EndHere()
{
	int a;
	cout << "End Here" << endl;
	cin >> a;
	exit(0);
}

bool compareSecondAscending(pair<int, int>a, pair<int, int>b)
{
	return a.second < b.second;
}

bool compareSecondDescending(pair<int, int>a, pair<int, int>b)
{
	return a.second > b.second;
}

bool compareDescending(int a, int b)
{
	return a > b;
}

int gcd(int a, int b) { return b ? gcd(b, a % b) : a; }

void bitwiseXor(char* result, char* srcA, char* srcB, int length)
{
	int i;
	int XorCount = length / sizeof(long);

	uint64_t* srcA64 = (uint64_t*)srcA;
	uint64_t* srcB64 = (uint64_t*)srcB;
	uint64_t* result64 = (uint64_t*)result;

	// finish all the word-by-word XOR
	for (i = 0; i < XorCount; i++) {
		result64[i] = srcA64[i] ^ srcB64[i];
	}
}

void aggregate_data(char* data_delta, int num_recv_chnks, char* ped)
{
	int i;

	char* tmp_buff = (char*)malloc(CHUNK_SIZE);
	char* tmp_data_delta = (char*)malloc(CHUNK_SIZE);
	char* addrA;
	char* res;
	char* tmp;
	//cout << 1 << endl;
	memcpy(tmp_data_delta, data_delta, CHUNK_SIZE);
	//cout << 2 << endl;

	addrA = tmp_data_delta;
	res = tmp_buff;

	for (i = 0; i < num_recv_chnks; i++) {

		bitwiseXor(res, addrA, ped + i * CHUNK_SIZE * sizeof(char), CHUNK_SIZE * sizeof(char));
		tmp = addrA;
		addrA = res;
		res = tmp;
	}

	memcpy(data_delta, addrA, CHUNK_SIZE);

	free(tmp_buff);
	free(tmp_data_delta);
}

void ReadChunk(char* readBuffer, int storeIndex)
{

	int ret;
	int fd;
	int location;
	char* tmp_buff = NULL;

	if (storeIndex < 0)
	{
		cout << "ReadChunk: storeIndex < 0, error" << endl;
		exit(0);
	}

	// read the old data from data_file
	ret = posix_memalign((void**)&tmp_buff, getpagesize(), CHUNK_SIZE);
	if (ret)
	{
		printf("ERROR: posix_memalign: %s\n", strerror(ret));
		exit(1);
	}

	fd = open(dataFileName.c_str(), O_RDONLY);
	
	storeIndex = 0;

	if (fd == -1)
	{
		cout << "open " << dataFileName << " failed, error" << endl;
		exit(0);
	}
	lseek(fd, storeIndex * CHUNK_SIZE, SEEK_SET);

	ret = read(fd, tmp_buff, CHUNK_SIZE);
	if (ret != CHUNK_SIZE)
	{
		printf("read data error!\n");
		exit(1);
	}

	memcpy(readBuffer, tmp_buff, CHUNK_SIZE);

	close(fd);
	free(tmp_buff);
}

void WriteChunk(char* writeBuffer, int storeIndex)
{

	int fd;
	int ret;
	char* tmp_buff = NULL;

	if (storeIndex < 0)
	{
		cout << "ReadChunk: storeIndex < 0, error" << endl;
		exit(0);
	}

	ret = posix_memalign((void**)&tmp_buff, getpagesize(), CHUNK_SIZE);
	if (ret)
	{
		printf("ERROR: posix_memalign: %s\n", strerror(ret));
		exit(1);
	}

	memcpy(tmp_buff, writeBuffer, CHUNK_SIZE);

	storeIndex = 0;

	// locate the operated offset
	fd = open(dataFileName.c_str(), O_RDWR);
	if (fd == -1)
	{
		cout << "open " << dataFileName << " failed, error" << endl;
		exit(0);
	}
	lseek(fd, storeIndex * CHUNK_SIZE, SEEK_SET);

	// write the new data 
	ret = write(fd, tmp_buff, CHUNK_SIZE);
	if (ret != CHUNK_SIZE)
	{
		printf("write data error!\n");
		exit(1);
	}


	close(fd);
	free(tmp_buff);

}