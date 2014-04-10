#ifndef FILEREADER_H
#define FILEREADER_H

#include <iostream>
#include <fstream>

#define BUFLENGTH 256*1024*1024

class FileReader
{
private:
	std::ifstream fs;
	char buffer[BUFLENGTH];
	size_t endOfBuf;
	std::string inPath;

public:
	FileReader();
	FileReader(std::string path){inPath = path;}

};

#endif
