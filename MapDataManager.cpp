#include "MapDataManager.h"
#include <io.h>
//#include "BWEM/src/bwem.h"      // update the path if necessary

using namespace MyBot;

MapDataManager & MapDataManager::Instance()
{
	static MapDataManager instance;
	return instance;
}

MapDataManager::MapDataManager()
	
{
}

