#include <fileReader.h>
#include <limits.h>

extern volatile bool loadAvail;
extern std::mutex mtx;

FileReader::FileReader(std::string inPath, size_t size)
{
	path=inPath;
	bufferLength = size;

	//fs = fopen(path.c_str(), "rb");
	fs.open(path.c_str(), std::ifstream::in | std::ifstream::binary);

	if (!(fs))
	{
		std::cerr << "ERROR FileReader::FileReader - Couldnt open file: " << path << std::endl;
		exit(EXIT_FAILURE);
	}

	std::string sys = "md5sum ";
	sys += inPath;
	std::cout << std::endl;
	std::cout << "Hash5 for " << inPath << ":" << std::endl;
	system(sys.c_str());
	std::cout << std::endl;

	loadBuffer = new char[bufferLength];
	workBuffer = new char[bufferLength];
	localLoad = true;
	endOfLoadBuf = 0;
	blockSize = BLOCKSIZE;
	readSize = blockSize;
	reloadBuffer();
	globalAdress = 0;
	offset = 0;
	asyncReload();
}

size_t FileReader::getRestBuffer()
{
	return endOfWorkBuf;
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
	unsigned int possibleVals[256]={};
	for (unsigned int i = 0; i < readSize; ++i)
	{
		possibleVals[(unsigned char)block[i]] += 1;
	}
	float tmp;
	for (unsigned int i = 0; i < 256; ++i)
	{
		tmp = (float)possibleVals[i]/(float)readSize;
		if (tmp>0)
			ret += -tmp * std::log2(tmp);
	}
	return ret;
}

bool FileReader::emptyBlock()
{
	for (size_t i = 0; i < endOfWorkBuf; ++i)
		if (workBuffer[i] > 0)
			return false;
	return true;
}

void FileReader::printBlock()
{
	for (unsigned int i = 0; i < readSize; i = i+16)
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

void FileReader::setOffset(size_t off)
{
	offset=off;
	block = &workBuffer[offset];
}

int FileReader::findFirstNonemptyBlock(int add)
{
	for (size_t i = offset+add; i+readSize < endOfWorkBuf; i += readSize)
	{
		for (size_t j = 0; j < readSize; ++j)
		{
			if (workBuffer[i+j] > 0)
			{
				return (int)i;
			}
		}
	}
	return -1;
}

bool FileReader::skipInputBuffer(int NumOfBuffers)
{
	for (int i = 0; i < NumOfBuffers; ++i)
		asyncReload();
	return true;
}

void FileReader::setBlockSize(size_t blockS)
{
	this->blockSize=blockS;
	this->readSize = blockSize;
}

bool FileReader::asyncReload()
{
	auto lambda = [this] () -> bool
	{
		bool ret = reloadBuffer();
		localMtx.lock();
		localLoad = true;
		localMtx.unlock();
		return ret;
	};

	localMtx.lock();
	while (localLoad == false)
	{
		localMtx.unlock();
		usleep(1000*5);
		localMtx.lock();
	}
	localLoad = false;
	//localMtx.unlock();
	std::future_status status;
	if (threadSync.valid())
	{
		while (true)
		{
			localMtx.unlock();
			try
			{
				status = threadSync.wait_for(std::chrono::milliseconds(20));
			} catch (std::future_error &e)
			{
				continue;
			}
			if (status == std::future_status::ready)
			{
				break;
			}
			localMtx.lock();
		}
		if (threadSync.get()==false || endOfLoadBuf==0 /*|| fs.bad() || fs.eof()*/)
		{
			localLoad = true;
			localMtx.unlock();
			return false;
		}
	}
	/*
	   if (fs.bad() || endOfLoadBuf==0 || fs.eof())
	   {
	   localMtx.lock();
	   localLoad = true;
	   localMtx.unlock();
	   return false;
	   }
	 */
	mtx.lock();
	while (loadAvail == false)
	{
		mtx.unlock();
		usleep(100*5);
		mtx.lock();
	}
	char *tmp = loadBuffer;
	loadBuffer = workBuffer;
	workBuffer = tmp;
	block = &workBuffer[0];
	offset = 0;
	endOfWorkBuf = endOfLoadBuf;
	readSize = blockSize;
	globalAdress += bufferLength;
	mtx.unlock();
	//std::cout << globalAdress << "\t" << endOfWorkBuf << std::endl;

	localLoad = false;
	localMtx.unlock();

	//even when not used, have to use future object, otherwise the out-of-scope destructor will wait for .get() (therefore no ASYNC LAUNCH ANYMORE)
	threadSync = std::async(std::launch::async, lambda);

	return true;
}

bool FileReader::reloadBuffer()
{
	mtx.lock();
	while (loadAvail == false)
	{
		mtx.unlock();
		usleep(100*5);
		mtx.lock();
	}
	loadAvail = false;
	mtx.unlock();
	if (fs.is_open())
	{
		fs.read((char *)loadBuffer, bufferLength);
		endOfLoadBuf = fs.gcount();
	}
	//endOfLoadBuf = fread(loadBuffer, sizeof(char), bufferLength, fs);
	if (endOfLoadBuf == 0)
	{
		mtx.lock();
		loadAvail=true;
		mtx.unlock();
		return false;
	}
	mtx.lock();
	loadAvail=true;
	mtx.unlock();
	return true;
}

bool FileReader::newBlock()
{
	offset += readSize;
	readSize = std::min(blockSize,endOfWorkBuf-offset);
	if (readSize == 0)
		return false;
	block = &workBuffer[offset];
	return true;
}

size_t FileReader::getBlockSize()
{
	return readSize;
}

void FileReader::reset()
{
	localMtx.lock();
	while (localLoad == false)
	{
		localMtx.unlock();
		usleep(1000*5);
		localMtx.lock();
	}
	localLoad = false;
	localMtx.unlock();
	//rewind(fs);
	fs.clear();
	fs.seekg(0,fs.beg);
	reloadBuffer();
	globalAdress = 0;
	localMtx.lock();
	localLoad = true;
	localMtx.unlock();
	asyncReload();
}

void FileReader::closeFile()
{
	if (fs.is_open())
	{
		fs.close();
		std::string sys = "md5sum ";
		sys += path;
		std::cout << std::endl;
		std::cout << "Hash5 for " << path << ":" << std::endl;
		system(sys.c_str());
		std::cout << std::endl;
	}
}
