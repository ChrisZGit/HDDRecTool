#include <imageCarver.h>

ImageCarver::ImageCarver(std::string in, std::string out)
{
	fileName = in.substr(0,in.length()-1);
	outputPath = out;
}

bool ImageCarver::carveImg()
{
	mmls();
	if (partition.size()==0)
		return false;
	for (size_t i = 0; i < partition.size(); ++i)
	{
		fls(partition.at(i).first, partition.at(i).second);
	}
	while(true);
	return true;
}

void ImageCarver::mmls()
{
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
		Database useMe = userDatas.at(count).second;
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
					useMe.push_back(pushMe);
				}
			}
		}
	}
}

