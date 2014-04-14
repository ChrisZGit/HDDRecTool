#include <raidRecover.h>

RaidRecover::RaidRecover()
{
	std::cerr << "Wrong Constructor" << std::endl;
}

RaidRecover::RaidRecover(std::string inPath, std::string outPath)
{
	handle = new FileHandler(inPath, outPath);
	system = new RaidSystem(handle);
}

void RaidRecover::setStripeSize(int i)
{
	system->setStripeSize(i);
}

void RaidRecover::setLostImages(int i)
{
	system->setLostImages(i);
}

void RaidRecover::setRaid(int i)
{
	system->setRaid(i);
}

void RaidRecover::run()
{
	bool found = system->raidCheck();
	if (found==true)
		std::cout << "Recovery successful: " << system->getRaid() << std::endl;
	else
		std::cout << "Recovery was aborted." << std::endl;
}

