#ifndef DEFINES_H
#define DEFINES_H

#define BUFLENGTH (2*1024*1024)

#define BLOCKSIZE (1024*64)

#include <string>
#include <vector>

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

typedef struct dbInfo
{
	std::string absoluteFileName;
	size_t offset;
	size_t dataSize;
	std::string hash;
	std::string dataChecksum;
	std::string headerChecksum;
	std::string md5Sum;
	bool foundInEDB;
} dbInfo;

typedef std::pair<edbInfo, dbInfo> FileInfo;
typedef std::pair<std::string, std::vector<FileInfo>> ThumbCacheFiles;
typedef std::pair<std::string, std::vector<ThumbCacheFiles>> UserFiles;
typedef std::pair<std::string, std::vector<UserFiles>> PartitionFiles;

std::string const TABLENAME="SystemIndex_0A";

#endif
