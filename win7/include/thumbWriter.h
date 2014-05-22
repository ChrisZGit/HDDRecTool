#ifndef THUMBWRITER_H_
#define THUMBWRITER_H_

#include <iostream>
#include <fstream>
#include <cstdlib>

#include <texMaker.h>
#include <xmlMaker.h>
#include <dataHandler.h>

class ThumbWriter
{
	private:
		std::string inPath;
		std::string outPath;
		bool writeTex;
		bool writeXml;
		bool edbOnly;
		bool isExtracted;
		size_t offset;

	public:
		ThumbWriter(std::string in, std::string out);

		void setTex(bool tex);
		void setXml(bool xml);
		void setEdb(bool edb);
		void setExtracted(bool ex);
		void setOffset(size_t off);

		void writeThumbs();


};

#endif
