#ifndef RAIDSYSTEM_H
#define RAIDSYSTEM_H

#include <vector>
#include <iostream>
#include <cstdlib>
#include <fstream>

#include <fileHandler.h>
#include <defines.h>

//#define CHECKSIZE 512

enum Raid
{
	Raid0=0,
	Raid1=1,
	Raid5_user=5,	//this one is only set by user
	Raid5_incomplete=5,
	Raid5_complete=5,
	Raid_unknown
};

/*
typedef struct pattern
{
	size_t drive;
	bool isParity;
} Pattern;
*/
class RaidSystem
{
private:
	
	FileHandler *handle;
	// if raidsystem, stripe or lostCount is set, we dont need to test for it
	// initial values are Raid_unknown, -1, -1
	Raid raidSystem;
	std::vector<std::string> dictionary;
	int stripeSize;
	int lostImages;
//	std::vector<Pattern> stripePattern;

	bool checkForNull(char *in, size_t size);
	bool checkForEqual(char *buf, char *in, size_t size);
	bool easyCheck();
//	bool intensiveCheck();
	void loadDictionary(std::string in="/usr/share/dict/american-english");
	bool calculateStripeSize();

public:
	RaidSystem();
	RaidSystem(FileHandler *fileHandler);
	Raid getRaid();
	size_t getStripeSize();
	void setStripeSize(int i);
	void setRaid(int i);
	void setLostImages(int i);

	bool recoverLostImage();
	bool raidCheck();

//	std::vector<Pattern> getPattern();
//	bool recoverLostImage();

};

#endif
