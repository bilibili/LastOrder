#pragma once
#include "BattleArmy.h"



class HydraliskArmy : public BattleArmy
{
public:
	HydraliskArmy() {}
	int getAttackPriority(BWAPI::Unit unit) override;
	void KiteTarget(UnitState& attacker, BWAPI::Unit target);
	void attack(UnitState& attacker, BWAPI::Unit target) override;
};