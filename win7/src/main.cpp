
#include <fstream>
#include <cstdlib>
#include <iostream>

#include <texMaker.h>
#include <xmlMaker.h>
#include <dataHandler.h>

int main(int argc, char *argv[])
{
	//Check the Parameters
	std::string inPath;
	std::string outPath;
	if (argc < 3)
	{
		std::cerr << "Not enough Parameters. Try './bin/runme [-p PathToInputFolder] [-o PathToOutput-Folder]'" << std::endl;
		std::cerr << "You can also add [-f tex] or [-f xml] to specify that you only want one output. Else both are created." << std::endl;
		return 1;
	} 

	bool in = false;
	bool out = false;
	bool tex = false;
	bool xml = false;
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
		else if (input.compare("-f") == 0)
		{
			std::string temp = argv[i+1];
			if (temp.compare("tex") == 0)
			{
				tex = true;
				std::cout << "Output File: Tex document" << std::endl;
			}
			else if (temp.compare("xml") == 0)
			{
				xml = true;
				std::cout << "Output File: XML document" << std::endl;
			}
		}
		else
		{
			continue;
		}
	}

	if (xml == false && tex == false)
		std::cout << "Output File: XML document and Tex document" << std::endl;

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

	if (tex == true)
	{
		std::cout << "Starting to write Tex File." << std::endl;
		TexMaker bla(outPath);
		auto abc = newOne.getGatheredInfos();
		bla.writeTex(abc);
		std::cout << "Done with writing Tex file." << std::endl;
	}
	else if (xml == true)
	{
		
		std::cout << "Starting to write XML File." << std::endl;
		XmlMaker bla2(outPath);
		auto abc = newOne.getGatheredInfos();
		bla2.writeXml(abc);
		std::cout << "Donw with writing XML File." << std::endl;
	}
	else
	{
		std::cout << "Starting to write Tex File." << std::endl;
		TexMaker bla(outPath);
		auto abc = newOne.getGatheredInfos();
		bla.writeTex(abc);
		std::cout << "Done with writing Tex File." << std::endl;
		
		std::cout << "Starting to write XML File." << std::endl;
		XmlMaker bla2(outPath);
		bla2.writeXml(abc);
		std::cout << "Done with writing XML File." << std::endl;
	}
	return  0;
}

