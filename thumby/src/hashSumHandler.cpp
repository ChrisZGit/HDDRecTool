#include <hashSumHandler.h>

static std::mutex globalMtx;
static size_t availableThreads=12;
static volatile bool killMe=false;

void waitForFreeThread()
{
	globalMtx.lock();
	while (availableThreads==0)
	{
		globalMtx.unlock();
		usleep(1000*500);
		globalMtx.lock();
	}
	availableThreads--;
	globalMtx.unlock();
}

bool lockIfFreeThread()
{
	globalMtx.lock();
	if (availableThreads > 0)
	{
		availableThreads--;
		globalMtx.unlock();
		return true;
	}
	globalMtx.unlock();
	return false;
}

void freeThread()
{
	globalMtx.lock();
	availableThreads++;
	globalMtx.unlock();
}

HashSumHandler::HashSumHandler()
{
	workingThreads=0;
	init = false;
}

void HashSumHandler::finishQueue()
{
	size_t maxThreads=10000;
	std::vector<std::future<void>> dummy;
	dummy.resize(maxThreads);
	maxThreads=0;
	auto lambda = [&] (std::pair<std::string, dbInfo *> entry, bool free)-> void
	{
		std::string sys = "md5sum ";
		sys += entry.first;
		FILE *pipe = popen(sys.c_str(), "r");

		if (!pipe)
			return;
		std::string all;
		char buf[33];
		while (!feof(pipe))
		{
			if (fgets(buf, 33, pipe) != NULL)
			{
				entry.second->md5Sum = buf;
				break;
			}
		}
		pclose(pipe);
		if (free == true)
		{
			freeThread();
		}
	};

	std::pair<std::string, dbInfo *> workOnMe;
	do 
	{
		mtx.lock();
		if (workList.empty())
			break;
		workOnMe = workList.front();
		workList.pop();
		mtx.unlock();
		if (lockIfFreeThread() == true && maxThreads < 10000)
		{
			dummy[maxThreads] = std::async(std::launch::async, lambda, workOnMe, true);
			maxThreads = maxThreads+1;
		} else
		{
			lambda(workOnMe,false);
		}
	} while (true);
	for (size_t i = 0; i < maxThreads; ++i)
		dummy.at(i).get();
	workingThreads--;
	mtx.unlock();
	freeThread();
}

void HashSumHandler::insert(std::string file, dbInfo *inf)
{
	std::pair<std::string, dbInfo *> pushMe;
	pushMe = std::make_pair(file, inf);
	mtx.lock();
	workList.push(pushMe);
	mtx.unlock();
	if (init == false)
	{
		init = true;
		waitForFreeThread();
		auto fun = std::bind(&HashSumHandler::finishQueue, this);
		firstThread = std::async(std::launch::async, fun);
		mtx.lock();
		workingThreads++;
		mtx.unlock();
	}
}

void HashSumHandler::waitForFinish()
{
	usleep(1000*200);
	mtx.lock();
	while (!(workList.empty()) && workingThreads != 0)
	{
		mtx.unlock();
		usleep(1000*500);
		mtx.lock();
	}
	mtx.unlock();
}

