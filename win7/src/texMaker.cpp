#include <texMaker.h>

TexMaker::TexMaker(std::string p)
{
	this->path = p + "thumbcaches.tex";
	file = new FileWriter(path, 8*1024*1024);
}

void TexMaker::writeTex(std::vector<PartitionFiles> &p, bool edb)
{
	edbOnly = edb;
	//(*file).open(path.c_str(), std::fstream::out);

	std::string ab = "abc";
	size_t si=3;
	file->writeToFile((char *)ab.c_str(), si);
	(*file) << "The following thumbcaches have been extracted by 'Thumby-Extractor' (c)crisbi (Christian Zoubek, Sabine Seufert)." << "\n";
	(*file) << "Thumbcaches are sorted by partitions, users and thumbcache-sizes." << "\n";
	if (p.empty())
	{
		(*file) << "No relevant partition found!" << "\n";
	}
	for (auto& in : p)
	{
		writePartition(in);
	}
	//(*file).close();
	file->closeFile();
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
	(*file) << "\t\t\\hline\\\\" << "\n";
	(*file) << "\t\tHorizontal res& " << pic.hRes << "\\\\" << "\n";
	(*file) << "\t\t\\hline\\\\" << "\n";
	(*file) << "\t\tVertikal res& " << pic.vRes << "\\\\" << "\n";
	(*file) << "\t\t\\hline\\\\" << "\n";
	(*file) << "\t\tBitdepth& " << pic.bitDepth << "\\\\" << "\n";
	(*file) << "\t\t\\hline\\\\" << "\n";
}

void TexMaker::writeSys(systemInfo &sys)
{
	(*file) << "\t\tDatasize& " << std::to_string(sys.size/1024) << "KB\\\\" << "\n";
	//(*file) << "\t\t\\hline\\\\" << "\n";
	//(*file) << "\t\tDate created& " << sys.dateCreated << "\\\\" << "\n";
	(*file) << "\t\t\\hline\\\\" << "\n";
	(*file) << "\t\tDate accessed& " << sys.dateAccessed << "\\\\" << "\n";
	(*file) << "\t\t\\hline\\\\" << "\n";
	(*file) << "\t\tDate modified& " << sys.dateModified << "\\\\" << "\n";
	(*file) << "\t\t\\hline\\\\" << "\n";
	(*file) << "\t\tMIME Type& " << cleanString(sys.MIMEType) << "\\\\" << "\n";
	(*file) << "\t\t\\hline\\\\" << "\n";
	(*file) << "\t\tFilename& " << cleanString(sys.fileName) << "\\\\" << "\n";
	(*file) << "\t\t\\hline\\\\" << "\n";
	(*file) << "\t\tOwner& " << cleanString(sys.owner) << "\\\\" << "\n";
	(*file) << "\t\t\\hline\\\\" << "\n";
}

void TexMaker::writeMeta(dbInfo &db)
{
	(*file) << "\t\tThumbsize & " << std::to_string(db.dataSize/1024) << "KB\\\\" << "\n";
	(*file) << "\t\t\\hline\\\\" << "\n";
	(*file) << "\t\tmd5Sum & " << db.md5Sum << "\\\\" << "\n";
	(*file) << "\t\t\\hline\\\\" << "\n";
}

void TexMaker::writeFile(FileInfo &info, bool write)
{
	if (edbOnly == true && info.second.foundInEDB == false)
		return;
	static size_t count=0;
	if (info.second.foundInEDB != write)
		return;
	(*file) << "\\begin{figure}[h]" << "\n";
	(*file) << "\t\\centering" << "\n";
	std::string tmp = info.second.absoluteFileName;
	std::string sys = "convert " + tmp + " " + tmp + ".png";
	if (system(sys.c_str()))
	{}
	(*file) << "\t\t\\includegraphics{" << tmp << ".png}"<< "\n";
	(*file) << "\t\\caption{Thumbnail 0x" << info.second.hash << "}" << "\n";
	(*file) << "\t\\label{fig:Pic" << std::to_string(count) << "}" << "\n";
	(*file) << "\\end{figure}" << "\n" << "\n";
	(*file) << "\\vspace*{3cm}\n" << "\n";

	if (info.second.foundInEDB==true)
	{
		(*file) << "Table with info on the next page!" << "\n";
		(*file) << "\\newpage" << "\n";
	}
	(*file) << "\\begin{table}[h]" << "\n";
	(*file) << "\t\\centering" << "\n";
	(*file) << "\t\\begin{tabular}{c|c}" << "\n";
	writeMeta(info.second);
	if (info.second.foundInEDB==true)
	{
		writeSys(info.first.sysInfo);
		writeExif(info.first.picInfo);
	}
	(*file) << "\t\\end{tabular}" << "\n";
	(*file) << "\t\\caption{Meta-Data for Figure~\\ref{fig:Pic" << std::to_string(count) << "}}" << "\n";
	(*file) << "\\end{table}" << "\n";
	(*file) << "\\newpage" << "\n";
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
	(*file) << "\\subsubsection{" << tmp << "}" << "\n";
	(*file) << "\\hspace*{\\fill} \\\\" << "\n";
	for (auto& in : tcf.second)
	{
		writeFile(in, true);
		(*file) << "\n";
	}
	(*file) << "\\newpage" << "\n";
	for (auto& in : tcf.second)
	{
		writeFile(in, false);
		(*file) << "\n";
	}
	(*file) << "\n" << "\n";
}

void TexMaker::writeUser(UserFiles &user)
{
	(*file) << "\\subsection{User " << user.first << "}" << "\n";
	if (user.second.empty())
	{
		(*file) << "No Thumbcaches found!" << "\n";
	}
	//for (auto& in : user.second)
	for (auto in = user.second.rbegin(); in != user.second.rend(); ++in)
	{
		writeThumb(*in);
	}
	(*file) << "\n";
}

void TexMaker::writePartition(PartitionFiles &partition)
{
	(*file) << "\\newpage" << "\n";
	(*file) << "\\section{NTFS-Partition with offset " << partition.first << "}" << "\n";	
	if (partition.second.empty())
	{
		(*file) << "No User with Thumbcaches found!" << "\n";
	}
	for (auto& in : partition.second)
	{
		writeUser(in);
	}
	(*file) << "\n";
}

