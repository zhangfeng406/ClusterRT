#include <iostream>
#include <cstring>
#include "ec.h"
#include "Common.h"
#include "NetworkUnit.h"
#include "MetaDataServer.h"
#include "Proxy.h"
#include "BaselineProxy.h"
#include "ERSProxy.h"
#include "SRSProxy.h"
extern "C" {
#include "cauchy.h"
}

using namespace std;

void usage()
{
	printf("Usage:\n");
	fprintf(stderr, "ERROR:Lack args!\n");
	fprintf(stderr, "\t- Args for encode: \"encode\", file name, k, m\n");
	fprintf(stderr, "\t- Args for decode: \"decode\"\n");
	fprintf(stderr, "\t- example.1: ./ECCoder encode test.jpg 4 2\n");
	fprintf(stderr, "\t- example.2: ./ECCoder decode\n");
}

int main(int argc, char** argv)
{


	if (argc < 2)
	{
		//testNetworkUnit();
		//while (true)
		//{
		//	char* temp = (char*)malloc(CHUNK_SIZE);

		//	free(temp);
		//}

		//int k = 8;
		//int m = 4;
		//int w = 8;
		//int* matrix = cauchy_original_coding_matrix(k, m, w);
		//jerasure_print_matrix(matrix, m, k, w);
		//usage();
		//exit(0);
	}
	else if (!strcmp(argv[1], "ElasticEC"))
	{
		int numberOfScale = argc - 2;
		int tempInt;
		vector<int> kPool;
		for (int i = 2; i < argc; i++)
		{
			tempInt = stoi(argv[i]);
			kPool.push_back(tempInt);
		}
		MDS(ElASTIC_EC, kPool);
	}
	else if (!strcmp(argv[1], "base"))
	{
		int numberOfScale = argc - 2;
		int tempInt;
		vector<int> kPool;
		for (int i = 2; i < argc; i++)
		{
			tempInt = stoi(argv[i]);
			kPool.push_back(tempInt);
		}
		MDS(BASELINE, kPool);
	}
	else if (!strcmp(argv[1], "ers"))
	{
		int numberOfScale = argc - 2;
		int tempInt;
		vector<int> kPool;
		for (int i = 2; i < argc; i++)
		{
			tempInt = stoi(argv[i]);
			kPool.push_back(tempInt);
		}
		MDS(ERS, kPool);
	}
	else if (!strcmp(argv[1], "srs"))
	{
		int numberOfScale = argc - 2;
		int tempInt;
		vector<int> kPool;
		for (int i = 2; i < argc; i++)
		{
			tempInt = stoi(argv[i]);
			kPool.push_back(tempInt);
		}
		MDS(SRS, kPool);
	}
	else if (!strcmp(argv[1], "idea")){
		int numberOfScale = argc - 2;
		int tempInt;
		vector<int> kPool;
		for (int i = 2; i < argc; i++)
		{
			tempInt = stoi(argv[i]);
			kPool.push_back(tempInt);
		}
		MDS(4, kPool);
	}
	else if (!strcmp(argv[1], "proxy"))
	{
		//cout << 1 << endl;
		if (!strcmp(argv[2], "ElasticEC"))
		{
			ProxyElasticEC();
		}
		else if (!strcmp(argv[2], "base"))
		{
			ProxyBaseline();
		}
		else if (!strcmp(argv[2], "ers"))
		{
			ProxyERS();
		}
		else if (!strcmp(argv[2], "srs"))
		{
			ProxySRS();
		}
	}
	else if (!strcmp(argv[1], "encode"))
	{
		int k = stoi(argv[3]);
		int m = stoi(argv[4]);
		encoder(argv[2], k, m);
	}
	else if (!strcmp(argv[1], "decode"))
	{
		decoder();
	}
	else
	{
		usage();
		exit(0);
	}

	return 0;
}