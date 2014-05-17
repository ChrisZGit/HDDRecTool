#ifndef _TEX_MAKER_H
#define _TEX_MAKER_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <defines.h>

class TexMaker
{
	private:
		std::string path;
		std::ofstream file;

		void writeExif(pictureInfo &pic);
		void writeSys(systemInfo &sys);
		void writeMeta(dbInfo &db);
		
		void writeFile(FileInfo &f);
		void writeThumb(ThumbCacheFiles &t);
		void writeUser(UserFiles &u);
		void writePartition(PartitionFiles &p);
		
	public:
		TexMaker(std::string p);

		void writeTex(std::vector<PartitionFiles> &p);
};

#endif
