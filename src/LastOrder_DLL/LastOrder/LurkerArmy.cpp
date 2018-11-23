#include "LurkerArmy.h"


void LurkerArmy::move(UnitState&  unit, BWAPI::Position targetPositio)
{
	Unit attacker = unit.unit;
	if (attacker->isBurrowed())
	{
		attacker->unburrow();
	}
	else
	{
		BattleArmy::smartMove(attacker, targetPositio);
	}
}

void LurkerArmy::attack(UnitState& unit, BWAPI::Unit target)
{
	Unit attacker = unit.unit;
	int lurkerRange = BWAPI::UnitTypes::Zerg_Lurker.groundWeapon().maxRange();
	//burrow closer
	if (attacker->getDistance(target) < lurkerRange - 1 * 32)
	{
		if (attacker->isBurrowed())
			smartAttackUnit(attacker, target);
		else
			attacker->burrow();
	}
	else
	{
		if (attacker->isBurrowed())
		{
			if (attacker->getDistance(target) < lurkerRange)
				smartAttackUnit(attacker, target);
			else
				attacker->unburrow();
		}
		else
		{
			smartMove(attacker, target->getPosition());
		}
	}
}


