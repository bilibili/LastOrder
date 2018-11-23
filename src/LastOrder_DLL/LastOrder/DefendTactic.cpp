#pragma once
#include "DefendTactic.h"
#include "InformationManager.h"



void ArmyDefendTactic::update()
{
	std::map<const Area*, std::set<BWAPI::Unit>>& enemyUnitsTargetRegion = \
		InformationManager::Instance().getDefendEnemyInfo();
	std::set<BWAPI::Unit> baseEnemy;
	if (enemyUnitsTargetRegion.find(BWEMMap.GetArea(TilePosition(attackPosition))) != enemyUnitsTargetRegion.end())
	{
		baseEnemy = enemyUnitsTargetRegion[BWEMMap.GetArea(TilePosition(attackPosition))];
	}

	bool isDefendStartBase = false;
	if (BWEMMap.GetArea(InformationManager::Instance().getOurNatrualLocation()) == BWEMMap.GetArea(TilePosition(attackPosition))
		|| BWEMMap.GetArea(Broodwar->self()->getStartLocation()) == BWEMMap.GetArea(TilePosition(attackPosition)))
	{
		isDefendStartBase = true;
	}

	int curFrame = Broodwar->getFrameCount();
	updateArmyInfo();
	if (ourArmyCount == 0)
	{
		//Broodwar->printf("end defend");
		state = END;
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
				enemyAssign, isRetreat, attackPosition, DefendTactic, firstGroundUnit, groundArmyCount, baseEnemy, encounteredEnemy, kiteAssign);
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

	if (isDefendStartBase == false && ourArmyCount > 0 && retreatCount * 10 / ourArmyCount >= 6)
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
		if (baseEnemy.empty())
		{
			noEnemyAccumulateFrame += 1;
		}
		else
		{
			noEnemyAccumulateFrame = 0;
		}
		if (noEnemyAccumulateFrame > 6 * 24)
		{
			logInfo("DefendTactic", "start retreat");
			setRetreat();
			return;
		}
	}
	else
	{
		if (BWEMMap.GetArea(TilePosition(centerPosition)) == BWEMMap.GetArea(TilePosition(attackPosition))
			|| centerPosition.getDistance(attackPosition) < 10 * 32)
		{
			//Broodwar->printf("end defend");
			state = END;
			logInfo("DefendTactic", "tactic end");
		}
	}
}


ArmyDefendTactic::ArmyDefendTactic()
{
	
}

