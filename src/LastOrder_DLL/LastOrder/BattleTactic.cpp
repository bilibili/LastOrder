#pragma once
#include "BattleTactic.h"
#include "InformationManager.h"

void BattleTactic::updateArmyInfo(bool usingRegroup)
{
	centerPosition.x = 0;
	centerPosition.y = 0;
	ourArmyCount = 0;
	groundArmyCount = 0;
	hasNearbyEnemy = false;
	for (auto const& army : tacticArmy)
	{
		if (!army.first.isFlyer())
		{
			groundArmyCount += army.second->getUnits().size() * army.first.supplyRequired();
		}
		army.second->setRegroupPosition(Positions::None);
		army.second->setRegrouping(false);
		army.second->setRegroupFrontLine(-1);
		if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
		{
			for (std::vector<UnitState>::iterator it = army.second->getUnits().begin();
				it != army.second->getUnits().end();)
			{
				if (it->unit->isIrradiated()) {
					Unit irradiatedUnit = it->unit;
					it = army.second->getUnits().erase(it);
					if (irradiatedUnit->isFlying()) {
						irradiatedUnit->move(Position(0, 0));
					}
					else {
						irradiatedUnit->move(InformationManager::Instance().GetEnemyBasePosition());
					}
				}
				else if (it->unit->exists())
				{
					ourArmyCount += 1;
					centerPosition.x += it->unit->getPosition().x;
					centerPosition.y += it->unit->getPosition().y;
					if (!hasNearbyEnemy) {
						map<UnitType, set<Unit>> enemyUnits =
							InformationManager::Instance().getUnitGridMap().GetUnits(it->unit->getTilePosition(), 7, Broodwar->enemy(),false);
						if (enemyUnits.size() > 0) {
							hasNearbyEnemy = true;
						}
					}
					it++;
				}
				else
				{
					it = army.second->getUnits().erase(it);
				}
			}
		}
	}
	if (ourArmyCount == 0)
		return;

	centerPosition.x = centerPosition.x / ourArmyCount;
	centerPosition.y = centerPosition.y / ourArmyCount;

	firstGroundUnit = nullptr;
	int minDistance = 999999;
	for (auto const& army : tacticArmy)
	{
		army.second->setCenterPosition(centerPosition);
		army.second->setNearByEnemy(hasNearbyEnemy);
		if (army.first.isFlyer())
			continue;
		if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
		{
			for (auto const& u : army.second->getUnits())
			{
				if (u.unit->getDistance(centerPosition) < minDistance)
				{
					minDistance = u.unit->getDistance(centerPosition);
					firstGroundUnit = u.unit;
				}
			}
		}
	}
	//Broodwar->drawCircleMap(centerPosition, 16, Colors::Yellow, true);
	if (!usingRegroup)
		return;
	
	int maxTargetDistance = -1, minTargetDistance = 999999;
	Position minDistancePos = Positions::None;
	bool hasZergling = false, hasUltraLisk = false;
	for (auto const& army : tacticArmy)
	{
		if (army.first.isFlyer())
			continue;
		int armySize = army.second->getUnits().size();
		if (armySize == 0)
			continue;
		if (army.first == UnitTypes::Zerg_Zergling)
			hasZergling = true;
		else if (army.first == UnitTypes::Zerg_Ultralisk)
			hasUltraLisk = true;
		std::vector<std::pair<int,Position>> distanceInfo;
		distanceInfo.reserve(armySize);
		map<Unit, int> groundDistanceSet;
		for (auto const& u : army.second->getUnits()) {
			int pLongth = 0;
			const CPPath& path = BWEMMap.GetPath(u.unit->getPosition(), attackPosition, &pLongth);
			//int pLongth = u.unit->getPosition().getDistance(attackPosition);
			groundDistanceSet[u.unit] = pLongth;
			distanceInfo.push_back(std::make_pair(pLongth, u.unit->getPosition()));
			if (pLongth > maxTargetDistance) {
				maxTargetDistance = pLongth;
			}
			if (pLongth < minTargetDistance) {
				minTargetDistance = pLongth;
				minDistancePos = u.unit->getPosition();
			}
		}
		army.second->setGroundDistanceSet(groundDistanceSet);
		std::sort(distanceInfo.begin(), distanceInfo.end());
		//int curID = std::min(10, armySize / 5);
		int curID = std::min(3, armySize / 5);
		if (army.first == UnitTypes::Zerg_Zergling)
			curID = std::min(6, armySize / 20);
		Position curRegroupPos = distanceInfo[curID].second;
		int lastID = armySize * 4 / 5;
		if (army.first == UnitTypes::Zerg_Ultralisk)
			lastID = armySize - 1;
		if (army.first == UnitTypes::Zerg_Zergling)
			lastID = armySize / 2;
		Position lastPos = distanceInfo[lastID].second;
		int lastPosDis = distanceInfo[lastID].first;
		army.second->setRegroupPosition(curRegroupPos);
		army.second->setLastPosition(lastPos);
		army.second->setlastLineDistance(lastPosDis);
	}
	bool tacticNeedRegroup = false;
	//Broodwar->printf("%d", maxTargetDistance - minTargetDistance);
	// dis=(army/10)+4, minimum is 5(army=10) and maximum is 15(army=110);
	int maxHeadTailDistance = ourArmyCount / 10 + 4;
	maxHeadTailDistance = (maxHeadTailDistance < 7 ? 7 : maxHeadTailDistance);
	maxHeadTailDistance = (maxHeadTailDistance > 15 ? 15 : maxHeadTailDistance);
	if (maxTargetDistance != -1 && maxTargetDistance - minTargetDistance > maxHeadTailDistance * 32) {
		tacticNeedRegroup = true;
	}

	if (tacticNeedRegroup == false)
		return;

	Position lastEndPosition = Positions::None;
	int lastEndDis = -1;
	// if has ultralisk, all unit go behind it
	if (hasUltraLisk) {
		lastEndPosition = tacticArmy[UnitTypes::Zerg_Ultralisk]->getLastPosition();
		lastEndDis = tacticArmy[UnitTypes::Zerg_Ultralisk]->getlastLineDistance();
	}
	// if has zergling, all unit except ultralisk go behind it
	if (hasZergling) {
		if (hasUltraLisk) {
			tacticArmy[UnitTypes::Zerg_Zergling]->setRegroupPosition(lastEndPosition);
			tacticArmy[UnitTypes::Zerg_Zergling]->setRegroupFrontLine(lastEndDis);
			lastEndPosition = tacticArmy[UnitTypes::Zerg_Zergling]->getLastPosition();
			lastEndDis = tacticArmy[UnitTypes::Zerg_Zergling]->getlastLineDistance();
		}
		else {
			lastEndPosition = tacticArmy[UnitTypes::Zerg_Zergling]->getLastPosition();
			lastEndDis = tacticArmy[UnitTypes::Zerg_Zergling]->getlastLineDistance();
		}
	}
	// if has zergling, all unit except ultralisk go behind it
	for (auto const& army : tacticArmy) {
		army.second->setRegrouping(true);
		if (army.first == UnitTypes::Zerg_Ultralisk || army.first == UnitTypes::Zerg_Zergling)
			continue;
		if (lastEndDis != -1) {
			// has zergling or ultralisk, place all units behind them
			army.second->setRegroupPosition(lastEndPosition);
			army.second->setRegroupFrontLine(lastEndDis);
		}
		else {
			army.second->setRegroupPosition(minDistancePos);
		}
	}
	
}

bool BattleTactic::isTacticEnd()
{
	if (state == END)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void BattleTactic::setAttackPosition(BWAPI::Position targetPosition, tacticType tactic)
{
	originAttackPosition = attackPosition = targetPosition;
}


bool BattleTactic::attackPositionHasEnemy()
{
	if (!BWAPI::Broodwar->isVisible(BWAPI::TilePosition(attackPosition)))
		return true;
	if (BWAPI::Broodwar->isVisible(BWAPI::TilePosition(attackPosition)))
	{
		Unitset enemySet = Broodwar->getUnitsOnTile(BWAPI::TilePosition(attackPosition));
		for (auto& eU : enemySet)
		{
			if (eU->getPlayer() == Broodwar->enemy() && eU->getType().isBuilding())
			{
				return true;
			}
		}
	}

	std::map<const Area*, std::map<BWAPI::Unit, buildingInfo>>& occupiedDetail = InformationManager::Instance().getEnemyOccupiedDetail();
	if (occupiedDetail.find(BWEMMap.GetArea(TilePosition(attackPosition))) != occupiedDetail.end())
	{
		std::map<BWAPI::Unit, buildingInfo >& buildingsDetail = occupiedDetail[BWEMMap.GetArea(TilePosition(attackPosition))];
		for (auto const& b : buildingsDetail)
		{
			logInfo("BattleTactic", "has no target, change attackPosition "
				+ to_string(b.second.initPosition.x) + " " + to_string(b.second.initPosition.y));
			attackPosition = Position(b.second.initPosition);
			return true;
		}
	}
	return false;

	// has no target to attack
	//if (!hasTargetToAttack)
	//{
	//	std::map<const Area*, std::map<BWAPI::Unit, buildingInfo>>& occupiedDetail = InformationManager::Instance().getEnemyOccupiedDetail();
	//	if (occupiedDetail.find(BWEMMap.GetArea(TilePosition(attackPosition))) != occupiedDetail.end())
	//	{
	//		std::map<BWAPI::Unit, buildingInfo >& buildingsDetail = occupiedDetail[BWEMMap.GetArea(TilePosition(attackPosition))];
	//		for (auto const& b : buildingsDetail)
	//		{
	//			logInfo("BattleTactic", "has no target, change attackPosition "
	//				+ to_string(b.second.initPosition.x) + " " + to_string(b.second.initPosition.y));
	//			attackPosition = Position(b.second.initPosition);
	//			return true;
	//		}
	//	}
	//	return false;
	//}
	//else
	//{
	//	return true;
	//}
}


void BattleTactic::addArmyUnit(BWAPI::Unit unit)	
{
	if (tacticArmy.find(unit->getType()) != tacticArmy.end())
	{
		tacticArmy[unit->getType()]->addUnit(unit);
	}
}


BattleTactic::BattleTactic()
{
	tacticArmy[BWAPI::UnitTypes::Zerg_Zergling] = new ZerglingArmy();
	tacticArmy[BWAPI::UnitTypes::Zerg_Mutalisk] = new MutaliskArmy();
	tacticArmy[BWAPI::UnitTypes::Zerg_Hydralisk] = new HydraliskArmy();
	tacticArmy[BWAPI::UnitTypes::Zerg_Overlord] = new OverLordArmy();
	tacticArmy[BWAPI::UnitTypes::Zerg_Lurker] = new LurkerArmy();
	tacticArmy[BWAPI::UnitTypes::Zerg_Scourge] = new ScourgeArmy();
	tacticArmy[BWAPI::UnitTypes::Zerg_Ultralisk] = new UltraliskArmy();
	tacticArmy[BWAPI::UnitTypes::Zerg_Devourer] = new DevourerArmy();
	tacticArmy[BWAPI::UnitTypes::Zerg_Guardian] = new GuardianArmy();
	
	accumulatRetreatFrame = 0;
	isRetreat = false;
	hasNearbyEnemy = false;
	noEnemyAccumulateFrame = 0;
}


BattleTactic::~BattleTactic()
{
	for (auto& army : tacticArmy)
	{
		delete army.second;
	}
}


void BattleTactic::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit == NULL || tacticArmy.find(unit->getType()) == tacticArmy.end())
		return;

	tacticArmy[unit->getType()]->removeUnit(unit);
}

void BattleTactic::onLurkerMorph()
{
	std::vector<UnitState>& army = tacticArmy[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits();
	for (std::vector<UnitState>::iterator it = army.begin(); it != army.end(); it++)
	{
		if (it->unit->isMorphing())
		{
			army.erase(it);
			break;
		}
	}
}

void BattleTactic::setRetreat()
{
	attackPosition = InformationManager::Instance().getRetreatDestination();
	isRetreat = true;
}




