#include "ScourgeArmy.h"

bool ScourgeArmy::targetFilter(UnitType type)
{
	return type.isFlyer();
}

int	ScourgeArmy::getAttackPriority(BWAPI::Unit unit)
{
	UnitType targetType = unit->getType();
	if (targetType == BWAPI::UnitTypes::Protoss_Interceptor)
	{
		// Usually not worth scourge at all.
		return -999;
	}

	// Arbiters first.
	if (targetType == BWAPI::UnitTypes::Terran_Science_Vessel
		|| targetType == BWAPI::UnitTypes::Protoss_Observer)
	{
		return 11;
	}
	if (targetType == BWAPI::UnitTypes::Protoss_Arbiter)
	{
		return 10;
	}

	if (targetType.airWeapon() != WeaponTypes::None
		|| targetType == UnitTypes::Protoss_Carrier
		|| targetType == UnitTypes::Protoss_Shuttle
		|| targetType == UnitTypes::Terran_Dropship)
	{
		return 9;
	}
	else
	{
		return 8;
	}
}





