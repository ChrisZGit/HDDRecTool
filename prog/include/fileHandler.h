#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <dirent.h>
#include <string.h>

#include <fileReader.h>
#include <defines.h>
#include <fileWriter.h>

class FileHandler
{
private:
	std::string inFolder;
	std::string outFolder;
	std::vector<FileReader*> inFiles;
	FileWriter *writer;

public:
	FileHandler();
	FileHandler(std::string inputFolder, std::string outputFolder);

	void reset();
	int findString(std::string seek);
	bool findGoodBlock();

	std::vector<FileReader*> getInFiles();
	FileReader *getFileReader(size_t num);
	FileWriter *getFileWriter();
};

#endif
