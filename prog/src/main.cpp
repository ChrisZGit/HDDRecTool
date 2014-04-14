
#include <fstream>
#include <cstdlib>
#include <iostream>

#include <raidRecover.h>

int main(int argc, char *argv[])
{
	//Check the Parameters
	std::string inPath;
	std::string outPath;
	if (argc < 3)
	{
		std::cerr << "Not enough Parameters. Try at least './bin/runme [Path to Image] [Path to Output-Folder]'" << std::endl;
		return 1;
	} 
	else if (argc < 6)
	{
		std::cout << "Running without Parameters." << std::endl;
		std::cout << "Maybe try next time './bin/runme [Path to Image] [Path to Output-Folder] [RaidVersion] [Stripesize] [Number of lost Images]'" << std::endl;
	}

	//Check the Raid System
	inPath=argv[1];
	if (!(inPath.at(inPath.size()-1) == '/'))
		inPath+="/";
	outPath = argv[2];
	if (!(outPath.at(outPath.size()-1) == '/'))
		outPath += "/";
	int raidVersion = 6,stripeSize, lostImages;

	RaidRecover raidR(inPath,outPath);
	//Raid System was set
	if (argc > 3)
	{
		raidVersion = atoi(argv[3]);
		if (raidVersion != 0 && raidVersion != 1 && raidVersion != 5)
		{
			std::cout << "No useful version was set. Tool is trying to estimate it on it's own." << std::endl;
		} else
		{
			raidR.setRaid(raidVersion);
		}
	}
	//Stripesize was set
	if (argc > 4)
	{
		stripeSize = atoi(argv[4]);
		raidR.setStripeSize(stripeSize);
	}

	//Number of lost Images was set
	if (argc > 5)
	{
		lostImages = atoi(argv[5]);
		raidR.setLostImages(lostImages);
	}

	//Call the Raid Recovery
	std::cout << "Interpreted parameters. Now starting analysing the raid-system." << std::endl;
	raidR.run();
	

	//Partition is checked
	//call partitionCheck-Class
	//this should return
		//Partition Type
		//Other Parameters


	//Image should be created and lost drives should be recreated
	//call createImages-Class
	//this should return if it was successfull
	//there is also the check, if the images could be restored completely and where the bugs were


	return  0;
}

