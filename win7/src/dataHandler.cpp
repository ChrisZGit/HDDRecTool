#include <dataHandler.h>

volatile bool loadAvail;
std::mutex mtx;

DataHandler::DataHandler(std::string in, std::string out)
{
	inPath = in;
	outPath = out;

	ImageCarver a(in, "./datas/");
	a.carveImg();
}

void DataHandler::startHandlers()
{
	DIR *dpdf;
	struct dirent *epdf;
	dpdf = opendir(inPath.c_str());
	if (dpdf != NULL)
	{ 
		while ((epdf=readdir(dpdf))!=NULL)
		{
			if (epdf->d_name[0] == '.') 
				continue;
			if (epdf->d_type == DT_REG)
			{
				//standard db-file
				std::string tmp = epdf->d_name;
				if (tmp.find(".db")!=std::string::npos)
				{
					tmp = inPath + tmp;
					DBHandler *a = new DBHandler(tmp, outPath);
					dbVec.push_back(a);
				}
				else if (tmp.find(".edb")!=std::string::npos)
				{
					edbHandler = new EDBHandler(inPath, tmp);
				}
			}
		}
	} 
	if (dbVec.size()==0 || !(edbHandler))
	{
		std::cout << "No valid files found" << std::endl;
		return;
	}
	if (edbHandler->startHandler() == false)
	{
		return;
	}
	for (unsigned int i = 0; i < dbVec.size();)
	{
		if (dbVec.at(i)->startHandler()==false)
		{
			dbVec.erase(dbVec.begin()+i);
			continue;
		}
		dbVec.at(i)->handlerFinished();
		++i;
	}
	std::cout << std::endl;
}

