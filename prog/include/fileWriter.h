#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <iostream>
#include <fstream>

#define BUFLENGTH 256*1024*1024

class FileWriter
{
private:
	std::ofstream fs;
	std::string outPath;

public:
	FileWriter();
	FileWriter(std::string path){outPath = path;}

};

#endif
