#include <fileWriter.h>

extern volatile bool loadAvail;
extern std::mutex mtx;

FileWriter::FileWriter(std::string path, size_t bufferS)
{
	outPath = path;
	init = true;
	bufferSize = bufferS;
	workBuffer = new char[bufferSize];
	writeBuffer = new char[bufferSize];
	pos = 0;
}

FileWriter::~FileWriter()
{
	//fs.close();
}

bool FileWriter::writeToFile(char *buf, size_t size)
{
	if (init == true)
	{
		fs.open(outPath.c_str(), std::fstream::out);
		if (!(fs))
		{
			std::cerr << "ERROR FileWriter::FileWriter - Couldnt open file: " << outPath << std::endl;
			exit(EXIT_FAILURE);
		}
		init = false;
	}
	size_t counter = 0;
	while (counter < size)
	{
		if (pos == bufferSize)
		{
			if (writeAsync() == false)
			{
				std::cerr << "Something went terribly wrong! " << std::endl;
				return false;
			}
			pos = 0;
			continue;
		}
		workBuffer[pos] = buf[counter];
		++pos;
		++counter;
	}
		//fs.write(buf, size);
	return true;
}

bool FileWriter::writeAsync()
{
	auto lambda = [&] () -> void
	{
		write();
		localMtx.unlock();
	};

	localMtx.lock();
	char *tmp = writeBuffer;
	writeBuffer = workBuffer;
	workBuffer = tmp;
	toWrite = pos;
	pos = 0;
	futureObj = std::async(std::launch::async, lambda);
	return true;
}

bool FileWriter::write()
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
	if(fs.is_open() && fs.good())
	{
		fs.write(writeBuffer, toWrite);
	}
	mtx.lock();
	loadAvail = true;
	mtx.unlock();

	if (fs.bad())
		return false;
	return true;
}

bool FileWriter::closeFile()
{
	if (fs)
	if (fs.is_open())
	{
		writeAsync();
		localMtx.lock();
		fs.close();
		localMtx.unlock();
	}
	return true;
}

void FileWriter::setPath(std::string path)
{
	closeFile();	
	pos = 0;
	outPath = path;
	init = true;
}

std::string FileWriter::getPath()
{
	return outPath;
}

FileWriter& operator<<(FileWriter &fs, size_t a)
{
	std::string b = std::to_string(a);
	fs << b;
	return fs;
}

FileWriter& operator<<(FileWriter &fs, std::string a)
{
	fs.writeToFile((char *)a.c_str(),a.size());
	return fs;
}

