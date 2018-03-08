#pragma once

#include "../Common.h"
#include "../UnitData.h"
#include "State.h"

namespace MyBot
{
	class MarineIdleState : public State
	{
	public:
		virtual string getName() override { return "Idle"; }
		virtual void action() override;
		virtual void action(BWAPI::Position targetPosition) override { this; }
		virtual void action(BWAPI::Unit targetUnit) override { this; }
	};

	class MarineMoveState : public State
	{
	public:
		virtual string getName() override { return "Move"; }
		virtual void action() override { this; }
		virtual void action(BWAPI::Position targetPosition) override;
		virtual void action(BWAPI::Unit targetUnit) override { this; }
	};

	class MarineAttackState : public State
	{
	public:
		virtual string getName() override { return "Attack"; }
		virtual void action() override { this; }
		virtual void action(BWAPI::Position targetPosition) override;
		virtual void action(BWAPI::Unit targetUnit) override;
	};

	class MarinePatrolState : public State
	{
	public:
		virtual string getName() override { return "Patrol"; }
		virtual void action() override { this; }
		virtual void action(BWAPI::Position targetPosition) override;
		virtual void action(BWAPI::Unit targetUnit) override { this; }
	};

	class MarineStopState : public State
	{
	public:
		virtual string getName() override { return "Stop"; }
		virtual void action() override;
		virtual void action(BWAPI::Position targetPosition) override { this; }
		virtual void action(BWAPI::Unit targetUnit) override { this; }
	};
}