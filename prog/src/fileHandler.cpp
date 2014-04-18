#include <fileHandler.h>
#include <iomanip>


FileHandler::FileHandler(std::string inputFolder, std::string outputFolder)
{
	inFolder = inputFolder;
	outFolder = outputFolder;
	DIR *dpdf;
	struct dirent *epdf;

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
	int maxSize = (BUFLENGTH/(imgs.size()+1))*2;
	maxSize = maxSize/BLOCKSIZE;
	maxSize = maxSize*BLOCKSIZE;
	maxSize = std::min(maxSize,256*1024*1024);
	std::cout << "Image-Buffers:\t" << maxSize/1024/1024 << "MB" << std::endl;
	for (unsigned int i = 0; i < imgs.size(); ++i)
	{
		FileReader *a = new FileReader(imgs.at(i),maxSize);
		inFiles.push_back(a);
	}
	writer = new FileWriter(outFolder+"recoveredImage.dd");
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
					if (inFiles.at(j)->reloadBuffer() == false)
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
			if (inFiles.at(0)->reloadBuffer() == false)
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
				if (inFiles.at(i)->reloadBuffer() == false)
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
			inFiles.at(i)->setOffset(erg);
		}
	}
	for (size_t i = 0; i < inFiles.size(); ++i)
	{
		inFiles.at(i)->setOffset(offset);
	}
	return true;
}

bool FileHandler::estimateStripeSize()
{
	auto lambda = [&] (size_t id) -> std::vector<std::pair<size_t, float>>
	{
		const int CHECKS = 64*1024*1024;
		std::vector<std::pair<size_t, float>> entropies;
		std::vector<int> guesses;
		if (id >= inFiles.size())
			return entropies;
		FileReader *me = inFiles.at(id);
		me->reset();
		int offset=0;
		float ent=0;
		int reloads=0;
		do
		{
			while ((offset=me->findFirstNonemptyBlock())!=-1)
			{
				me->setOffset(offset);
				ent = me->calcEntropyOfCurrentBlock();
				//std::pair<size_t, float> input((offset+reloads*me->getBufferLength())-63*512, ent);
				std::pair<size_t, float> input((offset+reloads*me->getBufferLength()), ent);
				me->setOffset(me->getBlockSize()+offset);
				entropies.push_back(input);
				if (entropies.size() >= CHECKS)
					break;
			}
			if (me->reloadBuffer()==false)
			{
				break;
			}
			++reloads;
			std::cout << "Thread: " << id << "\t" << entropies.size() << std::endl;
		} while (entropies.size() < CHECKS);
		guesses.push_back(1);

		/*
		if (id == 0 && entropies.size()>0)
		{
			size_t check=0;
			size_t last = 0;
			float val = 0.0f;
			check = entropies.at(0).first;
			last = entropies.at(0).first;
			val = entropies.at(0).second;
			for (unsigned int i = 1; i < entropies.size(); ++i)
			{
				auto in = entropies.at(i);
				if (last+me->getBlockSize() != in.first || 0.05*val > in.second || 2*val < in.second)
				{
					std::cout << last+me->getBlockSize() << "\t" << check << "\t" << (float)(last+me->getBlockSize()-check)/1024.0f << "KB\t to next readable Block:\t" << (float)(in.first-check)/1024.0f << "KB" << std::endl;
					check = in.first;
					val = in.second;
				}
				last = in.first;
			}
		}
		*/
		std::cout << "Thread: " << id << " finished " << std::endl;

		me->reset();
		return entropies;
	};
	unsigned int NUM_THREADS = inFiles.size();
	//std::vector<std::future<int>> futureResults(inFiles.size());
	//std::vector<int> results;
	std::vector<std::future<std::vector<std::pair<size_t,float>>>> futureResults(inFiles.size());
	std::vector<std::vector<std::pair<size_t,float>>> results;
	for (size_t i = 0; i < NUM_THREADS; ++i)
	{
		futureResults[i] = std::async(std::launch::async, lambda, i);
	}
	std::future_status status;
	while (results.size() < inFiles.size())
	{
		for (size_t i = 0; i < NUM_THREADS; ++i)
		{
			try
			{
				status = futureResults.at(i).wait_for(std::chrono::milliseconds(500));
			} 
			catch (std::future_error &e)
			{
				//std::cerr << "Future error " << e.what() << std::endl;
				continue;
			}
			if (status == std::future_status::ready)
			{
				std::cout << "Thread " << i << "\t pushed" << std::endl;
				std::vector<std::pair<size_t,float>> res = futureResults.at(i).get();
				results.push_back(res);
			}
		}
	}
	size_t check=0;
	size_t last = 0;
	float val = 0.0f;
	size_t *pointer = new size_t[results.size()];
	size_t *lowest = new size_t[results.size()];
	FileReader **files = new FileReader*[inFiles.size()];
	std::vector<size_t> adressEdges;
	size_t lastAdress = 0;
	for (unsigned int i = 0; i < results.size(); ++i)
	{
		files[i] = inFiles.at(i);
		pointer[i] = 0;
		while (results.at(i).at(pointer[i]).first < 1024*1024)
		{
			++pointer[i];
		}
	}
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
		
		for (unsigned int i = 0; i < results.size(); ++i)
		{
			size_t currentEdge = results.at(i).at(pointer[i]).second;
			if (results.at(i).at(pointer[i]).second > 6.0f)
			{
				adressEdges.push_back(results.at(i).at(pointer[i]).first/1024);
				pointer[i]++;
				if (pointer[i] >= results.at(i).size())
				{
					break;
				}
				while (results.at(i).at(pointer[i]).first-files[i]->getBlockSize() == results.at(i).at(pointer[i]-1).first)
				{
					pointer[i]++;
					if (pointer[i] >= results.at(i).size())
					{
						break;
					}
				}
				if (pointer[i] >= results.at(i).size())
				{
					break;
				}
				adressEdges.push_back((results.at(i).at(pointer[i]).first + files[i]->getBlockSize())/1024 );
			} else
			{
				pointer[i]++;
				if (pointer[i] >= results.at(i).size())
				{
					break;
				}
				while (results.at(i).at(pointer[i]).first-files[i]->getBlockSize() == results.at(i).at(pointer[i]-1).first)
				{
					pointer[i]++;
					if (pointer[i] >= results.at(i).size())
					{
						break;
					}
				}
				if (pointer[i] >= results.at(i).size())
				{
					break;
				}
			}
		}
	}
	for (auto in : adressEdges)
	{
		if (in % 1024 == 0)
		{
			std::cout << "1024:\t" << in << std::endl;
			continue;
		}
		if (in % 512 == 0)
		{
			std::cout << " 512:\t" << in << std::endl;
			continue;
		}
		if (in % 256 == 0)
		{
			std::cout << " 256:\t" << in << std::endl;
			continue;
		}
		if (in % 128 == 0)
		{
			std::cout << " 128:\t" << in << std::endl;
			continue;
		}
		if (in % 64 == 0)
		{
			std::cout << "  64:\t" << in << std::endl;
			continue;
		}
		if (in % 32 == 0)
		{
			std::cout << "  32:\t" << in << std::endl;
			continue;
		}
		if (in % 16 == 0)
		{
			std::cout << "  16:\t" << in << std::endl;
			continue;
		}
		if (in % 8 == 0)
		{
			std::cout << "   8:\t" << in << std::endl;
			continue;
		}
		if (in % 4 == 0)
		{
			std::cout << "   4:\t" << in << std::endl;
			continue;
		}
	}
	
	/*
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
		bool aligned=true;
		size_t currentLowest=0xFFFFFFFF;
		while (aligned == true)
		{
			for (unsigned int i = 0; i < results.size(); ++i)
			{
				if (pointer[i] >= results.at(i).size())
				{
					killMe = true;
					break;
				}
			}
			if (killMe == true)
			{
				break;
			}
			for (unsigned int i = 0; i < results.size(); ++i)
			{
				lowest[i] = results.at(i).at(pointer[i]).first;
				currentLowest = std::min(currentLowest,lowest[i]);
			}
			for (unsigned int i = 0; i < results.size(); ++i)
			{
				if(currentLowest != lowest[i])
				{
					aligned = false; 
					break;
				}
			}
			if ((currentLowest-lastAdress)/1024 < 100)
			{
				lastAdress = lastAdress+inFiles.at(0)->getBlockSize();
				for (;lastAdress < currentLowest; lastAdress += inFiles.at(0)->getBlockSize())
				{
					std::cout << std::setw(10) << lastAdress/1024 << "\t";
					for (unsigned int i = 0; i < results.size(); ++i)
					{
						std::cout << std::setw(10) << "0.00000\t"; 
					}
					std::cout << std::endl;
				}
			}
			else
			{
				std::cout << std::endl;
			}
			if (aligned == true)
			{
				lastAdress = currentLowest;
				std::cout << std::setw(10) << lowest[0]/1024 << "\t";
				for (unsigned int i = 0; i < results.size(); ++i)
				{
					if (results.at(i).at(pointer[i]).second == 0.0f)
					{
						std::cout << std::setw(10) << "0.00000\t";
					} else
					{
						std::cout << std::setw(10) << (float)results.at(i).at(pointer[i]).second << "\t"; 
					}
					pointer[i]++;
				}
				std::cout << std::endl;
			}
		}
		while (aligned == false)
		{
			for (unsigned int i = 0; i < results.size(); ++i)
			{
				if (pointer[i] >= results.at(i).size())
				{
					killMe = true;
					break;
				}
			}
			if (killMe == true)
			{
				break;
			}
			currentLowest = 0xFFFFFFFF;
			for (unsigned int i = 0; i < results.size(); ++i)
			{
				lowest[i] = results.at(i).at(pointer[i]).first;
				currentLowest = std::min(currentLowest,lowest[i]);
			}
			aligned = true;
			for (unsigned int i = 0; i < results.size(); ++i)
			{
				if(currentLowest != lowest[i])
				{
					aligned = false; 
					break;
				}
			}
			if (aligned == true)
				break;
			if ((currentLowest-lastAdress)/1024 < 100)
			{
				lastAdress = lastAdress+inFiles.at(0)->getBlockSize();
				for (;lastAdress < currentLowest; lastAdress += inFiles.at(0)->getBlockSize())
				{
					std::cout << std::setw(10) << lastAdress/1024 << "\t";
					for (unsigned int i = 0; i < results.size(); ++i)
					{
						std::cout << std::setw(10) << "0.00000\t"; 
					}
					std::cout << std::endl;
				}
			}
			else
			{
				std::cout << std::endl;
			}
			std::cout << std::setw(10) << currentLowest/1024 << "\t";
			for (unsigned int i = 0; i < results.size(); ++i)
			{
				if (results.at(i).at(pointer[i]).first > currentLowest)
				{
					std::cout << std::setw(10) << "0.00000\t"; 
				} else if (results.at(i).at(pointer[i]).second == 0.0f)
				{
					std::cout << std::setw(10) << "0.00000\t";
					pointer[i]++;
				} else
				{
					std::cout << std::setw(10) << (float)results.at(i).at(pointer[i]).second << "\t"; 
					pointer[i]++;
				}
			}
			lastAdress = currentLowest;
			std::cout << std::endl;
		}
	}
	*/
	return true;
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
		if (inFiles.at(i)->reloadBuffer()==false)
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
				eof=inFiles.at(i)->reloadBuffer();
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
					eof=inFiles.at(i)->reloadBuffer();
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


