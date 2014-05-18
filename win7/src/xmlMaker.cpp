#include <xmlMaker.h>

XmlMaker::XmlMaker(std::string p)
{
	this->path = p + "thumbcaches.xml";
}

void XmlMaker::writeXml(std::vector<PartitionFiles> &p)
{
	file.open(path.c_str(), std::fstream::out);

	file << "<Thumbcache-Extractor>" << std::endl;
	file << "\t<Author> Christian Zoubek, Sabine Seufert </Author>" << std::endl;
	file << "\t<Partition-Count> " << p.size() << "</Partition-Count>" << std::endl;
	if (p.empty())
	{
		//file << "\tNo relevant partition found!" << std::endl;
	}
	for (auto in : p)
	{
		writePartition(in);
	}
	file << "</Thumbcache-Extractor>" << std::endl;
	file.close();
}

std::string XmlMaker::cleanString(std::string &in)
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
		ret.insert(off, "\\");
		off += 2;
	}
	return ret;
}

void XmlMaker::writeExif(pictureInfo &pic)
{
	file << "\t\t\t\t\t\t";
	file << "<Exif>" << std::endl;
	file << "\t\t\t\t\t\t\t";
	file << "<Resolution> " << pic.hRes << "x" << pic.vRes << " </Resolution>" << std::endl;
	file << "\t\t\t\t\t\t\t";
	file << "<Bitdepth> " << pic.bitDepth << " </Bitdepth>" << std::endl;
	file << "\t\t\t\t\t\t";
	file << "</Exif>" << std::endl;
}

void XmlMaker::writeSys(systemInfo &sys)
{
	file << "\t\t\t\t\t\t";
	file << "<Systeminfo>" << std::endl;
	file << "\t\t\t\t\t\t\t";
	file << "<Datasize unit=\"KB\" size=\"" << std::to_string(sys.size/1024) << "\" />" << std::endl;
	file << "\t\t\t\t\t\t\t";
	file << "<Dates>" << std::endl;
	file << "\t\t\t\t\t\t\t\t" << "<Accessed> " << sys.dateAccessed << " </Accessed>" << std::endl;
	file << "\t\t\t\t\t\t\t\t" << "<Modified> " << sys.dateModified << " </Modified>" << std::endl;
	file << "\t\t\t\t\t\t\t";
	file << "</Dates>" << std::endl;
	file << "\t\t\t\t\t\t\t";
	file << "<MIME-Type>" << sys.MIMEType << " </MIME-Type>" << std::endl;
	file << "\t\t\t\t\t\t\t";
	file << "<Filename>" << sys.fileName << " </Filename>" << std::endl;
	file << "\t\t\t\t\t\t\t";
	file << "<Owner>" << sys.owner << "</Owner>" << std::endl;
	file << "\t\t\t\t\t\t";
	file << "</Systeminfo>" << std::endl;
}

void XmlMaker::writeMeta(dbInfo &db)
{
	file << "\t\t\t\t\t\t<Thumbsize unit = \"KB\" size=\"" << std::to_string(db.dataSize/1024) << "\" />" << std::endl;
	file << "\t\t\t\t\t\t<Md5Sum> " << db.md5Sum << " </Md5Sum>" << std::endl;
}

void XmlMaker::writeFile(FileInfo &info, bool write)
{
	if (info.second.foundInEDB != write)
		return;
	std::string tmp = info.second.absoluteFileName;
	std::string sys = "convert " + tmp + " " + tmp + ".png";
	if (system(sys.c_str()))
	{}
	file << "\t\t\t\t<Thumbnail>" << std::endl;
	file << "\t\t\t\t\t<img src=\"" << tmp << ".png\"/>"<< std::endl;
	file << "\t\t\t\t\t<Meta>" << std::endl;

	writeMeta(info.second);
	if (info.second.foundInEDB==true)
	{
		writeSys(info.first.sysInfo);
		writeExif(info.first.picInfo);
	}
	file << "\t\t\t\t\t</Meta>" << std::endl;
	file << "\t\t\t\t</Thumbnail>" << std::endl;
}

void XmlMaker::writeThumb(ThumbCacheFiles &tcf)
{
	file << "\t\t\t<" << tcf.first << ">" << std::endl;
	file << "\t\t\t\t<Filecount> " << tcf.second.size() << "</Filecount>" << std::endl;
	file << "\t\t\t\t<Ese_linked>"<< std::endl;
	for (auto in : tcf.second)
	{
		writeFile(in, true);
	}
	file << "\t\t\t\t</Ese_linked>"<< std::endl;
	file << "\t\t\t\t<Thumbnails_only>"<< std::endl;
	for (auto in : tcf.second)
	{
		writeFile(in, false);
	}
	file << "\t\t\t\t</Thumbnails_only>"<< std::endl;
	file << "\t\t\t\t</" << tcf.first << ">" << std::endl;
}

void XmlMaker::writeUser(UserFiles &user)
{
	file << "\t\t<User name=\"" << user.first << "\">" << std::endl;
	for (auto in : user.second)
	{
		writeThumb(in);
	}
	file << "\t\t</User>" << std::endl;
}

void XmlMaker::writePartition(PartitionFiles &partition)
{
	file << "\t<NTFS-Partition>" << std::endl;
	file << "\t\t<Offset>" << partition.first << " </Offset>"<< std::endl;	
	file << "\t\t<Usercount>" << std::endl;
	file << "\t\t\t" << partition.second.size() << std::endl;
	file << "\t\t</Usercount>" << std::endl;
	for (auto in : partition.second)
	{
		writeUser(in);
	}
	file << "\t</NTFS-Partition>";
	file << std::endl;
}

