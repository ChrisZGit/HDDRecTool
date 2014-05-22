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

typedef std::pair<std::string /*folderName*/, DBHandler *> SizeThumbs;
typedef std::vector<std::pair<std::string/*username*/, std::vector<SizeThumbs>>> UserThumbs;
typedef std::pair<EDBHandler *, UserThumbs> PartitionThumbs;
typedef std::vector<std::pair<std::string/*offset in partition table*/, PartitionThumbs>> AllPartitions;

class DataHandler
{
	private: 
		std::string inPath;
		std::string outPath;
		ImageCarver *imgCarver;

		AllPartitions thumbVec;
		std::vector<PartitionFiles> gatheredInfos;

	public:
		DataHandler(std::string in, std::string out);
		bool carveImg();
		bool initHandlers();
		bool startHandlers();
		void linkDBtoEDB();
		void setOffset(int offset);
		std::vector<PartitionFiles> getGatheredInfos();
};

#endif
