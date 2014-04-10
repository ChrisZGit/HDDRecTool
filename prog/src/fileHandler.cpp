#include <fileHandler.h>


FileHandler::FileHandler(std::string inputFolder, std::string outputFolder)
{
	inFolder = inputFolder;
	outFolder = outputFolder;
	DIR *dpdf;
	struct dirent *epdf;

	dpdf = opendir(inputFolder.c_str());
	if (dpdf != NULL)
	{
		while ((epdf=readdir(dpdf))!=NULL)
		{
			if (epdf->d_name[0] == '.')
				continue;
			std::string tmp = inputFolder+epdf->d_name;
			std::cout << tmp << std::endl;

			FileReader *a = new FileReader(tmp);
			inFiles.push_back(a);
		}
	}
}
