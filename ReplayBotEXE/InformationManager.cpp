#include "InformationManager.h"

using namespace MyBot;

InformationManager::InformationManager()
{
	// 맵 파일이름으로부터 맵 플레이어수 (2인용 맵, 3인용 맵, 4인용 맵, 8인용 맵) 
	// 및 맵 이름 (맵 파일이름이 조금 달라지더라도, 맵 이름으로 전략을 선택) 을 추출한다
	mapPlayerLimit = BWAPI::Broodwar->getStartLocations().size();
	mapFileName = BWAPI::Broodwar->mapFileName();
	std::replace(mapFileName.begin(), mapFileName.end(), ' ', '_');
	std::replace(mapFileName.begin(), mapFileName.end(), ',', '_');

	if (BWAPI::Broodwar->mapName().find("Benzene") != std::string::npos) {
		mapPlayerLimit = 2;
		mapName = "benzene";
	}
	else if (BWAPI::Broodwar->mapName().find("Destination") != std::string::npos) {
		mapPlayerLimit = 2;
		mapName = "destination";
	}
	else if (BWAPI::Broodwar->mapName().find("HeartbreakRidge") != std::string::npos) {
		mapPlayerLimit = 2;
		mapName = "heartbreakridge";
	}
	else if (BWAPI::Broodwar->mapName().find("Aztec") != std::string::npos) {
		mapPlayerLimit = 3;
		mapName = "aztec";
	}
	else if (BWAPI::Broodwar->mapName().find("TauCross") != std::string::npos) {
		mapPlayerLimit = 3;
		mapName = "taucross";
	}
	else if (BWAPI::Broodwar->mapName().find("Andromeda") != std::string::npos) {
		mapPlayerLimit = 4;
		mapName = "andromeda";
	}
	else if (BWAPI::Broodwar->mapName().find("CircuitBreaker") != std::string::npos) {
		mapPlayerLimit = 4;
		mapName = "circuitbreaker";
	}
	else if (BWAPI::Broodwar->mapName().find("EmpireoftheSun") != std::string::npos) {
		mapPlayerLimit = 4;
		mapName = "empireofthesun";
	}
	else if (BWAPI::Broodwar->mapName().find("Fortress") != std::string::npos) {
		mapPlayerLimit = 4;
		mapName = "fortress";
	}
	else if (BWAPI::Broodwar->mapName().find("Python") != std::string::npos) {
		mapPlayerLimit = 4;
		mapName = "python";
	}
	else if (BWAPI::Broodwar->mapName().find("Temple") != std::string::npos) {
		mapPlayerLimit = 4;
		mapName = "losttemple";
	}
	else {
		mapPlayerLimit = BWAPI::Broodwar->getStartLocations().size();
		mapName = BWAPI::Broodwar->mapName();
	}

	// 리플레이 파일의 총 프레임 카운트를 센다
	replayTotalFrameCount = 0;
	if (BWAPI::Broodwar->isReplay()) {
		replayTotalFrameCount = BWAPI::Broodwar->getReplayFrameCount();
	}

	// 실제로 게임을 플레이하는 ActivePlayer 들의 목록을 만든다

	// TODO : 옵저버 플레이어 제외시키기가 필요하다

	BWAPI::Playerset playerset = BWAPI::Broodwar->getPlayers();
	
	for (const auto& player : BWAPI::Broodwar->getPlayers()){

		if (!player->getUnits().empty() && !player->isNeutral()) {

			// TODO : you cannot trust this startLocID
			int startLocID = -1;
			for (const auto& startLocation : BWAPI::Broodwar->getStartLocations()) {
				startLocID++;
				if (player->getStartLocation() == startLocation) break;
			}
			
			activePlayers.insert(player);

			_unitData[player] = UnitData();
			_unitData[player].playerName = player->getName();
			_unitData[player].playerID = player->getID();
			_unitData[player].playerRace = player->getRace();

			_mainBaseLocations[player] = BWTA::getStartLocation(player);
			_mainBaseLocationChanged[player] = true;
			_occupiedBaseLocations[player] = std::list<BWTA::BaseLocation *>();
			_occupiedBaseLocations[player].push_back(_mainBaseLocations[player]);

			updateOccupiedRegions(player);

			_firstChokePoint[player] = nullptr;
			_firstExpansionLocation[player] = nullptr;
			_secondChokePoint[player] = nullptr;

		}
	}	

	updateChokePointAndExpansionLocation();
}

void InformationManager::update()
{
	updateUnitsInfo();

	// occupiedBaseLocation 이나 occupiedRegion 은 거의 안바뀌므로 자주 안해도 된다
	if (BWAPI::Broodwar->getFrameCount() % 120 == 0) {
		updateBaseLocationInfo();
	}


	if (BWAPI::Broodwar->getFrameCount() % 24 == 0) {

	}
}

void InformationManager::updateUnitsInfo()
{
	for (const auto& p : activePlayers) {
		_unitData[p].updateAllInfo();
	}
}

void InformationManager::addUnitInfo(BWAPI::Unit unit)
{
	if (unit->getType().isNeutral())
		return;

	_unitData[unit->getPlayer()].addUnitNBuilding(unit);
}

void InformationManager::onUnitCreate(BWAPI::Unit unit)
{
	if (unit->getType().isNeutral())
		return;

	_unitData[unit->getPlayer()].increaseCreateUnits(unit->getType());
}

void InformationManager::onUnitComplete(BWAPI::Unit unit)
{
	if (unit->getType().isNeutral())
		return;

	addUnitInfo(unit);
	_unitData[unit->getPlayer()].increaseCompleteUnits(unit->getType());
}

// 유닛이 파괴/사망한 경우, 해당 유닛 정보를 삭제한다
void InformationManager::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit->getType().isNeutral())
		return;

	_unitData[unit->getPlayer()].removeUnitNBuilding(unit);
	
	if(unit->isCompleted())
		_unitData[unit->getPlayer()].decreaseCompleteUnits(unit->getType());

	_unitData[unit->getPlayer()].decreaseCreateUnits(unit->getType());
}

bool InformationManager::isCombatUnitType(BWAPI::UnitType type) const
{
	// check for various types of combat units
	if (type.canAttack() ||
		type == BWAPI::UnitTypes::Terran_Medic ||
		type == BWAPI::UnitTypes::Protoss_Observer ||
		type == BWAPI::UnitTypes::Terran_Bunker ||
		type == type == BWAPI::UnitTypes::Zerg_Lurker)
	{
		return true;
	}
	return false;
}
/*
void InformationManager::getNearbyForce(std::vector<UnitInfo*> & unitInfo, BWAPI::Position p, BWAPI::Player player, int radius)
{
	unsigned int i = 0;
	std::map<BWAPI::UnitType, std::vector<UnitInfo> > & unitMap = _unitData[player].getUnitTypeMap();
	std::map<BWAPI::UnitType, std::vector<UnitInfo> >::iterator iter;

	for (iter = unitMap.begin(); iter != unitMap.end(); iter++)
	{
		std::vector<UnitInfo> & unitVector = iter->second;

		for (i = 0; i < unitVector.size(); i++)
		{
			UnitInfo * ui(&(unitVector[i]));

			if (isCombatUnitType(ui.Type()) && ui->isComplete())
			{
				// determine its attack range
				int range = 0;
				if (ui->Type().groundWeapon() != BWAPI::WeaponTypes::None)
				{
					range = ui->Type().groundWeapon().maxRange() + 40;
				}

				// if it can attack into the radius we care about
				if (ui->Pos().getDistance(p) <= (radius + range))
				{
					// add it to the vector
					unitInfo.push_back(ui);
				}
			}
		}
	}
}
*/
InformationManager & InformationManager::Instance()
{
	static InformationManager instance;
	return instance;
}


void InformationManager::updateBaseLocationInfo()
{
	/*
	_occupiedRegions[selfPlayer].clear();
	_occupiedRegions[enemyPlayer].clear();
	_occupiedBaseLocations[selfPlayer].clear();
	_occupiedBaseLocations[enemyPlayer].clear();

	// enemy 의 startLocation을 아직 모르는 경우
	if (_mainBaseLocations[enemyPlayer] == nullptr) {

		// how many start locations have we explored
		int exploredStartLocations = 0;
		bool enemyStartLocationFound = false;

		// an unexplored base location holder
		BWTA::BaseLocation * unexplored = nullptr;

		for (BWTA::BaseLocation * startLocation : BWTA::getStartLocations())
		{
			if (existsPlayerBuildingInRegion(BWTA::getRegion(startLocation->getTilePosition()), enemyPlayer))
			{
				if (enemyStartLocationFound == false) {
					enemyStartLocationFound = true;
					_mainBaseLocations[enemyPlayer] = startLocation;
					_mainBaseLocationChanged[enemyPlayer] = true;
				}
			}

			// if it's explored, increment
			if (BWAPI::Broodwar->isExplored(startLocation->getTilePosition()))
			{
				exploredStartLocations++;

			}
			// otherwise set it as unexplored base
			else
			{
				unexplored = startLocation;
			}
		}

		// if we've explored every start location except one, it's the enemy
		if (!enemyStartLocationFound && exploredStartLocations == ((int)BWTA::getStartLocations().size() - 1))
		{
			enemyStartLocationFound = true;
			_mainBaseLocations[enemyPlayer] = unexplored;
			_mainBaseLocationChanged[enemyPlayer] = true;
			_occupiedBaseLocations[enemyPlayer].push_back(unexplored);
		}
	}

	// update occupied base location
	// 어떤 Base Location 에는 아군 건물, 적군 건물 모두 혼재해있어서 동시에 여러 Player 가 Occupy 하고 있는 것으로 판정될 수 있다
	for (BWTA::BaseLocation * baseLocation : BWTA::getBaseLocations())
	{
		if (hasBuildingAroundBaseLocation(baseLocation, enemyPlayer))
		{
			_occupiedBaseLocations[enemyPlayer].push_back(baseLocation);
		}

		if (hasBuildingAroundBaseLocation(baseLocation, selfPlayer))
		{
			_occupiedBaseLocations[selfPlayer].push_back(baseLocation);
		}
	}

	// enemy의 mainBaseLocations을 발견한 후, 그곳에 있는 건물을 모두 파괴한 경우 _occupiedBaseLocations 중에서 _mainBaseLocations 를 선정한다
	if (_mainBaseLocations[enemyPlayer] != nullptr) {

		// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
		// 적 MainBaseLocation 업데이트 로직 버그 수정

		// 적군의 빠른 앞마당 건물 건설 + 아군의 가장 마지막 정찰 방문의 경우, 
		// enemy의 mainBaseLocations를 방문안한 상태에서는 건물이 하나도 없다고 판단하여 mainBaseLocation 을 변경하는 현상이 발생해서
		// enemy의 mainBaseLocations을 실제 방문했었던 적이 한번은 있어야 한다라는 조건 추가.  
		if (BWAPI::Broodwar->isExplored(_mainBaseLocations[enemyPlayer]->getTilePosition())) {

			if (existsPlayerBuildingInRegion(BWTA::getRegion(_mainBaseLocations[enemyPlayer]->getTilePosition()), enemyPlayer) == false)
			{
				for (std::list<BWTA::BaseLocation*>::const_iterator iterator = _occupiedBaseLocations[enemyPlayer].begin(), end = _occupiedBaseLocations[enemyPlayer].end(); iterator != end; ++iterator) {
					if (existsPlayerBuildingInRegion(BWTA::getRegion((*iterator)->getTilePosition()), enemyPlayer) == true) {
						_mainBaseLocations[enemyPlayer] = *iterator;
						_mainBaseLocationChanged[enemyPlayer] = true;
						std::cout << "_mainBaseLocations[enemyPlayer] changed by destruction as " << _mainBaseLocations[enemyPlayer]->getTilePosition().x << "," << _mainBaseLocations[enemyPlayer]->getTilePosition().y << std::endl;
						break;
					}
				}
			}
		}

		// BasicBot 1.1 Patch End //////////////////////////////////////////////////
	}

	// self의 mainBaseLocations에 대해, 그곳에 있는 건물이 모두 파괴된 경우 _occupiedBaseLocations 중에서 _mainBaseLocations 를 선정한다
	if (_mainBaseLocations[selfPlayer] != nullptr) {
		if (existsPlayerBuildingInRegion(BWTA::getRegion(_mainBaseLocations[selfPlayer]->getTilePosition()), selfPlayer) == false)
		{
			for (std::list<BWTA::BaseLocation*>::const_iterator iterator = _occupiedBaseLocations[selfPlayer].begin(), end = _occupiedBaseLocations[selfPlayer].end(); iterator != end; ++iterator) {
				if (existsPlayerBuildingInRegion(BWTA::getRegion((*iterator)->getTilePosition()), selfPlayer) == true) {
					_mainBaseLocations[selfPlayer] = *iterator;
					_mainBaseLocationChanged[selfPlayer] = true;
					break;
				}
			}
		}
	}

	// for each enemy building unit we know about
	updateOccupiedRegions(enemyPlayer);
	updateOccupiedRegions(selfPlayer);
	updateChokePointAndExpansionLocation();

	*/
}

void InformationManager::updateChokePointAndExpansionLocation()
{
	/*
	if (_mainBaseLocationChanged[selfPlayer] == true) {

		if (_mainBaseLocations[selfPlayer]) {

			BWTA::BaseLocation* sourceBaseLocation = _mainBaseLocations[selfPlayer];

			_firstChokePoint[selfPlayer] = BWTA::getNearestChokepoint(sourceBaseLocation->getTilePosition());

			double tempDistance;
			double closestDistance = 1000000000;
			for (BWTA::BaseLocation * targetBaseLocation : BWTA::getBaseLocations())
			{
				if (targetBaseLocation == _mainBaseLocations[selfPlayer]) continue;

				tempDistance = BWTA::getGroundDistance(sourceBaseLocation->getTilePosition(), targetBaseLocation->getTilePosition());
				if (tempDistance < closestDistance && tempDistance > 0) {
					closestDistance = tempDistance;
					_firstExpansionLocation[selfPlayer] = targetBaseLocation;
				}
			}

			closestDistance = 1000000000;
			for (BWTA::Chokepoint * chokepoint : BWTA::getChokepoints())
			{
				if (chokepoint == _firstChokePoint[selfPlayer]) continue;

				tempDistance = BWTA::getGroundDistance(sourceBaseLocation->getTilePosition(), BWAPI::TilePosition(chokepoint->getCenter()));
				if (tempDistance < closestDistance && tempDistance > 0) {
					closestDistance = tempDistance;
					_secondChokePoint[selfPlayer] = chokepoint;
				}
			}
		}
		_mainBaseLocationChanged[selfPlayer] = false;
	}

	if (_mainBaseLocationChanged[enemyPlayer] == true) {
		if (_mainBaseLocations[enemyPlayer]) {

			BWTA::BaseLocation* sourceBaseLocation = _mainBaseLocations[enemyPlayer];

			_firstChokePoint[enemyPlayer] = BWTA::getNearestChokepoint(sourceBaseLocation->getTilePosition());

			double tempDistance;
			double closestDistance = 1000000000;
			for (BWTA::BaseLocation * targetBaseLocation : BWTA::getBaseLocations())
			{
				if (targetBaseLocation == _mainBaseLocations[enemyPlayer]) continue;

				tempDistance = BWTA::getGroundDistance(sourceBaseLocation->getTilePosition(), targetBaseLocation->getTilePosition());
				if (tempDistance < closestDistance && tempDistance > 0) {
					closestDistance = tempDistance;
					_firstExpansionLocation[enemyPlayer] = targetBaseLocation;
				}
			}

			closestDistance = 1000000000;
			for (BWTA::Chokepoint * chokepoint : BWTA::getChokepoints())
			{
				if (chokepoint == _firstChokePoint[enemyPlayer]) continue;

				tempDistance = BWTA::getGroundDistance(sourceBaseLocation->getTilePosition(), BWAPI::TilePosition(chokepoint->getCenter()));
				if (tempDistance < closestDistance && tempDistance > 0) {
					closestDistance = tempDistance;
					_secondChokePoint[enemyPlayer] = chokepoint;
				}
			}
		}
		_mainBaseLocationChanged[enemyPlayer] = false;
	}
	*/
}


void InformationManager::updateOccupiedRegions(BWAPI::Player player)
{
	std::map<BWAPI::Unit, UnitInfo*> ::iterator iter;
	std::map<BWAPI::Unit, UnitInfo*> & allBuilding = _unitData[player].getAllBuildings();

	for (iter = allBuilding.begin(); iter != allBuilding.end(); iter++)
	{
		UnitInfo * ui = iter->second;

		BWTA::Region* region = BWTA::getRegion(BWAPI::TilePosition(ui->Pos()));
		if (region)
		{
			_occupiedRegions[player].insert(region);
		}
	}
}

// BaseLocation 주위 원 안에 player의 건물이 있으면 true 를 반환한다
bool InformationManager::hasBuildingAroundBaseLocation(BWTA::BaseLocation * baseLocation, BWAPI::Player player, int radius)
{
	// invalid regions aren't considered the same, but they will both be null
	if (!baseLocation)
	{
		return false;
	}

	std::map<BWAPI::Unit, UnitInfo*> ::iterator iter;
	std::map<BWAPI::Unit, UnitInfo*> & allBuilding = _unitData[player].getAllBuildings();

	for (iter = allBuilding.begin(); iter != allBuilding.end(); iter++)
	{
		UnitInfo * ui = iter->second;

		BWAPI::TilePosition buildingPosition(ui->Pos());

		// 직선거리는 가깝지만 다른 Region 에 있는 건물인 경우 무시한다 
		if (BWTA::getRegion(buildingPosition) != BWTA::getRegion(baseLocation->getTilePosition())) continue;
		// BasicBot 1.2 Patch End //////////////////////////////////////////////////

		if (buildingPosition.x >= baseLocation->getTilePosition().x - radius && buildingPosition.x <= baseLocation->getTilePosition().x + radius
			&& buildingPosition.y >= baseLocation->getTilePosition().y - radius && buildingPosition.y <= baseLocation->getTilePosition().y + radius)
		{
			return true;
		}
	}
	return false;
}

bool InformationManager::existsPlayerBuildingInRegion(BWTA::Region * region, BWAPI::Player player)
{
	// invalid regions aren't considered the same, but they will both be null
	if (region == nullptr || player == nullptr)
	{
		return false;
	}

	std::map<BWAPI::Unit, UnitInfo*> ::iterator iter;
	std::map<BWAPI::Unit, UnitInfo*> & allBuilding = _unitData[player].getAllBuildings();

	for (iter = allBuilding.begin(); iter != allBuilding.end(); iter++)
	{
		UnitInfo * ui = iter->second;

		if (BWTA::getRegion(BWAPI::TilePosition(ui->Pos())) == region)
		{
			return true;
		}
	}

	return false;
}

// BasicBot 1.1 Patch End //////////////////////////////////////////////////

std::set<BWTA::Region *> & InformationManager::getOccupiedRegions(BWAPI::Player player)
{
	return _occupiedRegions[player];
}

std::list<BWTA::BaseLocation *> & InformationManager::getOccupiedBaseLocations(BWAPI::Player player)
{
	return _occupiedBaseLocations[player];
}

BWTA::BaseLocation * InformationManager::getMainBaseLocation(BWAPI::Player player)
{
	return _mainBaseLocations[player];
}

BWTA::Chokepoint * InformationManager::getFirstChokePoint(BWAPI::Player player)
{
	return _firstChokePoint[player];
}
BWTA::BaseLocation * InformationManager::getFirstExpansionLocation(BWAPI::Player player)
{
	return _firstExpansionLocation[player];
}

BWTA::Chokepoint * InformationManager::getSecondChokePoint(BWAPI::Player player)
{
	return _secondChokePoint[player];
}


