#include <raidSystem.h>

RaidSystem::RaidSystem()
{
	std::cerr << "Wrong Constructor for RaidSystem." << std::endl;
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

size_t RaidSystem::getStripeSize()
{
	return stripeSize;
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
	unsigned int startAdress = 0;
	unsigned int raid1=0, raid5=0, misses=0;

	std::vector<FileReader *> inFiles = handle->getInFiles();
	if(inFiles.size() < 2)
	{
		std::cout << "\tThere are too few devices given for an easy Raid1-check." << std::endl;
		return false;
	}
	if (handle->findGoodBlock() == false)
	{
		std::cerr << "\tNo valid readable block found! Image too broken!" << std::endl;
		//return false;
	}

	for (int j = 0; j < CHECKSIZE; ++j)
	{
		buf1[j] = 0;
	}

	std::cout << "\tMake some tests to check if a valid Raid-version can be estimated." << std::endl;
	for (int count=0; count < 40000; ++count)
	{
		for (int i = 0; i < 5; ++i)
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
			std::cout << "\tCouldnt test enough blocks. heuristic guess takes place." << std::endl;
			break;
		}
	}
	std::cout << "\tHits for Raid1: " << raid1 << std::endl;
	std::cout << "\tHits for Raid5: " << raid5 << std::endl;
	std::cout << "\tMisses: " << misses << std::endl;
	handle->reset();
	if (raid1 > (misses+raid5)*1.5)
	{
		raidSystem = Raid1;
		std::cout << "Found a raid version with the easy check: 1" << std::endl;
		return true;
	} else if (raid5 > (misses+raid1)*1.5 && inFiles.size() >= 3)
	{
		raidSystem = Raid5_complete;
		std::cout << "Found a raid version with the easy check: 5 complete" << std::endl;
		return true;
	}
	//see if you can find 2 same partitiontables at 2 of all hdds, if so, its raid5_incomplete
	//otherwise it still can be raid5_inc or raid0
	std::vector<char *> tables;
	for (unsigned int i = 0; i < inFiles.size(); ++i)
	{
		char *buf = inFiles.at(i)->getBuffer();	
		if (checkForNull(buf, 512)==false)
			tables.push_back(buf);
	}
	if (tables.size() == 2)
	{
		if (checkForEqual(tables.at(0),tables.at(1),512)==true)
		{
			lostImages = 1;
			raidSystem = Raid5_incomplete;
			std::cout << "Found a raid version with the easy check: 5 incomplete" << std::endl;
			handle->reset();
			return true;
		}
	}
	if (raid1 < (100/(inFiles.size()+1))*misses && raid1 > 0.007*(float)misses)
	{
		lostImages = 1;
		raidSystem = Raid5_incomplete;
		std::cout << "Found a raid version with the easy check: 5 incomplete" << std::endl;
		handle->reset();
		return true;
	}
	else if (raid1 > (100/(inFiles.size()+1))*misses)
	{
		raidSystem = Raid1;
		std::cout << "Found a raid version with the easy check: 1" << std::endl;
		return true;
	}
	else
	{
		raidSystem = Raid0;
		std::cout << "Found a raid version with the easy check: 0" << std::endl;
		return true;
	}
//	handle->reset();
	return false;
}

//currently not needed
/*
bool RaidSystem::intensiveCheck()
{
	return true;
}
*/
//currently not needed
void RaidSystem::loadDictionary(std::string input)
{
	std::fstream in(input.c_str(), std::fstream::in);
	std::string tmp="";
	std::string row="";
	char cmp='\0';
	while (in.good() && !in.eof())
	{
		in >> tmp;
		if (tmp.size()>0)
		{
			if (tmp.at(0) != cmp)
			{
				cmp = tmp.at(0);
				dictionary.push_back(row);
				row ="";
			} else
			{
				if (tmp.at(tmp.size()-2) != 39)
					row += tmp+" ";
			}
		}
	}
	dictionary.at(13) += " NTFS";
	dictionary.at(33) += " http";
	dictionary.at(7) += " HTTP";
}

bool RaidSystem::calculateStripeSize()
{
	int ret = handle->estimateStripeSize();
	if (ret == -1)
		return false;
	this->stripeSize = ret;
	return true;
}

bool RaidSystem::recoverLostImage()
{
	if (lostImages == 0)
	{
		std::cout << "\tNo lost Images to recover" << std::endl;
		return true;
	} else if (lostImages > 1)
	{
		std::cerr << "\tToo many lost Images to recover" << std::endl;
		return false;
	}
	std::vector<FileReader *> inFiles = handle->getInFiles();
	for (unsigned int j = 0; j < inFiles.size(); ++j)
		inFiles.at(j)->reset();
	if (raidSystem == Raid5_incomplete)
	{
		size_t bufferlength = inFiles.at(0)->getBufferLength();
//		size_t blocklength = inFiles.at(0)->getBlockSize();
		double written = 0.0f;
		size_t runs=0;
		char *buf = new char[bufferlength];
		for (unsigned int j = 0; j < inFiles.size(); ++j)
			inFiles.at(j)->reset();
		while (true)
		{
			bufferlength = inFiles.at(0)->getBufferLength();
			for (unsigned int i = 0; i < bufferlength; ++i)
			{
				buf[i] = 0;
			}
			++runs;
			for (size_t j = 0; j < inFiles.size(); ++j)
			{
				FileReader *reader = inFiles.at(j);
				char *read = reader->getBuffer();
				for (unsigned int i = 0; i < bufferlength; ++i)
				{
					buf[i] = buf[i]^read[i];
				}
			}
			handle->getFileWriter()->writeToFile(buf,bufferlength);
			written += bufferlength/1024.0f/1024.0f;
			printf("\r%f MB finished", written);
			fflush(stdout);
			for (unsigned int j = 0; j < inFiles.size(); ++j)
			{
				if (inFiles.at(j)->asyncReload() == false)
				{
					handle->getFileWriter()->closeFile();
					std::cout << std::endl;
					for (unsigned int t = 0; t < inFiles.size(); ++t)
						inFiles.at(t)->reset();
					return true;
				}
			}
		}
		std::cout << std::endl;
		handle->getFileWriter()->closeFile();
		for (unsigned int j = 0; j < inFiles.size(); ++j)
			inFiles.at(j)->reset();
		return true;
	}
	for (unsigned int j = 0; j < inFiles.size(); ++j)
		inFiles.at(j)->reset();
	return false;
}

bool RaidSystem::raidCheck()
{
	bool found = false;
	if (raidSystem == Raid_unknown)
	{
		std::cout << "Starting with Raid check." << std::endl;
		found = easyCheck();
		if (found == false)
		{
			std::cout << "FAILED!\n Now the intensive check to estimate the raid version begins." << std::endl;
		} else 
		{
			char answer;
			std::cout << "Do you want to accept this? [y/n]";
			std::cin >> answer;
			if (answer == 'n')
			{
				std::cout << "Raid Version can not be estimated automatically. Must be handed over with the program start." << std::endl;
				return false;
			}
			//std::cout << "Found a raid version with the easy check: 5" << raidSystem  << std::endl;
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
					raidSystem = Raid5_incomplete;
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
			raidSystem = Raid5_incomplete;
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

	//raidType almost done
	if (found == false)
	{
		std::cerr << "No valid Raidsystem found! Have to abort!" << std::endl;
		return false;
	}
	if (raidSystem == Raid5_incomplete)
	{
		std::cout << "Incomplete Raid5-System. Will recover image and mount it internally." << std::endl;
		recoverLostImage();
		handle->addImage(handle->getFileWriter()->getPath());
	}

	//estimate the stripesize
	if (stripeSize == -1)
	{
		std::cout << "No stripesize specified! Trying to calculate Stripesize now." << std::endl;
		found = calculateStripeSize();
	} else
	{
		std::cout << "Using stripesize specified by user!" << std::endl;
		found = true;
	}
	if (found == true)
	{
		std::cout << "Stripesize: " << stripeSize << std::endl;
	} else
	{
		std::cerr << "No valid Stripesize found. User should estimate it on its own, or Image could not be recovered." << std::endl;
		return false;
	}

	// Datatype: std::vector<std::pair<int, bool>>
	// int: tells how the stripes are written (data and paritiy are handled seperately)
	// bool: if its parity or not (true = Parity)
	// Example:
	//	0,0		1,0		0,1
	//	1,1		2,0		3,0
	//	4,0		2,1		5,0
	std::cout << "Trying to estimate the stripemap." << std::endl;
	std::vector<std::pair<int, bool>> stripeMap;
	stripeMap = handle->estimateStripeMap();

	//if stripeMap valid, basst
	//else return false
	
	if (stripeMap.size() == 0)
	{
		std::cerr << "No valid Stripemap found. Has to abort Recovery." << std::endl;
		return false;
	}
	else
	{
		found = true;
	}

	return found;
}

