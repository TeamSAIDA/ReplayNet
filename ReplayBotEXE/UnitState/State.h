#pragma once

#include "../Common.h"
//#include "../UnitData.h"

using namespace std;

namespace MyBot
{
	class State
	{
	public:
		BWAPI::Unit unit;
		virtual string getName() = 0;
		virtual void action() = 0;
		virtual void action(BWAPI::Position targetPosition) = 0;
		virtual void action(BWAPI::Unit targetUnit) = 0;
	};
}
