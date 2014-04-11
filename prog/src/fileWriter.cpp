#include <fileWriter.h>

FileWriter::FileWriter(std::string path)
{
	outPath = outputPath;
	fs.open(outPath.c_str(), std::fstream::out);
	if (fs == NULL)
	{
		std::cerr << "ERROR FileReader::FileReader - Couldnt open file: " << outPath << std::endl;
	}
}

FileWriter::~FileWriter()
{
	fs.close();
}

bool FileWriter::writeToFile(char *buf, size_t size)
{
	if(fs.is_open() && fs.good())
	(
		fs.write(buf, size);
	)
}

