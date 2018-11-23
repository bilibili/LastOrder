#include "PatrolTactic.h"



PatrolTactic::PatrolTactic()
{
	for (auto army : tacticArmy)
	{
		if (army.second->getUnits().size() > 0)
		{
			for (auto armyUnit : army.second->getUnits())
			{
				overLordIdle.push_back(Patrol(armyUnit.unit));
			}
		}
	}
	if (!InformationManager::Instance().isHasNatrualBase())
		state = watchStartBase;
	else if (Broodwar->self()->getUpgradeLevel(UpgradeTypes::Pneumatized_Carapace) == 0 || Broodwar->getFrameCount() < startScoutTime)
		state = watchNaturalBase;
	else
		state = scoutOtherBase;
}

Position PatrolTactic::nextMovePosition() {
	BWAPI::Position returnValue = Positions::None;
	switch (state)
	{
	case PatrolTactic::watchStartBase:
		if (patrolPositions.size() == 0) {
			//auto const& cps = BWEMMap.GetArea(Broodwar->self()->getStartLocation())->ChokePoints();
			//for (auto const& c : cps) {
			//	if (!c->Blocked()) {
			//		patrolPositions.push_back(patrolTarget(TilePosition(c->Center())));
			//	}
			//
			patrolPositions.push_back(patrolTarget(TilePosition(InformationManager::Instance().getOurBaseToNatrualChokePosition())));
		}
		returnValue = Position(patrolPositions.back().location);
		patrolPositions.pop_back();
		if (InformationManager::Instance().isHasNatrualBase()) {
			state = watchNaturalBase;
			patrolPositions.clear();
		}
		break;
	case PatrolTactic::watchNaturalBase:
		if (patrolPositions.size() == 0) {
			auto const& cps = BWEMMap.GetArea(Broodwar->self()->getStartLocation())->ChokePoints();
			for (auto const& c : cps) {
				if (!c->Blocked()) {
					patrolPositions.push_back(patrolTarget(TilePosition(c->Center())));
				}
			}
			auto const& cps_n = BWEMMap.GetArea(InformationManager::Instance().getOurNatrualLocation())->ChokePoints();
			for (auto const& c : cps_n) {
				if (!c->Blocked() && Position(c->Center()) != InformationManager::Instance().getOurBaseToNatrualChokePosition()) {
					patrolPositions.push_back(patrolTarget(TilePosition(c->Center())));
				}
			}
		}
		returnValue = Position(patrolPositions.back().location);
		patrolPositions.pop_back();
		if (Broodwar->self()->getUpgradeLevel(UpgradeTypes::Pneumatized_Carapace) > 0 && Broodwar->getFrameCount() >= startScoutTime) {
			state = scoutOtherBase;
			patrolPositions.clear();
		}
		break;
	case PatrolTactic::scoutOtherBase:
		if (patrolPositions.size() == 0) {
			TilePosition enemyBase = TilePosition(InformationManager::Instance().GetEnemyBasePosition());
			std::map<const Area*, TilePosition>& myRegion = InformationManager::Instance().getBaseOccupiedRegions(BWAPI::Broodwar->self());
			for (auto const& a : BWEMMap.Areas())
			{
				if (a.AccessibleNeighbours().empty())
					continue;
				if (myRegion.find(&a) == myRegion.end())
					for (auto const& b : a.Bases()) {
						TilePosition basePosition = b.Location() + TilePosition(2, 1);
						patrolPositions.push_back(patrolTarget(basePosition, -basePosition.getDistance(enemyBase)));
					}
			}
			std::sort(patrolPositions.begin(), patrolPositions.end());
		}
		if (patrolPositions.size() == 0) {
			return Positions::None;
		}
		returnValue = Position(patrolPositions.back().location);
		patrolPositions.pop_back();
		break;
	}
	return returnValue;
}


void PatrolTactic::assignPatrolTask() {
	for (std::vector<Patrol>::iterator it = overLordIdle.begin(); it != overLordIdle.end();) {
		it->nextMovePosition = nextMovePosition();
		if (it->nextMovePosition != Positions::None) {
			overLordPatrols.push_back(*it);
			it = overLordIdle.erase(it);
		}
		else {
			it++;
		}
	}
}

void PatrolTactic::update()
{
	assignPatrolTask();
	for (std::vector<Patrol>::iterator it = overLordPatrols.begin(); it != overLordPatrols.end();) {
		if (Broodwar->isVisible(TilePosition(it->nextMovePosition)))
			//(it->patrolUnit->getPosition().getDistance(it->nextMovePosition) < 5 * 32 || Broodwar->isVisible(TilePosition(it->nextMovePosition)))
		{
			if (state == watchStartBase || state == watchNaturalBase)
			{
				if (it->patrolUnit->getPosition().getDistance(it->nextMovePosition) > 32)
				{
					BattleArmy::smartMove(it->patrolUnit, it->nextMovePosition);
					it++;
				}
				else
				{
					overLordIdle.push_back(*it);
					it = overLordPatrols.erase(it);
				}
			}
			else
			{
				overLordIdle.push_back(*it);
				it = overLordPatrols.erase(it);
			}
		}
		else {
			if (state == watchStartBase || state == watchNaturalBase) {
				BattleArmy::smartMove(it->patrolUnit, it->nextMovePosition);
				//Broodwar->drawLineMap(it->patrolUnit->getPosition(), it->nextMovePosition, BWAPI::Colors::Red);
			}
			else {
				Position nextMove = Astar::Instance().getAirPath(it->patrolUnit, it->nextMovePosition);
				BattleArmy::smartMove(it->patrolUnit, nextMove);
				//Broodwar->drawLineMap(it->patrolUnit->getPosition(), nextMove, BWAPI::Colors::Red);
			}
			it++;
		}
	}
}

void PatrolTactic::onUnitDestroy(BWAPI::Unit unit)
{
	for (std::vector<Patrol>::iterator it = overLordIdle.begin(); it != overLordIdle.end(); it++)
	{
		if (it->patrolUnit == unit) {
			overLordIdle.erase(it);
			break;
		}
	}
	for (std::vector<Patrol>::iterator it = overLordPatrols.begin(); it != overLordPatrols.end(); it++)
	{
		if (it->patrolUnit == unit) {
			overLordPatrols.erase(it);
			break;
		}
	}
}

