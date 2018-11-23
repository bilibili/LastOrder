#pragma once
#include "MutaliskHarassTactic.h"
#include "InformationManager.h"
#include "AttackManager.h"


void MutaliskHarassTactic::update()
{
	updateArmyInfo();
	MutaliskArmy* mutaArmy = dynamic_cast<MutaliskArmy*>(tacticArmy[UnitTypes::Zerg_Mutalisk]);
	ScourgeArmy* scourgeArmy = dynamic_cast<ScourgeArmy*>(tacticArmy[UnitTypes::Zerg_Scourge]);
	if (mutaArmy->getUnits().empty())
	{
		if (!scourgeArmy->getUnits().empty())
		{
			if (!isRetreat)
			{
				logInfo("MutaliskHarassTactic", "has no muta, has scourge, retreat");
				setRetreat();
				return;
			}
		}
		else
		{
			logInfo("MutaliskHarassTactic", "not has any unit, end");
			state = END;
			return;
		}
	}

	int curFrame = Broodwar->getFrameCount();
	//clear shared info
	unitAttackPath.clear();
	unitRetreatPath.clear();
	enemyAssign.clear();
	unitRetreatInfo.clear();
	
	//main update logic
	int retreatCount = 0;
	hasTargetToAttack = false;

	if (!mutaArmy->getUnits().empty())
		BattleArmy::logUnit = mutaArmy->getUnits().front().unit;

	encounteredEnemy.clear();
	for (auto& army : tacticArmy)
	{
		for (auto& armyU : army.second->getUnits())
		{
			if (!hasInitialize) {
				armyU.lastAttackingFrame = Broodwar->getFrameCount();
				armyU.starHarassFrame = Broodwar->getFrameCount();
			}
			army.second->airUnitMicroUpdate(armyU, unitAttackPath, unitRetreatPath, unitRetreatInfo, 
				enemyAssign, isRetreat, attackPosition, encounteredEnemy);
			if (army.first == UnitTypes::Zerg_Mutalisk)
			{
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
	}
	hasInitialize = true;

	//if 50 percent army decide to retreat, end attack
	if (!mutaArmy->getUnits().empty() && retreatCount * 10 / mutaArmy->getUnits().size() >= 8)
	{
		if (!isRetreat)
		{
			logInfo("MutaliskHarassTactic", "encounter strong enemy, retreat");
			setRetreat();
			return;
		}
	}
	
	if (!isRetreat)
	{
		if (!attackPositionHasEnemy())
		{
			logInfo("MutaliskHarassTactic", "do not has enemy");
			setRetreat();
			return;
		}
	}
	else
	{
		if (BWEMMap.GetArea(TilePosition(centerPosition)) == BWEMMap.GetArea(TilePosition(attackPosition))
			|| centerPosition.getDistance(attackPosition) < 8 * 32)
		{
			logInfo("MutaliskHarassTactic", "back to retreat base, end");
			state = END;
		}
	}
}


void MutaliskHarassTactic::setRetreat()
{
	logInfo("MutaliskHarassTactic", "start retreat");
	attackPosition = InformationManager::Instance().getRetreatDestination();
	isRetreat = true;
}


MutaliskHarassTactic::MutaliskHarassTactic()
{
	hasTargetToAttack = false;
}


