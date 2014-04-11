#include <raidSystem.h>

RaidSystem::RaidSystem()
{
	std::cerr << "Wrong Constructor" << std::endl;
}

RaidSystem::RaidSystem(FileHandler *fileHandler)
{
	handle = fileHandler;
	std::cout << (void *)handle << std::endl;
	raidSystem = Raid_unknown;
	stripeSize = -1;
	lostImages = -1;
	raidSystem = Raid_unknown;
	srand(time(NULL));
}

Raid RaidSystem::getRaid()
{
	return raidSystem;
}

void RaidSystem::setStripeSize(int i)
{
	stripeSize = i;
}

void RaidSystem::setRaid(int i)
{
	if (i == 0)
		raidSystem = Raid0;
	if (i == 1)
		raidSystem = Raid1;
	if (i == 5)
		raidSystem = Raid5;
}

void RaidSystem::setLostImages(int i)
{
	lostImages = i;
}

bool RaidSystem::checkForNull(char *in, size_t size)
{
	for (size_t i = 0; i < size; ++i)
	{
		if (in[i] > 0)
			return false;
	}
	return true;
}

bool RaidSystem::checkForEqual(char *buf, char *in, size_t size)
{
	int count = 0;
	for (size_t i = 0; i < size; ++i)
	{
		if (in[i] != buf[i])
			count++;
	}
	if (count > (size/100))
		return false;
	
	return true;
}

bool RaidSystem::raid1_check()
{
	char buf[BUFLENGTH/4];
	char *in;
	bool isNull=false;

	std::vector<FileReader *> inFiles = handle->getInFiles();
	if(inFiles.size() < 2)
	{
		std::cout << "There are too few devices given for an easy Raid1-check." << std::endl;
		return false;
	}
		
	while (inFiles.at(0)->emptyBlock()==true)
	{
		for (unsigned int i = 0; i < inFiles.size(); ++i)
			inFiles.at(i)->newBlock();
	}
	for (int j = 0; j < BUFLENGTH/4; ++j)
	{
		buf[j] = 0;
	}
	for (unsigned int j = 0; j < inFiles.size(); ++j)
	{
		in = inFiles.at(j)->getBuffer();
		for (int x = 0; x < BUFLENGTH/4; ++x)
		{
			buf[x] = buf[x]^in[x];
		}
	}
	isNull = checkForNull(buf, BUFLENGTH/4);
	if (isNull == true)
	{
		raidSystem = Raid1;
	}
	return isNull;
}

bool RaidSystem::easyCheck()
{
	//Here the easy raid5-check is implemented
	char buf[BUFLENGTH/4];
	char *in;
	bool isNull=false;

	std::vector<FileReader *> inFiles = handle->getInFiles();
	if(inFiles.size() < 3)
	{
		std::cout << "There are too few Devices for the easy Raid5-check." << std::endl;
		return false;
	}
	
	while (inFiles.at(0)->emptyBlock()==true)
	{
		for (unsigned int i = 0; i < inFiles.size(); ++i)
			inFiles.at(i)->newBlock();
	}
	for (int j = 0; j < BUFLENGTH/4; ++j)
	{
		buf[j] = 0;
	}
	for (unsigned int j = 0; j < inFiles.size(); ++j)
	{
		in = inFiles.at(j)->getBuffer();
		for (int x = 0; x < BUFLENGTH/4; ++x)
		{
			buf[x] = buf[x]^in[x];
		}
	}
	isNull = checkForEqual(buf, in, BUFLENGTH/4);
	if (isNull == true)
	{
		raidSystem = Raid5_complete;
	}

	return isNull;
}

bool RaidSystem::intensiveCheck()
{
	return false;
}

bool RaidSystem::raidCheck()
{
	bool found = false;
	if (raidSystem == Raid_unknown)
	{
		found = raid1_check();
		if (found == false)
		{
			std::cout << "Easy check on raid1 failed. Now begins the test on the Raid5 easy check." << std::endl;
			found = easyCheck();
		}
		if (found == false)
		{
			std::cout << "Two easy checks failed. Now the intensive check to estimate the raid version begins." << std::endl;
			found = intensiveCheck();
		}
	}
	return found;
}

