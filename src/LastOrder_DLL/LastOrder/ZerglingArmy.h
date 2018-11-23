#pragma once
#include "BattleArmy.h"



class ZerglingArmy : public BattleArmy
{

public:
	ZerglingArmy() {}

	int getAttackPriority(BWAPI::Unit unit);

	void attackScoutWorker(BWAPI::Unit unit);
};