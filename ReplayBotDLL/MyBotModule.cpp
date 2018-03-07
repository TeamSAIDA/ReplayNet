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

	// Config ���� ������ ���ŷӰ�, ���� �� ���� Config ���� ��ġ�� �������ִ� ���� ���ŷӱ� ������, 
	// Config �� ���Ϸκ��� �о������ �ʰ�, Config Ŭ������ ���� ����ϵ��� �մϴ�.
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
	// Fastest: 42 ms/frame.  1�ʿ� 24 frame. �Ϲ������� 1�ʿ� 24frame�� ���� ���Ӽӵ��� �մϴ�
	// Normal: 67 ms/frame. 1�ʿ� 15 frame
	// As fast as possible : 0 ms/frame. CPU�� �Ҽ��ִ� ���� ���� �ӵ�. 
	//BWAPI::Broodwar->setLocalSpeed(Config::BWAPIOptions::SetLocalSpeed);
	BWAPI::Broodwar->setLocalSpeed(0);
	
	// frameskip�� �ø��� ȭ�� ǥ�õ� ������Ʈ ���ϹǷ� �ξ� �����ϴ�
	//BWAPI::Broodwar->setFrameSkip(Config::BWAPIOptions::SetFrameSkip);
	BWAPI::Broodwar->setFrameSkip(10000000);

	// ȭ�� ǥ�ø� �ƿ� ���� �������ν� ���÷��� �Ľ� �ӵ��� �ش�ȭ�մϴ�
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

	// ȭ�� ��� �� ����� �Է� ó��
	// ���弭�������� Dependency�� ���� ���弭�� ���� UXManager �� �����ŵ�ϴ�
	UXManager::Instance().update();

	// BasicBot 1.1 Patch End //////////////////////////////////////////////////
}

// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
// Ÿ�Ӿƿ� �й�, �ڵ� �й� üũ �߰�

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

