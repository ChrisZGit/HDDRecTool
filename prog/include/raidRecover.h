#ifndef RAIDRECOVER_H
#define RAIDRECOVER_H

#include <iostream>

#include <fileHandler.h>
#include <raidSystem.h>

class RaidRecover
{
	private:
		FileHandler *handle;
		RaidSystem *system;

	public:
		RaidRecover();
		RaidRecover(std::string inPath, std::string outPath);

		void setStripeSize(int i);
		void setLostImages(int i);
		void setRaid(int i);

		void run();
};

#endif
