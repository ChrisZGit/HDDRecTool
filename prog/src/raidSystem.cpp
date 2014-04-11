#include <raidSystem.h>

RaidSystem::RaidSystem()
{
	std::cerr << "Wrong Constructor" << std::endl;
}

RaidSystem::RaidSystem(FileHandler *fileHandler)
{
	handle = fileHandler;
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
	unsigned int count = 0;
	for (size_t i = 0; i < size; ++i)
	{
		if (in[i] != buf[i])
			++count;
	}
	if (count > (size/100))
		return false;
	
	return true;
}

bool RaidSystem::checkIfRaid1()
{
	char buf[CHECKSIZE];
	char *checkAgainstMe;
	char *in;
	int startAdress = 0;
	int hits=0, misses=0;

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
	checkAgainstMe = inFiles.at(0)->getBuffer();
	for (int j = 0; j < CHECKSIZE; ++j)
	{
		buf[j] = 0;
	}

	for (int count=0; count < 5; ++count)
	{
		for (int i = 0; i < 20; ++i)
		{
			startAdress = rand() % (inFiles.at(0)->getBufferSize()-CHECKSIZE);
			for (unsigned int j = 1; j < inFiles.size(); ++j)
			{
				in = inFiles.at(j)->getBuffer();
				for (int x = 0; x < CHECKSIZE; ++x)
				{
					buf[x] = checkAgainstMe[x+startAdress]^in[x+startAdress];
				}
				if (checkForNull(buf, CHECKSIZE)==false)
				{
					++misses;
				} else
				{
					++hits;
				}
			}
		}
		do
		{
			for (unsigned int j = 0; j < inFiles.size(); ++j)
			{
				if (inFiles.at(j)->newBlock() == false)
				{
					std::cerr << "Too few written blocks to test perfectly, heuristic choice done." << std::endl;
					for (unsigned int x = 0; x < inFiles.size(); ++x)
						inFiles.at(x)->reset();
					if (hits > misses*10)
					{
						raidSystem = Raid1;
						return true;
					}
					return false;
				}
			}
		} while (inFiles.at(0)->emptyBlock()==true);
	}
	for (unsigned int j = 0; j < inFiles.size(); ++j)
		inFiles.at(j)->reset();
	if (hits > misses*10)
	{
		raidSystem = Raid1;
		return true;
	}
	return false;
}

bool RaidSystem::checkIfRaid5()
{
	char buf[CHECKSIZE];
	char *checkAgainstMe;
	char *in;
	int startAdress = 0;
	int hits=0, misses=0;

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
	checkAgainstMe = inFiles.at(0)->getBuffer();

	for (int count=0; count < 5; ++count)
	{
		for (int i = 0; i < 20; ++i)
		{
			startAdress = rand() % (inFiles.at(0)->getBufferSize()-CHECKSIZE);
			for (int j = 0; j < CHECKSIZE; ++j)
			{
				buf[j] = 0;
			}
			for (unsigned int j = 1; j < inFiles.size(); ++j)
			{
				in = inFiles.at(j)->getBuffer();
				for (int x = 0; x < CHECKSIZE; ++x)
				{
					buf[x] = buf[x]^in[x+startAdress];
				}
			}
			if (checkForEqual(buf, checkAgainstMe+startAdress, CHECKSIZE)==false)
			{
				++misses;
			} else
			{
				++hits;
			}
		}
		do
		{
			for (unsigned int j = 0; j < inFiles.size(); ++j)
			{
				if (inFiles.at(j)->newBlock() == false)
				{
					std::cerr << "Too few written blocks to test perfectly, heuristic choice done." << std::endl;
					for (unsigned int x = 0; x < inFiles.size(); ++x)
						inFiles.at(x)->reset();
					if (hits > misses*10)
					{
						raidSystem = Raid5_complete;
						return true;
					}
					return false;
				}
			}
		} while (inFiles.at(0)->emptyBlock()==true);
	}
	for (unsigned int j = 0; j < inFiles.size(); ++j)
		inFiles.at(j)->reset();
	if (hits > misses*10)
	{
		raidSystem = Raid5_complete;
		return true;
	}
	return false;
}

bool RaidSystem::recoverLostImage()
{
	if (lostImages == 0)
	{
		std::cout << "No lost Images to recover" << std::endl;
		return true;
	} else if (lostImages > 1)
	{
		std::cerr << "Too many lost Images to recover" << std::endl;
		return false;
	}
	char buf[BUFLENGTH];
	std::vector<FileReader *> inFiles = handle->getInFiles();
	for (unsigned int j = 0; j < inFiles.size(); ++j)
		inFiles.at(j)->reset();
	while (inFiles.at(0)->getBufferSize() > 0)
	{
		for (int i = 0; i < BUFLENGTH; ++i)
		{
			for (size_t j = 0; j < inFiles.size(); ++j)
			{
				buf[i] = buf[i]^inFiles.at(j)->getBuffer()[i];
			}
		}
		handle->getFileWriter()->writeToFile(buf,BUFLENGTH);
		for (unsigned int j = 0; j < inFiles.size(); ++j)
			inFiles.at(j)->newBlock();
	}
	return true;
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
		std::cout << "Starting with check for complete raid5" << std::endl;
		found = checkIfRaid5();
		if (found == false)
		{
			std::cout << "FAILED!\n Check against raid1." << std::endl;
			found = checkIfRaid1();
		}
		if (found == false)
		{
			std::cout << "FAILED!\n Now the intensive check to estimate the raid version begins." << std::endl;
			found = intensiveCheck();
		}
	}
	return found;
}

