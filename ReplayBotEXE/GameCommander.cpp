#include "GameCommander.h"

using namespace MyBot;

GameCommander::GameCommander(){
	isToFindError = false;
}
GameCommander::~GameCommander(){
}

void GameCommander::onStart() 
{
	LoggingManager::Instance().onStart();
}

void GameCommander::onEnd(bool isWinner)
{
	LoggingManager::Instance().onEnd(isWinner);
}

void GameCommander::onFrame()
{
	if (BWAPI::Broodwar->isPaused()) {
		return;
	}

	// 아군 베이스 위치. 적군 베이스 위치. 각 유닛들의 상태정보 등을 Map 자료구조에 저장/업데이트
	//InformationManager::Instance().update();
	
	// 각 유닛의 위치를 자체 MapGrid 자료구조에 저장
	//MapGrid::Instance().update();
		
	LoggingManager::Instance().update();

}

// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
// 일꾼 탄생/파괴 등에 대한 업데이트 로직 버그 수정 : onUnitShow 가 아니라 onUnitComplete 에서 처리하도록 수정
void GameCommander::onUnitShow(BWAPI::Unit unit)			
{
	// ResourceDepot 및 Worker 에 대한 처리
	//WorkerManager::Instance().onUnitShow(unit);
}

// BasicBot 1.1 Patch End //////////////////////////////////////////////////

void GameCommander::onUnitHide(BWAPI::Unit unit)			
{
	// 해당 unit은 id외에 조회가 안됨.

}

void GameCommander::onUnitCreate(BWAPI::Unit unit)
{ 
	//InformationManager::Instance().onUnitCreate(unit);

	//LoggingManager::Instance().onUnitCreate(unit);
}

// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
// 일꾼 탄생/파괴 등에 대한 업데이트 로직 버그 수정 : onUnitShow 가 아니라 onUnitComplete 에서 처리하도록 수정
void GameCommander::onUnitComplete(BWAPI::Unit unit)
{
	//InformationManager::Instance().onUnitComplete(unit);
}

// BasicBot 1.1 Patch End //////////////////////////////////////////////////

void GameCommander::onUnitDestroy(BWAPI::Unit unit)
{
	//InformationManager::Instance().onUnitDestroy(unit); 
}

void GameCommander::onUnitRenegade(BWAPI::Unit unit)
{
	// Vespene_Geyser (가스 광산) 에 누군가가 건설을 했을 경우
	//BWAPI::Broodwar->sendText("A %s [%p] has renegaded. It is now owned by %s", unit->getType().c_str(), unit, unit->getPlayer()->getName().c_str());

//	InformationManager::Instance().onUnitRenegade(unit);
}

void GameCommander::onUnitMorph(BWAPI::Unit unit)
{ 
//	InformationManager::Instance().onUnitMorph(unit);

	// Zerg 종족 Worker 의 Morph 에 대한 처리
//	WorkerManager::Instance().onUnitMorph(unit);

	//LoggingManager::Instance().onUnitMorph(unit);
}

void GameCommander::onUnitDiscover(BWAPI::Unit unit)
{
}

void GameCommander::onUnitEvade(BWAPI::Unit unit)
{
}

// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
// onNukeDetect, onPlayerLeft, onSaveGame 이벤트를 처리할 수 있도록 메소드 추가

void GameCommander::onNukeDetect(BWAPI::Position target)
{
}

void GameCommander::onPlayerLeft(BWAPI::Player player)
{
}

void GameCommander::onSaveGame(std::string gameName)
{
}

// BasicBot 1.1 Patch End //////////////////////////////////////////////////

void GameCommander::onSendText(std::string text)
{
}

void GameCommander::onReceiveText(BWAPI::Player player, std::string text)
{	
}

