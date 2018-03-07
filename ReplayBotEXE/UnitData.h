#pragma once

#include "Common.h"
#include "UnitInfo.h"

namespace MyBot
{
	class UnitData
	{
	public:
		UnitData();
		~UnitData();

		std::string			playerName;
		int					playerID;
		BWAPI::Race			playerRace;

		std::vector<UnitInfo*> & getUnitVector(BWAPI::UnitType);
//		std::vector<UnitInfo*> & getAllUnitVector() { return allUnits; };
		std::vector<UnitInfo*> & getBuildingVector(BWAPI::UnitType);
		std::map<BWAPI::UnitType, std::vector<UnitInfo*> > & getUnitTypeMap() {	return unitTypeMap;}
		std::map<BWAPI::UnitType, std::vector<UnitInfo*> > & getBuildingTypeMap() { return buildingTypeMap; }
		std::map<BWAPI::Unit, UnitInfo*> & getAllUnits() { return allUnits; }
		std::map<BWAPI::Unit, UnitInfo*> & getAllBuildings() { return allBuildings; }

		int const getCompletedCount(BWAPI::UnitType);
		int const getallCount(BWAPI::UnitType);

		void increaseCompleteUnits(BWAPI::UnitType);
		void increaseCreateUnits(BWAPI::UnitType);
		void decreaseCompleteUnits(BWAPI::UnitType);
		void decreaseCreateUnits(BWAPI::UnitType);

		void addUnitNBuilding(BWAPI::Unit);
		void removeUnitNBuilding(BWAPI::Unit);
		void updateAllInfo();

	private:
		std::map<BWAPI::UnitType, std::vector<UnitInfo*> > unitTypeMap;
		std::map<BWAPI::UnitType, std::vector<UnitInfo*> > buildingTypeMap;

		std::map<BWAPI::Unit, UnitInfo*> allUnits;
		std::map<BWAPI::Unit, UnitInfo*> allBuildings;

		std::map<BWAPI::UnitType, int> completedCount;
		std::map<BWAPI::UnitType, int> allCount;
	};
}