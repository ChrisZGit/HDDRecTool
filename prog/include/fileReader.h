#ifndef FILEREADER_H
#define FILEREADER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <mutex>
#include <thread>
#include <future>
#include <unistd.h>

#include <defines.h>

class FileReader
{
private:
	std::ifstream fs;
	char *loadBuffer;
	char *workBuffer;
	char *block;

	volatile bool localLoad;
	std::future<bool> threadSync;
	std::mutex localMtx;

	size_t globalAdress;
	size_t endOfLoadBuf;
	size_t endOfWorkBuf;
	bool bufferReady;
	int offset;
	size_t bufferLength;
	size_t blockSize;
	std::string path;

public:
	FileReader();
	FileReader(std::string inPath, size_t size);

	size_t getBufferLength() {return std::min(bufferLength,endOfWorkBuf);}
	size_t getBlockSize() {return blockSize;}

	char *getBuffer();
	size_t getBufferSize();
	float calcEntropyOfCurrentBlock();
	bool emptyBlock();
	void printBlock();

//	bool endOfImage();
	
	std::vector<std::string> getAllStringsInBlock();
	int findString(std::string seek);
	void setOffset(size_t in);
	int findFirstNonemptyBlock(int add=0);
	bool skipInputBuffer(int NumOfBuffers);
	void setBlockSize(size_t blockS);
	bool asyncReload();
	bool reloadBuffer();
	bool newBlock();
	void reset();
};
#endif
