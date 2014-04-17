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
	reloadBuffer();
	globalAdress = 0;
	offset = 0;
}

char *FileReader::getBuffer()
{
	return block;
}

size_t FileReader::getBufferSize()
{
	return blockSize;
}

float FileReader::calcEntropyOfCurrentBlock()
{
	float ret = 0.0f;
	unsigned int possibleVals[256];
	for (unsigned int i = 0; i < 256; ++i)
	{
		possibleVals[i]=0;
	}
	for (unsigned int i = 0; i < blockSize; ++i)
	{
		possibleVals[(unsigned char)block[i]] += 1;
	}
	float tmp;
	for (unsigned int i = 1; i < 256; ++i)
	{
		tmp = (float)possibleVals[i]/(float)blockSize;
		if (tmp>0)
			ret += -tmp * std::log2(tmp);
	}
	return ret;
}

bool FileReader::emptyBlock()
{
	for (size_t i = 0; i < endOfBuf; ++i)
		if (loadBuffer[i] > 0)
			return false;
	return true;
}

void FileReader::printBlock()
{
	for (unsigned int i = 0; i < blockSize; i = i+16)
	{
		for (int j = 0; j < 16; ++j)
		{
			if (block[i+j] == 0)
			{
				std::cout << ".";
			} else
				std::cout << block[i+j];
		}
		std::cout << std::endl;
	}
}

std::vector<std::string> FileReader::getAllStringsInBlock()
{
	std::vector<std::string> ret;
	unsigned int i = 0;
	while (i < blockSize)
	{
		std::string tmp((char *)(block+i));
		int len = tmp.length();

		if (len > 1)
		{
			size_t first=0,last=std::string::npos;
			do
			{
				last = std::string::npos;
				tmp = std::string(tmp,first,last);
				first=0;
				last = tmp.find(" ");
				std::string sub(tmp,first,last);
				ret.push_back(sub);
				first=last+1;
			} while (last != std::string::npos);
		}
		i += len+1;
	}
	return ret;
}

void FileReader::setOffset(size_t off)
{
	offset=off;
	block = &loadBuffer[offset];
}

int FileReader::findFirstNonemptyBlock(int add)
{
	for (size_t i = offset+add; i < endOfBuf-blockSize; i += blockSize)
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

int FileReader::findString(std::string seek)
{
	std::string tmp;
	tmp.assign((char *)block, blockSize);
	size_t ret = tmp.find(seek);
	if (ret == std::string::npos)
	{
		return -1;
	}
	std::cout << globalAdress + offset + ret << "\tat\t" << path << std::endl;
	return (int (globalAdress+offset+ret));
}

bool FileReader::skipInputBuffer(int NumOfBuffers)
{
	std::cout << "Skipping buffer for " << path << std::endl;
	if (fs.is_open() && fs.good() && !(fs.eof()))
	{
		fs.seekg((NumOfBuffers-1)*bufferLength + fs.tellg());
		endOfBuf = fs.readsome((char *)loadBuffer, bufferLength);
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
	//std::cout << "Reloading buffer for " << path << std::endl;
	if (fs.is_open() && fs.good() && !(fs.eof()))
	{
		endOfBuf = fs.readsome((char *)loadBuffer, bufferLength);
		offset = 0;
		globalAdress += bufferLength;
	} 
	if (fs.bad() || endOfBuf==0 || fs.eof())
	{
		std::cerr << "End of File reached" << std::endl;
		return false;
	}
	block = loadBuffer;
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
	block = &loadBuffer[offset];
	return true;
}

void FileReader::reset()
{
	if (fs.good())
		fs.seekg(0);
	reloadBuffer();
	globalAdress=0;
}

