#include <fileWriter.h>

FileWriter::FileWriter(std::string path)
{
	outPath = path;
}

FileWriter::~FileWriter()
{
	fs.close();
}

bool FileWriter::writeToFile(char *buf, size_t size)
{
	static bool init=true;
	if (init == true)
	{
		fs.open(outPath.c_str(), std::fstream::out);
		if (fs == NULL)
		{
			std::cerr << "ERROR FileReader::FileReader - Couldnt open file: " << outPath << std::endl;
		}
		init = false;
	}
	if(fs.is_open() && fs.good())
	{
		fs.write(buf, size);
	}
	return true;
}

bool FileWriter::closeFile()
{
	if (fs.is_open())
	{
		fs.close();
	}
	return true;
}

std::string FileWriter::getPath()
{
	return outPath;
}

