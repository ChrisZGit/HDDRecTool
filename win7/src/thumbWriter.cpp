#include <thumbWriter.h>

ThumbWriter::ThumbWriter(std::string in, std::string out)
{
	inPath = in;
	outPath = out;
	writeTex = false;
	writeXml = false;
	edbOnly = false;
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

void ThumbWriter::writeThumbs()
{
	DataHandler newOne(inPath, outPath);
	newOne.initHandlers();
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
		std::cout << "Donw with writing XML File." << std::endl;
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
