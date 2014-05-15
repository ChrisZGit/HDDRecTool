#ifndef DEFINES_H
#define DEFINES_H

#define BUFLENGTH (2*1024*1024)

#define BLOCKSIZE (1024*64)

#include <string>

typedef struct systemInfo
{
	size_t size;		//3
	std::string dateModified;//5
	std::string dateCreated;//6
	std::string dateAccessed;//7
	std::string MIMEType;//21
	std::string fileName;//31
	std::string owner;//61
	std::string cacheID;//213
} systemInfo;

typedef struct pictureInfo
{
	double hRes;//74
	double vRes;//75
	size_t bitDepth;//76
} pictureInfo;

typedef struct edbInfo
{
	systemInfo sysInfo;
	pictureInfo picInfo;
} edbInfo;

std::string const TABLENAME="SystemIndex_0A";

#endif
