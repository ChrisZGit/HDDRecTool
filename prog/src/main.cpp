
#include <stdexcept>
#include <chrono>
#include <thread>
#include <sstream>
#include <fstream>


int main(int argc, char *argv[])
{
	//Check the Parameters
	if(argc < 5)
	{
		std::cout << "Running without Parameters." << std::endl;
		std::cout << "Maybe try next time './bin/runme [Path to Image] [RaidVersion] [Stripesize] [Number of lost Images]'"
	}
	else if(argc < 2)
	{
        std::cerr << "Not enough Parameters. Try at least './bin/runme [Path to Image]'" << std::endl;
        return 1;
    }

	//Check the Raid System
	int raidVersion = 6;
	//Raid System was set
	if(argc > 2)
	{
		raidVersion = argv[3];
		if(raidVersion != 0 && raidVersion != 1 && raidVersion != 5)
		{
			std::cout << "No useful version was set. Tool is trying to estimate it on it's own." << std::endl;
		}
	}

	//Raid System was not set
	//call checkRaidSystem-Class
	//this should return
		//it's raid 0, 1 or 5
		//the stripesize
		//the stripemap


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

