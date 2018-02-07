#include "Common.h"
#include "UnitData.h"
#include "BWEM/src/utils.h"

using namespace MyBot;

UnitData::UnitData()
{
}

UnitData::~UnitData()
{
}
///////// get UnitInfo Vector
std::vector<UnitInfo*>& UnitData::getUnitVector(BWAPI::UnitType uType)
{
	// 아직 안만들어 졌을 경우 새로 생성
	if (unitTypeMap.find(uType) == unitTypeMap.end())
	{
		std::vector<UnitInfo*> newUnitType;
		unitTypeMap[uType] = newUnitType;
	}
	return unitTypeMap[uType];
}

std::vector<UnitInfo*>& UnitData::getBuildingVector(BWAPI::UnitType uType)
{
	// 아직 안만들어 졌을 경우 새로 생성
	if (buildingTypeMap.find(uType) == buildingTypeMap.end())
	{
		std::vector<UnitInfo*> newUnitType;
		buildingTypeMap[uType] = newUnitType;
	}
	return buildingTypeMap[uType];
}
int const MyBot::UnitData::getCompletedCount(BWAPI::UnitType uType)
{
	if (completedCount.find(uType) == completedCount.end())
		completedCount[uType] = 0;

	return completedCount[uType];
}
int const MyBot::UnitData::getallCount(BWAPI::UnitType uType)
{
	if (allCount.find(uType) == allCount.end())
		allCount[uType] = 0;

	return allCount[uType];
}
void MyBot::UnitData::increaseCompleteUnits(BWAPI::UnitType uType)
{
	if (completedCount.find(uType) == completedCount.end())
		completedCount[uType] = 0;

	completedCount[uType]++;
}
void MyBot::UnitData::increaseCreateUnits(BWAPI::UnitType uType)
{
	if (allCount.find(uType) == allCount.end())
		allCount[uType] = 0;
	allCount[uType]++;
}
void MyBot::UnitData::decreaseCompleteUnits(BWAPI::UnitType uType)
{
	completedCount[uType]--;
}
void MyBot::UnitData::decreaseCreateUnits(BWAPI::UnitType uType)
{
	allCount[uType]--;
}
//////// add UnitInfo
void UnitData::addUnitNBuilding(BWAPI::Unit u)
{
	BWAPI::UnitType type = u->getType();
	if (type == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode)
		type = BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode;

	std::map<BWAPI::Unit, UnitInfo*> & unitMap = type.isBuilding() ? getAllBuildings() : getAllUnits();
	std::vector<UnitInfo*> & unitVector = type.isBuilding() ? getBuildingVector(u->getType()) : getUnitVector(u->getType());
	
	if (unitMap.find(u) == unitMap.end())
	{
		UnitInfo* pUnit = new UnitInfo(u);
		unitMap[u] = pUnit;
		unitVector.push_back(pUnit);
	}
}

////////////// remove UnitInfo
void UnitData::removeUnitNBuilding(BWAPI::Unit u)
{
	BWAPI::UnitType type = u->getType();
	if (type == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode) 
		type = BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode;


	std::map<BWAPI::Unit, UnitInfo*> & unitMap = type.isBuilding() ? getAllBuildings() : getAllUnits();
	std::vector<UnitInfo*> & unitVector = type.isBuilding() ? getBuildingVector(u->getType()) : getUnitVector(u->getType());
	
	if (unitMap.find(u) != unitMap.end())
	{
		auto del_unit = find_if(unitVector.begin(), unitVector.end(), [u](UnitInfo* up) {return up->Unit() == u; });
		if (del_unit != unitVector.end()) {
			BWEM::utils::fast_erase(unitVector, distance(unitVector.begin(), del_unit));
		}
		else {
			//std::cout << "remove Unit Error" << std::endl;
		}

		delete unitMap[u];
		unitMap.erase(u);
	}
}

//////////// update UnitInfo
void UnitData::updateAllInfo()
{
	for (auto & u : allUnits)
		u.second->Update();

	for (auto & u : allBuildings)
		u.second->Update();
}