#include <fileReader.h>

FileReader::FileReader(std::string path)
{
	fs.open(path.c_str(), std::fstream::in);
	if (fs==NULL)
	{
		std::cerr << "ERROR FileReader::FileReader - Couldnt open file: " << path << std::endl;
	}
	endOfBuf = 0;
}

char *FileReader::getBuffer()
{
	return buffer;
}

size_t FileReader::getBufferSize()
{
	return endOfBuf;
}

bool FileReader::newBlock()
{
	if (fs.is_open() && fs.good() && !(fs.eof()))
	{
		endOfBuf = fs.readsome(buffer, BUFLENGTH);
	} 
	if (fs.bad() || endOfBuf==0 || fs.eof())
	{
		std::cerr << "End of File reached" << std::endl;
		return false;
	}
	return true;
}

