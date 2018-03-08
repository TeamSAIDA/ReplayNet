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

	// 리플레이 로그 파일 두번째 줄 : 맵파일이름(리플레이파일이름), 맵이름, 맵 최대플레이어수, 플레이어수, 총 프레임수
	replayDat << InformationManager::Instance().getMapFileName() 
		<< ", " << InformationManager::Instance().getMapName()
		<< ", " << InformationManager::Instance().getMapPlayerLimit()
		<< ", " << InformationManager::Instance().activePlayers.size()
		<< ", " << InformationManager::Instance().replayTotalFrameCount
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
	saveGameResult();

	// 리플레이 로그 파일 마지막 줄 : [EndGame]
	replayDat << "[EndGame]\n";
	replayDat.flush();
	replayDat.close();
}

void LoggingManager::update()
{
	saveSupplyLog();

	//saveUnitsLog();

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

				if (InformationManager::Instance().replayTotalFrameCount = BWAPI::Broodwar->getFrameCount() ) {
					
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
