#ifndef _IMAGE_CARVER_H
#define _IMAGE_CARVER_H

#include <iostream>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <sstream>
#include <string>

#include <defines.h>

typedef std::vector<std::pair<std::string/*inode*/, std::string /*filename*/>> Database;
typedef std::vector<std::pair<std::string /*username*/, Database>> UserDatas;
typedef std::vector<std::pair<size_t/*offset*/, UserDatas>> Partitions;

class ImageCarver
{
	private:
		std::string fileName;
		std::string outputPath;

		Partitions partition;

		void mmls();
		void fls(size_t offset, UserDatas &);
		void icat(size_t offset, std::string inode, std::string name);
		std::string carveEDB(size_t offset);
		
	public:
		~ImageCarver();
		ImageCarver(std::string, std::string);

		bool carveImg();

};

#endif
