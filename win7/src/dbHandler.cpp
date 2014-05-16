#include <dbHandler.h>

DBHandler::DBHandler(std::string fileN, std::string out)
{
	fileName = fileN;
	outputPath = out;
	std::string sys = "rm -f ";
	sys += outputPath + "/*";
	if (system(sys.c_str()))
	{}
	sys = "mkdir -p ";
	sys += outputPath;
	if (system(sys.c_str()))
	{}
	outputPath += "/";
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
		return false;
	}
	bool ret = fillInfoVector();
	fs.close();
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
		pushMe.md5Sum = "DEFAULT";
		dbVec.push_back(pushMe);
		if (pushMe.dataSize > 0)
		{
			ret = true;
		}
	}
	for (size_t i = 0; i < dbVec.size(); ++i)
	{
		if (dbVec.at(i).dataSize>0)
		{
			std::string fileN = outputPath;
			fileN += "/0x";
			fileN += dbVec.at(i).hash;
			hashHandler.insert(fileN, &dbVec.at(i));
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
	currentLine = "";
	std::getline(fs,currentLine);
	if (!(fs.eof()))
		return true;
	return false;
}

