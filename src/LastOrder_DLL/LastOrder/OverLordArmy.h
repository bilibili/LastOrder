#pragma once
#include "BattleArmy.h"



class OverLordArmy : public BattleArmy
{
	int splashDamgeRange;
public:
	OverLordArmy();
	TilePosition calOverlordNextMovePosition(UnitState& u, Position destination);
	void overlordUpdate(UnitState& unit, map<Unit, int>& sporeAssign, int unitIndex);

};