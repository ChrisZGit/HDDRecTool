#include <raidRecover.h>

RaidRecover::RaidRecover()
{
	std::cerr << "Wrong Constructor" << std::endl;
}

RaidRecover::RaidRecover(std::string inPath, std::string outPath)
{
	handle = new FileHandler(inPath, outPath);
	system = new RaidSystem(handle);
	std::cout << (void *)handle << std::endl;
}

void RaidRecover::setStripeSize(int i)
{
	system->setStripeSize(i);
}

void RaidRecover::setLostImages(int i)
{
	system->setStripeSize(i);
}

void RaidRecover::setRaid(int i)
{
	system->setRaid(i);
}

void RaidRecover::run()
{
	bool found = system->raidCheck();
	if (found==true)
		std::cout << "found " << std::endl;

}

