#ifndef _DB_HANDLER_H
#define _DB_HANDLER_H

#include <defines.h>
#include <vector>
#include <sstream>
#include <string>
#include <iostream>
#include <fstream>

class DBHandler
{
	private:
		std::string fileName;
		std::string outputPath;
		
		std::vector<dbInfo> dbVec;
		std::string currentLine;
		std::ifstream fs;

		void runThumbCacheViewer();
		bool fillInfoVector();
		std::string getInfoAtPos(size_t pos);
		bool getNextLine();

	public:
		DBHandler();
		DBHandler(std::string fileName, std::string out);

		bool startHandler();
};

#endif

