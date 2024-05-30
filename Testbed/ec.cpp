#include "ec.h"

void encoding(int k, int m, char** dataBlocks, char** parityBlocks, int blockSize)
{
	int* matrix = NULL;
	int w = 8;
	int isEncodingSuccessful;
	//get coding matrix
	matrix = reed_sol_vandermonde_coding_matrix(k, m, w);
	//execute encoding
	jerasure_matrix_encode(k, m, w, matrix, dataBlocks, parityBlocks, blockSize);

	if (isEncodingSuccessful == -1)
	{
		printf("encoding failed\n");
		exit(0);
	}
}

void decoding(int k, int m, char** dataBlocks, char** parityBlocks, int blockSize, int* erasures)
{
	int* matrix = NULL;
	int w = 8;
	int isDecodingSuccessful;
	//get coding matrix
	matrix = reed_sol_vandermonde_coding_matrix(k, m, w);
	//execute decoding
	isDecodingSuccessful = jerasure_matrix_decode(k, m, w, matrix, 1, erasures, dataBlocks, parityBlocks, blockSize);

	if (isDecodingSuccessful == -1)
	{
		printf("decoding failed\n");
		exit(0);
	}
}

vector<vector<int>> GetEncodingMatrix(int k, int m, int w)
{
	int coefficient;
	int* matrix = cauchy_original_coding_matrix(k, m, w);
	vector<vector<int>> returnEncodingMatrix;

	returnEncodingMatrix.resize(m);

	for (int i = 0; i < m; i++)
	{
		for (int j = 0; j < k; j++)
		{
			coefficient = matrix[i * k + j];
			returnEncodingMatrix[i].push_back(coefficient);
		}
	}

	return returnEncodingMatrix;
}

int GetSpecifiedCoefficient(vector<vector<int>>& matrix, int dataChunk, int parityChunk)
{
	int coefficient;

	//cout << "dataChunk=" << dataChunk << endl;
	//cout << "parityChunk=" << parityChunk << endl;

	coefficient = matrix[parityChunk][dataChunk];

	return coefficient;
}

void encodingDataToDeltaParity(char* data, char* encodedDelta, int coefficient, int chunkSize)
{
	//cout << "coe=" << coefficient << endl;
	//cout << "chunksize=" << chunkSize << endl;
	galois_w08_region_multiply(data, coefficient, chunkSize, encodedDelta, 0);
}

void EncodingData(char* data, char* encodedData, int dataChunkId, int parityChunkId)
{
	if (dataChunkId < 0)
	{
		cout << "EncodingData: dataChunkId < 0, error" << endl;
		exit(0);
	}
	if (parityChunkId < 0)
	{
		cout << "EncodingData: parityChunkId < 0, error" << endl;
		exit(0);
	}
	int coefficient = GetSpecifiedCoefficient(encodingMatrix, dataChunkId, parityChunkId);
	if (coefficient == -1)
	{
		cout << "EncodingData: error coefficient" << endl;
		exit(0);
	}
	encodingDataToDeltaParity(data, encodedData, coefficient, CHUNK_SIZE);
}

void ERS_EncodingData(char* data, char* encodedData, int dataChunkId, int parityChunkId)
{
	if (dataChunkId < 0)
	{
		cout << "ERS_EncodingData: dataChunkId < 0, error" << endl;
		exit(0);
	}
	if (parityChunkId < 0)
	{
		cout << "ERS_EncodingData: parityChunkId < 0, error" << endl;
		exit(0);
	}
	int coefficient = GetSpecifiedCoefficient(ersEncodingMatrix, dataChunkId, parityChunkId);
	if (coefficient == -1)
	{
		cout << "ERS_EncodingData: error coefficient" << endl;
		exit(0);
	}
	encodingDataToDeltaParity(data, encodedData, coefficient, CHUNK_SIZE);
}

void ERS_EncodingData(char* data, char* encodedData, int coefficient)
{
	if (coefficient == -1)
	{
		cout << "ERS_EncodingData: error coefficient" << endl;
		exit(0);
	}
	encodingDataToDeltaParity(data, encodedData, coefficient, CHUNK_SIZE);
}


int ERS_GetCoefficient(int dataChunkId, int parityChunkId)
{
	if (dataChunkId < 0)
	{
		cout << "ERS_GetCoefficient: dataChunkId < 0, error" << endl;
		exit(0);
	}
	if (parityChunkId < 0)
	{
		cout << "ERS_GetCoefficient: parityChunkId < 0, error" << endl;
		exit(0);
	}
	int coefficient = GetSpecifiedCoefficient(ersEncodingMatrix, dataChunkId, parityChunkId);
	if (coefficient == -1)
	{
		cout << "ERS_GetCoefficient: error coefficient" << endl;
		exit(0);
	}
	return coefficient;
}

int decoder()
{
	FILE* blockFile;
	FILE* metaFile;
	FILE* decodedFile;
	int w = 8;
	int k = 0;
	int m = 0;
	int blockSize = 0;
	int fileSize = 0;
	int failBlockNum = 0;
	int i, j;
	int decodeFileTotalWrite = 0;

	int* erasures;
	char* metaFileName = (char*)malloc(sizeof(char) * 100);
	char* blockFileName = (char*)malloc(sizeof(char) * 100);
	char* decodedFileName = (char*)malloc(sizeof(char) * 100);
	char* originalFileName = (char*)malloc(sizeof(char) * 100);
	char* extensionOfFileName;
	char* tempChar;
	char** dataBlocks;
	char** parityBlocks;

	//read meta data
	sprintf(metaFileName, "Coding/meta.txt");
	metaFile = fopen(metaFileName, "rb");
	if (metaFile == NULL)
	{
		printf("meta data lost, decoding failed\n");
		return -1;
	}
	if (fscanf(metaFile, "%s", originalFileName) != 1)
	{
		printf("original is not correct in meta data, decoding failed\n");
		return -1;
	}
	if (fscanf(metaFile, "%d %d", &k, &m) != 2)
	{
		printf("k and m are not correct in meta data, decoding failed\n");
		return -1;
	}
	if (fscanf(metaFile, "%d", &fileSize) != 1)
	{
		printf("file size is not correct in meta data, decoding failed\n");
		return -1;
	}
	if (fscanf(metaFile, "%d", &blockSize) != 1)
	{
		printf("block size is not correct in meta data, decoding failed\n");
		return -1;
	}
	fclose(metaFile);

	//initial according meta data
	erasures = (int*)malloc(sizeof(int) * (k + m));
	dataBlocks = (char**)malloc(sizeof(char*) * k);
	parityBlocks = (char**)malloc(sizeof(char*) * m);

	//get surviving blocks and mark the IDs of the fail blocks
	for (i = 1; i <= k; i++)
	{
		sprintf(blockFileName, "Coding/block_k%d", i);
		blockFile = fopen(blockFileName, "rb");
		if (blockFile == NULL)
		{
			erasures[failBlockNum] = i - 1;
			failBlockNum++;
		}
		else
		{
			dataBlocks[i - 1] = (char*)malloc(sizeof(char) * blockSize);
			fread(dataBlocks[i - 1], sizeof(char), blockSize, blockFile);
			fclose(blockFile);
		}
	}
	for (i = 1; i <= m; i++)
	{
		sprintf(blockFileName, "Coding/block_m%d", i);
		blockFile = fopen(blockFileName, "rb");
		if (blockFile == NULL)
		{
			erasures[failBlockNum] = k + i - 1;
			failBlockNum++;
		}
		else
		{
			parityBlocks[i - 1] = (char*)malloc(sizeof(char) * blockSize);
			fread(parityBlocks[i - 1], sizeof(char), blockSize, blockFile);
			fclose(blockFile);
		}
	}

	//check whether fault tolerance is exceeded
	if (failBlockNum > m)
	{
		printf("Exceeds fault tolerance and cannot be repaired\n");
		return -1;
	}

	//allocate space for failed blocks
	for (i = 0; i < failBlockNum; i++)
	{
		if (erasures[i] < k)
		{
			dataBlocks[erasures[i]] = (char*)malloc(sizeof(char) * blockSize);
		}
		else
		{
			parityBlocks[erasures[i] - k] = (char*)malloc(sizeof(char) * blockSize);
		}
	}

	//erasures[failBlockNum] must be -1
	erasures[failBlockNum] = -1;

	//execute decoding
	decoding(k, m, dataBlocks, parityBlocks, blockSize, erasures);

	/* creat decoded file */
	//get extension of decoded file
	tempChar = strchr(originalFileName, '.');
	if (tempChar != NULL)
	{
		extensionOfFileName = strdup(tempChar);
	}
	else
	{
		extensionOfFileName = strdup("");
	}
	sprintf(decodedFileName, "Coding/decoded%s", extensionOfFileName);


	//write decoded file
	decodedFile = fopen(decodedFileName, "wb");
	decodeFileTotalWrite = 0;
	for (i = 0; i < k; i++)
	{
		if (decodeFileTotalWrite + blockSize <= fileSize)
		{
			fwrite(dataBlocks[i], sizeof(char), blockSize, decodedFile);
			decodeFileTotalWrite += blockSize;
		}
		else
		{
			for (j = 0; j < blockSize; j++)
			{
				if (decodeFileTotalWrite < fileSize)
				{
					fprintf(decodedFile, "%c", dataBlocks[i][j]);
					decodeFileTotalWrite++;
				}
				else
				{
					break;
				}

			}
		}
	}
	fclose(decodedFile);

	//write faild blocks
	for (i = 0; i < failBlockNum; i++)
	{
		if (erasures[i] < k)
		{
			sprintf(blockFileName, "Coding/block_k%d", erasures[i] + 1);
			blockFile = fopen(blockFileName, "wb");
			fwrite(dataBlocks[erasures[i]], sizeof(char), blockSize, blockFile);
		}
		else
		{
			sprintf(blockFileName, "Coding/block_m%d", erasures[i] % k + 1);
			blockFile = fopen(blockFileName, "wb");
			fwrite(dataBlocks[erasures[i] % k], sizeof(char), blockSize, blockFile);
		}
		fclose(blockFile);
	}
	return 0;
}

int encoder(char* fileName, int k, int m)
{
	FILE* inputFile;
	FILE* metaFile;
	int w = 8;
	int blockSize = 0;
	int fileSize = 0;
	int encodingSize = 0;
	int ECParameterlimit = int(pow(2, w));
	int i;

	char* encodingData;
	char* metaFileName = (char*)malloc(sizeof(char) * 100);
	char** dataBlocks = (char**)malloc(sizeof(char*) * k);
	char** parityBlocks = (char**)malloc(sizeof(char*) * m);

	//check whether the input file exists
	if ((inputFile = fopen(fileName, "rb")) == NULL)
	{
		printf("input file not exist, retry please\n");
		//usage();
		return -1;
	}

	//check that k and m are correct
	if (k + m > ECParameterlimit)
	{
		printf("(k + m) cannot be greater than %d, retry please\n", ECParameterlimit);
		return -1;
	}

	//get file size
	fseek(inputFile, 0, SEEK_END);
	fileSize = ftell(inputFile);
	fseek(inputFile, 0, SEEK_SET);

	//get block size
	encodingSize = fileSize;
	while (encodingSize % k != 0)
	{
		encodingSize++;
	}
	blockSize = encodingSize / k;
	//if (blockSize * k != fileSize) {
	//	printf("input file size should divisable by k, retry please\n");
	//	return -1;
	//}

	for (i = 0; i < k; i++)
		dataBlocks[i] = (char*)malloc(sizeof(char) * blockSize);
	for (i = 0; i < m; i++)
		parityBlocks[i] = (char*)malloc(sizeof(char) * blockSize);

	//read input data
	encodingData = (char*)malloc(sizeof(char) * encodingSize);
	fread(encodingData, sizeof(char), fileSize, inputFile);
	for (i = fileSize; i < encodingSize; i++)
	{
		encodingData[i] = '0';
	}

	for (i = 0; i < k; i++)
	{
		dataBlocks[i] = encodingData + (i * blockSize);
	}

	//encode
	encoding(k, m, dataBlocks, parityBlocks, blockSize);

	/* Create Coding directory */
	i = mkdir("Coding", S_IRWXU);
	if (i == -1 && errno != EEXIST)
	{
		printf("Unable to create directory Coding\n");
		exit(0);
	}

	// write data into blocks
	for (i = 1; i <= k; i++) {
		char* blockName = (char*)malloc(sizeof(char) * 20);
		sprintf(blockName, "Coding/block_k%d", i);
		FILE* blockFile = fopen(blockName, "wb");
		fwrite(dataBlocks[i - 1], sizeof(char), blockSize, blockFile);
	}
	for (i = 1; i <= m; i++) {
		char* blockName = (char*)malloc(sizeof(char) * 20);
		sprintf(blockName, "Coding/block_m%d", i);
		FILE* blockFile = fopen(blockName, "wb");
		fwrite(parityBlocks[i - 1], sizeof(char), blockSize, blockFile);
	}

	//write meta data
	sprintf(metaFileName, "Coding/meta.txt");
	metaFile = fopen(metaFileName, "wb");
	fprintf(metaFile, "%s\n", fileName);
	fprintf(metaFile, "%d %d\n", k, m);
	fprintf(metaFile, "%d\n", fileSize);
	fprintf(metaFile, "%d\n", blockSize);
	fclose(metaFile);
	return 0;
}