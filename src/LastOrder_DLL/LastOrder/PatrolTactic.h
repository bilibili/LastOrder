#pragma once
#include "BattleTactic.h"

struct Patrol {
	Patrol(BWAPI::Unit u)
	{
		patrolUnit = u;
		nextMovePosition = BWAPI::Positions::None;
	}
	BWAPI::Unit patrolUnit;
	BWAPI::Position nextMovePosition;
};

struct patrolTarget {
	BWAPI::TilePosition		location;
	double					priority;

	patrolTarget(BWAPI::TilePosition t, double p = 0.0)
	{
		location = t;
		priority = p;
	}

	bool operator < (const patrolTarget & u)
	{
		if (priority < u.priority)
			return true;
		else
			return false;
	}
};

class PatrolTactic : public BattleTactic
{
	enum PatrolState { watchStartBase = 0, watchNaturalBase = 1, scoutOtherBase = 2 };
	PatrolState state;
	std::vector<double2>		moveDirections;
	std::vector<Patrol>			overLordPatrols;
	std::vector<Patrol>			overLordIdle;
	std::vector<patrolTarget>		patrolPositions;
	bool						defendNaturalBase;
	static const int			startScoutTime = 24 * 60 * 10;
	//static const int			startScoutTime = 24 * 60 * 0;
public:
	PatrolTactic();
	virtual void				update() override;
	//virtual bool				isTacticEnd() override { return overLordIdle.size()+ overLordPatrols.size() == 0; }
	virtual bool				isTacticEnd() override { return false; }
	virtual void				onUnitDestroy(BWAPI::Unit unit) override;
	virtual void				addArmyUnit(BWAPI::Unit unit) override { overLordIdle.push_back(Patrol(unit)); }
	Position					nextMovePosition();
	void						assignPatrolTask();
	bool						hasOverlord() { return overLordIdle.size() + overLordPatrols.size() > 0; }
};

