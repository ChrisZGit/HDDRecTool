#include <fileReader.h>

FileReader::FileReader(std::string inPath, size_t size)
{
	path=inPath;
	bufferLength = size;
	fs.open(path.c_str(), std::fstream::in);
	loadBuffer = new char[bufferLength];
	if (fs==NULL)
	{
		std::cerr << "ERROR FileReader::FileReader - Couldnt open file: " << path << std::endl;
	}
	endOfBuf = 0;
	blockSize = BLOCKSIZE;
	offset = 0;
	newBlock();
}

char *FileReader::getBuffer()
{
	return block;
}

size_t FileReader::getBufferSize()
{
	return blockSize;
}

bool FileReader::emptyBlock()
{
	for (size_t i = 0; i < endOfBuf; ++i)
		if (loadBuffer[i] > 0)
			return false;
	return true;
}

void FileReader::setOffset(size_t off)
{
	offset=off;
	block = &loadBuffer[offset];
}

int FileReader::findFirstNonemptyBlock()
{
	for (size_t i = offset; i < endOfBuf-blockSize; i += blockSize)
	{
		for (size_t j = 0; j < blockSize; ++j)
		{
			if (loadBuffer[i+j] > 0)
			{
				return (int)i;
			}
		}
	}
	return -1;
}

bool FileReader::skipInputBuffer(int NumOfBuffers)
{
	std::cout << "Skipping buffer for " << path << std::endl;
	if (fs.is_open() && fs.good() && !(fs.eof()))
	{
		fs.seekg((NumOfBuffers-1)*bufferLength + fs.tellg());
		endOfBuf = fs.readsome(loadBuffer, bufferLength);
		offset = 0;
	} 
	if (fs.bad() || endOfBuf==0 || fs.eof())
	{
		std::cerr << "End of File reached" << std::endl;
		return false;
	}
	return true;
}

void FileReader::setBlockSize(size_t blockS)
{
	this->blockSize=blockS;
}

bool FileReader::reloadBuffer()
{
	std::cout << "Reloading buffer for " << path << "\t" << std::endl;
	if (fs.is_open() && fs.good() && !(fs.eof()))
	{
		endOfBuf = fs.readsome(loadBuffer, bufferLength);
		offset = 0;
	} 
	if (fs.bad() || endOfBuf==0 || fs.eof())
	{
		std::cerr << "End of File reached" << std::endl;
		return false;
	}
	return true;
}

bool FileReader::newBlock()
{
	offset += blockSize;
	if (offset+blockSize >= endOfBuf)
	{
		offset -= blockSize;
		return false;
	}
	return true;
	/*
	if (fs.is_open() && fs.good() && !(fs.eof()))
	{
		endOfBuf = fs.readsome(loadBuffer, bufferLength);
	} 
	if (fs.bad() || endOfBuf==0 || fs.eof())
	{
		std::cerr << "End of File reached" << std::endl;
		return false;
	}
	return true;
	*/
}

void FileReader::reset()
{
	if (fs.good())
		fs.seekg(0);
}

