#pragma once
#include "Common.h"
#include "BattleTactic.h"
#include "ZerglingArmy.h"
#include "AstarPath.h"
#include "Time.cpp"
#include "TimeManager.cpp"




class MutaliskHarassTactic : public BattleTactic
{
	list<Position>			recentCenterPosition;

	list<BWAPI::TilePosition>		attackPath;
	list<BWAPI::TilePosition>		retreatPath;

	void					setRetreat();
	bool					hasInitialize = false;
public:
	MutaliskHarassTactic();
	virtual void			update() override;
};



