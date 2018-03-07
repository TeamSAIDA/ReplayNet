#include "LoggingManager.h"

using namespace MyBot;

LoggingManager & LoggingManager::Instance()
{
	static LoggingManager instance;
	return instance;
}

LoggingManager::LoggingManager()
{
}

void LoggingManager::onStart()
{
	// 리플레이 로그 파일 이름 : ReplayBot_숫자.csv
	long long startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	LogFilename = Config::BotInfo::BotName + "_" + to_string(startTime) + ".csv";
	LogFileFullPath = "bwapi-data\\write\\" + LogFilename;

	replayDat.open(LogFileFullPath.c_str());

	// 리플레이 로그 파일 첫번째 줄 : [StartGame]
	replayDat << "[StartGame]\n";

	// 리플레이 로그 파일 두번째 줄 : 맵파일이름(리플레이파일이름), 맵이름, 맵 최대플레이어수, 플레이어수
	replayDat << InformationManager::Instance().getMapFileName() 
		<< ", " << InformationManager::Instance().getMapName()
		<< ", " << InformationManager::Instance().getMapPlayerLimit()
		<< ", " << InformationManager::Instance().activePlayers.size()
		<< "\n";

	// 리플레이 로그 파일 세번째 줄 ~ : 플레이어 ID, 플레이어 이름, 종족, StartLocation X좌표, StartLocation Y좌표
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
	// 리플레이 로그 파일 마지막 줄 : [EndGame]
	replayDat << "[EndGame]\n";
	replayDat.flush();
	replayDat.close();
}

void LoggingManager::update()
{
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
	// 100 프레임 (5초) 마다 1번씩 로그를 기록합니다
	if (BWAPI::Broodwar->getFrameCount() % 100 != 0) {
		return;
	}

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
	// 100 프레임 (5초) 마다 1번씩 로그를 기록합니다
	if (BWAPI::Broodwar->getFrameCount() % 100 != 0) {
		return;
	}


	for (const auto& p : InformationManager::Instance().activePlayers) {

		for (auto & unit : p->getUnits())
		{
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
					<< '\n';
			}
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
	int defeatedPlayerCount = 0;

	for (const auto& p : InformationManager::Instance().activePlayers) {

		if (p->isDefeated()) {
			defeatedPlayerCount++;
		}
	}

	if (InformationManager::Instance().activePlayers.size() - defeatedPlayerCount == 1) {

		for (const auto& p : InformationManager::Instance().activePlayers) {

			// 패배한 플레이어 기록
			if (p->isDefeated()) {
				replayDat << p->getID()
					<< ", " << p->getName()
					<< ", " << p->getRace().getName()
					<< ", " << "isDefeated"
					<< "\n";
			}
			// 승리한 플레이어 기록
			else {
				replayDat << p->getID()
					<< ", " << p->getName()
					<< ", " << p->getRace().getName()
					<< ", " << "isVictorious"
					<< "\n";
			}
		}
	}
}

