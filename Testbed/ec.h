#ifndef _EC_H
#define _EC_H

#include <iostream>
#include <cstring>
#include <cmath>
#include <sys/stat.h>
#include <vector>

extern "C" {
#include "jerasure.h"   
#include "galois.h"
#include "reed_sol.h"
#include "cauchy.h"
}

#include "Common.h"

using namespace std;

void encoding(int k, int m, char** dataBlocks, char** parityBlocks, int blockSize);
void decoding(int k, int m, char** dataBlocks, char** parityBlocks, int blockSize, int* erasures);
int encoder(char* fileName, int k, int m);
int decoder();

vector<vector<int>> GetEncodingMatrix(int k, int m, int w);
/* dataChunk and parityChunk start from 0 */
int GetSpecifiedCoefficient(vector<vector<int>>& matrix, int dataChunk, int parityChunk);
void encodingDataToDeltaParity(char* data, char* encodedDelta, int coefficient, int chunkSize);
void EncodingData(char* data, char* encodedData, int dataChunkId, int parityChunkId);

/* encoding for ers */
void ERS_EncodingData(char* data, char* encodedData, int dataChunkId, int parityChunkId);
void ERS_EncodingData(char* data, char* encodedData, int coefficient);
int ERS_GetCoefficient(int dataChunkId, int parityChunkId);


#endif // _EC_H

