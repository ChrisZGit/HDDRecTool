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
		std::cout << "\tThere are too few devices given for an easy Raid1-check." << std::endl;
		return false;
	}
	if (handle->findGoodBlock() == false)
	{
		std::cerr << "\tNo valid readable block found! Image too broken" << std::endl;
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
			std::cout << "\tCouldnt test enough blocks. heuristic guess takes place." << std::endl;
			break;
		}
	}
//	std::cout << raid1 << " " << raid5 << " " << misses << std::endl;
	handle->reset();
	if (raid1 > (misses+raid5)*1.5)
	{
		raidSystem = Raid1;
		return true;
	} else if (raid5 > (misses+raid1)*1.5 && inFiles.size() >= 3)
	{
		raidSystem = Raid5_complete;
		return true;
	}
	return false;
}

bool RaidSystem::intensiveCheck()
{
	return false;
}

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
	std::cout << "\tLoading dictionary" << std::endl;
	loadDictionary();
	handle->reset();
	std::cout << "\tLoading done! Starting the actual calculation" << std::endl;

	std::vector<FileReader *> inFiles = handle->getInFiles();

	bool found=false;
	std::vector<std::string> strings;
	while (found == false)
	{
		for (unsigned int i = 0; i < inFiles.size(); ++i)
		{
			int adr = inFiles.at(i)->findFirstNonemptyBlock();
			if (adr != -1)
			{
				inFiles.at(i)->setOffset(adr);
				strings = inFiles.at(i)->getAllStringsInBlock();
				for (unsigned int j = 0; j < strings.size(); ++j)
				{
					size_t pos = std::string::npos;
					int index=strings.at(j)[0]-65;
					std::string findMe = strings.at(j);
					if (index >= 32 && index < 58)
					{
						index -= 6;
						pos = dictionary.at(index).find(findMe);
					} else if (index >= 0 && index < 26)
					{
						pos = dictionary.at(index).find(findMe);
					}
					if (!(pos == std::string::npos))
					{
						j = strings.size();
						std::cout << "\tStringblock found. Trying to estimate the stripesize next. " << std::endl;
						//	inFiles.at(i)->printBlock();
						//	std::cout << "FOUND: " << findMe << std::endl;
						std::cout << "\tStripesize couldn't be estimated with current block!" << std::endl;
					}
				}
			}
		}
		/*
		   for (unsigned int i = 0; i < dictionary.size(); ++i)
		   {
		   hdd=handle->findStringInBlock(dictionary.at(i));
		   if (hdd != -1)
		   {
		   std::cout << "Found\t" << dictionary.at(i) <<  "\tat hdd: " << hdd << std::endl;
		   }
		   if (i % 100==0)
		   {
		   std::cout << "seeking: " << dictionary.at(i) << std::endl;
		   }
		   }
		 */
		/*
		   std::string ahoj ="";
		//ahoj += (unsigned char)0xFF;
		//ahoj += (char)0xD8;
		//ahoj += 0xFF;
		//ahoj += 0xE0;
		hdd=handle->findStringInBlock(ahoj);
		if (hdd != -1)
		{
		std::cout << "Found\t" << ahoj <<  "\tat hdd: " << hdd << std::endl;
		}
		 */
		if (handle->reloadBuffers() == false)
		{
			//end reached
			std::cout << "\tEnd of File reached. No good block for the stripesize-estimation was found. Must abort recovery." << std::endl;
			return false;
		}
	}

	//std::string tmp = "be install";
	//int adress = handle->findString(tmp);
	//std::cout << adress << std::endl;
	return false;
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
			if (found == false)
			{
				std::cout << "FAILED!\n Intensive check delivers no result. Aborting the analysis." << std::endl;
				return false;
			} else 
			{
				std::cout << "Found a valid Raidsystem with intensive check: " << raidSystem << std::endl;
			}
		} else 
		{
			std::cout << "Found a raid version with the easy check: " << raidSystem  << std::endl;
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
	if (found == false)
	{
		std::cerr << "No valid Raidsystem found! Have to abort!" << std::endl;
		return false;
	}
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
		return false;
	}
	return found;
}

