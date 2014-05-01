#include <fileHandler.h>

static int maxSize;

volatile bool loadAvail;
std::mutex mtx;

FileHandler::FileHandler(std::string inputFolder, std::string outputFolder)
{
	inFolder = inputFolder;
	outFolder = outputFolder;
	DIR *dpdf;
	struct dirent *epdf;

	loadAvail = true;
	std::vector<std::string> imgs;

	dpdf = opendir(inputFolder.c_str());
	if (dpdf != NULL)
	{
		while ((epdf=readdir(dpdf))!=NULL)
		{
			if (epdf->d_name[0] == '.')
				continue;
			std::string tmp = inputFolder+epdf->d_name;
			std::cout << tmp << std::endl;
			
			imgs.push_back(tmp);
		}
	}
	if (imgs.empty())
	{
		std::cerr << "No valid directory or no valid datas in directory" << std::endl;
		exit(EXIT_FAILURE);
	}
	
	maxSize = (BUFLENGTH/(imgs.size()+1))/2*2;
	maxSize = maxSize/(1024*1024);
	maxSize = maxSize*(1024*1024);
	maxSize = std::min(maxSize,128*1024*1024);
	std::cout << "Image-Buffers:\t" << maxSize/1024/1024 << "MB" << std::endl;
	for (unsigned int i = 0; i < imgs.size(); ++i)
	{
		FileReader *a = new FileReader(imgs.at(i),maxSize);
		inFiles.push_back(a);
	}
	writer = new FileWriter(outFolder+"recoveredImage.dd",maxSize);
}

void FileHandler::addImage(std::string path)
{
	if (inFiles.size()==0)
	{
		maxSize = (BUFLENGTH/1+1)/2*2;
		maxSize = maxSize/BLOCKSIZE;
		maxSize = maxSize*BLOCKSIZE;
		maxSize = std::min(maxSize,256*1024*1024);
		FileReader *a = new FileReader(path,maxSize);
		inFiles.push_back(a);
	} else
	{
		FileReader *a = new FileReader(path,maxSize);
		inFiles.push_back(a);
	}
}

bool FileHandler::findGoodBlock()
{
	size_t offset = 0;

	int erg = -1;
	int sample=-1;
	bool found=false;
	
	static bool init=true;
	if (init == false)
	{
		for (size_t i = 0; i < inFiles.size(); ++i)
		{
			if (inFiles.at(i)->newBlock() == false)
			{
				for (unsigned int j = 0; j < inFiles.size(); ++j)
				{
					if (inFiles.at(j)->asyncReload() == false)
					{
						std::cout << "Couldnt find any good Blocks anymore. File end reached." << std::endl;
						return false;
					}	
				}
			}
		} 
	} else
	{
		init = false;
	}
	
	while (found == false)
	{
		int newBlocks=0;
		while ((sample = inFiles.at(0)->findFirstNonemptyBlock())==-1)
		{
			if (inFiles.at(0)->asyncReload() == false)
			{
				std::cout << "Couldn't load any good Blocks anymore. Good luck!" << std::endl;
				return false;
			}
			++newBlocks;
		}
		if (newBlocks > 0)
		{
			for (size_t i = 1; i < inFiles.size(); ++i)
			{
				inFiles.at(i)->skipInputBuffer(newBlocks);
			}
		}
		for (size_t i = 1; i < inFiles.size(); ++i)
		{
			erg = inFiles.at(i)->findFirstNonemptyBlock();
			if (erg == -1)
			{
				break;
			}
			erg = std::max(sample, erg);
		}
		if (erg == -1 || sample == -1)
		{
			for (size_t i = 0; i < inFiles.size(); ++i)
			{
				if (inFiles.at(i)->asyncReload() == false)
				{
					std::cout << "Couldnt find any good Blocks anymore. File end reached." << std::endl;
					return false;
				}
			}
			continue;
		}
		if (erg == sample)
		{
			offset = erg;
			break;
		}
		for (size_t i = 0; i < inFiles.size(); ++i)
		{
			inFiles.at(i)->setOffset(erg+inFiles.at(i)->getBlockSize());
		}
	}
	for (size_t i = 0; i < inFiles.size(); ++i)
	{
		inFiles.at(i)->setOffset(offset);
	}
	return true;
}

void FileHandler::setBlockSize(size_t block)
{
	for (auto in : inFiles)
		in->setBlockSize(block);
}
	

int FileHandler::estimateStripeSize()
{
	auto lambda = [&] (FileReader *me) -> std::vector<int>
	{
		const int CHECKS = 64*1024*1024;
		std::vector<std::pair<size_t, float>> entropies[12];
		//std::vector<std::pair<size_t, float>> entropies;
		me->reset();
		int done=0;
		int counters[12]={};
		int sizes[12] = {2048, 1024, 512, 256, 128, 64, 32, 16, 8, 4, 2, 1};

		int offset=0;
		float ent=0;
		int reloads=0;
		do
		{
			done = 12;
			for (int i = 0; i < 12; ++i)
			{
				me->setBlockSize(sizes[i]*1024);
				me->setOffset(0);
				while ((offset=me->findFirstNonemptyBlock())!=-1)
				{
					if (entropies[i].size() >= CHECKS)
						break;
					me->setOffset(offset);
					ent = me->calcEntropyOfCurrentBlock();
					std::pair<size_t, float> input((offset+reloads*me->getBufferLength())/1024, ent);
					me->setOffset(me->getBlockSize()+offset);
					entropies[i].push_back(input);
					if (entropies[i].size() >= CHECKS)
						break;
				}
				if (entropies[i].size() < CHECKS)
					done--;
			}
			std::string wr = "Threads working";
			printf("\r%s\t",wr.c_str());
			fflush(stdout);
			if (done == 12)
				break;
			if (me->asyncReload()==false)
			{
				break;
			}
			++reloads;
		} while (done == false);
		me->reset();

		std::vector<size_t> adressEdges;
		const float edge = 6.5f;
		for (unsigned int i = 0; i < 12; ++i)
		{
			for (unsigned int j = 0; j < entropies[i].size();)
			{
				if (entropies[i].at(j).second > edge)
				{
					adressEdges.push_back(entropies[i].at(j).first);
					do {
						++j;
						if (j < entropies[i].size() && entropies[i].at(j).second < 0.25f && entropies[i].at(j-1).second > edge)
						{
							adressEdges.push_back(entropies[i].at(j).first);
							break;
						}
						if (j >= entropies[i].size())
							break;
					} while (entropies[i].at(j).first-sizes[i] == entropies[i].at(j-1).first);
				} else {
					do
					{
						++j;
						if (j >= entropies[i].size())
							break;
					} while (entropies[i].at(j).first-sizes[i] == entropies[i].at(j-1).first);
				}
			}
			for (auto in : adressEdges)
			{
				if (in % 2048 == 0)
					counters[0] += 1;
				if (in % 1024 == 0)
					counters[1] += 1;
				if (in % 512 == 0)
					counters[2] += 1;
				if (in % 256 == 0)
					counters[3] += 1;
				if (in % 128 == 0)
					counters[4] += 1;
				if (in % 64 == 0)
					counters[5] += 1;
				if (in % 32 == 0)
					counters[6] += 1;
				if (in % 16 == 0)
					counters[7] += 1;
				if (in % 8 == 0)
					counters[8] += 1;
				if (in % 4 == 0)
					counters[9] += 1;
				if (in % 2 == 0)
					counters[10] += 1;
				if (in % 1 == 0)
					counters[11] += 1;
			}
		}
		std::vector<int> ret;
		for (int i = 0; i < 12; ++i)
		{
			ret.push_back(counters[i]);
		}
		return ret;
	}; //lambda function end

	int stripeSize = -1;
	int counters[12]={};
	unsigned int NUM_THREADS = inFiles.size();
	std::vector<std::future<std::vector<int>>> futureResults(inFiles.size());
	std::vector<std::vector<int>> results;
	for (size_t i = 0; i < NUM_THREADS; ++i)
	{
		futureResults[i] = std::async(std::launch::async, lambda, inFiles.at(i));
	}
	std::future_status status;
	while (results.size() < inFiles.size())
	{
		for (size_t i = 0; i < NUM_THREADS; ++i)
		{
			while (results.size() < i+1)
			{
				try
				{
					status = futureResults.at(i).wait_for(std::chrono::milliseconds(500));
				} 
				catch (std::future_error &e)
				{
					continue;
				}
				if (status == std::future_status::ready)
				{
					//		std::cout << "Thread " << i << "\t pushed" << std::endl;
					std::vector<int> res = futureResults.at(i).get();
					results.push_back(res);
				}
			}
		}
	}
	int value[12] = {2048, 1024, 512, 256, 128, 64, 32, 16, 8, 4, 2, 1};
	for (unsigned int i=0; i < results.size();++i)
	{
		for (unsigned int j = 0; j < results.at(i).size(); ++j)
		{
			counters[j] += results.at(i).at(j);
		}
	}
	float min = 1000.0f;
	int index = 0;
	for (unsigned int i = 1; i < 12; ++i)
	{
		if (counters[i-1] == 0 || counters[i] == 0)
			continue;
		float tmp = (float)counters[i-1]/(float)counters[i]; 
		if (tmp < min)
		{
			min = tmp;
			index = i;
		}
	}
	stripeSize = value[index];
	int x = 0;
	std::cout << "Found:\n";
	std::cout << "2048:\t" << counters[x] << std::endl;
	++x;
	std::cout << "1024:\t" << counters[x] << "\t" << (float)counters[x]/(float)counters[x-1] << std::endl;
	++x;
	std::cout << " 512:\t" << counters[x] << "\t" << (float)counters[x]/(float)counters[x-1] << std::endl;
	++x;
	std::cout << " 256:\t" << counters[x] << "\t" << (float)counters[x]/(float)counters[x-1] <<  std::endl;
	++x;
	std::cout << " 128:\t" << counters[x] << "\t" << (float)counters[x]/(float)counters[x-1] <<  std::endl;
	++x;
	std::cout << "  64:\t" << counters[x] << "\t" << (float)counters[x]/(float)counters[x-1] <<  std::endl;
	++x;
	std::cout << "  32:\t" << counters[x] << "\t" << (float)counters[x]/(float)counters[x-1] <<  std::endl;
	++x;
	std::cout << "  16:\t" << counters[x] << "\t" << (float)counters[x]/(float)counters[x-1] <<  std::endl;
	++x;
	std::cout << "   8:\t" << counters[x] << "\t" << (float)counters[x]/(float)counters[x-1] <<  std::endl;
	++x;
	std::cout << "   4:\t" << counters[x] << "\t" << (float)counters[x]/(float)counters[x-1] <<  std::endl;
	++x;
	std::cout << "   2:\t" << counters[x] << "\t" << (float)counters[x]/(float)counters[x-1] <<  std::endl;
	++x;
	std::cout << "   1:\t" << counters[x] << "\t" << (float)counters[x]/(float)counters[x-1] <<  std::endl;
	if (stripeSize == 2048)
	{
		std::cout << "Not one good block found. Need to abort" << std::endl;
		return false;
	}
	std::cout << "It's probably:\t" << stripeSize << std::endl;
	char answer='y';
	std::cout << "Estimated Stripesize: " << stripeSize << "KB" << std::endl;
	std::cout << "It's just with simple heuristic. Would you like to continue with this result?" << std::endl;
	std::cout << "Answer with 'y' for 'yes', 'n' for 'no' or 'h' for 'hint' [y/n/h] ";
	//std::cin >> answer;
	if (answer == 'y')
	{
		stripeSize *= 1024;
	}
	else if (answer == 'n')
	{
		std::cout << "With which stripesize do you wish to continue? [1-1024] (in kB) ";
		std::cin >> stripeSize;
		stripeSize *= 1024;
	}
	else
	{
		std::cerr << "Try a bigger BlockSize in 'include/defines.h'. Maybe, the heuristic works better than. Otherwise you have to estimate it on your own." << std::endl;
	}
	for (size_t i = 0; i < inFiles.size(); ++i)
	{
		inFiles.at(i)->setBlockSize(stripeSize);
	}

	/*
	   for (unsigned int i = 0; i < results.size(); ++i)
	   {
	   pointer[i] = 0;
	   while (results.at(i).at(pointer[i]).first < 1024)
	   {
	   ++pointer[i];
	   }
	   }
	   size_t lastAdress=0;
	   while (true)
	   {
	   bool killMe = false;
	   for (unsigned int i = 0; i < results.size(); ++i)
	   {
	   if (pointer[i] >= results.at(i).size())
	   {
	   killMe = true;
	   break;
	   }
	   }
	   if (killMe == true)
	   break;
	   size_t currentLowest = -1;
	   for (unsigned int i = 0; i < results.size(); ++i)
	   {
	   size_t low = results.at(i).at(pointer[i]).first;
	   currentLowest = std::min(currentLowest,low);
	   }
	   if (lastAdress + 32 < currentLowest)
	   {
	   lastAdress = currentLowest-32;
	   }
	   for (; lastAdress < currentLowest; lastAdress += inFiles.at(0)->getBlockSize()/1024)
	   {
	   std::cout << std::setw(10) << lastAdress << "\t";
	   for (unsigned int i = 0; i < results.size(); ++i)
	   {
	   std::cout << std::setw(10) << "0.00000\t"; 
	   }
	   std::cout << std::endl;
	   }
	   std::cout << std::setw(10) << currentLowest << "\t";
	   for (unsigned int i = 0; i < results.size(); ++i)
	   {
	   if (results.at(i).at(pointer[i]).first > currentLowest)
	   {
	   std::cout << std::setw(10) << "0.00000\t";
	//std::cout << std::setw(10) << results.at(i).at(pointer[i]).first << "\t\t"; 
	} else
	{
	std::cout << std::setw(10) << (float)results.at(i).at(pointer[i]).second << "\t"; 
	//std::cout << std::setw(10) << results.at(i).at(pointer[i]).first << "\t\t"; 
	pointer[i]++;
	}
	}
	std::cout << std::endl;
	lastAdress = currentLowest + inFiles.at(0)->getBlockSize();
	}
	 */
	return stripeSize;
}

std::vector<size_t> FileHandler::estimateStripeMap(bool isRaid5)
{
	auto lambda = [&] (FileReader *me) -> std::vector<std::pair<size_t, float>>
	{
		const int CHECKS = 64*1024*1024;
		std::vector<std::pair<size_t, float>> entropies;
		me->reset();

		int offset=0;
		float ent=0;
		int reloads=0;
		do
		{
			while ((offset=me->findFirstNonemptyBlock())!=-1)
			{
				if (entropies.size() >= CHECKS)
					break;
				me->setOffset(offset);
				ent = me->calcEntropyOfCurrentBlock();
				if (ent < 0.25f)
				{
					me->setOffset(me->getBlockSize()+offset);
					continue;
				}
				std::pair<size_t, float> input((offset+reloads*me->getBufferLength())/1024, ent);
				me->setOffset(me->getBlockSize()+offset);
				entropies.push_back(input);
				if (entropies.size() >= CHECKS)
					break;
			}
			std::string wr = "Threads working\t";
			printf("\r%s\t",wr.c_str());
			fflush(stdout);
			if (me->asyncReload()==false)
			{
				break;
			}
			++reloads;
		} while (entropies.size() < CHECKS);
		me->reset();
		return entropies;
	};
	//lambda function end

	unsigned int NUM_THREADS = inFiles.size();
	std::vector<std::pair<size_t, float>> entropies;

	std::vector<std::future<std::vector<std::pair<size_t,float>>>> futureResults(inFiles.size());
	std::vector<std::vector<std::pair<size_t,float>>> results;
	for (size_t i = 0; i < NUM_THREADS; ++i)
	{
		futureResults[i] = std::async(std::launch::async, lambda, inFiles.at(i));
	}
	std::future_status status;
	while (results.size() < inFiles.size())
	{
		for (size_t i = 0; i < NUM_THREADS; ++i)
		{
			while (results.size() < i+1)
			{
				try
				{
					status = futureResults.at(i).wait_for(std::chrono::milliseconds(500));
				} 
				catch (std::future_error &e)
				{
					continue;
				}
				if (status == std::future_status::ready)
				{
					std::vector<std::pair<size_t, float>> ent = futureResults.at(i).get();
					if (ent.empty())
					{
						std::cout << "Empty at " << i << std::endl;
					}
					results.push_back(ent);
				}
			}
		}
	}
	std::cout << std::endl;
	std::cout << "\tEntropie calculation finished" << std::endl;

	std::vector<std::vector<std::pair<bool, std::vector<size_t>>>> map;
	map.resize(results.size());

	int modOp = inFiles.size()*inFiles.at(0)->getBlockSize()/1024;

	unsigned int *pointer = new unsigned int[results.size()]{}; 
	for (unsigned int i = 0; i < results.size(); ++i)
	{
		pointer[i] = 0;
		while (results.at(i).at(pointer[i]).first < 1024)
		{
			++pointer[i];
		}
	}
	bool blockBegin=true;
	std::vector<size_t> pushMe;
	while (true)
	{
		bool killMe = false;
		bool sameHeight=true;
		size_t currentLowest = -1;
		while (sameHeight==true)
		{
			for (unsigned int i = 0; i < results.size(); ++i)
			{
				if (pointer[i] >= results.at(i).size()-1)
				{
					killMe = true;
					break;
				}
			}
			if (killMe == true)
				break;
			bool sameLevel = true;
			currentLowest = results.at(0).at(pointer[0]).first;
			for (unsigned int i = 1; i < results.size(); ++i)
			{
				size_t low = results.at(i).at(pointer[i]).first;
				if (currentLowest != low)
					sameLevel = false;
				currentLowest = std::min(currentLowest,low);
			}
			if (sameLevel == true)
			{
				for (unsigned int i = 0; i < results.size(); ++i)
				{
					++pointer[i];
				}
			} else
			{
				blockBegin = false;
				for (unsigned int i = 0; i < results.size(); ++i)
				{
					if (results.at(i).at(pointer[i]+1).first == currentLowest+inFiles.at(0)->getBlockSize()/1024)
					{
						blockBegin = true;
					}
				}
				break;
			}
		}
		if (killMe==true)
			break;
		pushMe.clear();
		int pos = currentLowest % modOp;
		pos = pos / (inFiles.at(0)->getBlockSize()/1024);
		for (unsigned int i = 0; i < results.size(); ++i)
		{
			if (results.at(i).at(pointer[i]).first > currentLowest)
			{
				pushMe.push_back(i);
			} else 
			{
				pointer[i]++;
			}
		}
		std::pair<bool,std::vector<size_t>> bla(blockBegin, pushMe);
		map.at(pos).push_back(bla);
	}

	//DEBUG OUTPUT
	/*
	for (int i = 0; i < map.size(); ++i)
	{
		std::cout << i*inFiles.at(0)->getBlockSize()/1024 << ":" << std::endl;
		std::cout << "Begin:" << std::endl;
		for (int j = 0; j < map.at(i).size(); ++j)
		{
			if (map.at(i).at(j).first == true)
			{
				for (auto in : map.at(i).at(j).second)
				{
					std::cout << "\t" << in;
				}
				std::cout << std::endl;
			}
		}
		std::cout << "End:" << std::endl;
		for (int j = 0; j < map.at(i).size(); ++j)
		{
			if (map.at(i).at(j).first == false)
			{
				for (auto in : map.at(i).at(j).second)
				{
					std::cout << "\t" << in;
				}
				std::cout << std::endl;
			}
		}
	}
	*/

	std::vector<std::vector<int>> mappe;
	mappe.resize(results.size());
	for (unsigned int i = 0; i < results.size();++i)
	{
		mappe.at(i).resize(results.size(),-1);
		for (unsigned int j = 0; j < results.size(); ++j)
		{
			mappe[i][j] = -1;
		}
	}

	bool unchanged=true;
	int *hits = new int[results.size()];
	int Bhits = 0;
	int Ehits = 0;
	int BClear, EClear;

	for (unsigned int x = 0; x < map.size(); ++x)
	{
		int BPos=0;
		int EPos=results.size()-1;
		if (isRaid5==true)
			EPos--;
		while (!map.at(x).empty())
		{
			unchanged = true;
			for (unsigned int i = 0; i < results.size(); ++i)
			{
				hits[i] = 0;
			}
			Bhits = 0;
			Ehits = 0;
			BClear=-1;
			EClear=-1;
			for (unsigned int j = 0; j < map.at(x).size(); ++j)
			{
				if (map.at(x).at(j).first == true)
				{
					std::vector<size_t> tmp = map.at(x).at(j).second;
					if (tmp.size() == 1)
					{
						Bhits += 1;
						hits[tmp.at(0)] += 1;
					}
				} else
				{
					std::vector<size_t> tmp = map.at(x).at(j).second;
					if (tmp.size() == 1)
					{
						Ehits += 1;
						hits[tmp.at(0)] += 4096;
					}
				}
			}
			for (unsigned int i = 0; i < results.size(); ++i)
			{
				if ((hits[i]%4096) > 0.6f*Bhits)
				{
					BClear = i;
					mappe.at(x).at(i) = BPos;
					BPos++;
					unchanged = false;
				} else if ((hits[i]>>12) > 0.6f*Ehits)
				{
					EClear = i;
					mappe.at(x).at(i) = EPos;
					EPos--;
					unchanged = false;
				}
			}
			if (unchanged == true)
			{
				map.at(x).clear();
				continue;
			}
			for (unsigned int j = 0; j < map.at(x).size(); ++j)
			{
				if (map.at(x).at(j).first == true)
				{
					if (map.at(x).at(j).second.size()==1)
					{
						map.at(x).at(j).second.clear();
						continue;
					}
					for (unsigned int t = 0; t < map.at(x).at(j).second.size(); ++t)
					{
						if ((int)map.at(x).at(j).second.at(t) == BClear)
						{
							map.at(x).at(j).second.erase(map.at(x).at(j).second.begin()+t);
						}
					}
				} else
				{
					if (map.at(x).at(j).second.size()==1)
					{
						map.at(x).at(j).second.clear();
						continue;
					}
					for (unsigned int t = 0; t < map.at(x).at(j).second.size(); ++t)
					{
						if ((int)map.at(x).at(j).second.at(t) == EClear)
						{
							map.at(x).at(j).second.erase(map.at(x).at(j).second.begin()+t);
						}
					}
				}
			}
		}
	}
	std::vector<size_t> stripeMap;

	if (isRaid5 == true)
	{
		std::vector<int> parities;
		parities.resize(results.size(),-1);
		int p=0;
		for (unsigned int i = 0; i < mappe.size();++i)
		{
			bool hit = false;
			for (unsigned int j = 0; j < mappe.at(i).size(); ++j)
			{
				if (mappe.at(i).at(j) == -1)
				{
					if (hit == true)
					{
						hit = false;
						break;
					}
					p = j;
					hit = true;
				}
			}
			if (hit == true)
			{
				mappe.at(i).at(p) = -2;
				parities.at(p) = p;
			}
		}
		bool solutionFound=true;
		for (auto in : parities)
		{
			if (in == -1)
			{
				solutionFound = false;
			}
		}
		if (solutionFound == false)
		{
			std::vector<int> missing;
			for (unsigned int i = 0; i < parities.size(); ++i)
			{
				if (parities.at(i) == -1)
					missing.push_back(i);
			}
			for (unsigned int i = 0; i < mappe.size();++i)
			{
				for (unsigned int j = 0; j < mappe.at(i).size(); ++j)
				{
					bool used=false;
					if (mappe.at(i).at(j) == -1)
					{
						for (unsigned int k = 0; k < missing.size(); ++k)
						{
							if (missing.at(k) == (int)j)
							{
								used=true;
								mappe.at(i).at(j) = -2;
								missing.erase(missing.begin()+k);
							}
						}
						if (used==true)
							break;
					}
				}
			}
		}

		std::cout << "Found this map: " << std::endl;
		for (unsigned int i = 0; i < mappe.size();++i)
		{
			int next = 0;
			while (next < (int)results.size()-1)
			{
				for (unsigned int j = 0; j < mappe.at(i).size(); ++j)
				{
					if (mappe.at(i).at(j) == -2 && next == 0)
					{
						std::cout << "P" << "\t";
					} else
					{
						if (next == 0)
							std::cout << mappe.at(i).at(j) << "\t";
						if (next == mappe.at(i).at(j))
							stripeMap.push_back(j);
					}
				}
				next++;
			}
			std::cout << std::endl;
		}
	} else
	{
		std::cout << "Found this map: " << std::endl;
		int next = 0;
		while (next < (int)results.size())
		{
			for (unsigned int j = 0; j < mappe.at(0).size(); ++j)
			{
				if (next == mappe.at(0).at(j))
					stripeMap.push_back(j);
				if (next == 0)
					std::cout << mappe.at(0).at(j) << "\t";
			}
			next++;
		}
		std::cout << std::endl;
	}
	/*
	   char answer;
	   std::cout << "Are you happy with the estimated Stripemap? [y/n] ";
	   std::cin >> answer;
	   if (answer == 'y')
	   {
	   std::cout << "Continuing with restoring the image." << std::endl;
	   }
	   else
	   {
	   std::cout << "Please enter the Stripemap as an array beginning with the top row, starting with 1 as numeration. 0 is a parity block. Example: " << std::endl;
	   std::cout << "1 \n 2 \n 0 \n 0 \n 3 \n 4 ..." << std::endl;
	   for (unsigned int i = 0; i < mappe.size();++i)
	   {
	   for (unsigned int j = 0; j < mappe.at(i).size(); ++j)
	   {
	   std::cin >> mappe[i][j];
	   }
	   }
	   }
	 */
	/*
	 * DEBUG OUTPUT -  DO NOT ERASE ME
	 *
	 unsigned int *pointer = new unsigned int[results.size()]{}; 

	 for (unsigned int i = 0; i < results.size(); ++i)
	 {
	 pointer[i] = 0;
	 while (results.at(i).at(pointer[i]).first < 1024)
	 {
	 ++pointer[i];
	 }
	 }
	 size_t lastAdress=0;
	 while (true)
	 {
	 bool killMe = false;
	 for (unsigned int i = 0; i < results.size(); ++i)
	 {
	 if (pointer[i] >= results.at(i).size())
	 {
	 killMe = true;
	 break;
	 }
	 }
	 if (killMe == true)
	 break;
	 size_t currentLowest = -1;
	 for (unsigned int i = 0; i < results.size(); ++i)
	 {
	 size_t low = results.at(i).at(pointer[i]).first;
	 currentLowest = std::min(currentLowest,low);
	 }
	 if (lastAdress + 32 < currentLowest)
	 {
	 lastAdress = currentLowest-32;
	 }
	 for (; lastAdress < currentLowest; lastAdress += inFiles.at(0)->getBlockSize()/1024)
	 {
	 std::cout << std::setw(10) << lastAdress << "\t";
	 for (unsigned int i = 0; i < results.size(); ++i)
	 {
	 std::cout << std::setw(10) << "0.00000\t"; 
	 }
	 std::cout << std::endl;
	 }
	 std::cout << std::setw(10) << currentLowest << "\t";
	 for (unsigned int i = 0; i < results.size(); ++i)
	 {
	 if (results.at(i).at(pointer[i]).first > currentLowest)
	 {
	 std::cout << std::setw(10) << "0.00000\t";
	 } else
	 {
	 std::cout << std::setw(10) << (float)results.at(i).at(pointer[i]).second << "\t"; 
	 pointer[i]++;
	 }
	 }
	 std::cout << std::endl;
	 lastAdress = currentLowest + inFiles.at(0)->getBlockSize();
	 }
	 */

	/*
	//find the device with the partition table
	//estimate it right there
	std::pair<int,bool> test(0, false);
	stripeMap.push_back(test);
	test.first = 1;
	stripeMap.push_back(test);
	test.first = 2;
	stripeMap.push_back(test);
	test.first = 3;
	stripeMap.push_back(test);

	//search for a part, where the stripe map can be estimated
	 */
	//fill the vector


	//sort it then, knowing which one the first is, and estimate the other two


	for (size_t i = 0; i < inFiles.size(); ++i)
	{
		inFiles.at(i)->setBlockSize(inFiles.at(i)->getBlockSize());
	}
	return stripeMap;
}

void FileHandler::reset()
{
	for (unsigned int i = 0; i < inFiles.size(); ++i)
	{
		inFiles.at(i)->reset();
	}
}

int FileHandler::findStringInBlock(std::string seek)
{
	for (unsigned int i = 0; i < inFiles.size(); ++i)
	{
		int adr = inFiles.at(i)->findFirstNonemptyBlock();
		if (adr==-1)
		{
		} else
		{
			inFiles.at(i)->setOffset(adr);
			adr = inFiles.at(i)->findString(seek);
		}
		if (adr > 0)
		{
			return adr;
		}
	}
	return -1;
}

bool FileHandler::reloadBuffers()
{
	//std::cout << "reload Buffers" << std::endl;
	for (unsigned int i = 0; i < inFiles.size(); ++i)
	{
		//if (inFiles.at(i)->reloadBuffer()==false)
		if (inFiles.at(i)->asyncReload()==false)
			return false;
	}
	return true;
}

int FileHandler::findString(std::string seek)
{
	reset();
	int found = -1;
	bool eof=true;
	while (found == -1)
	{
		for (unsigned int i = 0; i < inFiles.size(); ++i)
		{
			found = inFiles.at(i)->findFirstNonemptyBlock();
			if (found == -1)
			{
				eof=inFiles.at(i)->asyncReload();
			} else
			{
				inFiles.at(i)->setOffset(found);
				found = inFiles.at(i)->findString(seek);
				if (found != -1)
				{
					break;
				}
				if (inFiles.at(i)->newBlock() == false)
				{
					eof=inFiles.at(i)->asyncReload();
				}
			}
			if (eof==false)
				return -1;
		}
	}	
	return found;
}

std::vector<FileReader *> FileHandler::getInFiles()
{
	return inFiles;
}

FileReader * FileHandler::getFileReader(size_t num)
{
	if (num >= inFiles.size())
	{
		std::cerr << "ERROR in FileHandler::getFileReader - num smaller than available Images" << std::endl;
		return NULL;
	}
	return inFiles.at(num);
}

FileWriter *FileHandler::getFileWriter()
{
	return writer;
}


