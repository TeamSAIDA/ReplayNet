#include "MarineState.h"
#include "../InformationManager.h"

using namespace MyBot;

void MarineIdleState::action()
{
	cout << "Action Idle" << endl;

	BWTA::Chokepoint* firstChokePoint = BWTA::getNearestChokepoint(InformationManager::Instance().getMainBaseLocation(InformationManager::Instance().selfPlayer)->getTilePosition());

	if (this->unit->isCompleted())
	{
		CommandUtil::attackMove(this->unit, firstChokePoint->getCenter());
	}
}

void MarineMoveState::action(BWAPI::Position targetPosition)
{
	cout << "Action Move" << endl;
}

void MarineAttackState::action(BWAPI::Position targetPosition)
{
	cout << "Action Position Attack" << endl;

	BWAPI::Unitset unitset = BWAPI::Broodwar->getUnitsInRadius(this->unit->getPosition(), 7 * TILE_SIZE);

	bool hasFoundZealot = false;
	BWAPI::Unit enemy = nullptr;
	int distance = 10000000;

	for (auto & u : unitset) {
		if (u->getPlayer() == BWAPI::Broodwar->enemy()
			&& u->getType() == BWAPI::UnitTypes::Protoss_Zealot) {
			hasFoundZealot = true;
			
			if (this->unit->getDistance(u) < distance)
			{
				distance = this->unit->getDistance(u);
				enemy = u;
			}
		}
	}

	if (distance < 2 * TILE_SIZE)
	{
		BWTA::Chokepoint* firstChokePoint = BWTA::getNearestChokepoint(InformationManager::Instance().getMainBaseLocation(InformationManager::Instance().selfPlayer)->getTilePosition());
		CommandUtil::move(this->unit, firstChokePoint->getCenter());
	}
	else
	{
		CommandUtil::attackUnit(this->unit, enemy);
	}

	CommandUtil::attackMove(this->unit, targetPosition);
}

void MarineAttackState::action(BWAPI::Unit targetUnit)
{
	cout << "Action Unit Attack" << endl;

	CommandUtil::attackUnit(this->unit, targetUnit);
}

void MarinePatrolState::action(BWAPI::Position targetPosition)
{
	cout << "Action Patrol" << endl;
}

void MarineStopState::action()
{
	cout << "Action Stop" << endl;
}