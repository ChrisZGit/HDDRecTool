
#include <fstream>
#include <cstdlib>
#include <iostream>

#include <texMaker.h>
#include <dataHandler.h>

int main(int argc, char *argv[])
{
	//Check the Parameters
	std::string inPath;
	std::string outPath;
	if (argc < 3)
	{
		std::cerr << "Not enough Parameters. Try './bin/runme [-p PathToInputFolder] [-o PathToOutput-Folder]'" << std::endl;
		return 1;
	} 

	bool in = false;
	bool out = false;
	for (int i = 1; i < argc-1; i=i+1)
	{
		std::string input = argv[i];
		if (input.compare("-p") == 0)
		{
			inPath = argv[i+1];
			in = true;
			std::cout << "Input Folder: " << inPath << std::endl;
		}
		else if (input.compare("-o") == 0)
		{
			outPath = argv[i+1];
			out = true;
			std::cout << "Output Folder: " << outPath << std::endl;
		}
		else
		{
			continue;
		}
	}

	if (in == true && out == true)
	{
		std::cout << "Valid Input" << std::endl;
	} else
	{
		std::cout << "Not a valid Input. Please try again." << std::endl;
		std::cout << "Please use: './bin/runme [-p PathToInputfolder] [-o PathToOutput-Folder]'" << std::endl;
		return 1;
	}

	if (!(inPath.at(inPath.size()-1) == '/'))
		inPath+="/";
	if (!(outPath.at(outPath.size()-1) == '/'))
		outPath += "/";

	DataHandler newOne(inPath, outPath);
	newOne.initHandlers();
	newOne.startHandlers();
	newOne.linkDBtoEDB();

	TexMaker bla(outPath);
	auto abc = newOne.getGatheredInfos();
	bla.writeTex(abc);
	return  0;
}

