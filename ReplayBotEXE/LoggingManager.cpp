#include "LoggingManager.h"
#include <io.h>
#include "BWEM/src/bwem.h"      // update the path if necessary

using namespace MyBot;

LoggingManager & LoggingManager::Instance()
{
	static LoggingManager instance;
	return instance;
}

LoggingManager::LoggingManager()
	:gameFrameCount(0)
	, firstPlayerOutcome("D")
	, secondPlayerOutcome("D")
{
}


void LoggingManager::onStart()
{
	// 리플레이 로그 파일 이름 : ReplayBot_숫자.csv

	long long startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	//데이터의 출처에 따라 hard cording
	fileOwner = "CHS";
	LogFilename = Config::BotInfo::BotName + "_" + to_string(startTime) + ".csv";
	logfilePath = "bwapi-data\\write\\";
	LogFileFullPath = logfilePath + LogFilename;
	if (InformationManager::Instance().activePlayers.size() != 2){
		return;
	}
	replayDat.open(LogFileFullPath.c_str());

	// 리플레이 로그 파일 첫번째 줄 : [StartGame]
	replayDat << "[StartGame]\n";

	// 리플레이 로그 파일 두번째 줄 : 맵파일이름(리플레이파일이름), 맵이름, 맵 최대플레이어수, 플레이어수
	replayDat << "MapFileName, MapName, MapPlayerLimit, activePlayers" << '\n';
	replayDat << InformationManager::Instance().getMapFileName()
		<< ", " << InformationManager::Instance().getMapName()
		<< ", " << InformationManager::Instance().getMapPlayerLimit()
		<< ", " << InformationManager::Instance().activePlayers.size()
		<< "\n";

	// 리플레이 로그 파일 세번째 줄 ~ : 플레이어 ID, 플레이어 이름, 종족, StartLocation X좌표, StartLocation Y좌표
	replayDat << "플레이어 ID, 플레이어 이름, 종족, StartLocation X좌표, StartLocation Y좌표" << '\n';
	for (const auto& p : InformationManager::Instance().activePlayers) {
		replayDat << p->getID()
			<< ", " << p->getName()
			<< ", " << p->getRace().getName()
			<< ", " << p->getStartLocation().x
			<< ", " << p->getStartLocation().y
			<< '\n';
	}
}

void LoggingManager::onEnd(bool isWinner)
{	


	//리플레이 로그파일 validation for delete
	boolean isValidation = true;
	if (firstPlayerOutcome == secondPlayerOutcome){ //승패가 예측 되지 않은 경우 리플레이 파일을 만들지 않는다.
		isValidation = false;
	}
	else if (gameFrameCount < 20 * 60 * 3){ //게임 시작 후 3분 이내 끝난 경기의 경우 리플레이 파일을 만들지 않는다.
		isValidation = false;
	}


	//리플레이 로그파일 rename
	string mapName = InformationManager::Instance().getMapName().c_str();
	string tempStr;
	const char* mapNameText = mapName.c_str();
	char numNpoint[] = "1234567890";
	for (unsigned int i = 0; i < strlen(mapNameText); i++)
	{
		if ((mapNameText[i] & 0x80) == 0x80 || isalpha(mapNameText[i]))
		{
			tempStr = tempStr + mapNameText[i];
		}
		else{
			for (unsigned int j = 0; j < strlen(numNpoint); ++j){
				if (numNpoint[j] == mapNameText[i]) tempStr = tempStr + mapNameText[i];
			}
		}

	}
	mapName = tempStr;
	string mapHashVal = BWAPI::Broodwar->mapHash();

	int mapPlayerLimit = BWAPI::Broodwar->getStartLocations().size();



	boolean isFirstPlyer = true;
	for (const auto& p : InformationManager::Instance().activePlayers){
		if (isFirstPlyer)
		{
			firstPlayerTribe = p->getRace().getName().substr(0, 1);
			firstPlayerStartingPointX = to_string(p->getStartLocation().x);
			firstPlayerStartingPointY = to_string(p->getStartLocation().y);
			isFirstPlyer = false;
		}
		else
		{
			secondPlayerTribe = p->getRace().getName().substr(0, 1);
			secondPlayerStartingPointX = to_string(p->getStartLocation().x);
			secondPlayerStartingPointY = to_string(p->getStartLocation().y);
		}
	}
	int renameResult, removeResult;
	string playDate = InformationManager::Instance().getMapFileName();
	playDate = playDate.substr(0, 8);
	//맵이름_맵해시값_가능플레이어수_1번플레이어종족_1번플레이어스타팅포인트X_1번플레이어스타팅포인트Y_1번플레이어승패_2번플레이어종족_2번플레이어스타팅포인트X_2번플레이어스타팅포인트Y_2번플레이어승패_총프레임카운트_경기날짜_출처
	std::string newName = mapName + "_" + mapHashVal + "_" + std::to_string(mapPlayerLimit) + "_" + firstPlayerTribe + "_" + firstPlayerStartingPointX + "_" + firstPlayerStartingPointY + "_" + firstPlayerOutcome + "_"
		+ secondPlayerTribe + "_" + secondPlayerStartingPointX + "_" + secondPlayerStartingPointY + "_" + secondPlayerOutcome + "_" + std::to_string(gameFrameCount) + "_" + fileOwner + ".csv";
	std::string newFileName = logfilePath + newName;

	const char* newLogfileName = newFileName.c_str();
	const char* oldLogfileName = LogFileFullPath.c_str();

	const char* removeFilePath = newLogfileName;
	
	// 리플레이 로그 파일 마지막 줄 : [EndGame]
	replayDat << newFileName + '\n';
	replayDat << "[EndGame]\n";
	replayDat.flush();
	replayDat.close();
	//이미 parsing된 파일이 있으면 지우고 다시 생성
	boolean isSamePaserData = true;
	_finddata_t fpd;
	long dataHandle;
	int resultHandle;
	string paserDIRpath = logfilePath + "*.*";
	dataHandle = _findfirst(paserDIRpath.c_str(), &fpd);
	while (resultHandle != -1){
		string str = fpd.name;
		if (str == newName) isSamePaserData = false;
		resultHandle = _findnext(dataHandle, &fpd);
	}
	if (isSamePaserData) remove(newLogfileName);
	renameResult = rename(oldLogfileName, newLogfileName);
	if (isValidation || renameResult != 0)
	{
		//removeResult = remove(removeFilePath);
	}

	//맵 정보 가져오기
	boolean isNewMap = true;
	_finddata_t fd;
	long handle;
	int result = 1;
	string DIRpath = logfilePath + "mapInfo\\*.*";
	handle = _findfirst(DIRpath.c_str(), &fd);  //현재 폴더 내 모든 파일을 찾는다.
	while (result != -1)
	{
		string str = fd.name;
		str = str.substr(0, str.size() - 4);
		if (str == mapHashVal) isNewMap = false;
		result = _findnext(handle, &fd);
	}
	_findclose(handle);
	if (isNewMap){
		makeMapData(mapHashVal);
	}
}

void LoggingManager::makeMapData(string mapHashVal)
{
	mapInfoFileName = mapHashVal;
	mapInfoFileFullPath = logfilePath + "mapInfo\\" + mapInfoFileName + ".csv";
	mapDat.open(mapInfoFileFullPath.c_str());

	// load map name
	mapFileName = BWAPI::Broodwar->mapFileName();

	// load map info
	mapWidthTileRes = BWAPI::Broodwar->mapWidth();
	mapWidthPixelRes = mapWidthTileRes * TILE_SIZE;
	mapWidthWalkRes = mapWidthTileRes * 4;

	mapHeightTileRes = BWAPI::Broodwar->mapHeight();
	mapHeightPixelRes = mapHeightTileRes * TILE_SIZE;
	mapHeightWalkRes = mapHeightTileRes * 4;
	// 맵 정보 생성 파일 첫번째 줄 : [Start]
	mapDat << "[Start]\n";
	mapDat << "지나갈 수 있는 곳은 1 아닌 곳은 0\n";
	mapDat << "mapWidthWalkRes : " << mapWidthWalkRes << ", " << "mapHeightWalkRes : " << mapHeightWalkRes << "\n";
	for (int i = 0; i < mapHeightWalkRes; i++){
		for (int j = 0; j < mapWidthWalkRes; j++){
			BWAPI::WalkPosition walkPosition;
			walkPosition.x = j;
			walkPosition.y = i;
			int info;
			if (BWAPI::Broodwar->isWalkable(walkPosition)) info = 1;
			else info = 0;
			if (j != 0) mapDat << " ";
			mapDat << info;
		}
		mapDat << "\n";
	}
	mapDat << "\n";
	///     - 0: Low ground
	///     - 1: Low ground doodad
	///     - 2: High ground
	///     - 3: High ground doodad
	///     - 4: Very high ground
	///     - 5: Very high ground doodad
	mapDat << "높이에 따라 숫자(0: Low ground, 1: Low ground doodad, 2: High ground, 3: High ground doodad, 4: Very high ground, 5: Very high ground doodad\n";
	mapDat << "mapWidthWalkRes : " << mapWidthWalkRes << ", " << "mapHeightWalkRes : " << mapHeightWalkRes << "\n";
	for (int i = 0; i < mapHeightWalkRes; i++){
		for (int j = 0; j < mapWidthWalkRes; j++){
			BWAPI::TilePosition tilePosition;
			tilePosition.x = j / 4;
			tilePosition.y = i / 4;
			int info = BWAPI::Broodwar->getGroundHeight(tilePosition);
			if (j != 0) mapDat << " ";
			mapDat << info;
		}
		mapDat << "\n";
	}
	//map 미네랄 정보
	mapDat << "미네랄 : 1 가스 : 2 중립 건물 : 3 \n";
	mapDat << "mapWidthWalkRes : " << mapWidthWalkRes << ", " << "mapHeightWalkRes : " << mapHeightWalkRes << '\n';
	for (int i = 0; i < mapHeightWalkRes; i++){
		for (int j = 0; j < mapWidthWalkRes; j++){
			int info = mireralInfoWalkArray[j][i];
			if (j != 0) mapDat << " ";
			mapDat << info;
		}
		mapDat << "\n";
	}
	// 맵 정보 생성 파일 마지막 줄 : [End]
	mapDat << "[End]\n";
	mapDat.flush();
	mapDat.close();
}

void LoggingManager::update()
{
	//getMineralInfo();

	saveSupplyLog();

	saveUnitsLog();

	saveGameResult();
}

void LoggingManager::onUnitCreate(BWAPI::Unit unit)
{
	saveUnitCreate(unit);
}

void LoggingManager::onUnitMorph(BWAPI::Unit unit)
{
	saveUnitMorph(unit);
}

void LoggingManager::saveSupplyLog()
{
	// 10 프레임 (0.5초) 마다 1번씩 로그를 기록합니다
	if (BWAPI::Broodwar->getFrameCount() % 10 != 0) {
		return;
	}
	replayDat << "ID, FrameCount, supplyUsed, supplyTotal" << '\n';
	for (const auto& p : InformationManager::Instance().activePlayers) {
		replayDat << p->getID() 
			<< ", " << BWAPI::Broodwar->getFrameCount() 
			<< ", " << p->supplyUsed() 
			<< ", " << p->supplyTotal() 
			<< '\n';
	}
}

void LoggingManager::saveUnitsLog()
{
	// 10 프레임 (0.5초) 마다 1번씩 로그를 기록합니다
	if (BWAPI::Broodwar->getFrameCount() % 10 != 0) {
		return;
	}
	int CompletedUnitSize = 0;
	for (const auto& p : InformationManager::Instance().activePlayers){
		for (auto & unit : p->getUnits())
		{
			if (unit != nullptr && unit->isCompleted()) CompletedUnitSize++;
		}
	}
	replayDat << "start-activeList" << '\n';
	replayDat << "Frame Count : " << BWAPI::Broodwar->getFrameCount() << '\n';
	replayDat << "size : " << CompletedUnitSize << '\n';
	replayDat << "getID()player아이디, getFrameCount(), getType().getName(), getID() 유닛ID, getHitPoints(), getPosition().x, getPosition().y, isAttacking(), isUnderAttack(), exists(), getAcidSporeCount(), getAddon() // Unit type, getAirWeaponCooldown(), getAngle(), getBottom(), getBuildType() // Unit type, getBuildUnit() // Unit, getCarrier(), getDefenseMatrixPoints(), getDefenseMatrixTimer(), getEnergy(), getEnsnareTimer(), getGroundWeaponCooldown(), getHatchery() // Unit, getHitPoints(), getID(), getInitialHitPoints(), getInitialPosition().x, getInitialPosition().y, getInitialResources(), getInitialTilePosition().x, getInitialTilePosition().y, getInitialType() // Unit type, getInterceptorCount(), getIrradiateTimer(), getKillCount(), getLastAttackingPlayer() //BWAPI::Player, getLastCommandFrame(), getLeft(), getLockdownTimer(), getMaelstromTimer(), getNydusExit() //Unit, getOrder() //Order, getOrderTarget() // Unit, getOrderTargetPosition().x, getOrderTargetPosition().y, getOrderTimer(), getPlagueTimer(), getPlayer() // Player, getPowerUp() //Unit, getRallyPosition().x, getRallyPosition().y, getRallyUnit() //Unit, getRegion() //BWAPI::region, getRemainingBuildTime(), getRemainingResearchTime(), getRemainingTrainTime(), getRemainingUpgradeTime(), getRemoveTimer(), getReplayID(), getResourceGroup(), getResources(), getRight(), getScarabCount(), getSecondaryOrder() //Order, getShields(), getSpaceRemaining(), getSpellCooldown(), getSpiderMineCount(), getStasisTimer(), getStimTimer(), getTarget() //Unit, getTargetPosition().x, getTargetPosition().y, getTech() //TechType, getTilePosition().x, getTilePosition().y, getTop(), getTransport() //Unit, getType() //UnitType, getUpgrade() //UpgradeType, getVelocityX(), getVelocityY(), hasNuke(), hasPath(targetUnit), isAccelerating(), isAttackFrame(), isAttacking(), isBeingConstructed(), isBeingGathered(), isBeingHealed(), isBlind(), isBraking(), isBurrowed(), isCarryingGas(), isCarryingMinerals(), isCloaked(), isCompleted(), isConstructing(), isDefenseMatrixed(), isDetected(), isEnsnared(), isFlying(), isFollowing(), isGatheringGas(), isGatheringMinerals(), isHallucination(), isHoldingPosition(), isIdle(), isInterruptible(), isInvincible(), isInWeaponRange(targetUnit), isIrradiated(), isLifted(), isLoaded(), isLockedDown(), isMaelstrommed(), isMorphing(), isMoving(), isParasited(), isPatrolling(), isPlagued(), isPowered(), isRepairing(), isResearching(), isSelected(), isSieged(), isStartingAttack(), isStasised(), isStimmed(), isStuck(), isTargetable(), isTraining(), isUnderAttack(), isUnderDarkSwarm(), isUnderDisruptionWeb(), isUnderStorm(), isUpgrading(), isVisible() // is visible in enemy player, buildAddon(unitType), morph(unitType), research(techType), upgrade(upgradeType), setRallyPoint(targetUnit), burrow(), unburrow(), cloak(), decloak(), siege(), unsiege(), lift(), unload(targetUnit), haltConstruction(), cancelConstruction(), cancelAddon(), cancelMorph(), cancelResearch(), cancelUpgrade(), useTech(techType, targetPosition), canCommand()" << '\n';
	BWAPI::Player p1 = nullptr;
	BWAPI::Player p2 = nullptr;
	if (gameFrameCount < BWAPI::Broodwar->getFrameCount()) gameFrameCount = BWAPI::Broodwar->getFrameCount(); //게임의 총 프레임 카운트를 저장한다.
	for (const auto& p : InformationManager::Instance().activePlayers){
		for (auto & unit : p->getUnits()){
			if (unit != nullptr && unit->isCompleted())
			{
				if (p1 == nullptr && p->getID() == 0) p1 = unit->getPlayer();
				else if (p1 != nullptr && p2 == nullptr && p->getID() == 1) p2 = unit->getPlayer();
				else if (p1 != nullptr && p2 != nullptr) break;
			}
		}
	}
	int isAttackingSum = 0;
	int isUnderAttackSum = 0;
	BWAPI::Player enemyPlayer;
	for (const auto& p : InformationManager::Instance().activePlayers) {
		
		for (auto & unit : p->getUnits())
		{
			if (unit->getPlayer() == p2){
				enemyPlayer = p1;
			}
			else{
				enemyPlayer = p2;
			}
			BWAPI::Unit targetUnit = unit->getTarget();
			BWAPI::Position targetPosition = unit->getTargetPosition();
			BWAPI::UnitType unitType = unit->getType();
			BWAPI::TechType techType = unit->getTech();
			BWAPI::UpgradeType upgradeType = unit->getUpgrade();
			if (unit->isAttacking()) isAttackingSum++;
			if (unit->isUnderAttack()) isUnderAttackSum++;
			if (unit != nullptr && unit->isCompleted())
			{
				replayDat << p->getID()
					<< ", " << BWAPI::Broodwar->getFrameCount()
					<< ", " << unit->getType().getName()
					<< ", " << unit->getID()
					<< ", " << unit->getHitPoints()
					<< ", " << unit->getPosition().x
					<< ", " << unit->getPosition().y
					<< ", " << unit->isAttacking()
					<< ", " << unit->isUnderAttack()
					<< ", " << unit->exists()
					<< ", " << unit->getAcidSporeCount()
					<< ", " << unit->getAddon() // Unit type
					<< ", " << unit->getAirWeaponCooldown()
					<< ", " << unit->getAngle()
					<< ", " << unit->getBottom()
					<< ", " << unit->getBuildType() // Unit type
					<< ", " << unit->getBuildUnit() // Unit
					<< ", " << unit->getCarrier()
					//<< ", " << unit->getClosestUnit(const UnitFilter &pred = nullptr, int radius = 999999)
					<< ", " << unit->getDefenseMatrixPoints()
					<< ", " << unit->getDefenseMatrixTimer()
					//<< ", " << unit->getDistance(Position target)
					//<< ", " << unit->getDistance(Unit target)
					<< ", " << unit->getEnergy()
					<< ", " << unit->getEnsnareTimer()
					<< ", " << unit->getGroundWeaponCooldown()
					<< ", " << unit->getHatchery() // Unit
					<< ", " << unit->getHitPoints()
					<< ", " << unit->getID()
					<< ", " << unit->getInitialHitPoints()
					<< ", " << unit->getInitialPosition().x
					<< ", " << unit->getInitialPosition().y
					<< ", " << unit->getInitialResources()
					<< ", " << unit->getInitialTilePosition().x
					<< ", " << unit->getInitialTilePosition().y
					<< ", " << unit->getInitialType() // Unit type
					<< ", " << unit->getInterceptorCount()
					//<< ", " << unit->getInterceptors() // Unitset
					<< ", " << unit->getIrradiateTimer()
					<< ", " << unit->getKillCount()
					//<< ", " << unit->getLarva() // Unitset
					<< ", " << unit->getLastAttackingPlayer() //BWAPI::Player
					//<< ", " << unit->getLastCommand()// BWAPI::UnitCommand
					<< ", " << unit->getLastCommandFrame()
					<< ", " << unit->getLeft()
					//<< ", " << unit->getLoadedUnits() // Unitset
					<< ", " << unit->getLockdownTimer()
					<< ", " << unit->getMaelstromTimer()
					<< ", " << unit->getNydusExit() //Unit
					<< ", " << unit->getOrder() //Order
					<< ", " << unit->getOrderTarget() // Unit
					<< ", " << unit->getOrderTargetPosition().x
					<< ", " << unit->getOrderTargetPosition().y
					<< ", " << unit->getOrderTimer()
					<< ", " << unit->getPlagueTimer()
					<< ", " << unit->getPlayer() // Player
					<< ", " << unit->getPowerUp() //Unit
					<< ", " << unit->getRallyPosition().x
					<< ", " << unit->getRallyPosition().y
					<< ", " << unit->getRallyUnit() //Unit
					<< ", " << unit->getRegion() //BWAPI::region
					<< ", " << unit->getRemainingBuildTime()
					<< ", " << unit->getRemainingResearchTime()
					<< ", " << unit->getRemainingTrainTime()
					<< ", " << unit->getRemainingUpgradeTime()
					<< ", " << unit->getRemoveTimer()
					<< ", " << unit->getReplayID()
					<< ", " << unit->getResourceGroup()
					<< ", " << unit->getResources()
					<< ", " << unit->getRight()
					<< ", " << unit->getScarabCount()
					<< ", " << unit->getSecondaryOrder() //Order
					<< ", " << unit->getShields()
					<< ", " << unit->getSpaceRemaining()
					<< ", " << unit->getSpellCooldown()
					<< ", " << unit->getSpiderMineCount()
					<< ", " << unit->getStasisTimer()
					<< ", " << unit->getStimTimer()
					<< ", " << unit->getTarget() //Unit
					<< ", " << unit->getTargetPosition().x
					<< ", " << unit->getTargetPosition().y
					<< ", " << unit->getTech() //TechType
					<< ", " << unit->getTilePosition().x
					<< ", " << unit->getTilePosition().y
					<< ", " << unit->getTop()
					//<< ", " << unit->getTrainingQueue() //UnitType
					<< ", " << unit->getTransport() //Unit
					<< ", " << unit->getType() //UnitType
					//<< ", " << unit->getUnitsInRadius(int radius, UnitFilter &pred = nullptr)
					//<< ", " << unit->getUnitsInWeaponRange(WeaponType weapon, UnitFilter &pred = nullptr)
					<< ", " << unit->getUpgrade() //UpgradeType
					<< ", " << unit->getVelocityX()
					<< ", " << unit->getVelocityY()
					<< ", " << unit->hasNuke()
					//<< ", " << unit->hasPath(Position target)
					<< ", " << unit->hasPath(targetUnit)
					<< ", " << unit->isAccelerating()
					<< ", " << unit->isAttackFrame()
					<< ", " << unit->isAttacking()
					<< ", " << unit->isBeingConstructed()
					<< ", " << unit->isBeingGathered()
					<< ", " << unit->isBeingHealed()
					<< ", " << unit->isBlind()
					<< ", " << unit->isBraking()
					<< ", " << unit->isBurrowed()
					<< ", " << unit->isCarryingGas()
					<< ", " << unit->isCarryingMinerals()
					<< ", " << unit->isCloaked()
					<< ", " << unit->isCompleted()
					<< ", " << unit->isConstructing()
					<< ", " << unit->isDefenseMatrixed()
					<< ", " << unit->isDetected()
					<< ", " << unit->isEnsnared()
					<< ", " << unit->isFlying()
					<< ", " << unit->isFollowing()
					<< ", " << unit->isGatheringGas()
					<< ", " << unit->isGatheringMinerals()
					<< ", " << unit->isHallucination()
					<< ", " << unit->isHoldingPosition()
					<< ", " << unit->isIdle()
					<< ", " << unit->isInterruptible()
					<< ", " << unit->isInvincible()
					<< ", " << unit->isInWeaponRange(targetUnit)
					<< ", " << unit->isIrradiated()
					<< ", " << unit->isLifted()
					<< ", " << unit->isLoaded()
					<< ", " << unit->isLockedDown()
					<< ", " << unit->isMaelstrommed()
					<< ", " << unit->isMorphing()
					<< ", " << unit->isMoving()
					<< ", " << unit->isParasited()
					<< ", " << unit->isPatrolling()
					<< ", " << unit->isPlagued()
					<< ", " << unit->isPowered()
					<< ", " << unit->isRepairing()
					<< ", " << unit->isResearching()
					<< ", " << unit->isSelected()
					<< ", " << unit->isSieged()
					<< ", " << unit->isStartingAttack()
					<< ", " << unit->isStasised()
					<< ", " << unit->isStimmed()
					<< ", " << unit->isStuck()
					<< ", " << unit->isTargetable()
					<< ", " << unit->isTraining()
					<< ", " << unit->isUnderAttack()
					<< ", " << unit->isUnderDarkSwarm()
					<< ", " << unit->isUnderDisruptionWeb()
					<< ", " << unit->isUnderStorm()
					<< ", " << unit->isUpgrading()
					<< ", " << unit->isVisible(enemyPlayer) // is visible in enemy player
					//Unit Commands
					//<< ", " << unit->issueCommand(UnitCommand command) = 0
					//<< ", " << unit->attack(Position target, bool shiftQueueCommand = false)
					//<< ", " << unit->attack(Unit target, bool shiftQueueCommand = false)
					//<< ", " << unit->build(UnitType type, TilePosition target = TilePositions::None)
					<< ", " << unit->buildAddon(unitType)
					//<< ", " << unit->train(UnitType type = UnitTypes::None)
					<< ", " << unit->morph(unitType)
					<< ", " << unit->research(techType)
					<< ", " << unit->upgrade(upgradeType)
					<< ", " << unit->setRallyPoint(targetUnit)
					//<< ", " << unit->move(Position target, bool shiftQueueCommand = false)
					//<< ", " << unit->patrol(Position target, bool shiftQueueCommand = false)
					//<< ", " << unit->holdPosition(bool shiftQueueCommand = false)
					//<< ", " << unit->stop(bool shiftQueueCommand = false)
					//<< ", " << unit->follow(Unit target, bool shiftQueueCommand = false)
					//<< ", " << unit->gather(Unit target, bool shiftQueueCommand = false)
					//<< ", " << unit->returnCargo(bool shiftQueueCommand = false)
					//<< ", " << unit->repair(Unit target, bool shiftQueueCommand = false)
					<< ", " << unit->burrow()
					<< ", " << unit->unburrow()
					<< ", " << unit->cloak()
					<< ", " << unit->decloak()
					<< ", " << unit->siege()
					<< ", " << unit->unsiege()
					<< ", " << unit->lift()
					///<< ", " << unit->land(targetTilePosition)
					//<< ", " << unit->load(Unit target, bool shiftQueueCommand = false)
					<< ", " << unit->unload(targetUnit)
					//<< ", " << unit->unloadAll(bool shiftQueueCommand = false)
					//<< ", " << unit->unloadAll(Position target, bool shiftQueueCommand = false)
					//<< ", " << unit->rightClick(Position target, bool shiftQueueCommand = false)
					//<< ", " << unit->rightClick(Unit target, bool shiftQueueCommand = false)
					<< ", " << unit->haltConstruction()
					<< ", " << unit->cancelConstruction()
					<< ", " << unit->cancelAddon()
					//<< ", " << unit->cancelTrain(int slot = -2)
					<< ", " << unit->cancelMorph()
					<< ", " << unit->cancelResearch()
					<< ", " << unit->cancelUpgrade()
					<< ", " << unit->useTech(techType, targetPosition)
					//<< ", " << unit->useTech(TechType tech, Unit target = nullptr)
					///<< ", " << unit->placeCOP(targetTilePosition)

					//Command Verifiers
					//<< ", " << unit->canIssueCommand(UnitCommand command, bool checkCanUseTechPositionOnPositions = true, bool checkCanUseTechUnitOnUnits = true, bool checkCanBuildUnitType = true, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					//<< ", " << unit->canIssueCommandGrouped(UnitCommand command, bool checkCanUseTechPositionOnPositions = true, bool checkCanUseTechUnitOnUnits = true, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibilityGrouped = true, bool checkCommandibility = true)
					<< ", " << unit->canCommand()
					//<< ", " << unit->canCommandGrouped(bool checkCommandibility = true)
					//<< ", " << unit->canIssueCommandType(UnitCommandType ct, bool checkCommandibility = true)
					//<< ", " << unit->canIssueCommandTypeGrouped(UnitCommandType ct, bool checkCommandibilityGrouped = true, bool checkCommandibility = true)
					//<< ", " << unit->canTargetUnit(Unit targetUnit, bool checkCommandibility = true)
					//<< ", " << unit->canAttack(bool checkCommandibility = true)
					//<< ", " << unit->canAttack(Position target, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					//<< ", " << unit->canAttack(Unit target, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					//<< ", " << unit->canAttackGrouped(bool checkCommandibilityGrouped = true, bool checkCommandibility = true)
					//<< ", " << unit->canAttackGrouped(Position target, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibilityGrouped = true, bool checkCommandibility = true)
					/*<< ", " << unit->canAttackGrouped(Unit target, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibilityGrouped = true, bool checkCommandibility = true)
					<< ", " << unit->canAttackMove(bool checkCommandibility = true)
					<< ", " << unit->canAttackMoveGrouped(bool checkCommandibilityGrouped = true, bool checkCommandibility = true)
					<< ", " << unit->canAttackUnit(bool checkCommandibility = true)
					<< ", " << unit->canAttackUnit(Unit targetUnit, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canAttackUnitGrouped(bool checkCommandibilityGrouped = true, bool checkCommandibility = true)
					<< ", " << unit->canAttackUnitGrouped(Unit targetUnit, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibilityGrouped = true, bool checkCommandibility = true)
					<< ", " << unit->canBuild(bool checkCommandibility = true)
					<< ", " << unit->canBuild(UnitType uType, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canBuild(UnitType uType, BWAPI::TilePosition tilePos, bool checkTargetUnitType = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canBuildAddon(bool checkCommandibility = true)
					<< ", " << unit->canBuildAddon(UnitType uType, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canTrain(bool checkCommandibility = true)
					<< ", " << unit->canTrain(UnitType uType, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canMorph(bool checkCommandibility = true)
					<< ", " << unit->canMorph(UnitType uType, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canResearch(bool checkCommandibility = true)
					<< ", " << unit->canResearch(TechType type, bool checkCanIssueCommandType = true)
					<< ", " << unit->canUpgrade(bool checkCommandibility = true)
					<< ", " << unit->canUpgrade(UpgradeType type, bool checkCanIssueCommandType = true)
					<< ", " << unit->canSetRallyPoint(bool checkCommandibility = true)
					<< ", " << unit->canSetRallyPoint(Position target, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canSetRallyPoint(Unit target, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canSetRallyPosition(bool checkCommandibility = true)
					<< ", " << unit->canSetRallyUnit(bool checkCommandibility = true)
					<< ", " << unit->canSetRallyUnit(Unit targetUnit, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canMove(bool checkCommandibility = true)
					<< ", " << unit->canMoveGrouped(bool checkCommandibilityGrouped = true, bool checkCommandibility = true)
					<< ", " << unit->canPatrol(bool checkCommandibility = true)
					<< ", " << unit->canPatrolGrouped(bool checkCommandibilityGrouped = true, bool checkCommandibility = true)
					<< ", " << unit->canFollow(bool checkCommandibility = true)
					<< ", " << unit->canFollow(Unit targetUnit, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canGather(bool checkCommandibility = true)
					<< ", " << unit->canGather(Unit targetUnit, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canReturnCargo(bool checkCommandibility = true)
					<< ", " << unit->canHoldPosition(bool checkCommandibility = true)
					<< ", " << unit->canStop(bool checkCommandibility = true)
					<< ", " << unit->canRepair(bool checkCommandibility = true)
					<< ", " << unit->canRepair(Unit targetUnit, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canBurrow(bool checkCommandibility = true)
					<< ", " << unit->canUnburrow(bool checkCommandibility = true)
					<< ", " << unit->canCloak(bool checkCommandibility = true)
					<< ", " << unit->canDecloak(bool checkCommandibility = true)
					<< ", " << unit->canSiege(bool checkCommandibility = true)
					<< ", " << unit->canUnsiege(bool checkCommandibility = true)
					<< ", " << unit->canLift(bool checkCommandibility = true)
					<< ", " << unit->canLand(bool checkCommandibility = true)
					<< ", " << unit->canLand(TilePosition target, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canLoad(bool checkCommandibility = true)
					<< ", " << unit->canLoad(Unit targetUnit, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canUnloadWithOrWithoutTarget(bool checkCommandibility = true)
					<< ", " << unit->canUnloadAtPosition(Position targDropPos, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canUnload(bool checkCommandibility = true)
					<< ", " << unit->canUnload(Unit targetUnit, bool checkCanTargetUnit = true, bool checkPosition = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canUnloadAll(bool checkCommandibility = true)
					<< ", " << unit->canUnloadAllPosition(bool checkCommandibility = true)
					<< ", " << unit->canUnloadAllPosition(Position targDropPos, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canRightClick(bool checkCommandibility = true)
					<< ", " << unit->canRightClick(Position target, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canRightClick(Unit target, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canRightClickGrouped(bool checkCommandibilityGrouped = true, bool checkCommandibility = true)
					<< ", " << unit->canRightClickGrouped(Position target, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibilityGrouped = true, bool checkCommandibility = true)
					<< ", " << unit->canRightClickGrouped(Unit target, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibilityGrouped = true, bool checkCommandibility = true)
					<< ", " << unit->canRightClickPosition(bool checkCommandibility = true)
					<< ", " << unit->canRightClickPositionGrouped(bool checkCommandibilityGrouped = true, bool checkCommandibility = true)
					<< ", " << unit->canRightClickUnit(bool checkCommandibility = true)
					<< ", " << unit->canRightClickUnit(Unit targetUnit, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canRightClickUnitGrouped(bool checkCommandibilityGrouped = true, bool checkCommandibility = true)
					<< ", " << unit->canRightClickUnitGrouped(Unit targetUnit, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibilityGrouped = true, bool checkCommandibility = true)
					<< ", " << unit->canHaltConstruction(bool checkCommandibility = true)
					<< ", " << unit->canCancelConstruction(bool checkCommandibility = true)
					<< ", " << unit->canCancelAddon(bool checkCommandibility = true)
					<< ", " << unit->canCancelTrain(bool checkCommandibility = true)
					<< ", " << unit->canCancelTrainSlot(bool checkCommandibility = true)
					<< ", " << unit->canCancelTrainSlot(int slot, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canCancelMorph(bool checkCommandibility = true)
					<< ", " << unit->canCancelResearch(bool checkCommandibility = true)
					<< ", " << unit->canCancelUpgrade(bool checkCommandibility = true)
					<< ", " << unit->canUseTechWithOrWithoutTarget(bool checkCommandibility = true)
					<< ", " << unit->canUseTechWithOrWithoutTarget(BWAPI::TechType tech, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canUseTech(BWAPI::TechType tech, Position target, bool checkCanTargetUnit = true, bool checkTargetsType = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canUseTech(BWAPI::TechType tech, Unit target = nullptr, bool checkCanTargetUnit = true, bool checkTargetsType = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canUseTechWithoutTarget(BWAPI::TechType tech, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canUseTechUnit(BWAPI::TechType tech, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canUseTechUnit(BWAPI::TechType tech, Unit targetUnit, bool checkCanTargetUnit = true, bool checkTargetsUnits = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canUseTechPosition(BWAPI::TechType tech, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canUseTechPosition(BWAPI::TechType tech, Position target, bool checkTargetsPositions = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					<< ", " << unit->canPlaceCOP(bool checkCommandibility = true)
					<< ", " << unit->canPlaceCOP(TilePosition target, bool checkCanIssueCommandType = true, bool checkCommandibility = true)
					*/
					<< '\n';
			}
		}
	}
	replayDat << "Sum(isAtacking) : " << isAttackingSum << '\n';
	replayDat << "Sum(isUnderAttack) : " << isUnderAttackSum << '\n';
	replayDat << "end-activeList" << '\n';
}

void LoggingManager::onMineralCheck(BWAPI::Unit unit){
	if (BWAPI::Broodwar->getFrameCount() < 10){
		//미네랄이면 1 가스면 2 중립 건물이면 3
		if (unit->getType().getName() == "Resource_Mineral_Field" || unit->getType().getName() == "Resource_Mineral_Field_Type_2" || unit->getType().getName() == "Resource_Mineral_Field_Type_3"){
			mireralInfoArray[unit->getPosition().x][unit->getPosition().y] = 1;
			mireralInfoWalkArray[unit->getPosition().x/8][unit->getPosition().y/8] = 1;
			mireralInfoTileArray[unit->getPosition().x/32][unit->getPosition().y/32] = 1;
		}
		else if (unit->getType().getName() == "Resource_Vespene_Geyser"){
			mireralInfoArray[unit->getPosition().x][unit->getPosition().y] = 2;
			mireralInfoWalkArray[unit->getPosition().x / 8][unit->getPosition().y / 8] = 2;
			mireralInfoTileArray[unit->getPosition().x / 32][unit->getPosition().y / 32] = 2;
		}
		else if (unit->getType().getName() == "Special_Crashed_Norad_II" || unit->getType().getName() == "Special_Ion_Cannon"
			|| unit->getType().getName() == "Special_Power_Generator" || unit->getType().getName() == "Special_Psi_Disrupter"
			|| unit->getType().getName() == "Special_Cerebrate" || unit->getType().getName() == "Special_Cerebrate_Daggoth"
			|| unit->getType().getName() == "Special_Mature_Chrysalis" || unit->getType().getName() == "Special_Overmind"
			|| unit->getType().getName() == "Special_Overmind_Cocoon" || unit->getType().getName() == "Special_Overmind_With_Shell"
			|| unit->getType().getName() == "Special_Khaydarin_Crystal_Form" || unit->getType().getName() == "Special_Protoss_Temple"
			|| unit->getType().getName() == "Special_Stasis_Cell_Prison" || unit->getType().getName() == "Special_Warp_Gate" || unit->getType().getName() == "Special_XelNaga_Temple"){
			mireralInfoArray[unit->getPosition().x][unit->getPosition().y] = 3;
			mireralInfoWalkArray[unit->getPosition().x / 8][unit->getPosition().y / 8] = 3;
			mireralInfoTileArray[unit->getPosition().x / 32][unit->getPosition().y / 32] = 3;
		}
	}
}
void LoggingManager::saveUnitCreate(BWAPI::Unit unit)
{
	replayDat << unit->getPlayer()->getID()
		<< ", " << BWAPI::Broodwar->getFrameCount()
		<< ", " << unit->getType().getName()
		<< ", " << "is created at"
		<< "," << unit->getPlayer()->supplyUsed()
		<< "," << unit->getPlayer()->supplyTotal()
		<< '\n';
}

void LoggingManager::saveUnitMorph(BWAPI::Unit unit)
{
	replayDat << unit->getPlayer()->getID()
		<< ", " << BWAPI::Broodwar->getFrameCount()
		<< ", " << unit->getType().getName()
		<< ", " << "is morphed to"
		<< "," << unit->getPlayer()->supplyUsed()
		<< "," << unit->getPlayer()->supplyTotal()
		<< '\n';
}

void LoggingManager::saveGameResult()
{
	// 게임 시작 후 3분 이내에 게임이 끝난 경우

	// 게임 도중 상대방이 Left 해서 내가 승리한 경우 -> isOnePlayerLeftGame 를 파악하면 정상적으로 처리 가능하지만, 진짜로 경기 내용에서 이겼는지는 확인 필요

	//		옵저버 플레이어들이 남아있는 경우

	//		내가 지고있는 상황인데, 상대방이 갑자기 나간 경우 (ex: 화장실 or 다른 약속 때문에 or 지루해서)  : 승리도 패배도 아닌, Draw 로 처리. 학습에서 제외시키기


	// 내가 Left 해서 갑자기 리플레이가 종료된 경우 -> 누가 패배 한 것인지, 확인이 필요하다

	// 비등비등한 상황, 심지어 숫자상으로는 이기고 있는 상황에서도, 
	// 유닛상성상 / 전략상성상 이기기 힘들다고 판단하여 먼저 Left 하는 경우가 있기 때문에 판단이 쉽지는 않다

	//		내 인구수 가 0 이 된 경우 : 확실한 패배

	//		내 인구수 가 상대방 인구수 보다 크게 적은 경우 : 패배

	//		내 건물수 가 상대방 건물수 보다 크게 적은 경우 : 패배

	//		내가 최근에 gg 혹은 GG 혹은 ㅎㅎ 혹은 ㅎ 혹은 ㅈㅈ 를 먼저 친 경우 : 패배

	//		테란 대 타종족인 경우 : 테란의 패배 (리플레이 데이터의 작성자가 테란 종족 유저 이기 때문)

	//	확인 필요한 경우 : 
	
	//		내 컴퓨터가 게임 도중 꺼진 경우

	//		나 혹은 상대방이 Draw 를 유도해서, 상대방이 먼저 나간 경우 : 


	// 두 명의 플레이어만 존재하는 게임인 경우
	if (InformationManager::Instance().activePlayers.size() == 2) {

		BWAPI::Player winner = nullptr;
		BWAPI::Player loser = nullptr;

		// 한 명의 플레이어가 먼저 나가서, 패자가 명확하게 식별이 되는 경우 (승자도 명확하게 식별이 가능)
		// 패자를 기록한 후, 바로 승자를 기록한다
		bool isOnePlayerLeftGame = false;
		
		for (const auto& p1 : InformationManager::Instance().activePlayers) {

			// isDefeated() 로는 체크가 되지 않기 때문에
			// leftGame() 으로 확인한다

			if (p1->leftGame()) {

				replayDat << p1->getID()
					<< ", " << p1->getName()
					<< ", " << p1->getRace().getName()
					<< ", " << "leftGame at " << BWAPI::Broodwar->getFrameCount()
					<< "\n";

				isOnePlayerLeftGame = true;

				loser = p1;
				printLoserPlayer(loser);
				
				for (const auto& p2 : InformationManager::Instance().activePlayers) {

					if (p2->leftGame() == false) {
						winner = p2;
						printWinnerPlayer(winner);
						break;
					}
				}
				break;
			}
		}

		// 두 명의 플레이어 중 아무도 나가지 않았는데, 게임이 종료된 경우 = 리플레이 녹화자가 패배한 경우인데, 이 때 누가 패배한 것인지를 명확히 알수는 없다
		if (isOnePlayerLeftGame == false) {

			// 리플레이 를 재생시키고 있는 경우
			// 마지막 프레임에서 판단 ?  마지막 근처 프레임에서 판단 ?
			if (InformationManager::Instance().replayTotalFrameCount > 0) {

				if (InformationManager::Instance().replayTotalFrameCount == BWAPI::Broodwar->getFrameCount() ) {
					
					BWAPI::Player p1Player = nullptr;
					BWAPI::Player p2Player = nullptr;
				
					BWAPI::Race p1Race, p2Race;
					int playerCount = 1;
					for (const auto& p : InformationManager::Instance().activePlayers) {
						if (playerCount == 1) {
							p1Player = p;
							p1Race = p->getRace();
						}
						else if (playerCount == 2) {
							p2Player = p;
							p2Race = p->getRace();
						}
						playerCount++;
					}

					bool hasFoundLoser = false;

					// 패배한 플레이어를 찾는다					
					for (const auto& p1 : InformationManager::Instance().activePlayers) {

						bool isDefeated = false;

						// 인구수 0가 되었으면 패배한 것이 명백
						if (p1->supplyUsed() == 0) {
							isDefeated = true;
							hasFoundLoser = true;

							replayDat << p1->getID()
								<< ", " << p1->getName()
								<< ", " << p1->getRace().getName()
								<< ", " << "supplyUsed 0"
								<< "\n";

							break;
						}

						if (isDefeated == false) {
							for (const auto& p2 : InformationManager::Instance().activePlayers) {

								if (p1 == p2) {
									continue;
								}
								else {
									// 다른 플레이어보다 인구수가 현격하게 적으면 패배
									// TODO : 패배 판단조건 추가 필요
									// TODO : 게임 후반부에 먼저 gg 를 친 사람이 패배
									if (p1->supplyUsed() < p2->supplyUsed() - 40) {
										isDefeated = true;
										hasFoundLoser = true;

										replayDat << p1->getID()
											<< ", " << p1->getName()
											<< ", " << p1->getRace().getName()
											<< ", " << "supplyUsed is " << p2->supplyUsed() - p1->supplyUsed() << " less than other player"
											<< "\n";

										break;
									}
								}
							}
						}
						
						// 패배한 플레이어를 찾았으면, 승리한 플레이어를 찾는다
						if (hasFoundLoser) {

							loser = p1;

							for (const auto& p2 : InformationManager::Instance().activePlayers) {

								if (p1 == p2) {
									continue;
								}
								else {
									winner = p2;
									break;
								}
							}

							break;
						}
					}


					if (hasFoundLoser == false) {

						// 테란 플레이어 vs 타 종족 플레이어 의 대결이었으면 테란 플레이어의 패배로 간주한다
						// TODO : 테란 vs 테란 의 대결인 경우 누구의 패배인지 파악이 되지 않는 문제가 있다 
						if (p1Race == BWAPI::Races::Terran && p2Race != BWAPI::Races::Terran) {
							loser = p1Player;
							winner = p2Player;
						}
						else if (p1Race != BWAPI::Races::Terran && p2Race == BWAPI::Races::Terran) {
							loser = p2Player;
							winner = p1Player;
						}
					}

					printLoserPlayer(loser);
					printWinnerPlayer(winner);

				}
			}
		}

		// 첫번째 플레이어, 두번째 플레이어가 이겼는지 졌는지 전역변수에 저장 -> 파일이름 rename 에 활용
		boolean isFirstPlyer = true;
		for (const auto& p : InformationManager::Instance().activePlayers) {

			if (p == winner) {
				firstPlayerOutcome = "W";
				secondPlayerOutcome = "L";
			}
			else if (p == loser) {
				firstPlayerOutcome = "L";
				secondPlayerOutcome = "W";
			}

			break;
		}
	}
}

void LoggingManager::printLoserPlayer(BWAPI::Player p)
{
	if (p == nullptr) {
		return;
	}

	replayDat << p->getID()
		<< ", " << p->getName()
		<< ", " << p->getRace().getName()
		<< ", " << "isDefeated"
		<< "\n";
}

void LoggingManager::printWinnerPlayer(BWAPI::Player p)
{
	if (p == nullptr) {
		return;
	}

	replayDat << p->getID()
		<< ", " << p->getName()
		<< ", " << p->getRace().getName()
		<< ", " << "isVictorious"
		<< "\n";
}
