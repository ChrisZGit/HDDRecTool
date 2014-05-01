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
#include <iomanip>

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

	void addImage(std::string path);
	bool findGoodBlock();
	int estimateStripeSize();
	std::vector<size_t> estimateStripeMap(bool isRaid5);
	void reset();
	int findStringInBlock(std::string seek);
	bool reloadBuffers();
	void setBlockSize(size_t block);
	int findString(std::string seek);
	
	std::vector<FileReader*> getInFiles();
	FileReader *getFileReader(size_t num);
	FileWriter *getFileWriter();
};

#endif
