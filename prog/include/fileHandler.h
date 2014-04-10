#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <fstream>
#include <iostream>
#include <vector>
#include <dirent.h>
#include <string.h>

#include <fileReader.h>
//#include <fileWriter.h>

class FileHandler
{
private:
	std::string inFolder;
	std::string outFolder;
	std::vector<FileReader*> inFiles;

public:
	FileHandler();
	FileHandler(std::string inputFolder, std::string outputFolder);

	//std::vector<FileReader*> getInFiles();
	char * getBuffer(int buf);
};

#endif
