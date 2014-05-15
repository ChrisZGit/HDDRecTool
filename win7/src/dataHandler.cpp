#include <dataHandler.h>

volatile bool loadAvail;
std::mutex mtx;

DataHandler::DataHandler()
{
	EDBHandler bla("./databases/","vol2-C..ProgramData.Microsoft.Search.Data.Applications.Windows.Windows.edb");
	bla.startHandler();
	edbInfo t;
	bool ret;
	ret = bla.getHash("8dc67ebcfe5e4459", t);
	if (ret == true)
		std::cout << t.sysInfo.fileName << std::endl;
}
