#include <fstream>
#include <cstdlib>
#include <iostream>

#include <thumbWriter.h>

int main(int argc, char *argv[])
{
	//Check the Parameters
	std::string inPath;
	std::string outPath;
	if (argc < 3)
	{
		std::cerr << "Not enough Parameters. Try './bin/runme [-p PathToInputFolder] [-o PathToOutput-Folder]'" << std::endl;
		std::cerr << "You can also add:" << std::endl;
		std::cerr << "[-f tex] or [-f xml] to specify that you only want one output. Else both are created." << std::endl;
		std::cerr << "[-i edb] to specify that you only want files which have an entry in the Windows-EDB." << std::endl;
		return 1;
	} 

	bool in = false;
	bool out = false;
	bool tex = false;
	bool xml = false;
	bool edb = false;
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
		else if (input.compare("-i") == 0)
		{
			std::string temp = argv[i+1];
			if (temp.compare("edb") == 0)
			{
				edb = true;
				std::cout << "Output Info: With EDB-Entry only" << std::endl;
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

	ThumbWriter thumb(inPath, outPath);
	thumb.setTex(tex);
	thumb.setXml(xml);
	thumb.setEdb(edb);
	thumb.writeThumbs();

	return  0;
}

