#ifndef _XML_MAKER_H
#define _XML_MAKER_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <defines.h>

class XmlMaker
{
	private:
		std::string path;
		std::ofstream file;
		bool edbOnly;

		std::string cleanString(std::string &);
		void writeExif(pictureInfo &pic);
		void writeSys(systemInfo &sys);
		void writeMeta(dbInfo &db);
		
		void writeFile(FileInfo &f, bool);
		void writeThumb(ThumbCacheFiles &t);
		void writeUser(UserFiles &u);
		void writePartition(PartitionFiles &p);
		
	public:
		XmlMaker(std::string p);

		void writeXml(std::vector<PartitionFiles> &p, bool edb);
};

#endif
