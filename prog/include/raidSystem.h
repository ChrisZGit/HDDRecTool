#ifndef RAIDSYSTEM_H
#define RAIDSYSTEM_H

#include <vector>
#include <iostream>
#include <cstdlib>

#include <fileHandler.h>
#include <defines.h>

#define CHECKSIZE 512

enum Raid
{
	Raid0=0,
	Raid1=1,
	Raid5=5,	//this one is only set by user
	Raid5_corrupt=5,
	Raid5_complete=5,
	Raid_unknown
};

typedef struct pattern
{
	size_t drive;
	bool isParity;
} Pattern;

class RaidSystem
{
private:
	
	FileHandler *handle;
	// if raidsystem, stripe or lostCount is set, we dont need to test for it
	// initial values are Raid_unknown, -1, -1
	Raid raidSystem;
	size_t stripeSize;
	int lostImages;
	std::vector<Pattern> stripePattern;

	//raid1_check easiest, afterwards easyCheck, to check for raid5_complete
	//intensive_check for rest, to determine raid1 or raid5_corrupt, or too much corrupt
	//to do anything with it
	bool checkForNull(char *in, size_t size);
	bool checkForEqual(char *buf, char *in, size_t size);
	bool checkIfRaid1();
	bool checkIfRaid5();
	bool intensiveCheck();

public:
	RaidSystem();
	RaidSystem(FileHandler *fileHandler);

	bool raidCheck();

	Raid getRaid();
	size_t getStripeSize();
	std::vector<Pattern> getPattern();
	bool recoverLostImage();

	void setStripeSize(int i);
	void setRaid(int i);
	void setLostImages(int i);
};

#endif
