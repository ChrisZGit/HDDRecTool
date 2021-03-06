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
		std::cerr << "Not enough Parameters. Try './bin/runme [-(d)p PathToInputFolderWithImage] [-o PathToOutputFolder]'" << std::endl;
		std::cerr << "If you use [-dp] instead of [-p] it's expected that there are the extracted databases." << std::endl;
		std::cerr << "You can also add:" << std::endl;
		std::cerr << "[-f tex] or [-f xml] to specify that you only want one output. Else both are created." << std::endl;
		std::cerr << "[-i edb] to specify that you only want files which have an entry in the Windows-EDB." << std::endl;
		std::cerr << "[-a offset] to specify the offset of the partition you want to analyse. Other partitions are ignored in that case." << std::endl;
		return 1;
	} 

	bool in = false;
	bool in2 = false;
	bool out = false;
	bool tex = false;
	bool xml = false;
	bool edb = false;
	bool extracted = false;
	int offset=-1;
	for (int i = 1; i < argc-1; i=i+1)
	{
		std::string input = argv[i];
		if (input.compare("-p") == 0)
		{
			inPath = argv[i+1];
			in = true;
			std::cout << "Input Folder: " << inPath << std::endl;
			++i;
		}
		else if (input.compare("-o") == 0)
		{
			outPath = argv[i+1];
			out = true;
			std::cout << "Output Folder: " << outPath << std::endl;
			++i;
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
			++i;
		}
		else if (input.compare("-i") == 0)
		{
			std::string temp = argv[i+1];
			if (temp.compare("edb") == 0)
			{
				edb = true;
				std::cout << "Output Info: With EDB-Entry only" << std::endl;
			}
			++i;
		}
		else if (input.compare("-dp") == 0)
		{
			inPath = argv[i+1];
			extracted = true;
			in2 = true;
			std::cout << "Directory with Thumbcaches: " << inPath << std::endl;
			++i;
		}
		else if (input.compare("-a") == 0)
		{
			offset = atoi(argv[i+1]);
			std::cout << "Partition with offset: " << offset << std::endl;
			++i;
		}
		else
		{
			continue;
		}
	}

	if (xml == false && tex == false)
		std::cout << "Output File: XML document and Tex document" << std::endl;

	//folder with image and another folder with databases was in the input. not valid
	if (in == true && extracted == true)
	{
		std::cout << "Not a valid Input. Please try again." << std::endl;
		std::cerr << "Please use: './bin/runme [-(d)p PathToInputFolderWithImage] [-o PathToOutputFolder]'" << std::endl;
		std::cerr << "You can also add:" << std::endl;
		std::cerr << "[-f tex] or [-f xml] to specify that you only want one output. Else both are created." << std::endl;
		std::cerr << "[-i edb] to specify that you only want files which have an entry in the Windows-EDB." << std::endl;
		std::cerr << "[-a offset] to specify the offset of the partition you want to analyse. Other partitions are ignored in that case." << std::endl;
		return 1;
	} //folder with databases and an offset in an image was in the input. not valid
	else if (offset != -1 && extracted == true)
	{
		std::cout << "Not a valid Input. Please try again." << std::endl;
		std::cerr << "Please use: './bin/runme [-(d)p PathToInputFolderWithImage] [-o PathToOutputFolder]'" << std::endl;
		std::cerr << "You can also add:" << std::endl;
		std::cerr << "[-f tex] or [-f xml] to specify that you only want one output. Else both are created." << std::endl;
		std::cerr << "[-i edb] to specify that you only want files which have an entry in the Windows-EDB." << std::endl;
		std::cerr << "[-a offset] to specify the offset of the partition you want to analyse. Other partitions are ignored in that case." << std::endl;
		return 1;
		
	}

	if (in == true && out == true)
	{
		std::cout << "Valid Input" << std::endl;
	}
	else if (in2 == true && out == true)
	{
		std::cout << "Valid Input" << std::endl;
	}
	else
	{
		std::cout << "Not a valid Input. Please try again." << std::endl;
		std::cerr << "Please use: './bin/runme [-(d)p PathToInputFolderWithImage] [-o PathToOutputFolder]'" << std::endl;
		std::cerr << "You can also add:" << std::endl;
		std::cerr << "[-f tex] or [-f xml] to specify that you only want one output. Else both are created." << std::endl;
		std::cerr << "[-i edb] to specify that you only want files which have an entry in the Windows-EDB." << std::endl;
		std::cerr << "[-a offset] to specify the offset of the partition you want to analyse. Other partitions are ignored in that case." << std::endl;
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
	thumb.setExtracted(extracted);
	thumb.setOffset(offset);
	thumb.writeThumbs();

	return  0;
}

