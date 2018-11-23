#pragma once
#include "Common.h"


struct UnitState
{
	enum microState { MOVE = 0, ATTACK = 1, KEEP = 2, RETREAT = 3, STOP = 4, SEIGE = 5 };

	UnitState(BWAPI::Unit u)
	{
		state = MOVE;
		unit = u;
		keepMoveStartFrame = 0;
		retreatFlag = 0;
		enemyTarget = nullptr;
		stopStartFrame = 0;
		stopEndFrame = 0;
		nextMovePosition = TilePositions::None;
		performDance = true;
		seigeFlag = false;
		lastAttackingFrame = 0;
		starHarassFrame = 0;
		hasAssignTarget = false;
		ETAtoTarget = -1;
		nonAttackingDuration = 0;
		underKiteUnit = nullptr;
		lastunderKitedFrame = 0;
	}

	microState state;
	BWAPI::Unit unit;
	int keepMoveStartFrame;
	int retreatFlag;
	Unit enemyTarget;
	int stopStartFrame;
	int stopEndFrame;
	TilePosition nextMovePosition;
	bool performDance;
	bool seigeFlag;
	int lastAttackingFrame;
	int starHarassFrame;
	bool hasAssignTarget;
	int ETAtoTarget; // estimated time arrival to target(frame count), fairly loose
	int nonAttackingDuration; // time since last actural attacking
	Unit underKiteUnit;
	int lastunderKitedFrame;
};
