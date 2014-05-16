#include <dataHandler.h>

volatile bool loadAvail;
std::mutex mtx;

DataHandler::DataHandler(std::string in, std::string out)
{
	inPath = in;
	outPath = out;

	imgCarver = new ImageCarver(in, "./databases/");
}

void DataHandler::initHandlers()
{
	if (imgCarver->carveImg() == false)
		return;

	DIR *dpdf;
	struct dirent *epdf;
	std::string pathName = "./databases/";
	dpdf = opendir(pathName.c_str());

	if (dpdf != NULL)
	{ 
		while ((epdf=readdir(dpdf))!=NULL)
		{
			if (epdf->d_name[0] == '.') 
				continue;
			if (epdf->d_type == DT_REG)
			{
				std::cout << "Shouldn't hit this currently" << std::endl;
			} else if (epdf->d_type == DT_DIR)
			{
				std::string outName = outPath + epdf->d_name;
				std::string sys = "rm -f " + outName;
				sys += "/*";
				if (system(sys.c_str()))
				{}
				sys = "mkdir -p " + outName;
				if (system(sys.c_str()))
				{}

				PartitionThumbs *t = new PartitionThumbs;
				std::pair<std::string, PartitionThumbs> tt;
				tt = std::make_pair(outName, *t);
				thumbVec.push_back(tt);

				DIR *dpdf2;
				struct dirent *epdf2;
				std::string pathName2 = pathName+epdf->d_name;
				dpdf2 = opendir(pathName2.c_str());
				pathName2 += "/";
				if (dpdf2 != NULL)
				{ 
					while ((epdf2=readdir(dpdf2))!=NULL)
					{
						if (epdf2->d_name[0] == '.') 
							continue;
						if (epdf2->d_type == DT_REG)
						{
							std::string fileName = epdf2->d_name;
							if (fileName.find(".edb") != std::string::npos)
							{
								EDBHandler *b = new EDBHandler(pathName2,fileName);
								thumbVec.back().second.first = b;
							}
						} else if (epdf2->d_type == DT_DIR)
						{
							std::string outFolder = outName + "/" + epdf2->d_name + "/";
							std::pair<std::string, std::vector<SizeThumbs>> pushMeLater;
							pushMeLater.first = outFolder;
				
							sys = "rm -f " + outFolder;
							sys += "/*";
							if (system(sys.c_str()))
							{}
							sys = "mkdir -p " + outFolder;
							if (system(sys.c_str()))
							{}

							DIR *dpdf3;
							struct dirent *epdf3;
							std::string pathName3 = pathName2+epdf2->d_name;
							dpdf3 = opendir(pathName3.c_str());
							if (dpdf3 != NULL)
							{ 
								while ((epdf3=readdir(dpdf3))!=NULL)
								{
									if (epdf3->d_name[0] == '.') 
										continue;
									if (epdf3->d_type == DT_REG)
									{
										std::string fileName = epdf3->d_name;
										if (fileName.find(".db") != std::string::npos)
										{
											std::string oo = outFolder + epdf3->d_name;
											oo.pop_back();
											oo.pop_back();
											oo.pop_back();
											DBHandler *neu = new DBHandler(pathName3+"/"+fileName, oo);
											std::pair<std::string, DBHandler *> pushMe;
											pushMe = std::make_pair(oo, neu);
											pushMeLater.second.push_back(pushMe);
										}
									}
								}
							}
							thumbVec.back().second.second.push_back(pushMeLater);
						}
					}
				} 
			}
		}
	}
}

void DataHandler::startHandlers()
{
	for (auto in1 : thumbVec)
	{
		if (!(in1.second.first))
		{
			std::cout << "No EDB-File found" << std::endl;
			continue;
		}
		in1.second.first->startHandler();
		for (auto in2 : in1.second.second)
		{
			for (auto in3 : in2.second)
			{
				in3.second->startHandler();
			}
		}
		for (auto in2 : in1.second.second)
		{
			for (auto in3 : in2.second)
			{
				in3.second->handlerFinished();
			}
		}
	}
}

