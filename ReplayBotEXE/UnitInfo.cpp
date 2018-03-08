#include "Common.h"
#include "UnitInfo.h"

using namespace MyBot;

void MyBot::UnitInfo::Update()
{
	if (m_unit->exists())
	{
		m_hide = false;
		m_lastPosition = m_unit->getPosition();
		m_completed = m_unit->isCompleted();
		m_HP = m_unit->getHitPoints();
		m_Shield = m_unit->getShields();
	}
	else
		m_hide = true;
}

void UnitInfo::setState(State * state)
{
	if (pState)
	{
		delete pState;
	}

	pState = state;
	pState->unit = Unit();
}

string UnitInfo::getState()
{
	return pState->getName();
}

void UnitInfo::action()
{
	pState->action();
}

void UnitInfo::action(BWAPI::Position targetPosition)
{
	pState->action(targetPosition);
}

void UnitInfo::action(BWAPI::Unit targetUnit)
{
	pState->action(targetUnit);
}

MyBot::UnitInfo::UnitInfo(BWAPI::Unit unit)
{
//	std::cout << "Unit Infor" << unit->getPosition() << ", " << unit->getType() << std::endl;
	m_unit = unit;
	m_type = unit->getType();
	m_unitID = unit->getID();
	m_player = unit->getPlayer();
	m_lastPosition = unit->getPosition();
	m_completed = true;
	m_hide = false;
	pState = nullptr;
}

MyBot::UnitInfo::UnitInfo()
{
}

MyBot::UnitInfo::~UnitInfo()
{
	if (pState)
	{
		delete pState;
	}
}
