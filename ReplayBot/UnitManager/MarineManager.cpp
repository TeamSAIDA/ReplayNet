#include "MarineManager.h"
#include "../InformationManager.h"
//#include "../UnitInfo.h"

using namespace MyBot;

MarineManager::MarineManager()
{
}

MarineManager & MarineManager::Instance()
{
	static MarineManager instance;
	return instance;
}

void MarineManager::update()
{

	if (BWAPI::Broodwar->getFrameCount() < 300)
	{
		return;
	}

	if (BWAPI::Broodwar->getFrameCount() % 12 != 0)
	{
		return;
	}

	vector<UnitInfo*> & marineList = InformationManager::Instance().getUnitData(BWAPI::Broodwar->self()).getUnitVector(BWAPI::UnitTypes::Terran_Marine);
	if (InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy()) != nullptr)
	{
		BWAPI::Position targetPosition = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy())->getPosition();

		if (marineList.size() > 12)
		{
			for (int i = 0; i < marineList.size(); i++)
			{
				marineList[i]->setState(new MarineAttackState());
				marineList[i]->action(targetPosition);
			}
		}
		else
		{
			for (int i = 0; i < marineList.size(); i++)
			{
				marineList[i]->setState(new MarineIdleState());
				marineList[i]->action();
			}
		}
	}
}