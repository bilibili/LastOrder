#include "HydraliskArmy.h"



void HydraliskArmy::KiteTarget(UnitState& attacker, BWAPI::Unit target)
{
	Unit rangedUnit = attacker.unit;
	double range(rangedUnit->getType().groundWeapon().maxRange());
	if (rangedUnit->getType() == BWAPI::UnitTypes::Zerg_Hydralisk && BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Grooved_Spines))
	{
		range = 5 * 32;
	}

	bool kite(true);

	// Kite if configured as a "KiteLonger" unit, or if the enemy's range is shorter than ours.
	// Note: Assumes that the enemy does not have range upgrades.

	if ((range <= target->getType().groundWeapon().maxRange()))
	{
		kite = false;
	}

	// Kite if we're not ready yet: Wait for the weapon.
	double dist(rangedUnit->getDistance(target));
	double speed(rangedUnit->getType().topSpeed());
	double timeToEnter = 0.0;                      // time to reach firing range
	if (speed > .00001)                            // don't even visit the same city as division by zero
	{
		timeToEnter = std::max(0.0, dist - range) / speed;
	}
	if (timeToEnter >= rangedUnit->getGroundWeaponCooldown() ||
		target->getType().isBuilding() || target->isFlying())
	{
		kite = false;
	}

	if (rangedUnit->isSelected())
	{
		Broodwar->drawTextMap(rangedUnit->getPosition().x, rangedUnit->getPosition().y - 2 * 32, "%d", int(rangedUnit->getGroundWeaponCooldown()));
	}

	if (kite)
	{
		// Run away.
		//using one step search
		TilePosition nextMove = calNextMovePosition(attacker, target->getPosition(), Position(0, 0), 3);
		//BattleArmy::smartMove(rangedUnit, Position(nextMove));
		rangedUnit->move(Position(nextMove));
	}
	else
	{
		BattleArmy::smartAttackUnit(rangedUnit, target);
		//rangedUnit->attack(target);
	}
}

void HydraliskArmy::attack(UnitState& attacker, BWAPI::Unit target)
{
	KiteTarget(attacker, target);
}


int HydraliskArmy::getAttackPriority(BWAPI::Unit unit)
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
	if ((type.groundWeapon() != BWAPI::WeaponTypes::None && !type.isWorker()) || type == BWAPI::UnitTypes::Terran_Bunker
		|| type == BWAPI::UnitTypes::Protoss_High_Templar
		|| type == UnitTypes::Protoss_Reaver)
	{
		return 11;
	}

	else if (type.airWeapon() != BWAPI::WeaponTypes::None && !type.isBuilding())
	{
		return 9;
	}
	else if (type.isWorker())
	{
		return 8;
	}

	// next is special buildings
	else if (type == BWAPI::UnitTypes::Protoss_Pylon || type == BWAPI::UnitTypes::Zerg_Spire)
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

