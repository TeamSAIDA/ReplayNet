#pragma once

#include "Common.h"

#include "UnitData.h"
#include "InformationManager.h"
#include "LoggingManager.h"

//#include ".\BWTA\MapData.h" 

namespace MyBot
{
	/// 로깅 을 수행하는 코드가 들어가는 class
	class LoggingManager
	{
		LoggingManager();

	public:
		/// static singleton 객체를 리턴합니다
		static LoggingManager &	Instance();

		/// 경기가 시작될 때 일회적으로 로깅 초기 세팅 관련 로직을 실행합니다
		void onStart();

		///  경기가 종료될 때 일회적으로 로깅 결과 정리 관련 로직을 실행합니다
		void onEnd(bool isWinner);

		/// 경기 진행 중 매 프레임마다 경기 로깅 관련 로직을 실행합니다
		void update();

		void onUnitCreate(BWAPI::Unit unit);
		void onUnitMorph(BWAPI::Unit unit);
		/// 맵정보 파일에 미네랄 정보를 저장합니다.
		void onMineralCheck(BWAPI::Unit unit);

		///경기 후 맵에 대한 정보를 저장
		void makeMapData(string mapHash);

		//게임 Frame Count
		int gameFrameCount;
		string fileOwner;
		string firstPlayerTribe;
		string firstPlayerStartingPointX;
		string firstPlayerStartingPointY;
		string firstPlayerOutcome;
		string secondPlayerTribe;
		string secondPlayerStartingPointX;
		string secondPlayerStartingPointY;
		string secondPlayerOutcome;

	private:

		std::string LogFilename;
		std::string LogFileFullPath;
		std::string logfilePath;

		std::string mapInfoFileName;
		std::string mapInfoFileFullPath;
		std::string mapFileName;

		// load map info
		int mapWidthTileRes;
		int mapWidthPixelRes;
		int mapWidthWalkRes;

		int mapHeightTileRes;
		int mapHeightPixelRes;
		int mapHeightWalkRes;

		int mireralInfoArray[4096][4096];
		int mireralInfoWalkArray[512][512];
		int mireralInfoTileArray[128][128];

		std::ofstream replayDat;
		std::ofstream mapDat;

		/// 각 플레이어의 인구수를 저장합니다
		void saveSupplyLog();

		/// 각 플레이어의 유닛 현황을 저장합니다
		void saveUnitsLog();


		/// 각 플레이어의 빌드 실행을 저장합니다
		void saveUnitCreate(BWAPI::Unit unit);

		/// 각 플레이어의 빌드 실행을 저장합니다
		void saveUnitMorph(BWAPI::Unit unit);

		/// 게임 승리 / 패배 플레이어를 저장합니다
		void saveGameResult();

		/// 해당 플레이어가 패배했다고 저장합니다
		void printLoserPlayer(BWAPI::Player p);

		/// 해당 플레이어가 승리했다고 저장합니다
		void printWinnerPlayer(BWAPI::Player p);

	};
}
