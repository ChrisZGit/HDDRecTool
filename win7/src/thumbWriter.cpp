#include <thumbWriter.h>

ThumbWriter::ThumbWriter(std::string in, std::string out)
{
	inPath = in;
	outPath = out;
	writeTex = false;
	writeXml = false;
	edbOnly = false;
	isExtracted = false;
	offset = -1;
}

void ThumbWriter::setTex(bool tex)
{
	writeTex = tex;
}

void ThumbWriter::setXml(bool xml)
{
	writeXml = xml;
}

void ThumbWriter::setEdb(bool edb)
{
	edbOnly = edb;
}

void ThumbWriter::setExtracted(bool ex)
{
	isExtracted = ex;
}

void ThumbWriter::setOffset(int off)
{
	offset = off;
}

void ThumbWriter::writeThumbs()
{
	DataHandler newOne(inPath, outPath);
	if (isExtracted == true)
	{
		std::cout << "Image doesn't have to be extracted. Starting to work with the given Thumbcaches." << std::endl;
		std::string sys = "rm -rf ./databases314159265/";
		if (system(sys.c_str()))
		{}
		sys = "mkdir ./databases314159265";
		if (system(sys.c_str()))
		{}
		sys += "/0";
		if (system(sys.c_str()))
		{}
		sys += "/Userinput";
		if (system(sys.c_str()))
		{}
		sys = "cp " + inPath + "/*.db ./databases314159265/0/Userinput/";
		if (system(sys.c_str()))
		{}
		sys = "cp " + inPath + "/*.edb ./databases314159265/0/";
		if (system(sys.c_str()))
		{}
	}
	else
	{
		if (newOne.carveImg() == false)
		{
			std::cerr << "Carving image failed. Have to abort." << std::endl;
			exit(1);
		}
	}
	if (!(offset <= -1))
	{
		std::cout << "Offset to a partition has been set. Starting to work on that." << std::endl;
		newOne.setOffset(offset);
	}

	if (newOne.initHandlers() == false)
	{
		std::cout << "Could not init handlers.\nHave to abort" << std::endl;
		return;
	}
	newOne.startHandlers();
	newOne.linkDBtoEDB();

	if (writeTex == true)
	{
		std::cout << "Starting to write Tex File." << std::endl;
		TexMaker bla(outPath);
		auto abc = newOne.getGatheredInfos();
		bla.writeTex(abc, edbOnly);
		std::cout << "Done with writing Tex file." << std::endl;
	}
	else if (writeXml == true)
	{
		std::cout << "Starting to write XML File." << std::endl;
		XmlMaker bla2(outPath);
		auto abc = newOne.getGatheredInfos();
		bla2.writeXml(abc, edbOnly);
		std::cout << "Done with writing XML File." << std::endl;
	}
	else
	{
		std::cout << "Starting to write Tex File." << std::endl;
		TexMaker bla(outPath);
		auto abc = newOne.getGatheredInfos();
		bla.writeTex(abc, edbOnly);
		std::cout << "Done with writing Tex File." << std::endl;

		std::cout << "Starting to write XML File." << std::endl;
		XmlMaker bla2(outPath);
		bla2.writeXml(abc, edbOnly);
		std::cout << "Done with writing XML File." << std::endl;
	}
}
