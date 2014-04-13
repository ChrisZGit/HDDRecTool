#ifndef FILEREADER_H
#define FILEREADER_H

#include <iostream>
#include <fstream>
#include <vector>

#include <defines.h>

//TODO: get - offset, get - globalAdress

class FileReader
{
private:
	std::ifstream fs;
	char *loadBuffer;
	char *block;
	size_t globalAdress;
	size_t endOfBuf;
	int offset;
	size_t bufferLength;
	size_t blockSize;
	std::string path;

public:
	FileReader();
	FileReader(std::string inPath, size_t size);

	char *getBuffer();
	size_t getBufferSize();
	bool emptyBlock();
	void printBlock();

	bool endOfImage();
	
	std::vector<std::string> getAllStringsInBlock();
	int findString(std::string seek);
	bool skipInputBuffer(int NumOfBuffers);
	void setBlockSize(size_t blockS);
	void setOffset(size_t in);
	int findFirstNonemptyBlock();
	bool reloadBuffer();

	bool newBlock();
	void reset();
};
#endif
