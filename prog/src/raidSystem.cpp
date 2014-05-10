#include <raidSystem.h>

RaidSystem::RaidSystem()
{
	std::cerr << "Wrong Constructor for Raid system." << std::endl;
	exit(EXIT_FAILURE);
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
	stripeSize = i*1024;
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
		std::cout << "\tThere are too few devices for a Raid system." << std::endl;
		return false;
	}
	if (handle->findGoodBlock() == false)
	{
		std::cerr << "\tNo valid readable block found! Disc too broken!" << std::endl;
		return false;
	}

	for (int j = 0; j < CHECKSIZE; ++j)
	{
		buf1[j] = 0;
	}

	std::cout << "\tTesting 150k blocks to check if a valid Raid-version can be estimated." << std::endl;
	for (int count=0; count < 30000; ++count)
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
			if (raid1_miss == 0)
			{
				++raid1;
			}
			else if (checkForEqual(buf5,checkAgainstMe+startAdress,CHECKSIZE) == true)
			{
				++raid5;
			} else
			{
				++misses;
			}
		}
		if (handle->findGoodBlock() == false)
		{
			std::cout << "\tCouldnt find enough testable blocks. Trying to estimate with heuristics." << std::endl;
			break;
		}
	}
	std::cout << "\tHits for Raid1\t: " << std::setw(6) << raid1 << "\t(mirrored blocks)"<< std::endl;
	std::cout << "\tHits for Raid5\t: " << std::setw(6) << raid5 << "\t(blocks with corresponding parity block)"<< std::endl;
	std::cout << "\tMisses\t\t: "       << std::setw(6) << misses << "\t(none of above)" << std::endl;
	handle->reset();
	if (raid1 > (misses+raid5)*1.5)
	{
		raidSystem = Raid1;
		std::cout << "Found a Raid 1 system." << std::endl;
		return true;
	} else if (raid5 > (misses+raid1)*1.5 && inFiles.size() >= 3)
	{
		raidSystem = Raid5_complete;
		std::cout << "Found a complete Raid 5 system." << std::endl;
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
			std::cout << "Found mirrored partition table. Has to be an incomplete Raid 5 system." << std::endl;
			handle->reset();
			return true;
		}
	}
	if (raid1 < (100/(inFiles.size()+1))*misses && (raid1+raid5) > 0.007*(float)misses)
	{
		lostImages = 1;
		raidSystem = Raid5_incomplete;
		std::cout << "Some mirrored blocks and some parity blocks found. It's probably an incomplete Raid 5 system." << std::endl;
		handle->reset();
		return true;
	}
	else if (raid1 > (100/(inFiles.size()+1))*misses)
	{
		raidSystem = Raid1;
		std::cout << "Above average mirrored blocks found. Has to be a Raid 1 system." << std::endl;
		return true;
	}
	else
	{
		raidSystem = Raid0;
		std::cout << "Too few mirrored blocks or parity infos found. Has to be a Raid 0 system." << std::endl;
		return true;
	}
	return false;
}

bool RaidSystem::calculateStripeSize()
{
	int ret = handle->estimateStripeSize();
	if (ret == -1)
		return false;
	this->stripeSize = ret;
	return true;
}

bool RaidSystem::buildDataImage(std::string path)
{
	std::vector<FileReader *> inFiles = handle->getInFiles();
	FileWriter *writeMe = handle->getFileWriter();
	writeMe->setPath(path);
	handle->reset();

	for (unsigned int j = 0; j < inFiles.size(); ++j)
	{
		inFiles.at(j)->reset();
		inFiles.at(j)->setBlockSize(stripeSize);
	}
	if (raidSystem == Raid5_user || raidSystem == Raid5_incomplete || raidSystem == Raid5_complete)
	{
		unsigned int finished = 0;
		const size_t bufferSize=(inFiles.size()-1)*stripeSize;
		char *buffer = new char[bufferSize];
		int *blocks = new int[inFiles.size()]{};
		char *parity = new char[stripeSize]{};
		size_t localStripe=stripeSize;

		unsigned int pointer=0;
		size_t written = 0;
		while (true)
		{
			int order = 1;
			for (unsigned int i = 0; i < inFiles.size(); ++i)
			{
				blocks[i]=0;
			}
			for (unsigned int i = 0; i < inFiles.size()-1; ++i)
			{
				blocks[stripeMap.at(i+pointer)]=order++;
			}
			order = 0;
			size_t skip = 0;
			do
			{
				for (unsigned int i = 0; i < inFiles.size(); ++i)
				{
					//use blocks in correct order
					if (blocks[i] == order && order == 0)
					{
						if (inFiles.at(i)->newBlock() == false)
						{
							if (inFiles.at(i)->asyncReload()== false )
							{
								++finished;
							}
						}
						order++;
						continue;
					}
					if (blocks[i] == order && order > 0)
					{
						char *tmp = inFiles.at(i)->getBuffer();
						localStripe = inFiles.at(i)->getBlockSize();
						for (unsigned int j = 0; j < localStripe; ++j)
						{
							buffer[j+skip] = tmp[j];
							parity[j] = parity[j]^buffer[j+skip];
						}
						skip += localStripe;
						if (inFiles.at(i)->newBlock() == false)
						{
							if (inFiles.at(i)->asyncReload()== false )
							{
								++finished;
							}
						}
						++order;
					}
				}
				if (finished == inFiles.size())
				{
					written += skip;
					printf("\r%zu B written", written);
					fflush(stdout);
					writeMe->writeToFile(buffer, skip);
					writeMe->closeFile();
					return true;
				}
			} while ((unsigned int)order < inFiles.size());
			written += skip;
			printf("\r%zu B written", written);
			fflush(stdout);
			writeMe->writeToFile(buffer, skip);
			pointer = (pointer + inFiles.size()-1)%((stripeMap.size()));
			if (finished == inFiles.size())
			{
				writeMe->closeFile();
				return true;
			}
		}
		return true;
	} else if (raidSystem == Raid1)
	{
		return false;
	} else if (raidSystem == Raid0)
	{
		size_t localStripe=stripeSize;
		const size_t bufferSize=(inFiles.size())*stripeSize;
		char *buffer = new char[bufferSize];

		double written = 0.0f;
		while (true)
		{
			localStripe = inFiles.at(0)->getBlockSize();
			for (unsigned int i = 0; i < stripeMap.size(); ++i)
			{
				char *tmp = inFiles.at(stripeMap.at(i))->getBuffer();
				for (unsigned int j = 0; j < localStripe; ++j)
				{
					buffer[j+(i)*localStripe] = tmp[j];
				}
			}
			written += inFiles.size()*localStripe/1024.0f/1024.0f;
			printf("\r%f MB written", written);
			writeMe->writeToFile(buffer, localStripe*inFiles.size());
			for (unsigned int i = 0; i < inFiles.size(); ++i)
			{
				if (inFiles.at(i)->newBlock() == false)
				{
					if (inFiles.at(i)->asyncReload()== false )
					{
						writeMe->closeFile();
						return true;
					}
				}
			}
		}
		return true;
	} else
		return false;
	return false;
}

bool RaidSystem::recoverLostImage()
{
	if (lostImages == 0)
	{
		std::cout << "\tNo lost disc to recover." << std::endl;
		return true;
	} else if (lostImages > 1)
	{
		std::cerr << "\tToo many lost discs to recover." << std::endl;
		return false;
	}
	std::vector<FileReader *> inFiles = handle->getInFiles();
	for (unsigned int j = 0; j < inFiles.size(); ++j)
	{
		inFiles.at(j)->setBlockSize(128*1024);
		inFiles.at(j)->reset();
	}
	if (raidSystem == Raid5_incomplete)
	{
		size_t blocklength = inFiles.at(0)->getBlockSize();
		double written = 0.0f;
		size_t runs=0;
		char *buf = new char[blocklength];
		while (true)
		{
			blocklength = inFiles.at(0)->getBlockSize();
			for (unsigned int i = 0; i < blocklength; ++i)
			{
				buf[i] = 0;
			}
			++runs;
			for (size_t j = 0; j < inFiles.size(); ++j)
			{
				FileReader *reader = inFiles.at(j);
				char *read = reader->getBuffer();
				for (unsigned int i = 0; i < blocklength; ++i)
				{
					buf[i] = buf[i]^read[i];
				}
			}
			handle->getFileWriter()->writeToFile(buf,blocklength);
			written += blocklength/1024.0f/1024.0f;
			printf("\r%f MB written", written);
			fflush(stdout);
			for (unsigned int j = 0; j < inFiles.size(); ++j)
			{
				if (inFiles.at(j)->newBlock() == false)
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

void RaidSystem::printAllInfos()
{
	std::cout << std::endl;
	std::cout << "All information compact: " << std::endl << std::endl;
	std::vector<FileReader *> inFiles = handle->getInFiles();
	std::cout << "Found Raid system: ";
	if (raidSystem == Raid_unknown)
	{
		std::cout << "No valid found";
	} else if (raidSystem == Raid0)
	{
		std::cout << "Raid 0";
	} else if (raidSystem == Raid1)
	{
		std::cout << "Raid 1";
	} else if (raidSystem == Raid5_incomplete)
	{
		std::cout << "Raid 5 - with one disc missing";
	} else if (raidSystem == Raid5_complete)
	{
		std::cout << "Raid 5";
	}
	std::cout << std::endl;
	std::cout << "Stripe/Chunk-size:\t" << stripeSize/1024 << "KB" << std::endl;
	std::cout << "Device order:" << std::endl;
	size_t size = inFiles.size();
	int parityPlate = size-1;
	if (raidSystem == Raid5_incomplete || raidSystem == Raid5_complete)
		size -= 1;
	for (unsigned int i = 0; i < size; ++i)
	{
		parityPlate += i;
		std::cout << i+1 << ") " << inFiles.at(stripeMap.at(i))->getPath() << std::endl;
		parityPlate -= stripeMap.at(i);
	}
	if (raidSystem == Raid5_incomplete || raidSystem == Raid5_complete)
		std::cout << size+1 << ") " << inFiles.at(parityPlate)->getPath() << std::endl;
}

bool RaidSystem::raidCheck(std::string path)
{
	bool found = false;
	if (raidSystem == Raid_unknown)
	{
		std::cout << "Starting with Raid check." << std::endl;
		found = easyCheck();
		if (found == false)
		{
			std::cout << "No valid Raid system found!" << std::endl;
			return false;
		} else 
		{
			char answer='y';
			std::cout << "Do you want to accept this? [y/n] ";
			std::cin >> answer;
			if (answer == 'n')
			{
				std::cout << "Raid system can not be estimated automatically. Must be handed with the program start." << std::endl;
				return false;
			}
		}
	} else if (raidSystem == Raid5_user)
	{
		if (lostImages == -1)
		{
			std::cout << "User input: Raid 5! Checking for lost discs or if it's a Raid 1 system." << std::endl;
			found = easyCheck();
			if (found == false)
			{
				if (raidSystem == Raid1)
				{
					std::cerr << "User input wrong. Estimated Raid 1 system!" << std::endl;
					return false;
				} else
				{
					std::cout << "Discs missing. Will recover disc later!" << std::endl;
					raidSystem = Raid5_incomplete;
				}
				found = true;
			} else
			{
				std::cout << "Raid 5 system is complete. No missing discs indicated!" << std::endl;
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
			std::cerr << "User specifies Raid 0 system with lost discs. Cannot be recovered!" << std::endl;
			return false;
		}
		found = true;
	}

	//raidType almost done
	if (found == false)
	{
		std::cerr << "No valid Raid system found! Have to abort!" << std::endl;
		return false;
	}
	if (raidSystem == Raid5_incomplete)
	{
		std::cout << "Incomplete Raid 5 system. Will recover disc and mount it internally." << std::endl;
		recoverLostImage();
		handle->addImage(handle->getFileWriter()->getPath());
	}

	//estimate the stripesize
	if (stripeSize == -1)
	{
		std::cout << "No Stripesize specified! Trying to calculate Stripesize now." << std::endl;
		found = calculateStripeSize();
	} else
	{
		std::cout << "Using Stripesize specified by user!" << std::endl;
		found = true;
	}
	if (found == true)
	{
		std::cout << "Stripesize: " << stripeSize/1024 << "KB" << std::endl;
	} else
	{
		std::cerr << "No valid Stripesize found. User should estimate it on its own, or disc could not be recovered." << std::endl;
		return false;
	}
	handle->setBlockSize(stripeSize);

	std::cout << "Trying to estimate the Stripemap." << std::endl;
	if (raidSystem == Raid5_user || raidSystem == Raid5_incomplete || raidSystem == Raid5_complete)
	{
		stripeMap = handle->estimateStripeMap(true);
	} else if (raidSystem == Raid0)
	{
		stripeMap = handle->estimateStripeMap(false);
	} else
	{
		std::cout << "No need to estimate Stripemap because of Raid 1 system." << std::endl;
		found = true;
	}

	if (stripeMap.size() == 0 && !(raidSystem == Raid1))
	{
		std::cerr << "No valid Stripemap found. Have to abort Recovery." << std::endl;
		return false;
	} else if (raidSystem == Raid1)
	{
	} else
	{
		std::cout << "Rebuild the imagefile out of the Raid discs." << std::endl;
		found = buildDataImage(path);
	}

	if (found == true)
	{
		printAllInfos();
	}
	return found;
}

