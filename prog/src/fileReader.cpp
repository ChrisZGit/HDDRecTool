#include <fileReader.h>

static volatile bool loadAvail=true;
static std::mutex mtx;

FileReader::FileReader(std::string inPath, size_t size)
{
	path=inPath;
	bufferLength = size;
	fs.open(path.c_str(), std::fstream::in);
	loadBuffer = new char[bufferLength];
	workBuffer = new char[bufferLength];
	localLoad = true;
	if (fs==NULL)
	{
		std::cerr << "ERROR FileReader::FileReader - Couldnt open file: " << path << std::endl;
	}
	endOfLoadBuf = 0;
	blockSize = BLOCKSIZE;
	reloadBuffer();
	asyncReload();
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
	for (unsigned int i = 0; i < 256; ++i)
	{
		tmp = (float)possibleVals[i]/(float)blockSize;
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
	block = &workBuffer[offset];
}

int FileReader::findFirstNonemptyBlock(int add)
{
	for (size_t i = offset+add; i+blockSize < endOfWorkBuf; i += blockSize)
	{
		for (size_t j = 0; j < blockSize; ++j)
		{
			if (workBuffer[i+j] > 0)
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
	/*
	if (fs.is_open() && fs.good() && !(fs.eof()))
	{
		fs.seekg((NumOfBuffers-1)*bufferLength + fs.tellg());
		endOfWorkBuf = fs.readsome((char *)workBuffer, bufferLength);
		offset = 0;
	} 
	if (fs.bad() || endOfWorkBuf==0 || fs.eof())
	{
		std::cerr << "End of File reached" << std::endl;
		return false;
	}
	*/
	for (int i = 0; i < NumOfBuffers; ++i)
		asyncReload();
	return true;
}

void FileReader::setBlockSize(size_t blockS)
{
	this->blockSize=blockS;
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
	localMtx.unlock();
	std::future_status status;
	if (threadSync.valid())
	{
		while (true)
		{
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
		}
		if (threadSync.get()==false || endOfLoadBuf==0 || fs.bad() || fs.eof())
		{
			localMtx.lock();
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
	char *tmp = loadBuffer;
	loadBuffer = workBuffer;
	workBuffer = tmp;
	block = workBuffer;
	offset = 0;
	endOfWorkBuf = endOfLoadBuf;
	globalAdress += bufferLength;

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
	if (fs.is_open() && fs.good() && !(fs.eof()))
	{
		endOfLoadBuf = fs.readsome((char *)loadBuffer, bufferLength);
		//offset = 0;
		//globalAdress += bufferLength;
	} 
	if (fs.bad() || endOfLoadBuf==0 || fs.eof())
	{
		mtx.lock();
		loadAvail=true;
		mtx.unlock();
		//	std::cerr << "End of File reached" << std::endl;
		return false;
	}
	mtx.lock();
	loadAvail=true;
	mtx.unlock();
	//block = loadBuffer;
	return true;
}

bool FileReader::newBlock()
{
	offset += blockSize;
	if (offset+blockSize >= endOfWorkBuf)
	{
		offset -= blockSize;
		return false;
	}
	block = &workBuffer[offset];
	return true;
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
	if (fs.good())
		fs.seekg(0);
	reloadBuffer();
	localMtx.lock();
	localLoad = true;
	localMtx.unlock();
	asyncReload();
	globalAdress=0;
}

