#pragma once
#include "BattleArmy.h"



class ScourgeArmy : public BattleArmy
{
public:
	ScourgeArmy() {}

	int	getAttackPriority(BWAPI::Unit unit) override;
	bool targetFilter(UnitType type) override;

};

