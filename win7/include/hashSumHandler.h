#ifndef _HASH_SUM_HANDLER_H
#define _HASH_SUM_HANDLER_H

#include <thread>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <mutex>
#include <future>
#include <queue>

#include <defines.h>

class HashSumHandler
{
	private:
		bool init;
		std::future<void> firstThread;
		std::mutex mtx;
		size_t workingThreads;
		//std::queue<std::pair<std::string, dbInfo *>> workList;
		std::queue<std::pair<std::string, dbInfo *>> workList;
		

	public:
		HashSumHandler();
		void insert(std::string, dbInfo *);
		void finishQueue();
		bool isEmpty();
		void waitForFinish();
};




#endif
