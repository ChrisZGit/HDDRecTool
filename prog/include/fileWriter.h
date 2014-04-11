#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <iostream>
#include <fstream>

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
