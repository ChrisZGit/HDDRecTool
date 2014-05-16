#ifndef _DATA_HANDLER_H
#define _DATA_HANDLER_H

#include <mutex>
#include <dirent.h>
#include <iostream>
#include <string>

#include <imageCarver.h>
#include <defines.h>
#include <edbHandler.h>
#include <dbHandler.h>
class DataHandler
{
	private: 
		std::string inPath;
		std::string outPath;
		std::vector<DBHandler *> dbVec;
		EDBHandler *edbHandler;

	public:
		DataHandler(std::string in, std::string out);
		void startHandlers();
};

#endif
