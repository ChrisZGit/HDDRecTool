#ifndef _EDB_HANDLER_H
#define _EDB_HANDLER_H

#include <defines.h>
#include <vector>
#include <sstream>
#include <string>
#include <iostream>
#include <fstream>

class EDBHandler
{
	private:
		//directory built with .export extension
		//filename in dir: SystemIndex_0A.7 //also -T callee-flag
		//delete directory
		std::vector<edbInfo> edbVec;
		std::string pathToFile;
		std::string fileName;
		std::string currentLine;
		std::ifstream fs;

		void runEseDB();
		bool fillInfoVector();
		std::string getInfoAtPos(size_t pos);
		bool getNextLine();

	public:
		EDBHandler();
		EDBHandler(std::string path, std::string fileName);

		bool getHash(std::string hash, edbInfo &info);
		
		bool startHandler();
};

#endif

