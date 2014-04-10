
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




	
	return  0;
}

