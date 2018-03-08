#pragma once

#include "../UnitState/MarineState.h"

namespace MyBot
{
	class MarineManager
	{
		MarineManager();

	public:
		static MarineManager & Instance();
		void update();
	};
}