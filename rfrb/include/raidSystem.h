#ifndef RAIDSYSTEM_H
#define RAIDSYSTEM_H

#include <vector>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <iomanip>

#include <fileHandler.h>
#include <defines.h>

enum Raid
{
	Raid0,
	Raid1,
	Raid5_user,	//this one is only set by user
	Raid5_incomplete,
	Raid5_complete,
	Raid_unknown
};

class RaidSystem
{
private:
	
	FileHandler *handle;
	// if raidsystem, stripe or lostCount is set, we dont need to test for it
	// initial values are Raid_unknown, -1, -1
	Raid raidSystem;
	std::vector<std::string> dictionary;
	int stripeSize;
	std::vector<size_t> stripeMap;
	int lostImages;

	bool checkForNull(char *in, size_t size);
	bool checkForEqual(char *buf, char *in, size_t size);
	bool easyCheck();
	bool calculateStripeSize();
	bool buildDataImage(std::string path);

public:
	RaidSystem();
	RaidSystem(FileHandler *fileHandler);
	Raid getRaid();
	size_t getStripeSize();
	void setStripeSize(int i);
	void setRaid(int i);
	void setLostImages(int i);

	bool recoverLostImage();
	bool raidCheck(std::string path);
	void printAllInfos();

//	std::vector<Pattern> getPattern();
//	bool recoverLostImage();

};

#endif
