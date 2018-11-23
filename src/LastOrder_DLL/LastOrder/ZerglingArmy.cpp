#include "ZerglingArmy.h"
#include "InformationManager.h"


void ZerglingArmy::attackScoutWorker(BWAPI::Unit unit)
{
	int zerglingAttackRange = BWAPI::UnitTypes::Zerg_Zergling.groundWeapon().maxRange();

	for(auto const& u : units)
	{
		int distance = u.unit->getDistance(unit);
		if (distance <= zerglingAttackRange)
		{
			smartAttackUnit(u.unit, unit);
		}
		else
		{
			double2 direc = unit->getPosition() - u.unit->getPosition();
			double2 direcNormal = direc / direc.len();

			int targetx = unit->getPosition().x + int(direcNormal.x * 32 * 2);
			int targety = unit->getPosition().y + int(direcNormal.y * 32 * 2);
			BWAPI::Position target(targetx, targety);
			smartMove(u.unit, target);
		}
	}
}


// get the attack priority of a type in relation to a zergling
int ZerglingArmy::getAttackPriority(BWAPI::Unit unit)
{
	BWAPI::UnitType type = unit->getType();

	if (type == BWAPI::UnitTypes::Zerg_Egg || type == BWAPI::UnitTypes::Zerg_Larva)
	{
		return 0;
	}

	if ((type == UnitTypes::Terran_SCV && unit->isRepairing()) || type == UnitTypes::Terran_Medic) {
		return 13;
	}

	// highest priority is something that can attack us or aid in combat
	if ((type.groundWeapon() != BWAPI::WeaponTypes::None && !type.isFlyer()) || type == BWAPI::UnitTypes::Terran_Bunker
		|| type == UnitTypes::Protoss_Reaver)
	{
		return 11;
	}
	else if (type.isSpellcaster() && !type.isFlyer())
	{
		return 10;
	}
	else if (type.isWorker())
	{
		return 9;
	}
	else if (type.isRefinery())
	{
		return 8;
	}

	else if (type.isResourceDepot())
	{
		return 7;
	}
	// next is special buildings
	else if (type == BWAPI::UnitTypes::Protoss_Pylon || type == BWAPI::UnitTypes::Zerg_Spire)
	{
		return 6;
	}
	else if (type.isBuilding())
	{
		return 5;
	}
	// next is buildings that cost gas
	else if (type.gasPrice() > 0)
	{
		return 4;
	}
	else if (type.mineralPrice() > 0)
	{
		return 3;
	}
	// then everything else
	else
	{
		return 1;
	}
}
