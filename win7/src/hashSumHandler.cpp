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
		return true;
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
	do 
	{
		mtx.lock();
		auto me = workList.front();
		workList.pop();
		mtx.unlock();
		
		std::string sys = "md5sum ";
		sys += me.first;
		if (system(sys.c_str())){}
		mtx.lock();
		if (workList.empty())
			break;
		mtx.unlock();
		std::cout << workList.size() << std::endl;
	} while (true);
	workingThreads--;
	mtx.unlock();
	freeThread();
}

void HashSumHandler::insert(std::string file, std::string &hash)
{
	std::pair<std::string, std::string &> pushMe(file,hash);
	mtx.lock();
	workList.push(pushMe);
	mtx.unlock();
	if (init == false)
	{
		init = true;
		waitForFreeThread();
		mtx.lock();
		workingThreads++;
		mtx.unlock();
		firstThread = std::async(std::launch::async, &HashSumHandler::finishQueue, this);
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
		std::cout << workingThreads << " " << workList.size() << std::endl;
	}
	mtx.unlock();
}

