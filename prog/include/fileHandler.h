#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <chrono>
#include <future>
#include <dirent.h>
#include <map>
#include <string.h>
#include <thread>

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
	bool reloadBuffers();
	int findStringInBlock(std::string seek);
	int findString(std::string seek);
	bool findGoodBlock();
	bool estimateStripeSize();

	std::vector<FileReader*> getInFiles();
	FileReader *getFileReader(size_t num);
	FileWriter *getFileWriter();
};

#endif
