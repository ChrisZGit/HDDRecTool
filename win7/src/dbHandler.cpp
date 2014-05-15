#include <dbHandler.h>

DBHandler::DBHandler(std::string fileN, std::string out)
{
	fileName = fileN;
	size_t pos = fileName.find_last_of("/");
	size_t end = fileName.length()-3;
	std::string sub = fileName.substr(pos, end-pos);
	outputPath = out;
	outputPath += sub;
	outputPath += "/";
	std::string sys = "rm -rf ";
	sys += outputPath;
	if (system(sys.c_str()))
	{}
	sys = "mkdir ";
	sys += outputPath;
	if (system(sys.c_str()))
	{}
}

bool DBHandler::startHandler()
{
	runThumbCacheViewer();
	std::string dbFile;
	dbFile = outputPath;
	dbFile += "Report.csv";
	fs.open(dbFile.c_str(), std::fstream::in);

	if (!(fs))
	{
		//no file found, as there was none built
		std::string sys = "rm -rf ";
		sys += outputPath;
		if (system(sys.c_str()))
		{}
		return false;
	}
	bool ret = fillInfoVector();
	fs.close();
	if (ret == false)
	{
		std::string sys = "rm -rf ";
		sys += outputPath;
		if (system(sys.c_str()))
		{}
	}
	return ret;
}

void DBHandler::runThumbCacheViewer()
{
#ifndef WINDOWS
	std::cout << "Now running TCV: " << std::endl;
	std::string sys = "./bin/thumbCV ";
	sys += fileName;
	sys += " ";
	sys += outputPath;
	sys += " > /dev/null";
	std::cout << outputPath << std::endl;
	if (system(sys.c_str()))
	{}
	std::cout << std::endl;
#endif
}

bool DBHandler::handlerFinished()
{
	hashHandler.waitForFinish();
	return true;
}

bool DBHandler::fillInfoVector()
{
	dbInfo pushMe;
	bool ret = false;
	for (int i = 0; i < 9; ++i)
		getNextLine();
	while (getNextLine() == true)
	{
		std::string tmp = getInfoAtPos(1);
		if (tmp.size()==1)
			continue;
		pushMe.offset = stoul(tmp);
		tmp = getInfoAtPos(3);
		pushMe.dataSize = stoul(tmp);
		tmp = getInfoAtPos(4);
		pushMe.hash = tmp.substr(2);
		tmp = getInfoAtPos(5);
		pushMe.dataChecksum = tmp.substr(2);
		tmp = getInfoAtPos(6);
		pushMe.headerChecksum = tmp.substr(2);
		dbVec.push_back(pushMe);
		if (pushMe.dataSize > 0)
		{
			ret = true;
			std::string fileN = outputPath;
			fileN += "/0x";
			fileN += pushMe.hash;
			hashHandler.insert(fileN, dbVec.back().md5Sum);
		}
	}
	return ret;
}

std::string DBHandler::getInfoAtPos(size_t pos)
{
	std::string ret;
	std::stringstream ss;
	ss << currentLine.c_str();
	for (size_t i = 0; i <= pos; ++i)
	{
		std::getline(ss, ret, ',');
	}
	return ret;
}

bool DBHandler::getNextLine()
{
	std::getline(fs,currentLine);
	if (!(fs.eof()))
		return true;
	return false;
}

