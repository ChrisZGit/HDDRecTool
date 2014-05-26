#include <edbHandler.h>

EDBHandler::EDBHandler(std::string path, std::string fileN)
{
	pathToFile = path;
	fileName = fileN;
}

bool EDBHandler::startHandler()
{
	runEseDB();
	std::string eseFile;
	eseFile = fileName;
	eseFile += ".export/";
	eseFile += TABLENAME;
	eseFile += ".7";
	fs.open(eseFile.c_str(), std::fstream::in);

	if (!(fs))
	{
		std::cerr << "Couldn't open ese-file!" << std::endl;
		std::cerr << eseFile << std::endl;
		return false;
	}
	bool ret = fillInfoVector();
	fs.close();
#ifndef WINDOWS
	eseFile = fileName;
	eseFile += ".export";
	std::string sys = "rm -rf ";
	sys += eseFile;
	if (system(sys.c_str()))
	{}
#endif
	return ret;
}

void EDBHandler::runEseDB()
{
#ifndef WINDOWS
	std::cout << "Now running esedbexport-tool: " << std::endl;
	std::string sys = "esedbexport ";
	sys += pathToFile;
	sys += fileName;
	sys += " -T ";
	sys += TABLENAME;
	if (system(sys.c_str()))
	{}
	std::cout << std::endl;
#endif
}

bool EDBHandler::fillInfoVector()
{
	edbInfo pushMe;
	getNextLine();
	while (getNextLine() == true)
	{
		std::string tmp = getInfoAtPos(4);
		if (tmp[0] == '*' || tmp.size()==0)
			continue;
		pushMe.sysInfo.size = std::stoul(tmp);
		pushMe.sysInfo.dateModified = getInfoAtPos(6);
		pushMe.sysInfo.dateCreated = getInfoAtPos(5);
		pushMe.sysInfo.dateAccessed = getInfoAtPos(8);
		pushMe.sysInfo.MIMEType = getInfoAtPos(22);
		pushMe.sysInfo.fileName = getInfoAtPos(32);
		pushMe.sysInfo.owner = getInfoAtPos(62);
		pushMe.sysInfo.cacheID = getInfoAtPos(214);

		tmp = getInfoAtPos(75);
		if (tmp[0] == '*' || tmp.size()==0)
			pushMe.picInfo.hRes = 0;
		else
			pushMe.picInfo.hRes = std::stod(tmp);
		
		tmp = getInfoAtPos(76);
		if (tmp[0] == '*' || tmp.size()==0)
			pushMe.picInfo.vRes = 0;
		else
			pushMe.picInfo.vRes = std::stod(tmp);
		
		tmp = getInfoAtPos(77);
		if (tmp[0] == '*' || tmp.size()==0)
			pushMe.picInfo.bitDepth = 0;
		else
			pushMe.picInfo.bitDepth = std::stoul(tmp);

		edbVec.push_back(pushMe);
	}
	return true;
}

std::string EDBHandler::getInfoAtPos(size_t pos)
{
	std::string ret;
	std::stringstream ss;
	ss << currentLine.c_str();
	for (size_t i = 0; i < pos; ++i)
	{
		std::getline(ss, ret, '\t');
	}
	return ret;
}

bool EDBHandler::getHash(std::string hash, edbInfo &info)
{
	for (auto in : edbVec)
	{
		if (in.sysInfo.cacheID == hash)
		{
			info = in;
			return true;
		}
	}
	return false;
}

bool EDBHandler::getNextLine()
{
	std::getline(fs,currentLine);
	if (!(fs.eof()))
		return true;
	return false;
}

