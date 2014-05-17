#include <texMaker.h>

TexMaker::TexMaker(std::string p)
{
	this->path = p + "thumbcaches.tex";
}

void TexMaker::writeTex(std::vector<PartitionFiles> &p)
{
	file.open(path.c_str(), std::fstream::out);

	file << "The following thumbcaches have been extracted by 'Thumby-Extractor' (c)crisbi (Christian Zoubek, Sabine Seufert)." << std::endl;
	file << "Thumbcaches are sorted by partitions, users and thumbcache-sizes." << std::endl;
	if (p.empty())
	{
		file << "No relevant partition found!" << std::endl;
	}
	for (auto in : p)
	{
		writePartition(in);
	}
	file.close();
}

void TexMaker::writeFile(FileInfo &info)
{
	static size_t count=0;
	file << "\\begin{figure}[h]" << std::endl;
	file << "\t\\centering" << std::endl;
	std::string tmp = info.second.absoluteFileName;
	std::string sys = "convert " + tmp + " " + tmp + ".png";
	if (system(sys.c_str()))
	{}
	file << "\t\t\\includegraphics{" << tmp << ".png}"<< std::endl;
	file << "\\end{figure}" << std::endl;
	file << "Bla" << std::endl;
		file << "\\newpage" << std::endl;
	++count;
}

void TexMaker::writeThumb(ThumbCacheFiles &tcf)
{
	std::string tmp = tcf.first;
	size_t off = tmp.find("_");
	if (off != std::string::npos)
	{
		tmp.replace(off,1," ");
	}
	file << "\\subsubsection{" << tmp << "}" << std::endl;
	for (auto in : tcf.second)
	{
		writeFile(in);
		file << std::endl;
	}
	file << std::endl << std::endl;
}

void TexMaker::writeUser(UserFiles &user)
{
	file << "\\subsection{User " << user.first << "}" << std::endl;
	if (user.second.empty())
	{
		file << "No Thumbcaches found!" << std::endl;
	}
	for (auto in : user.second)
	{
		writeThumb(in);
	}
	file << std::endl << std::endl;
}

void TexMaker::writePartition(PartitionFiles &partition)
{
	file << "\\section{NTFS-Partition with offset " << partition.first << "}" << std::endl;	
	if (partition.second.empty())
	{
		file << "No User with Thumbcaches found!" << std::endl;
	}
	for (auto in : partition.second)
	{
		writeUser(in);
	}
	file << std::endl << std::endl << std::endl;
}

