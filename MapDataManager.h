#pragma once


#include "MapDataManager.h"

//#include ".\BWTA\MapData.h" 

namespace MyBot
{
	/// 로깅 을 수행하는 코드가 들어가는 class
	class MapDataManager
	{
		MapDataManager();

	public:
		/// static singleton 객체를 리턴합니다
		static MapDataManager &	Instance();


	private:

	};
}
