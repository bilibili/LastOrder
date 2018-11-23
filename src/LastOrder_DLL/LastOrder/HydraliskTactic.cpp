#pragma once
#include "HydraliskTactic.h"
#include "InformationManager.h"
#include "AttackManager.h"


void HydraliskTactic::update()
{
	int curFrame = Broodwar->getFrameCount();
	updateArmyInfo();
	if (ourArmyCount == 0)
	{
		state = END;
		logInfo("HydraliskTactic", "has no army, tactic end");
		return;
	}

	//clear shared info
	unitAttackPath.clear();
	unitRetreatPath.clear();
	enemyAssign.clear();
	unitRetreatInfo.clear();

	//main update logic
	int retreatCount = 0;
	hasTargetToAttack = false;
	encounteredEnemy.clear();
	for (auto it = kiteAssign.begin(); it != kiteAssign.end();) {
		if (!it->first->exists() || !it->second->exists()) {
			it = kiteAssign.erase(it);
		}
		else {
			it++;
		}
	}

	for (auto const& army : tacticArmy)
	{
		for (auto& armyU : army.second->getUnits())
		{
			army.second->microUpdate(armyU, unitAttackPath, unitRetreatPath, unitRetreatInfo, 
				enemyAssign, isRetreat, attackPosition, HydraliskPushTactic, firstGroundUnit, groundArmyCount,
				set<Unit>(), encounteredEnemy, kiteAssign);
			if (armyU.state == UnitState::RETREAT)
			{
				retreatCount++;
			}
			if (armyU.enemyTarget != nullptr)
			{
				hasTargetToAttack = true;
			}
		}
	}

	//if 50 percent army decide to retreat, end attack
	if (ourArmyCount > 0 && retreatCount * 10 / ourArmyCount >= 6)
	{
		if (!isRetreat)
		{
			logInfo("HydraliskTactic", "encounter strong enemy, retreat");
			setRetreat();
			return;
		}
	}

	if (!isRetreat)
	{
		if (!attackPositionHasEnemy())
		{
			logInfo("HydraliskTactic", "start retreat, no enemy");
			setRetreat();
			return;
		}
	}
	else
	{
		if (BWEMMap.GetArea(TilePosition(centerPosition)) == BWEMMap.GetArea(TilePosition(attackPosition))
			|| centerPosition.getDistance(attackPosition) < 10 * 32)
		{
			state = END;
			logInfo("HydraliskTactic", "retreat back to base, tactic end");
		}
	}
}


HydraliskTactic::HydraliskTactic()
{
}
