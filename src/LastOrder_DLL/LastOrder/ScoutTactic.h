#pragma once
#include "Common.h"
#include "BattleTactic.h"
#include "ZerglingArmy.h"
#include "WorkerManager.h"
#include "AstarPath.h"
#include <ctime>



struct Scout{
	Scout(BWAPI::Unit u)
	{
		scoutUnit = u;
		refinedTarget = BWAPI::Positions::None;
		TileTarget = BWAPI::Broodwar->self()->getStartLocation();
		nextMovePosition = BWAPI::TilePositions::None;
		startAstar = false;
	}
	BWAPI::Unit scoutUnit;
	BWAPI::Position		refinedTarget;
	BWAPI::TilePosition TileTarget;
	BWAPI::TilePosition nextMovePosition;
	BWAPI::TilePosition naturalTileTarget;
	bool				startAstar;
};


struct scoutTarget
{
	BWAPI::TilePosition location;
	double				priority;

	scoutTarget(BWAPI::TilePosition t, double p)
	{
		location = t;
		priority = p;
	}

	bool operator < (const scoutTarget & u)
	{
		if (priority < u.priority)
			return true;
		else
			return false;
	}
};



class ScoutTactic : public BattleTactic
{
	enum ScoutState { scoutForEnemyBase = 0, scoutForEnemyInfo = 1 };
	ScoutState state;
	bool endFlag;

	std::vector<scoutTarget>	scoutLocation;
	std::vector<double2>		moveDirections;
	std::vector<double2>		enemyBaseSquarePoints;
	std::vector<std::pair<int,int>>	enemyBaseSquareAstar;
	std::vector<Scout>			overLordIdle;
	std::vector<Scout>			overLordScouts;

	std::vector<scoutTarget>	zerglingScoutLocation;
	std::vector<scoutTarget>	droneScoutLocation;
	std::vector<int>			blockDirection;
	BWAPI::TilePosition			getNextScoutLocation();
	BWAPI::TilePosition			droneDodge(Scout& drone);
	BWAPI::Position				astarSearch(Scout& drone);
	BWAPI::Position				moveAroundEnemyBase(Scout& drone);
	void						assignScoutWork();
	void						overlordMove(Scout& overlord);
	void						generateScoutLocation();
	int							safe_count;
	int							wanderingID;

public:
	ScoutTactic();

	virtual void				update() override;
	virtual bool				isTacticEnd() override;
	virtual void				onUnitDestroy(BWAPI::Unit unit) override;
	virtual void				addArmyUnit(BWAPI::Unit unit) override { overLordIdle.push_back(Scout(unit)); }
	bool						HasZergling();

};



