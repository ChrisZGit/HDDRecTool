#ifndef RAIDRECOVER_H
#define RAIDRECOVER_H

#include <iostream>
#include <string>

#include <fileHandler.h>
#include <raidSystem.h>

class RaidRecover
{
	private:
		std::string outP, inP;
		FileHandler *handle;
		RaidSystem *system;

	public:
		RaidRecover();
		RaidRecover(std::string inPath, std::string outPath); //this should get a FileHandler....not make a new one

		void setStripeSize(int i);
		void setLostImages(int i);
		void setRaid(int i);

		void run(); //this calls the raid-check
};

#endif
