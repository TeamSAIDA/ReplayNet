/*
+----------------------------------------------------------------------+
| ReplayBot                                                            |
+----------------------------------------------------------------------+
| Samsung SDS - Xeed Lab - SAIDA                                       |
+----------------------------------------------------------------------+
|                                                                      |
+----------------------------------------------------------------------+
| Author: Tekseon Shin  <tekseon.shin@gmail.com>                       |
+----------------------------------------------------------------------+
*/

/*
+----------------------------------------------------------------------+
| UAlbertaBot                                                          |
+----------------------------------------------------------------------+
| University of Alberta - AIIDE StarCraft Competition                  |
+----------------------------------------------------------------------+
|                                                                      |
+----------------------------------------------------------------------+
| Author: David Churchill <dave.churchill@gmail.com>                   |
+----------------------------------------------------------------------+
*/

#include "MyBotModule.h"
#include "BWEM/src/examples.h"

using namespace BWAPI;
using namespace BWTA;
using namespace MyBot;

void MyBotModule::onStart(){

	time_t t;
	srand((unsigned int)(time(&t)));

	// Config 파일 관리가 번거롭고, 배포 및 사용시 Config 파일 위치를 지정해주는 것이 번거롭기 때문에, 
	// Config 를 파일로부터 읽어들이지 않고, Config 클래스의 값을 사용하도록 합니다.
	if (Config::BWAPIOptions::EnableCompleteMapInformation)
	{
		BWAPI::Broodwar->enableFlag(BWAPI::Flag::CompleteMapInformation);
	}

	if (Config::BWAPIOptions::EnableUserInput)
	{
		BWAPI::Broodwar->enableFlag(BWAPI::Flag::UserInput);
	}

	Broodwar->setCommandOptimizationLevel(1);

	try
	{
		Broodwar << "Map initialization..." << std::endl;

		//theMap.Initialize();
		//theMap.EnableAutomaticPathAnalysis();
		//bool startingLocationsOK = theMap.FindBasesForStartingLocations();
		//assert(startingLocationsOK);
	}
	catch (const std::exception & e)
	{
		Broodwar << "EXCEPTION: " << e.what() << std::endl;
	}

	// Speedups for automated play, sets the number of milliseconds bwapi spends in each frame
	// Fastest: 42 ms/frame.  1초에 24 frame. 일반적으로 1초에 24frame을 기준 게임속도로 합니다
	// Normal: 67 ms/frame. 1초에 15 frame
	// As fast as possible : 0 ms/frame. CPU가 할수있는 가장 빠른 속도. 
	//BWAPI::Broodwar->setLocalSpeed(Config::BWAPIOptions::SetLocalSpeed);
	BWAPI::Broodwar->setLocalSpeed(0);
	
	// frameskip을 늘리면 화면 표시도 업데이트 안하므로 훨씬 빠릅니다
	//BWAPI::Broodwar->setFrameSkip(Config::BWAPIOptions::SetFrameSkip);
	BWAPI::Broodwar->setFrameSkip(10000000);

	// 화면 표시를 아예 하지 않음으로써 리플레이 파싱 속도를 극대화합니다
	BWAPI::Broodwar->setGUI(false);

	//std::cout << "Map analyzing started" << std::endl;
	BWTA::readMap();
	BWTA::analyze();
	BWTA::buildChokeNodes();
	//std::cout << "Map analyzing finished" << std::endl;

	gameCommander.onStart();
}

void MyBotModule::onEnd(bool isWinner){
	gameCommander.onEnd(isWinner);
}

void MyBotModule::onFrame(){

	gameCommander.onFrame();

	// 화면 출력 및 사용자 입력 처리
	// 빌드서버에서는 Dependency가 없는 빌드서버 전용 UXManager 를 실행시킵니다
	UXManager::Instance().update();

	// BasicBot 1.1 Patch End //////////////////////////////////////////////////
}

// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
// 타임아웃 패배, 자동 패배 체크 추가

void MyBotModule::onUnitCreate(BWAPI::Unit unit){
	gameCommander.onUnitCreate(unit);
}

void MyBotModule::onUnitDestroy(BWAPI::Unit unit){
	gameCommander.onUnitDestroy(unit);

	try
	{
		if (unit->getType().isMineralField())    theMap.OnMineralDestroyed(unit);
		else if (unit->getType().isSpecialBuilding()) theMap.OnStaticBuildingDestroyed(unit);
	}
	catch (const std::exception & e)
	{
		Broodwar << "EXCEPTION: " << e.what() << std::endl;
	}
}

void MyBotModule::onUnitMorph(BWAPI::Unit unit){
	gameCommander.onUnitMorph(unit);
}

void MyBotModule::onUnitRenegade(BWAPI::Unit unit){
	gameCommander.onUnitRenegade(unit);
}

void MyBotModule::onUnitComplete(BWAPI::Unit unit){
	gameCommander.onUnitComplete(unit);
}

void MyBotModule::onUnitDiscover(BWAPI::Unit unit){
	gameCommander.onUnitDiscover(unit);
}

void MyBotModule::onUnitEvade(BWAPI::Unit unit){
	gameCommander.onUnitEvade(unit);
}

void MyBotModule::onUnitShow(BWAPI::Unit unit){
	gameCommander.onUnitShow(unit);
}

void MyBotModule::onUnitHide(BWAPI::Unit unit){
	gameCommander.onUnitHide(unit);
}

void MyBotModule::onNukeDetect(BWAPI::Position target){
	gameCommander.onNukeDetect(target);
}

void MyBotModule::onPlayerLeft(BWAPI::Player player){
	gameCommander.onPlayerLeft(player);
}

void MyBotModule::onSaveGame(std::string gameName){
	gameCommander.onSaveGame(gameName);
}

void MyBotModule::ParseTextCommand(const std::string & commandString)
{
	// Make sure to use %s and pass the text as a parameter,
	// otherwise you may run into problems when you use the %(percent) character!

	if (commandString == "afap") {
		BWAPI::Broodwar->setLocalSpeed(0);
		BWAPI::Broodwar->setFrameSkip(0);
	}
	else if (commandString == "fast") {
		BWAPI::Broodwar->setLocalSpeed(24);
		BWAPI::Broodwar->setFrameSkip(0);
	}
	else if (commandString == "slow") {
		BWAPI::Broodwar->setLocalSpeed(42);
		BWAPI::Broodwar->setFrameSkip(0);
	}
	else if (commandString == "endthegame") {
		//bwapi->setFrameSkip(16);   // Not needed if using setGUI(false).
		BWAPI::Broodwar->setGUI(false);
	}
}

void MyBotModule::onSendText(std::string text)
{
	ParseTextCommand(text);

	gameCommander.onSendText(text);

	BWAPI::Broodwar->sendText("%s", text.c_str());

	BWEM::utils::MapDrawer::ProcessCommand(text);
}

void MyBotModule::onReceiveText(BWAPI::Player player, std::string text){

	gameCommander.onReceiveText(player, text);
}

