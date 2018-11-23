#pragma once
#include "BattleArmy.h"
#include "AstarPath.h"




class MutaliskArmy : public BattleArmy
{
public:
	MutaliskArmy() {}
	int getAttackPriority(BWAPI::Unit unit) override;

	void MutaDanceTarget(UnitState& attacker);
	void attack(UnitState&  attacker, BWAPI::Unit target) override;

};