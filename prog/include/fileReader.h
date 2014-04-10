#ifndef FILEREADER_H
#define FILEREADER_H

#include <iostream>
#include <fstream>

#define BUFLENGTH 64*1024*1024

class FileReader
{
private:
	std::ifstream fs;
	char buffer[BUFLENGTH];
	size_t endOfBuf;

public:
	FileReader();
	FileReader(std::string path);

	char *getBuffer();
	size_t getBufferSize();
	bool newBlock();
};

#endif
