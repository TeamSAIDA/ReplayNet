#include "UXManager.h"

using namespace MyBot;

UXManager::UXManager()
{
}

UXManager & UXManager::Instance()
{
	static UXManager instance;
	return instance;
}


void UXManager::onStart()
{
}

void UXManager::update()
{
	//std::cout << 1;

	drawGameInformationOnScreen(5, 5);

	//std::cout << 2;

	if (Config::Debug::DrawEnemyUnitInfo)
	{
//		drawUnitStatisticsOnScreen(400, 20);
	}

	//std::cout << 3;

	if (Config::Debug::DrawBWTAInfo)
	{
		drawBWTAResultOnMap();
	}

	//std::cout << 4;

	if (Config::Debug::DrawMapGrid)
	{
		drawMapGrid();
	}
	
	//std::cout << 9;

	if (Config::Debug::DrawUnitHealthBars)
	{
//		drawUnitExtendedInformationOnMap();
		drawUnitIdOnMap();
	}

	// 공격
	if (Config::Debug::DrawUnitTargetInfo)
	{
		drawUnitTargetOnMap();

		// 미사일, 럴커의 보이지않는 공격등을 표시
		drawBulletsOnMap();
	}
	
	//std::cout << 14;

	// draw tile position of mouse cursor
	if (Config::Debug::DrawMouseCursorInfo)
	{
		int mouseX = BWAPI::Broodwar->getMousePosition().x + BWAPI::Broodwar->getScreenPosition().x;
		int mouseY = BWAPI::Broodwar->getMousePosition().y + BWAPI::Broodwar->getScreenPosition().y;
		BWAPI::Broodwar->drawTextMap(mouseX + 20, mouseY, "(%d, %d)", (int)(mouseX/TILE_SIZE), (int)(mouseY/TILE_SIZE));
	}
}

void UXManager::drawGameInformationOnScreen(int x, int y)
{
	BWAPI::Broodwar->drawTextScreen(x, y, "\x04Players:");

	int i = 0;
	for (const auto& p : InformationManager::Instance().activePlayers) {
		BWAPI::Broodwar->drawTextScreen(x + 50 + i * 50, y, "%s (%s) ", p->getName().c_str(), p->getRace().getName().c_str());
	}

	y += 12;

	BWAPI::Broodwar->drawTextScreen(x, y, "\x04Map:");
	BWAPI::Broodwar->drawTextScreen(x + 50, y, "\x03%s (%d x %d size)", BWAPI::Broodwar->mapFileName().c_str(), BWAPI::Broodwar->mapWidth(), BWAPI::Broodwar->mapHeight());
	BWAPI::Broodwar->setTextSize();
	y += 12;

	BWAPI::Broodwar->drawTextScreen(x, y, "\x04Time:");
	BWAPI::Broodwar->drawTextScreen(x + 50, y, "\x04%d", BWAPI::Broodwar->getFrameCount());
	BWAPI::Broodwar->drawTextScreen(x + 90, y, "\x04%4d:%3d", (int)(BWAPI::Broodwar->getFrameCount() / (23.8 * 60)), (int)((int)(BWAPI::Broodwar->getFrameCount() / 23.8) % 60));
}

void UXManager::drawAPM(int x, int y)
{
	int bwapiAPM = BWAPI::Broodwar->getAPM();
	BWAPI::Broodwar->drawTextScreen(x, y, "APM : %d", bwapiAPM);
}

void UXManager::drawPlayers()
{
	BWAPI::Playerset players = BWAPI::Broodwar->getPlayers();
	for (auto p : players)
		BWAPI::Broodwar << "Player [" << p->getID() << "]: " << p->getName() << " is in force: " << p->getForce()->getName() << std::endl;
}

void UXManager::drawForces()
{
	BWAPI::Forceset forces = BWAPI::Broodwar->getForces();
	for (auto f : forces)
	{
		BWAPI::Playerset players = f->getPlayers();
		BWAPI::Broodwar << "Force " << f->getName() << " has the following players:" << std::endl;
		for (auto p : players)
			BWAPI::Broodwar << "  - Player [" << p->getID() << "]: " << p->getName() << std::endl;
	}
}
/* 임시로 해당 Function을 막는다. Unit Data 확정 후 추후 수정 예정*/
void UXManager::drawUnitExtendedInformationOnMap()
{
	int verticalOffset = -10;

	// draw enemy units
	for (const auto & kv : InformationManager::Instance().getUnitData(BWAPI::Broodwar->enemy()).getAllUnits())
	{
		const UnitInfo * ui(kv.second);

		const BWAPI::Position & pos = ui->Pos();

		int left = pos.x - ui->Type().dimensionLeft();
		int right = pos.x + ui->Type().dimensionRight();
		int top = pos.y - ui->Type().dimensionUp();
		int bottom = pos.y + ui->Type().dimensionDown();

		// 적 유닛이면 주위에 박스 표시
		if (!BWAPI::Broodwar->isVisible(BWAPI::TilePosition(pos)))
		{
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, top), BWAPI::Position(right, bottom), BWAPI::Colors::Grey, false);
			BWAPI::Broodwar->drawTextMap(BWAPI::Position(left + 3, top + 4), "%s", ui->Type().getName().c_str());
		}

		// 유닛의 HitPoint 남아있는 비율 표시
		if (!ui->Type().isResourceContainer() && ui->Type().maxHitPoints() > 0)
		{
			double hpRatio = (double)ui->HP() / (double)ui->Type().maxHitPoints();

			BWAPI::Color hpColor = BWAPI::Colors::Green;
			if (hpRatio < 0.66) hpColor = BWAPI::Colors::Orange;
			if (hpRatio < 0.33) hpColor = BWAPI::Colors::Red;

			int ratioRight = left + (int)((right - left) * hpRatio);
			int hpTop = top + verticalOffset;
			int hpBottom = top + 4 + verticalOffset;

			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Grey, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(ratioRight, hpBottom), hpColor, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Black, false);

			int ticWidth = 3;

			for (int i(left); i < right - 1; i += ticWidth)
			{
				BWAPI::Broodwar->drawLineMap(BWAPI::Position(i, hpTop), BWAPI::Position(i, hpBottom), BWAPI::Colors::Black);
			}
		}

		// 유닛의 Shield 남아있는 비율 표시
		if (!ui->Type().isResourceContainer() && ui->Type().maxShields() > 0)
		{
			double shieldRatio = (double)ui->Shield() / (double)ui->Type().maxShields();

			int ratioRight = left + (int)((right - left) * shieldRatio);
			int hpTop = top - 3 + verticalOffset;
			int hpBottom = top + 1 + verticalOffset;

			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Grey, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(ratioRight, hpBottom), BWAPI::Colors::Blue, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Black, false);

			int ticWidth = 3;

			for (int i(left); i < right - 1; i += ticWidth)
			{
				BWAPI::Broodwar->drawLineMap(BWAPI::Position(i, hpTop), BWAPI::Position(i, hpBottom), BWAPI::Colors::Black);
			}
		}

	}

	// draw neutral units and our units
	for (auto & unit : BWAPI::Broodwar->getAllUnits())
	{
		if (unit->getPlayer() == BWAPI::Broodwar->enemy())
		{
			continue;
		}

		const BWAPI::Position & pos = unit->getPosition();

		int left = pos.x - unit->getType().dimensionLeft();
		int right = pos.x + unit->getType().dimensionRight();
		int top = pos.y - unit->getType().dimensionUp();
		int bottom = pos.y + unit->getType().dimensionDown();

		//BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, top), BWAPI::Position(right, bottom), BWAPI::Colors::Grey, false);

		// 유닛의 HitPoint 남아있는 비율 표시
		if (!unit->getType().isResourceContainer() && unit->getType().maxHitPoints() > 0)
		{
			double hpRatio = (double)unit->getHitPoints() / (double)unit->getType().maxHitPoints();

			BWAPI::Color hpColor = BWAPI::Colors::Green;
			if (hpRatio < 0.66) hpColor = BWAPI::Colors::Orange;
			if (hpRatio < 0.33) hpColor = BWAPI::Colors::Red;

			int ratioRight = left + (int)((right - left) * hpRatio);
			int hpTop = top + verticalOffset;
			int hpBottom = top + 4 + verticalOffset;

			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Grey, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(ratioRight, hpBottom), hpColor, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Black, false);

			int ticWidth = 3;

			for (int i(left); i < right - 1; i += ticWidth)
			{
				BWAPI::Broodwar->drawLineMap(BWAPI::Position(i, hpTop), BWAPI::Position(i, hpBottom), BWAPI::Colors::Black);
			}
		}

		// 유닛의 Shield 남아있는 비율 표시
		if (!unit->getType().isResourceContainer() && unit->getType().maxShields() > 0)
		{
			double shieldRatio = (double)unit->getShields() / (double)unit->getType().maxShields();

			int ratioRight = left + (int)((right - left) * shieldRatio);
			int hpTop = top - 3 + verticalOffset;
			int hpBottom = top + 1 + verticalOffset;

			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Grey, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(ratioRight, hpBottom), BWAPI::Colors::Blue, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Black, false);

			int ticWidth = 3;

			for (int i(left); i < right - 1; i += ticWidth)
			{
				BWAPI::Broodwar->drawLineMap(BWAPI::Position(i, hpTop), BWAPI::Position(i, hpBottom), BWAPI::Colors::Black);
			}
		}

		// Mineral / Gas 가 얼마나 남아있는가
		if (unit->getType().isResourceContainer() && unit->getInitialResources() > 0)
		{

			double mineralRatio = (double)unit->getResources() / (double)unit->getInitialResources();

			int ratioRight = left + (int)((right - left) * mineralRatio);
			int hpTop = top + verticalOffset;
			int hpBottom = top + 4 + verticalOffset;

			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Grey, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(ratioRight, hpBottom), BWAPI::Colors::Cyan, true);
			BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hpTop), BWAPI::Position(right, hpBottom), BWAPI::Colors::Black, false);

			int ticWidth = 3;

			for (int i(left); i < right - 1; i += ticWidth)
			{
				BWAPI::Broodwar->drawLineMap(BWAPI::Position(i, hpTop), BWAPI::Position(i, hpBottom), BWAPI::Colors::Black);
			}
		}
	}
}
#if 0
// 아군이 입은 피해 누적값, 적군이 입은 피해 누적값, 적군의 UnitType 별 파악된 Unit 숫자를 표시
void UXManager::drawUnitStatisticsOnScreen(int x, int y)
{
	int currentY = y;

	// 아군이 입은 피해 누적값
	BWAPI::Broodwar->drawTextScreen(x, currentY, "\x03 Self Loss:\x04 Minerals: \x1f%d \x04Gas: \x07%d",
		InformationManager::Instance().getUnitData(BWAPI::Broodwar->self()).getMineralsLost(), 
		InformationManager::Instance().getUnitData(BWAPI::Broodwar->self()).getGasLost());
	currentY += 10;

	// 아군 모든 유닛 숫자 합계
	//BWAPI::Broodwar->drawTextScreen(x, currentY, "\x03 allUnitCount: %d", BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::AllUnits));
	//currentY += 10;

	// 아군 건설/훈련 완료한 유닛 숫자 합계
	//BWAPI::Broodwar->drawTextScreen(x, currentY, "\x03 completedUnitCount: %d", BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::AllUnits));
	//currentY += 10;

	// 아군 건설/훈련중인 유닛 숫자 합계
	//BWAPI::Broodwar->drawTextScreen(x, currentY, "\x03 incompleteUnitCount: %d", BWAPI::Broodwar->self()->incompleteUnitCount(BWAPI::UnitTypes::AllUnits));
	//currentY += 10;

	// 아군 유닛 파괴/사망 숫자 누적값
	//BWAPI::Broodwar->drawTextScreen(x, currentY, "\x03 deadUnitCount: %d", BWAPI::Broodwar->self()->deadUnitCount(BWAPI::UnitTypes::AllUnits));
	//currentY += 10;

	// 상대방 유닛을 파괴/사망 시킨 숫자 누적값
	//BWAPI::Broodwar->drawTextScreen(x, currentY, "\x03 killedUnitCount: %d", BWAPI::Broodwar->self()->killedUnitCount(BWAPI::UnitTypes::AllUnits));
	//currentY += 10;

	//BWAPI::Broodwar->drawTextScreen(x, currentY, "\x03 UnitScore: %d", BWAPI::Broodwar->self()->getUnitScore());
	//BWAPI::Broodwar->drawTextScreen(x, currentY, "\x03 RazingScore: %d", BWAPI::Broodwar->self()->getRazingScore());
	//BWAPI::Broodwar->drawTextScreen(x, currentY, "\x03 BuildingScore: %d", BWAPI::Broodwar->self()->getBuildingScore());
	//BWAPI::Broodwar->drawTextScreen(x, currentY, "\x03 KillScore: %d", BWAPI::Broodwar->self()->getKillScore());

	// 적군이 입은 피해 누적값
	BWAPI::Broodwar->drawTextScreen(x, currentY, "\x03 Enemy Loss:\x04 Minerals: \x1f%d \x04Gas: \x07%d",
		InformationManager::Instance().getUnitData(BWAPI::Broodwar->enemy()).getMineralsLost(),
		InformationManager::Instance().getUnitData(BWAPI::Broodwar->enemy()).getGasLost());

	// 적군의 UnitType 별 파악된 Unit 숫자를 표시
	BWAPI::Broodwar->drawTextScreen(x,		 currentY + 20, "\x04 UNIT NAME");
	BWAPI::Broodwar->drawTextScreen(x + 110, currentY + 20, "\x04 Created");
	BWAPI::Broodwar->drawTextScreen(x + 150, currentY + 20, "\x04 Dead");
	BWAPI::Broodwar->drawTextScreen(x + 190, currentY + 20, "\x04 Alive");

	int yspace = 0;
	for (BWAPI::UnitType t : BWAPI::UnitTypes::allUnitTypes())
	{
		int numCreatedUnits = InformationManager::Instance().getUnitData(BWAPI::Broodwar->enemy()).getNumCreatedUnits(t);
		int numDeadUnits = InformationManager::Instance().getUnitData(BWAPI::Broodwar->enemy()).getNumDeadUnits(t);
		int numUnits = InformationManager::Instance().getUnitData(BWAPI::Broodwar->enemy()).getNumUnits(t);

		if (numUnits > 0)
		{
			BWAPI::Broodwar->drawTextScreen(x,		 currentY + 30 + ((yspace)* 10), "%s", t.getName().c_str());
			BWAPI::Broodwar->drawTextScreen(x + 120, currentY + 30 + ((yspace)* 10), "%d", numCreatedUnits);
			BWAPI::Broodwar->drawTextScreen(x + 160, currentY + 30 + ((yspace)* 10), "%d", numDeadUnits);
			BWAPI::Broodwar->drawTextScreen(x + 200, currentY + 30 + ((yspace)* 10), "%d", numUnits);
			yspace++;
		}
	}
	yspace++;

	// 아군의 UnitType 별 파악된 Unit 숫자를 표시
	for (BWAPI::UnitType t : BWAPI::UnitTypes::allUnitTypes())
	{
		int numCreatedUnits = InformationManager::Instance().getUnitData(BWAPI::Broodwar->self()).getNumCreatedUnits(t);
		int numDeadUnits = InformationManager::Instance().getUnitData(BWAPI::Broodwar->self()).getNumDeadUnits(t);
		int numUnits = InformationManager::Instance().getUnitData(BWAPI::Broodwar->self()).getNumUnits(t);

		if (numUnits > 0)
		{
			BWAPI::Broodwar->drawTextScreen(x, currentY + 30 + ((yspace)* 10), "%s", t.getName().c_str());
			BWAPI::Broodwar->drawTextScreen(x + 120, currentY + 30 + ((yspace)* 10), "%d", numCreatedUnits);
			BWAPI::Broodwar->drawTextScreen(x + 160, currentY + 30 + ((yspace)* 10), "%d", numDeadUnits);
			BWAPI::Broodwar->drawTextScreen(x + 200, currentY + 30 + ((yspace)* 10), "%d", numUnits);
			yspace++;
		}
	}
}
#endif
void UXManager::drawBWTAResultOnMap()
{
	//we will iterate through all the base locations, and draw their outlines.
	for (std::set<BWTA::BaseLocation*>::const_iterator i = BWTA::getBaseLocations().begin(); i != BWTA::getBaseLocations().end(); i++)
	{
		BWAPI::TilePosition p = (*i)->getTilePosition();
		BWAPI::Position c = (*i)->getPosition();

		//draw outline of Base location 
		BWAPI::Broodwar->drawBoxMap(p.x * 32, p.y * 32, p.x * 32 + 4 * 32, p.y * 32 + 3 * 32, BWAPI::Colors::Blue);

		//draw a circle at each mineral patch
		for (BWAPI::Unitset::iterator j = (*i)->getStaticMinerals().begin(); j != (*i)->getStaticMinerals().end(); j++)
		{
			BWAPI::Position q = (*j)->getInitialPosition();
			BWAPI::Broodwar->drawCircleMap(q.x, q.y, 30, BWAPI::Colors::Cyan);
		}

		//draw the outlines of vespene geysers
		for (BWAPI::Unitset::iterator j = (*i)->getGeysers().begin(); j != (*i)->getGeysers().end(); j++)
		{
			BWAPI::TilePosition q = (*j)->getInitialTilePosition();
			BWAPI::Broodwar->drawBoxMap(q.x * 32, q.y * 32, q.x * 32 + 4 * 32, q.y * 32 + 2 * 32, BWAPI::Colors::Orange);
		}

		//if this is an island expansion, draw a yellow circle around the base location
		if ((*i)->isIsland())
			BWAPI::Broodwar->drawCircleMap(c, 80, BWAPI::Colors::Yellow);
	}

	//we will iterate through all the regions and draw the polygon outline of it in green.
	for (std::set<BWTA::Region*>::const_iterator r = BWTA::getRegions().begin(); r != BWTA::getRegions().end(); r++)
	{
		BWTA::Polygon p = (*r)->getPolygon();
		for (int j = 0; j<(int)p.size(); j++)
		{
			BWAPI::Position point1 = p[j];
			BWAPI::Position point2 = p[(j + 1) % p.size()];
			BWAPI::Broodwar->drawLineMap(point1, point2, BWAPI::Colors::Green);
		}
	}

	//we will visualize the chokepoints with red lines
	for (std::set<BWTA::Region*>::const_iterator r = BWTA::getRegions().begin(); r != BWTA::getRegions().end(); r++)
	{
		for (std::set<BWTA::Chokepoint*>::const_iterator c = (*r)->getChokepoints().begin(); c != (*r)->getChokepoints().end(); c++)
		{
			BWAPI::Position point1 = (*c)->getSides().first;
			BWAPI::Position point2 = (*c)->getSides().second;
			BWAPI::Broodwar->drawLineMap(point1, point2, BWAPI::Colors::Red);
		}
	}
}

void UXManager::drawMapGrid()
{
	int	cellSize = MapGrid::Instance().getCellSize();
	int	mapWidth = MapGrid::Instance().getMapWidth();
	int mapHeight = MapGrid::Instance().getMapHeight();
	int	rows = MapGrid::Instance().getRows();
	int	cols = MapGrid::Instance().getCols();

	for (int i = 0; i<cols; i++)
	{
		BWAPI::Broodwar->drawLineMap(i*cellSize, 0, i*cellSize, mapHeight, BWAPI::Colors::Blue);
	}

	for (int j = 0; j<rows; j++)
	{
		BWAPI::Broodwar->drawLineMap(0, j*cellSize, mapWidth, j*cellSize, BWAPI::Colors::Blue);
	}

	for (int r = 0; r < rows; r+=2)
	{
		for (int c = 0; c < cols; c+=2)
		{
			GridCell & cell = MapGrid::Instance().getCellByIndex(r, c);

			BWAPI::Broodwar->drawTextMap(cell.center.x - cellSize / 2, cell.center.y - cellSize / 2, "%d,%d", c, r);
			//BWAPI::Broodwar->drawTextMap(cell.center.x, cell.center.y + 10, "Last seen at %d", cell.timeLastVisited);
		}
	}

}

void UXManager::drawUnitIdOnMap() {
	for (const auto& p : InformationManager::Instance().activePlayers) {
		for (auto & unit : p->getUnits()) {
			BWAPI::Broodwar->drawTextMap(unit->getPosition().x, unit->getPosition().y + 5, "\x07%d", unit->getID());
		}
	}
}


void UXManager::drawUnitTargetOnMap() 
{
	for (const auto& p : InformationManager::Instance().activePlayers) {
		for (auto & unit : p->getUnits())
		{
			if (unit != nullptr && unit->isCompleted() && !unit->getType().isBuilding() && !unit->getType().isWorker())
			{
				BWAPI::Unit targetUnit = unit->getTarget();
				if (targetUnit != nullptr && targetUnit->getPlayer() != p) {
					BWAPI::Broodwar->drawCircleMap(unit->getPosition(), dotRadius, BWAPI::Colors::Red, true);
					BWAPI::Broodwar->drawCircleMap(targetUnit->getTargetPosition(), dotRadius, BWAPI::Colors::Red, true);
					BWAPI::Broodwar->drawLineMap(unit->getPosition(), targetUnit->getTargetPosition(), BWAPI::Colors::Red);
				}
				else if (unit->isMoving()) {
					BWAPI::Broodwar->drawCircleMap(unit->getPosition(), dotRadius, BWAPI::Colors::Orange, true);
					BWAPI::Broodwar->drawCircleMap(unit->getTargetPosition(), dotRadius, BWAPI::Colors::Orange, true);
					BWAPI::Broodwar->drawLineMap(unit->getPosition(), unit->getTargetPosition(), BWAPI::Colors::Orange);
				}
			}
		}
	}
}

// Bullet 을 Line 과 Text 로 표시한다. Cloaking Unit 의 Bullet 표시에 쓰인다
void UXManager::drawBulletsOnMap()
{
	for (auto &b : BWAPI::Broodwar->getBullets())
	{
		BWAPI::Position p = b->getPosition();
		double velocityX = b->getVelocityX();
		double velocityY = b->getVelocityY();

		// 아군 것이면 녹색, 적군 것이면 빨간색 ? 
		BWAPI::Broodwar->drawLineMap(p, p + BWAPI::Position((int)velocityX, (int)velocityY), BWAPI::Colors::Red);
		BWAPI::Broodwar->drawTextMap(p, "%c%s", BWAPI::Text::Red, b->getType().c_str());
	}
}

