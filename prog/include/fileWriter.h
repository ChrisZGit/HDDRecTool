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
	FileWriter(std::string path);
	~FileWriter();

	bool writeToFile(char *buf, size_t size);
	bool closeFile();
	std::string getPath();

};

#endif
