#ifndef RAIDSYSTEM_H
#define RAIDSYSTEM_H

#include <vector>
#include <iostream>

enum Raid
{
	Raid0,
	Raid1,
	Raid5_corrupt,
	Raid5_complete,
	Raid_unknown
};

typedef struct pattern
{
	int drive;
	bool isParity;
} Pattern;

class RaidSystem
{
private:
	
	Raid raidSystem;
	std::vector<Pattern> stripePattern;

	bool easyCheck();
	bool intensiveCheck();

public:
	RaidSystem();

	bool raidCheck();

	Raid getRaid();
	int getStripeSize();
	std::vector<Pattern> getPattern();
};

#endif
