#pragma once
#include "BattleArmy.h"



class LurkerArmy : public BattleArmy
{
public:
	LurkerArmy() {}
	void attack(UnitState& unit, BWAPI::Unit target) override;
	void move(UnitState&  unit, BWAPI::Position targetPositio) override;
};

