#ifndef FILEREADER_H
#define FILEREADER_H

#include <iostream>
#include <fstream>

#define BUFLENGTH 16*1024*1024

class FileReader
{
private:
	std::ifstream fs;
	char buffer[BUFLENGTH];
	size_t endOfBuf;
	std::string path;

public:
	FileReader();
	FileReader(std::string inPath);

	char *getBuffer();
	size_t getBufferSize();
	bool emptyBlock();
	bool newBlock();
	void reset();
};

#endif
