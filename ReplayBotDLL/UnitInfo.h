#pragma once

#include "Common.h"
#include "UnitState\State.h"

namespace MyBot
{
	class UnitInfo
	{
	public :
		UnitInfo();
		~UnitInfo();
		UnitInfo(BWAPI::Unit);

		void Update();

		BWAPI::Unit Unit() const { return m_unit; }
		BWAPI::UnitType Type() const { return m_type; }
		BWAPI::Position Pos() const { return m_lastPosition; }
		BWAPI::Player	Player() const { return m_player; }
		int		ID() const { return m_unitID; }
		bool	isComplete() const { return m_completed; }
		int		HP() const { return m_HP; }
		int		Shield() const { return m_Shield; }

		//enemy 용
		bool isHide() const { return m_hide; }

		const bool operator == (BWAPI::Unit unit) const
		{
			return m_unitID == unit->getID();
		}
		const bool operator == (const UnitInfo & rhs) const
		{
			return (m_unitID == rhs.m_unitID);
		}
		const bool operator < (const UnitInfo & rhs) const
		{
			return (m_unitID < rhs.m_unitID);
		}

		// 상태
		void setState(State *state);
		string getState();
		void action();
		void action(BWAPI::Position targetPosition);
		void action(BWAPI::Unit targetUnit);

	private :
		BWAPI::Unit						m_unit;
		BWAPI::UnitType				m_type;
		BWAPI::Player					m_player;

		int									m_unitID;
		int									m_HP;
		int									m_Shield;

		BWAPI::Position				m_lastPosition;
		bool								m_completed;
		bool								m_hide;

		// 상태
		State *pState;
	};

	
}