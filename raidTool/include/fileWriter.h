#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <iostream>
#include <unistd.h>
#include <mutex>
#include <thread>
#include <future>
#include <fstream>

class FileWriter
{
private:
	std::ofstream fs;
	std::string outPath;
	size_t bufferSize;
	bool init;
	size_t pos;
	size_t toWrite;
	std::mutex localMtx;
	bool writeAsync();
	bool write();
	std::future<void> futureObj;

	char *workBuffer;
	char *writeBuffer;

public:
	FileWriter();
	FileWriter(std::string path,size_t bufferSize);
	~FileWriter();

	bool writeToFile(char *buf, size_t size);
	bool closeFile();
	std::string getPath();
	void setPath(std::string path);

};

#endif
