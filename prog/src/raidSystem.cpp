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
		raidSystem = Raid5_user;
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

bool RaidSystem::easyCheck()
{
	char buf1[CHECKSIZE];
	char buf5[CHECKSIZE];
	char *checkAgainstMe;
	char *in;
	int startAdress = 0;
	int raid1=0, raid5=0, misses=0;

	std::vector<FileReader *> inFiles = handle->getInFiles();
	if(inFiles.size() < 2)
	{
		std::cout << "There are too few devices given for an easy Raid1-check." << std::endl;
		return false;
	}
	if (handle->findGoodBlock() == false)
	{
		std::cerr << "No valid readable block found! Image too broken" << std::endl;
		return false;
	}

	for (int j = 0; j < CHECKSIZE; ++j)
	{
		buf1[j] = 0;
	}

	for (int count=0; count < 500; ++count)
	{
		for (int i = 0; i < 20; ++i)
		{
			checkAgainstMe = inFiles.at(0)->getBuffer();
			startAdress = rand() % (inFiles.at(0)->getBufferSize()-CHECKSIZE);
			int raid1_miss = 0;
			int raid1_hit = 0;
	for (int j = 0; j < CHECKSIZE; ++j)
	{
		buf5[j] = 0;
	}
			for (unsigned int j = 1; j < inFiles.size(); ++j)
			{
				in = inFiles.at(j)->getBuffer();
				for (int x = 0; x < CHECKSIZE; ++x)
				{
					buf1[x] = checkAgainstMe[x+startAdress]^in[x+startAdress];
					buf5[x] = buf5[x]^in[x+startAdress];
				}
				if (checkForNull(buf1, CHECKSIZE)==false)
				{
					++raid1_miss;
					for (int x = 0; x < CHECKSIZE; ++x)
					{
						buf1[x] = 0;
					}
				} else
				{
					++raid1_hit;
				}
			}
			if (checkForEqual(buf5,checkAgainstMe,CHECKSIZE) == true)
			{
				++raid5;
			} else if (raid1_miss == 0)
			{
				++raid1;
			} else
			{
				++misses;
			}
		}
		if (handle->findGoodBlock() == false)
		{
			std::cout << "Couldnt test enough blocks. heuristic guess takes place." << std::endl;
			break;
		}
	}
	std::cout << raid1 << " " << raid5 << " " << misses << std::endl;
	for (unsigned int j = 0; j < inFiles.size(); ++j)
		inFiles.at(j)->reset();
	if (raid1 > (misses+raid5)*2)
	{
		raidSystem = Raid1;
		return true;
	} else if (raid5 > (misses+raid1)*2)
	{
		raidSystem = Raid5_complete;
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
		std::cout << "Starting with check for complete raid5 or raid1" << std::endl;
		found = easyCheck();
		if (found == false)
		{
			std::cout << "FAILED!\n Now the intensive check to estimate the raid version begins." << std::endl;
			found = intensiveCheck();
		}
	} else if (raidSystem == Raid5_user)
	{
		if (lostImages == -1)
		{
			std::cout << "User input: Raid5! Checking for lost Images or if it's raid1" << std::endl;
			found = easyCheck();
			if (found == false)
			{
				if (raidSystem == Raid1)
				{
					std::cerr << "User input wrong. Estimated raid1!" << std::endl;
					return false;
				} else
				{
					std::cout << "Images missing. Will recover image later!" << std::endl;
					raidSystem = Raid5_corrupt;
				}
				found = true;
			} else
			{
				std::cout << "Raid 5 is complete. No missing images indicated!" << std::endl;
			}
		} else if (lostImages == 0)
		{
			raidSystem = Raid5_complete;
			found = true;
		} else
		{
			raidSystem = Raid5_corrupt;
			found = true;
		}
	} else if (raidSystem == Raid1)
	{
		found = true;
	} else if (raidSystem == Raid0)
	{
		if (lostImages > 0)
		{
			std::cerr << "User specifies Raid0 with lost images. Cannot be recovered!" << std::endl;
			return false;
		}
		found = true;
	}
	return found;
}

