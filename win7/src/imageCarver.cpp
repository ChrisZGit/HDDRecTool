#include <imageCarver.h>

ImageCarver::~ImageCarver()
{
	std::string sys = "rm -rf ";
	sys += outputPath;
	if (system(sys.c_str()))
	{}
}

ImageCarver::ImageCarver(std::string in, std::string out)
{
	fileName = in.substr(0,in.length()-1);
	outputPath = out;
}

bool ImageCarver::carveImg()
{
	mmls();
	if (partition.size()==0)
	{
		std::cout << "None found! Have to abort" << std::endl;
		return false;
	}
	std::cout << "Found: " << partition.size() << " partitions" << std::endl;
	std::string sys = "rm -rf ";
	sys += outputPath;
	if (system(sys.c_str()))
	{}
	sys = "mkdir ";
	sys += outputPath;
	if (system(sys.c_str()))
	{}

	for (size_t i = 0; i < partition.size(); ++i)
	{
		std::cout << "Starting fls for partition with offset " << partition.at(i).first << std::endl;
		fls(partition.at(i).first, partition.at(i).second);
	}
	bool ret = false;
	for (size_t i = 0; i < partition.size(); ++i)
	{
		std::string dirName = outputPath+std::to_string(partition.at(i).first);
		sys = "rm -rf ";
		sys += dirName;
		if (system(sys.c_str()))
		{}
		std::string edbInode = carveEDB(partition.at(i).first);
		if (edbInode == "")
		{
			std::cout << "No EDB for partition with offset " << partition.at(i).first << " found." << std::endl; 
			continue;
		}
		ret = true;
		sys = "mkdir ";
		sys += dirName;
		if (system(sys.c_str()))
		{}
		dirName += "/";
		icat(partition.at(i).first, edbInode, dirName+"Windows.edb");
		for (size_t j = 0; j < partition.at(i).second.size(); ++j)
		{
			Database me = partition.at(i).second.at(j).second;
			if (me.size() != 0)
			{
				std::string name = dirName+partition.at(i).second.at(j).first;
				sys = "rm -rf ";
				sys += name;
				if (system(sys.c_str()))
				{}
				sys = "mkdir ";
				sys += name;
				if (system(sys.c_str()))
				{}
				name += "/";
				std::cout << "Carving for DB-Files of User " << partition.at(i).second.at(j).first << std::endl;
				for (size_t k = 0; k < me.size(); ++k)
				{
					std::string file = name + me.at(k).second;
					std::string inode = me.at(k).first;
					size_t offset = partition.at(i).first;
					icat(offset, inode, file);
				}
			}
		}
	}
	return ret;
}

void ImageCarver::mmls()
{
	std::cout << "Running 'mmls' to catch all NTFS-Partitions" << std::endl;
	FILE *pipe;
	char buf[256];
	std::string sys = "mmls ";
	sys += fileName;
	pipe = popen(sys.c_str(), "r");
	
	if (!(pipe))
		return;
	while (!(feof(pipe)))
	{
		std::stringstream ss;
		std::string type;
		size_t offset;
		if (fgets(buf, 256, pipe) != NULL)
		{
			ss << buf;
			ss >> type; //index
			ss >> type; //entry
			ss >> offset; //startoffset
			ss >> type; //end
			ss >> type; //length
			ss >> type;
			if (type.find("NTFS") != std::string::npos)
			{
				std::pair<size_t, UserDatas> pushMe;
				UserDatas *user = new UserDatas;
				pushMe = std::make_pair(offset, *user);
				partition.push_back(pushMe);
			}
		}
	}
	pclose(pipe);
}

std::string ImageCarver::carveEDB(size_t offset)
{
	std::cout << "Carving for EDB-File" << std::endl;
	const char *findStrings[6] = {"ProgramData", "Microsoft", "Search", "Data", "Applications", "Windows"};
	char buf[256];
	
	std::string sys = "fls -D ";
	sys += fileName;
	sys += " -o ";
	sys += std::to_string(offset);
	sys += " ";

	std::string inode = "";
	for (size_t i = 0; i < 6; ++i)
	{
		std::string call = sys + inode;
		inode = "";
		FILE *pipe = popen(call.c_str(), "r");
	
		if (!(pipe))
			return "";
		while (!(feof(pipe)))
		{
			if (fgets(buf, 256, pipe) != NULL)
			{
				std::string line = buf;
				std::stringstream ss;
				if (line.find(findStrings[i]) != std::string::npos)
				{
					ss << line;
					ss >> line;
					ss >> inode;
					inode.pop_back();
					break;
				}
			}
		}
		if (inode == "")
			break;
	}
	if (inode == "")
		return inode;

	std::string call = "fls " + fileName + " -o " + std::to_string(offset) + " " + inode;
	FILE *pipe = popen(call.c_str(), "r");
	if (!(pipe))
		return "";
	inode = "";
	while (!(feof(pipe)))
	{
		if (fgets(buf, 256, pipe) != NULL)
		{
			std::string line = buf;
			std::stringstream ss;
			if (line.find(".edb") != std::string::npos)
			{
				ss << line;
				ss >> line;
				ss >> inode;
				inode.pop_back();
				break;
			}
		}
	}
	return inode;
}

void ImageCarver::fls(size_t offset, UserDatas &userDatas)
{
	const char *findStrings[6] = {"Users", "AppData", "Local", "Microsoft", "Windows", "Explorer"};
	std::vector<std::string> userInodes;
	FILE *pipe;
	char buf[256];
	std::string sys = "fls -D ";
	sys += fileName;
	sys += " -o ";
	sys += std::to_string(offset);
	pipe = popen(sys.c_str(), "r");

	if (!(pipe))
		return;
	std::string inode;
	while (!(feof(pipe)))
	{
		std::stringstream ss;
		std::string type;
		if (fgets(buf, 256, pipe) != NULL)
		{
			std::string line = buf;
			if (line.find(findStrings[0]) != std::string::npos)
			{
				ss << line;
				ss >> line;
				ss >> inode;
				inode.pop_back();
				break;
			}
		}
	}
	//Users found
	if (inode.empty())
		return;
	std::cout << "Found some Users. Will take a closer look." << std::endl;
	std::string newCall = sys;
	newCall += " " + inode;

	pipe = popen(newCall.c_str(), "r");
	if (!(pipe))
		return;
	while (!(feof(pipe)))
	{
		std::stringstream ss;
		std::string name;
		if (fgets(buf, 256, pipe) != NULL)
		{
			std::string line = buf;
			ss << buf;  
			ss >> name;
			ss >> inode; //user inode
			ss >> name; //user name
			inode.pop_back();
			name = line.substr(line.find(name), std::string::npos);
			std::pair<std::string, Database> pushMe;
			Database *newOne = new Database;
			name.pop_back();
			pushMe = make_pair(name, *newOne);
			userDatas.push_back(pushMe);
			userInodes.push_back(inode);
		}
	}
	int count=-1;
	for (auto in : userInodes)
	{
		count++;
		inode = in;
		for (size_t i = 0; i < 5; ++i)
		{
			if (inode == "")
				break;
			newCall = sys;
			newCall += " " + inode;
			pipe = popen(newCall.c_str(), "r");
			inode="";
			if (!(pipe))
				return;
			while (!(feof(pipe)))
			{
				if (fgets(buf, 256, pipe) != NULL)
				{
					std::string line = buf;
					std::stringstream ss;
					if (line.find(findStrings[i+1]) != std::string::npos)
					{
						ss << line;
						ss >> line;
						ss >> inode;
						inode.pop_back();
						break;
					}
				}
			}
		}
		if (inode == "")
			continue;
		//found directory to databases
		std::cout << "Found a directory to relevant databases of User " << userDatas.at(count).first << std::endl;
		newCall = "fls -o ";
		newCall += std::to_string(offset);
		newCall += " " + fileName + " ";
		newCall += inode;

		pipe = popen(newCall.c_str(), "r");
		inode="";
		if (!(pipe))
			return;
		while (!(feof(pipe)))
		{
			if (fgets(buf, 256, pipe) != NULL)
			{
				std::string line = buf;
				std::string name;
				std::stringstream ss;
				if (line.find(".db") != std::string::npos)
				{
					ss << line;
					ss >> line;
					ss >> inode;
					ss >> name;
					inode.pop_back();

					std::pair<std::string, std::string> pushMe;
					pushMe = std::make_pair(inode, name);
					userDatas.at(count).second.push_back(pushMe);
				}
			}
		}
	}
}

void ImageCarver::icat(size_t offset, std::string inode, std::string name)
{
	std::cout << "Extracting (icat) file " << name << " at inode " << inode << std::endl;
	std::string sys = "icat ";
	sys += fileName;
	sys += " -o " + std::to_string(offset);
	sys += " " + inode;
	sys += " > " + name; 
	if (system(sys.c_str()))
	{}
}


