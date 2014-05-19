#include <texMaker.h>

TexMaker::TexMaker(std::string p)
{
	this->path = p + "thumbcaches.tex";
}

void TexMaker::writeTex(std::vector<PartitionFiles> &p, bool edb)
{
	edbOnly = edb;
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

std::string TexMaker::cleanString(std::string &in)
{
	std::string ret = in;
	size_t off = 0;
	while ((off=ret.find("\\",off)) != std::string::npos)
	{
		ret.replace(off,1,"/");
		off += 1;
	}
	off=0;
	while ((off=ret.find("_",off)) != std::string::npos)
	{
		//ret.replace(off,1,"/");
		ret.insert(off, "\\");
		off += 2;
	}
	return ret;
}

void TexMaker::writeExif(pictureInfo &pic)
{
	file << "\t\t\\hline\\\\" << std::endl;
	file << "\t\tHorizontal res& " << pic.hRes << "\\\\" << std::endl;
	file << "\t\t\\hline\\\\" << std::endl;
	file << "\t\tVertikal res& " << pic.vRes << "\\\\" << std::endl;
	file << "\t\t\\hline\\\\" << std::endl;
	file << "\t\tBitdepth& " << pic.bitDepth << "\\\\" << std::endl;
	file << "\t\t\\hline\\\\" << std::endl;
}

void TexMaker::writeSys(systemInfo &sys)
{
	file << "\t\tDatasize& " << std::to_string(sys.size/1024) << "KB\\\\" << std::endl;
	//file << "\t\t\\hline\\\\" << std::endl;
	//file << "\t\tDate created& " << sys.dateCreated << "\\\\" << std::endl;
	file << "\t\t\\hline\\\\" << std::endl;
	file << "\t\tDate accessed& " << sys.dateAccessed << "\\\\" << std::endl;
	file << "\t\t\\hline\\\\" << std::endl;
	file << "\t\tDate modified& " << sys.dateModified << "\\\\" << std::endl;
	file << "\t\t\\hline\\\\" << std::endl;
	file << "\t\tMIME Type& " << cleanString(sys.MIMEType) << "\\\\" << std::endl;
	file << "\t\t\\hline\\\\" << std::endl;
	file << "\t\tFilename& " << cleanString(sys.fileName) << "\\\\" << std::endl;
	file << "\t\t\\hline\\\\" << std::endl;
	file << "\t\tOwner& " << cleanString(sys.owner) << "\\\\" << std::endl;
	file << "\t\t\\hline\\\\" << std::endl;
}

void TexMaker::writeMeta(dbInfo &db)
{
	file << "\t\tThumbsize & " << std::to_string(db.dataSize/1024) << "KB\\\\" << std::endl;
	file << "\t\t\\hline\\\\" << std::endl;
	file << "\t\tmd5Sum & " << db.md5Sum << "\\\\" << std::endl;
	file << "\t\t\\hline\\\\" << std::endl;
}

void TexMaker::writeFile(FileInfo &info, bool write)
{
	if (edbOnly == true && info.second.foundInEDB == false)
		return;
	static size_t count=0;
	if (info.second.foundInEDB != write)
		return;
	file << "\\begin{figure}[h]" << std::endl;
	file << "\t\\centering" << std::endl;
	std::string tmp = info.second.absoluteFileName;
	std::string sys = "convert " + tmp + " " + tmp + ".png";
	if (system(sys.c_str()))
	{}
	file << "\t\t\\includegraphics{" << tmp << ".png}"<< std::endl;
	file << "\t\\caption{Thumbnail 0x" << info.second.hash << "}" << std::endl;
	file << "\t\\label{fig:Pic" << std::to_string(count) << "}" << std::endl;
	file << "\\end{figure}" << std::endl << std::endl;

	if (info.second.foundInEDB==true)
		file << "\\newpage" << std::endl;
	file << "\\begin{table}[h]" << std::endl;
	file << "\t\\centering" << std::endl;
	file << "\t\\begin{tabular}{c|c}" << std::endl;
	writeMeta(info.second);
	if (info.second.foundInEDB==true)
	{
		writeSys(info.first.sysInfo);
		writeExif(info.first.picInfo);
	}
	file << "\t\\end{tabular}" << std::endl;
	file << "\t\\caption{Meta-Data for Figure~\\ref{fig:Pic" << std::to_string(count) << "}}" << std::endl;
	file << "\\end{table}" << std::endl;
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
		writeFile(in, true);
		file << std::endl;
	}
	file << "\\newpage" << std::endl;
	for (auto in : tcf.second)
	{
		writeFile(in, false);
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
	file << "\\newpage" << std::endl;
	for (auto in : partition.second)
	{
		writeUser(in);
	}
	file << std::endl << std::endl << std::endl;
}

