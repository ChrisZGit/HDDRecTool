#include <dbHandler.h>

DBHandler::DBHandler(std::string fileN, std::string out)
{
	fileName = fileN;
	size_t pos = fileName.find_last_of("/");
	size_t end = fileName.length()-3;
	std::string sub = fileName.substr(pos, end-pos);
	std::cout << fileName << std::endl;
	outputPath = out;
	std::string sys = "rm -rf ";
	sys += outputPath;
	sys += sub;
	if (system(sys.c_str()))
	{}
	sys = "mkdir ";
	sys += outputPath;
	sys += sub;
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
		std::cerr << "Couldn't open db-file!" << std::endl;
		std::cerr << dbFile << std::endl;
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
	if (system(sys.c_str()))
	{}
	std::cout << std::endl;
#endif
}

bool DBHandler::fillInfoVector()
{
	dbInfo pushMe;
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
		std::cout << pushMe.hash << std::endl;
	}
	std::cout << dbVec.size() << std::endl;
	return true;
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

