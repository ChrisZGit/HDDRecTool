#include <fileReader.h>

FileReader::FileReader(std::string inPath)
{
	path=inPath;
	fs.open(path.c_str(), std::fstream::in);
	buffer = new char[BUFLENGTH];
	if (fs==NULL)
	{
		std::cerr << "ERROR FileReader::FileReader - Couldnt open file: " << path << std::endl;
	}
	endOfBuf = 0;
	newBlock();
}

char *FileReader::getBuffer()
{
	return buffer;
}

size_t FileReader::getBufferSize()
{
	return endOfBuf;
}

bool FileReader::emptyBlock()
{
	for (size_t i = 0; i < endOfBuf; ++i)
		if (buffer[i] > 0)
			return false;
	return true;
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

void FileReader::reset()
{
	if (fs.good())
		fs.seekg(0);
}

