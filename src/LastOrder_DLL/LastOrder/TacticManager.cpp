#pragma once
#include "TacticManager.h"


TacticManager& TacticManager::Instance()
{
	static TacticManager a;
	return a;
}


void TacticManager::addTactic(tacticType tactic, BWAPI::Position attackPosition)
{
	// already have the same tactic
	if (myTactic.find(tacKey(tactic, attackPosition)) != myTactic.end())
		return;

	if (tactic == MutaliskHarassTac)
	{
		myTactic[tacKey(tactic, attackPosition)] = new MutaliskHarassTactic();
	}
	else if (tactic == HydraliskPushTactic)
	{
		myTactic[tacKey(tactic, attackPosition)] = new HydraliskTactic();
	}
	else if (tactic == DefendTactic)
	{
		//BWAPI::Broodwar->printf("trigger defend!!!!");
		myTactic[tacKey(tactic, attackPosition)] = new ArmyDefendTactic();
	}
	else if (tactic == ScoutTac)
	{
		myTactic[tacKey(tactic, attackPosition)] = new ScoutTactic();
	}
	else if (tactic == PatrolTac)
	{
		myTactic[tacKey(tactic, attackPosition)] = new PatrolTactic();
	}
	else if (tactic == AirdropTac)
	{
		myTactic[tacKey(tactic, attackPosition)] = new AirdropTactic();
	}

	myTactic[tacKey(tactic, attackPosition)]->setAttackPosition(attackPosition, tactic);
}


void TacticManager::addTacticUnit(tacticType tactic, BWAPI::Position attackPosition, BWAPI::Unit unit)
{
	if (myTactic.find(tacKey(tactic, attackPosition)) == myTactic.end() || !unit->exists())
		return;
	myTactic[tacKey(tactic, attackPosition)]->addArmyUnit(unit);
}


void TacticManager::addTacticArmy(tacticType tactic, BWAPI::Position attackPosition, std::map<BWAPI::UnitType, BattleArmy*>& Army, BWAPI::UnitType unitType, int count)
{
	if (myTactic.find(tacKey(tactic, attackPosition)) == myTactic.end() || count == 0)
		return;

	std::map<BWAPI::UnitType, BattleArmy*>& tacticArmy = myTactic[tacKey(tactic, attackPosition)]->getArmy();
	std::vector<UnitState>& targetArmy = tacticArmy[unitType]->getUnits();
	std::vector<UnitState>& sourceArmy = Army[unitType]->getUnits();

	if (int(sourceArmy.size()) >= count)
	{
		targetArmy.insert(targetArmy.end(), sourceArmy.end() -= count, sourceArmy.end());
		sourceArmy.erase(sourceArmy.end() -= count, sourceArmy.end());
	}
}


void TacticManager::update()
{
	for(auto tactic : myTactic)
	{
		tactic.second->update();
	}

	checkTacticEnd();

}


bool TacticManager::isTacticRun(tacticType tactic, BWAPI::Position attackPosition)
{
	if (myTactic.find(tacKey(tactic, attackPosition)) != myTactic.end())
		return true;
	else
		return false;
}

bool TacticManager::isOneTacticRun(tacticType tactic)
{
	for(auto const& tac : myTactic)
	{
		if (tac.first.tacName == tactic)
			return true;
	}
	return false;
}

BWAPI::Position TacticManager::getTacticPosition(tacticType tactic)
{
	for(auto const& tac : myTactic)
	{
		if (tac.first.tacName == tactic)
			return tac.first.attackPosition;
	}
	return BWAPI::Positions::None;
}

void TacticManager::onUnitDestroy(BWAPI::Unit unit)
{
	for(auto const& tactic : myTactic)
	{
		tactic.second->onUnitDestroy(unit);
	}
}

void TacticManager::onLurkerMorph()
{
	for (auto t : myTactic)
	{
		t.second->onLurkerMorph();
	}
}


void TacticManager::checkTacticEnd()
{
	for (auto it = myTactic.begin(); it != myTactic.end();)
	{
		if (it->second->isTacticEnd())
		{
			AttackManager::Instance().addTacticRemainArmy(it->second->getArmy());
			delete it->second;
			myTactic.erase(it++);
		}
		else
		{
			it++;
		}
	}
}

void TacticManager::onUnitShow(BWAPI::Unit unit)
{
	for(auto const& tactic : myTactic)
	{
		tactic.second->onUnitShow(unit);
	}
}

map<BWAPI::UnitType, int>	TacticManager::getTacArmyForceDetail(tacticType tactic, BWAPI::Position attackPosition)
{
	if (myTactic.find(tacKey(tactic, attackPosition)) == myTactic.end())
		return map<BWAPI::UnitType, int>();
	std::map<BWAPI::UnitType, BattleArmy*>& tacArmy = myTactic.find(tacKey(tactic, attackPosition))->second->getArmy();
	map<BWAPI::UnitType, int> defendTacArmySupply;
	for (auto it = tacArmy.begin(); it != tacArmy.end(); it++)
	{
		defendTacArmySupply[it->first] = it->second->getUnits().size();
	}
	return defendTacArmySupply;
}

int TacticManager::getTacArmyForce(tacticType tactic, BWAPI::Position attackPosition)
{
	if (myTactic.find(tacKey(tactic, attackPosition)) == myTactic.end())
		return 0;
	std::map<BWAPI::UnitType, BattleArmy*>& tacArmy = myTactic.find(tacKey(tactic, attackPosition))->second->getArmy();
	int defendTacArmySupply = 0;
	for (auto it = tacArmy.begin(); it != tacArmy.end(); it++)
	{
		defendTacArmySupply += it->first.supplyRequired() * it->second->getUnits().size();
	}
	return defendTacArmySupply;
}

std::map<BWAPI::UnitType, BattleArmy*>&	TacticManager::getTacArmy(tacticType tactic, BWAPI::Position attackPosition) {
	return myTactic.find(tacKey(tactic, attackPosition))->second->getArmy();
}


bool TacticManager::isAssignScoutZergling()
{
	ScoutTactic* scoutTac = dynamic_cast<ScoutTactic*>(myTactic.find(tacKey(ScoutTac, BWAPI::Positions::None))->second);
	return scoutTac->HasZergling();
}

bool TacticManager::isAssignPatrolOverlord() {
	PatrolTactic* patrolTac = dynamic_cast<PatrolTactic*>(myTactic.find(tacKey(PatrolTac, BWAPI::Positions::None))->second);
	return patrolTac->hasOverlord();
}


int TacticManager::addDefendArmyType(BattleTactic* tac, BWAPI::UnitType addArmyType, int needArmy, BWAPI::Position defendPosition)
{
	if (needArmy <= 0)
	{
		return needArmy;
	}
	int Remain = 0;
	int Send = 0;
	Remain = tac->getArmy()[addArmyType]->getUnits().size();
	int armySupply = addArmyType.supplyRequired();
	Send = needArmy / armySupply < Remain ? needArmy / armySupply : Remain;

	addTacticArmy(DefendTactic, defendPosition, tac->getArmy(), addArmyType, Send);
	needArmy -= Send * armySupply;
	return needArmy;
}
