#include "BattleArmy.h"


map<string, vector<string>> BattleArmy::armyTypeCounterDict = decltype(armyTypeCounterDict)
{
	{ "ground_groundWeapon", { "ground_groundWeapon", "ground_allWeapon" }},
	{ "ground_airWeapon", { "air_groundWeapon", "air_allWeapon" } },
	{ "ground_allWeapon", { "ground_groundWeapon", "ground_allWeapon", "air_groundWeapon", "air_allWeapon" } },

	{ "underGround_groundWeapon", { "ground_groundWeapon", "ground_allWeapon" } },

	{ "air_groundWeapon", { "ground_airWeapon", "ground_allWeapon" } },
	{ "air_airWeapon", { "air_airWeapon", "air_allWeapon" } },
	{ "air_allWeapon", { "ground_airWeapon", "ground_allWeapon", "air_airWeapon", "air_allWeapon" } }
};

map<string, vector<string>> BattleArmy::armyTypeOverwhelmDict = decltype(armyTypeOverwhelmDict)
{
	{ "ground_groundWeapon", { "ground_airWeapon" }},
	{ "ground_airWeapon", { "air_airWeapon" } },
	{ "ground_allWeapon", { "ground_airWeapon", "air_airWeapon" } },

	{ "underGround_groundWeapon", { "ground_airWeapon" } },

	{ "air_groundWeapon", { "ground_groundWeapon" } },
	{ "air_allWeapon", { "ground_groundWeapon", "air_groundWeapon" } }
};

map<UnitType, string> BattleArmy::armyToType = decltype(armyToType)
{
	{ BWAPI::UnitTypes::Zerg_Ultralisk, "ground_groundWeapon" },
	{ BWAPI::UnitTypes::Zerg_Mutalisk, "air_allWeapon" },
	{ BWAPI::UnitTypes::Zerg_Zergling, "ground_groundWeapon" },
	{ BWAPI::UnitTypes::Zerg_Lurker, "underGround_groundWeapon" },
	{ BWAPI::UnitTypes::Zerg_Hydralisk, "ground_allWeapon" },
	{ BWAPI::UnitTypes::Zerg_Scourge, "air_airWeapon" },
	{ BWAPI::UnitTypes::Zerg_Devourer, "air_airWeapon" },
	{ BWAPI::UnitTypes::Zerg_Guardian, "air_groundWeapon" }
};


int BattleArmy::sightRadius = 10;
bool BattleArmy::needLog = false;
Unit BattleArmy::logUnit = nullptr;


BattleArmy::BattleArmy()
{
	units.reserve(100);

	int lookAheadStep = 3;
	for (int x = -1 * lookAheadStep; x <= lookAheadStep; x++)
	{
		for (int y = -1 * lookAheadStep; y <= lookAheadStep; y++)
		{
			//internal node
			if (x != -1 * lookAheadStep && x != lookAheadStep &&
				y != -1 * lookAheadStep && y != lookAheadStep)
				continue;
			moveDirections.push_back(std::pair<int, int>(x, y));
		}
	}
	
	movePositionOrder = { BWAPI::UnitTypes::Zerg_Ultralisk, BWAPI::UnitTypes::Zerg_Mutalisk, BWAPI::UnitTypes::Zerg_Zergling,
		BWAPI::UnitTypes::Zerg_Lurker, BWAPI::UnitTypes::Zerg_Hydralisk, BWAPI::UnitTypes::Zerg_Devourer,
		BWAPI::UnitTypes::Zerg_Guardian, BWAPI::UnitTypes::Zerg_Scourge };
	needRegroupFlag = false;
	hasNearbyEnemy = false;
	regroupFrontLine = -1;
}


int	BattleArmy::onlyGroundAttackPriority(BWAPI::Unit unit)
{
	BWAPI::UnitType type = unit->getType();

	if (type == BWAPI::UnitTypes::Zerg_Egg || type == BWAPI::UnitTypes::Zerg_Larva)
	{
		return 0;
	}

	if (type == UnitTypes::Terran_Bunker)
	{
		return 12;
	}

	// highest priority is something that can attack us or aid in combat
	if (type.groundWeapon() != BWAPI::WeaponTypes::None && !type.isFlyer() 
		|| type == UnitTypes::Protoss_Reaver)
	{
		return 12;
	}
	else if (type.isSpellcaster() && !type.isFlyer())
	{
		return 11;
	}
	else if (type.airWeapon() != BWAPI::WeaponTypes::None && !type.isFlyer())
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
		return 7;
	}
	else if (type.isRefinery())
	{
		return 6;
	}
	else if (type.isResourceDepot())
	{
		return 5;
	}
	else if (type.isBuilding())
	{
		return 4;
	}
	// next is buildings that cost gas
	else if (type.gasPrice() > 0)
	{
		return 3;
	}
	else if (type.mineralPrice() > 0)
	{
		return 2;
	}
	// then everything else
	else
	{
		return 1;
	}
}


int	BattleArmy::onlyAirAttackPriority(BWAPI::Unit unit)
{
	BWAPI::UnitType type = unit->getType();

	// highest priority is something that can attack us or aid in combat
	if ((type.airWeapon() != BWAPI::WeaponTypes::None) && type.isFlyer())
	{
		return 12;
	}
	else if ((type.groundWeapon() != BWAPI::WeaponTypes::None) && type.isFlyer())
	{
		return 11;
	}
	else if (type.isFlyer())
	{
		return 10;
	}
	else
	{
		return 1;
	}
}


int	BattleArmy::getAttackPriority(BWAPI::Unit unit)
{
	BWAPI::UnitType curType = units.front().unit->getType();
	if (curType.groundWeapon() == BWAPI::WeaponTypes::None)
	{
		return onlyAirAttackPriority(unit);
	}
	else if (curType.airWeapon() == BWAPI::WeaponTypes::None)
	{
		return onlyGroundAttackPriority(unit);
	}
	else
	{
		return groundAirAttackPriority(unit);
	}
}


int BattleArmy::groundAirAttackPriority(BWAPI::Unit unit)
{
	BWAPI::UnitType type = unit->getType();
	if (type == BWAPI::UnitTypes::Zerg_Egg || type == BWAPI::UnitTypes::Zerg_Larva)
	{
		return 0;
	}

	if ((type.airWeapon() != BWAPI::WeaponTypes::None && type != UnitTypes::Protoss_Interceptor) ||
		type == BWAPI::UnitTypes::Protoss_Carrier || type == UnitTypes::Terran_Bunker)
	{
		return 12;
	}
	else if (type.groundWeapon() != BWAPI::WeaponTypes::None || type == UnitTypes::Protoss_Reaver)
	{
		return 11;
	}
	// next is special buildings
	else if (type == BWAPI::UnitTypes::Protoss_Pylon || type == BWAPI::UnitTypes::Zerg_Spire)
	{
		return 10;
	}
	// next is buildings that cost gas
	else if (type.gasPrice() > 0)
	{
		return 9;
	}
	else if (type.mineralPrice() > 0)
	{
		return 8;
	}
	// then everything else
	else
	{
		return 1;
	}
}


bool BattleArmy::targetFilter(UnitType type)
{
	UnitType curType = units.front().unit->getType();
	if (curType.isFlyer())
	{
		if (curType.groundWeapon() == WeaponTypes::None)
		{
			if (curType.airWeapon() != WeaponTypes::None)
				return type.isFlyer();
			else
				return false;
		}
		else
		{
			if (curType.airWeapon() != WeaponTypes::None)
				return true;
			else
				return !type.isFlyer();
		}
	}
	else
	{
		if (curType.airWeapon() == WeaponTypes::None)
		{
			if (curType.groundWeapon() != WeaponTypes::None)
				return !type.isFlyer();
			else
				return false;
		}
		else
		{
			if (curType.groundWeapon() != WeaponTypes::None)
				return true;
			else
				return type.isFlyer();
		}
	}
}


void BattleArmy::parsedArmyScore(const map<UnitType, set<Unit>>& units, map<string, std::pair<int, int>>& result, bool hasDetector, bool isOur)
{
	if (isOur)
	{
		for (auto const& u : units)
		{
			//special case for our defend building
			if (u.first.isBuilding())
			{
				if (u.first.canAttack() == false)
					continue;
				for (auto & unit : u.second)
				{
					if (isUnitAttacking(unit))
						armyTypeSaveFunc(unit, u.first, 1, result, hasDetector);
				}
			}
			else
			{
				if (u.first.isWorker())
					continue;
				armyTypeSaveFunc(*u.second.begin(), u.first, u.second.size(), result, hasDetector);
			}
		}
	}
	else
	{
		for (auto const& u : units)
		{
			if (u.first.isBuilding() || u.first.isWorker())
				continue;
			armyTypeSaveFunc(*u.second.begin(), u.first, u.second.size(), result, hasDetector);
		}
	}
}


//for different group of muta to check weather to attack
std::pair<int, set<Unit>> BattleArmy::getCanAttackTargets(const UnitState& curUnit, const map<UnitType, set<Unit>>& ourUnits,
	const map<UnitType, set<Unit>>& enemyUnits, map<Unit, int>& unitRetreatInfo, 
	bool ourHasDetector, bool enemyHasDetector, tacticType curTactic)
{
	auto returnFunc = [&](int retreatFlag)
	{
		set<Unit> allTargets;
		if (retreatFlag != 2) {
			for (auto& army : enemyUnits)
			{
				allTargets.insert(army.second.begin(), army.second.end());
			}
		}
		else {
			for (auto const& army : enemyUnits)
			{
				for (auto const& u : army.second) {
					TilePosition tps = TilePosition(u->getPosition());
					if (InformationManager::Instance().getEnemyInfluenceMap(tps.x, tps.y).strongAirForce > 0) {
						Broodwar->drawCircleMap(u->getPosition(), 10, Colors::Brown, false);
					}
					else {
						Broodwar->drawCircleMap(u->getPosition(), 10, Colors::Cyan, false);
						allTargets.insert(u);
					}
				}
			}
		}
		return std::pair<int, set<Unit>>(retreatFlag, allTargets);
	};

	if (unitRetreatInfo.find(curUnit.unit) != unitRetreatInfo.end())
	{
		int retreatFlag = unitRetreatInfo[curUnit.unit];
		return returnFunc(retreatFlag);
	}

	set<string> overWhelmEnemyArmy;
	map<string, std::pair<int, int>> ourArmyTypeScoreDict;
	parsedArmyScore(ourUnits, ourArmyTypeScoreDict, enemyHasDetector, true);

	//only has cannon under fog
	if (enemyUnits.empty())
	{
		map<string, std::pair<int, int>> tmpScore = InformationManager::Instance().getMaxInfluenceMapValue(curUnit.unit->getTilePosition(), sightRadius);
		int retreatFlag = needRetreat(ourArmyTypeScoreDict, tmpScore, overWhelmEnemyArmy, ourHasDetector, enemyHasDetector);
		//can only be 2/0
		return std::pair<int, set<Unit>>(retreatFlag, set<Unit>());
	}
	else
	{
		map<string, std::pair<int, int>> enemyArmyTypeScoreDict;
		parsedArmyScore(enemyUnits, enemyArmyTypeScoreDict, ourHasDetector, false);

		int maxCount = 0;
		TilePosition maxPosition;
		for (auto const& army : enemyUnits)
		{
			for (auto const& u : army.second)
			{
				int tmpCount = 0;
				TilePosition centerPosition = TilePosition(u->getPosition());
				gridInfo& cell = InformationManager::Instance().getEnemyInfluenceMap(centerPosition.x, centerPosition.y);
				if (!cell.buildingScore.empty())
				{
					map<string, std::pair<int, int>>& tmpScore = cell.buildingScore;
					for (auto& b : tmpScore)
					{
						tmpCount += b.second.second;
					}
					if (tmpCount > maxCount)
					{
						maxCount = tmpCount;
						maxPosition = centerPosition;
					}
				}
			}
		}
		map<string, std::pair<int, int>> maxCannon = InformationManager::Instance().getEnemyInfluenceMap(maxPosition.x, maxPosition.y).buildingScore;
		for (auto& item : maxCannon)
		{
			enemyArmyTypeScoreDict[item.first].second += item.second.second;
		}
		int retreatFlag = needRetreat(ourArmyTypeScoreDict, enemyArmyTypeScoreDict, overWhelmEnemyArmy, ourHasDetector, enemyHasDetector, curTactic);
		
		//if (retreatFlag == 1 && curTactic == MutaliskHarassTac && curUnit.unit->getType() == UnitTypes::Zerg_Mutalisk) {
		//	for (auto& item : maxCannon)
		//	{
		//		if ((item.first == "ground_allWeapon" || item.first == "ground_airWeapon")) {
		//			enemyArmyTypeScoreDict[item.first].second -= item.second.second;
		//		}
		//	}
		//	set<string> overWhelmEnemyArmy2;
		//	int seigeFlag = needRetreat(ourArmyTypeScoreDict, enemyArmyTypeScoreDict, overWhelmEnemyArmy2, ourHasDetector, enemyHasDetector, curTactic);
		//	if (seigeFlag == 0) {
		//		retreatFlag = 2;
		//	}
		//}

		//share retreat info to all nearby units
		if (retreatFlag > 0) {
			for (auto& a : ourUnits)
			{
				// in mutalisk harass, the state of scourge does not broadcast to mutalisk
				if (curTactic == MutaliskHarassTac && curUnit.unit->getType() == UnitTypes::Zerg_Scourge && a.first == UnitTypes::Zerg_Mutalisk)
					continue;
				if (curUnit.unit->getType() == UnitTypes::Zerg_Scourge)
					continue;

				for (auto& u : a.second)
				{
					if (curUnit.unit->getPosition().getDistance(u->getPosition()) < 4 * 32) {
						unitRetreatInfo[u] = retreatFlag;
					}
				}
			}
		}

		return returnFunc(retreatFlag);
	}
}


Unit BattleArmy::chooseTarget(UnitState& unit, set<Unit>& units, map<Unit, int>& assignTargets, map<Unit, Unit>& kiteAssign)
{
	UnitType curType = unit.unit->getType();
	int bestScore = -999999;
	Unit bestTarget = nullptr;
	Unit rangedUnit = unit.unit;
	Unit defaultTarget = nullptr;
	for (auto const& target : units)
	{
		if (target->isVisible() &&
			target->isDetected() &&
			target->getType() != BWAPI::UnitTypes::Zerg_Larva &&
			target->getType() != BWAPI::UnitTypes::Zerg_Egg &&
			targetFilter(target->getType()))
		{
			int priority = getAttackPriority(target);     // 0..12
			int range = rangedUnit->getDistance(target);           // 0..map size in pixels
			int hasBeenTargets = 0;

			if (assignTargets.find(target) != assignTargets.end())
			{
				int defaultLowValue = -20;
				if (target->getType().isBuilding())
				{
					defaultLowValue = -50;
				}
				//can be a target
				if (assignTargets[target] > defaultLowValue)
				{
					// Let's say that 1 priority step/has been target is worth 160 pixels (5 tiles).
					if (curType.airWeapon() != WeaponTypes::None)
					{
						hasBeenTargets = 2 * 32;
					}
					else
					{
						//hasBeenTargets = 32;
						hasBeenTargets = 0;
					}
				}
				else
				{
					//can also be a dafault target except for scourge
					//if (unit.unit->getType() == UnitTypes::Zerg_Scourge)
					//continue;
					continue;
				}
			}
			if (unit.underKiteUnit != nullptr && unit.underKiteUnit->exists()) {
				if (kiteAssign.find(target) != kiteAssign.end() && kiteAssign[target] != unit.unit) {
					continue;
				}
			}

			int score = 0;
			if (curType.isFlyer())
			{
				// Let's say that 1 priority step is worth (10 tiles).
				// keep range to attack closer enemy first
				score += 10 * 32 * priority - range + hasBeenTargets;

				/*
				int curHP = target->getHitPoints();
				if (curHP <= 15)
				{
				score += 3 * 32;
				}
				*/

				// the target lose every 40% hp, the distance is supposed to be closer as 1 tile.
				int hpPercent = int((1.0*(target->getHitPoints() + target->getShields() - 1) / (target->getType().maxHitPoints() + target->getType().maxShields()))*100.0);
				score += (2 - hpPercent / 40) * 32;
				// If hp is too low, the priority should be small to avoid wasting too much fire
				if (target->getHitPoints() <= 5) {
					score -= 1 * 32;
				}
			}
			else if (curType.airWeapon() != WeaponTypes::None)
			{
				score += 2 * 32 * priority - range + hasBeenTargets;
			}
			else
			{
				score += 32 * priority - range + hasBeenTargets;
			}

			if (score > bestScore)
			{
				bestScore = score;
				bestTarget = target;
			}
		}
	}

	if (unit.unit->getType() == UnitTypes::Zerg_Zergling && unit.underKiteUnit != nullptr && unit.underKiteUnit->exists()) {
		if (kiteAssign.find(unit.underKiteUnit) == kiteAssign.end()) {
			bestTarget = unit.underKiteUnit;
			kiteAssign[bestTarget]=unit.unit;
			//Broodwar->drawLineMap(unit.unit->getPosition(),bestTarget->getPosition(),Colors::Cyan);
		}
	}

	if (bestTarget != nullptr && !BattleArmy::needLog)
	{
		if (assignTargets.find(bestTarget) == assignTargets.end())
		{
			assignTargets[bestTarget] = bestTarget->getHitPoints() + bestTarget->getShields();
		}
		assignTargets[bestTarget] = assignTargets[bestTarget] - rangedUnit->getType().airWeapon().damageAmount();
	}
	return bestTarget;
}


std::pair<int, Unit> BattleArmy::assignTarget(UnitState& unit, map<Unit, int>& assignTargets, map<Unit, int>& unitRetreatInfo,
	int ourArmyRadius, tacticType curTactic, Position attackPosition, set<BWAPI::Unit>& defendEnemy, map<UnitType, set<Unit>>& encounteredEnemy, map<Unit, Unit>& kiteAssign)
{
	bool commonLog = false;
	if (unit.unit == BattleArmy::logUnit && Broodwar->getFrameCount() % 24 == 0 || BattleArmy::needLog)
		commonLog = true;

	//all visual units and buildings
	if (curTactic == MutaliskHarassTac)
	{
		ourArmyRadius = 10;
	}
	else
	{
		//zergling army is cheaper
		ourArmyRadius = 10;
	}
	map<UnitType, set<Unit>> ourUnits =
		InformationManager::Instance().getUnitGridMap().GetUnits(unit.unit->getTilePosition(), ourArmyRadius, Broodwar->self(), commonLog);
	map<UnitType, set<Unit>> enemyUnits =
		InformationManager::Instance().getUnitGridMap().GetUnits(unit.unit->getTilePosition(), 12, Broodwar->enemy(), commonLog);
	if (curTactic == DefendTactic && defendEnemy.size() > 0)
	{
		for (auto& u : defendEnemy)
		{
			enemyUnits[u->getType()].insert(u);
		}
	}

	bool ourHasDetector = false;
	bool enemyHasDetector = false;
	int attackingSunken = 0;
	int allSunken = 0;
	int attackingSpore = 0;

	bool hasDefender = false;
	for (auto& u : ourUnits)
	{
		if (checkDetectorFunc(u.first))
		{
			ourHasDetector = true;
		}
		if (u.first == UnitTypes::Zerg_Sunken_Colony)
		{
			hasDefender = true;
		}
		//exclude retreating unit
		for (auto it = u.second.begin(); it != u.second.end();)
		{
			if (u.first == UnitTypes::Zerg_Sunken_Colony)
			{
				allSunken += 1;
				if (isUnitAttacking(*it))
				{
					attackingSunken += 1;
				}
			}
			if (u.first == UnitTypes::Zerg_Spore_Colony)
			{
				if (isUnitAttacking(*it))
				{
					attackingSpore += 1;
				}
			}

			if (unitRetreatInfo.find(*it) != unitRetreatInfo.end() && unitRetreatInfo[*it] == 1)
			{
				it = u.second.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

	bool hasLongAttackRangeEnemy = false;
	bool hasGroundAttacker = false;
	bool hasAirAttacker = false;
	bool nearDefendPosition = false;
	for (auto& u : enemyUnits)
	{
		encounteredEnemy[u.first].insert(u.second.begin(), u.second.end());

		if (checkDetectorFunc(u.first))
		{
			enemyHasDetector = true;
		}
		if (u.first == UnitTypes::Terran_Siege_Tank_Siege_Mode ||
			u.first == UnitTypes::Protoss_Reaver ||
			u.first == UnitTypes::Protoss_Carrier)
		{
			hasLongAttackRangeEnemy = true;
		}

		if (!u.first.isFlyer() && u.first.groundWeapon() != WeaponTypes::None)
		{
			hasGroundAttacker = true;
		}
		if (u.first.isFlyer() && (u.first.groundWeapon() != WeaponTypes::None || u.first.airWeapon() != WeaponTypes::None ))
		{
			hasAirAttacker = true;
		}

		if (curTactic == DefendTactic)
		{
			for (auto& eU : u.second)
			{
				if (eU->getPosition().getDistance(attackPosition) < 10 * 32)
				{
					nearDefendPosition = true;
					break;
				}
			}
		}
	}

	if (enemyUnits.empty())
	{
		return std::pair<int, Unit>(0, nullptr);
	}
	else
	{
		if (curTactic == DefendTactic)
		{
			bool isDefendStartBase = false;
			bool hasNoCounterDefender = false;
			if (BWEMMap.GetArea(InformationManager::Instance().getOurNatrualLocation()) == BWEMMap.GetArea(TilePosition(attackPosition))
				|| BWEMMap.GetArea(Broodwar->self()->getStartLocation()) == BWEMMap.GetArea(TilePosition(attackPosition)))
			{
				isDefendStartBase = true;
				bool hasSunken = false;
				bool hasSpore = false;
				std::map<const Area*, std::map<BWAPI::Unit, buildingInfo>>& ourBuildings = InformationManager::Instance().getSelfOccupiedDetail();
				if (ourBuildings.find(BWEMMap.GetArea(TilePosition(attackPosition))) != ourBuildings.end())
				{
					for (auto& building : ourBuildings[BWEMMap.GetArea(TilePosition(attackPosition))])
					{
						if (building.first->getType() == UnitTypes::Zerg_Sunken_Colony)
						{
							hasSunken = true;
						}
						if (building.first->getType() == UnitTypes::Zerg_Spore_Colony && hasAirAttacker)
						{
							hasSpore = true;
						}
					}
					if (hasGroundAttacker && !hasSunken)
					{
						hasNoCounterDefender = true;
					}
				}
			}

			//special attack codition in defend tactic
			if (isDefendStartBase && (
				attackingSunken > 0 ||
				attackingSpore > 0 || 
				hasLongAttackRangeEnemy || 
				nearDefendPosition))
			{
				set<Unit> allTargets;
				for (auto& army : enemyUnits)
				{
					allTargets.insert(army.second.begin(), army.second.end());
				}
				Unit bestTarget = chooseTarget(unit, allTargets, assignTargets);
				return std::pair<int, Unit>(0, bestTarget);
			}
		}

		std::pair<int, set<Unit>> targets = getCanAttackTargets(unit, ourUnits, enemyUnits, unitRetreatInfo, ourHasDetector, enemyHasDetector, curTactic);
		if (targets.second.empty() || unit.unit->getType() == UnitTypes::Zerg_Overlord)
		{
			return std::pair<int, Unit>(targets.first, nullptr);
		}
		Unit bestTarget = chooseTarget(unit, targets.second, assignTargets, kiteAssign);
		return std::pair<int, Unit>(targets.first, bestTarget);
	}
}


void BattleArmy::addUnit(BWAPI::Unit u)
{
	if (u == NULL)
		return;

	units.push_back(UnitState(u));
}


void BattleArmy::removeUnit(BWAPI::Unit u)
{
	for (std::vector<UnitState>::iterator it = units.begin(); it != units.end(); it++)
	{
		if (it->unit == u)
		{
			units.erase(it);
			break;
		}
	}
}

bool BattleArmy::hasUnit(Unit u)
{
	for (auto const& unit : units)
	{
		if (unit.unit == u)
		{
			return true;
		}
	}
	return false;
}


void BattleArmy::airUnitMicroUpdate(UnitState& unit, map<Unit, Position>& unitAttackPath, map<Unit, Position>& unitRetreatPath,
	map<Unit, int>& unitRetreatInfo, map<Unit, int>& enemyAssign, bool isRetreat, Position attackPosition, map<UnitType, set<Unit>>& encounteredEnemy)
{
	if (unit.unit->isSelected())
	{
		Broodwar->drawCircleMap(unit.unit->getPosition(), BattleArmy::pathShareRadius * 32, Colors::Red, false);
		if (unit.state == UnitState::MOVE)
		{
			Broodwar->drawCircleMap(unit.unit->getPosition(), 16, Colors::Green, true);
		}
		else if (unit.state == UnitState::ATTACK)
		{
			Broodwar->drawCircleMap(unit.unit->getPosition(), 16, Colors::Red, true);
		}
		else if (unit.state == UnitState::STOP)
		{
			Broodwar->drawCircleMap(unit.unit->getPosition(), 16, Colors::Purple, true);
		}
		else if (unit.state == UnitState::RETREAT)
		{
			Broodwar->drawCircleMap(unit.unit->getPosition(), 16, Colors::Black, true);
		}
		else if (unit.state == UnitState::SEIGE)
		{
			Broodwar->drawCircleMap(unit.unit->getPosition(), 16, Colors::Brown, true);
		}

		Astar::Instance().getAirPath(unit.unit, attackPosition, true);
	}

	if (InformationManager::Instance().getEnemyInfluenceMap(TilePosition(unit.unit->getPosition()).x, TilePosition(unit.unit->getPosition()).y).psionicStormDamage > 0) {
		unit.unit->move(Position(Broodwar->self()->getStartLocation()));
		return;
	}

	//update path for both attack and retreat
	Position nearestAttackPath;
	Position nearestRetreatPath;
	if (unitAttackPath.find(unit.unit) == unitAttackPath.end())
	{
		nearestAttackPath = Astar::Instance().getAirPath(unit.unit, attackPosition);
		nearestRetreatPath = Astar::Instance().getAirPath(unit.unit, InformationManager::Instance().getRetreatDestination());
		map<UnitType, set<Unit>> nearyByOurUnits =
			InformationManager::Instance().getUnitGridMap().GetUnits(unit.unit->getTilePosition(), 6, Broodwar->self(), false);
		for (auto& army : nearyByOurUnits)
		{
			for (auto& u : army.second)
			{
				if (unitAttackPath.find(u) == unitAttackPath.end())
				{
					unitAttackPath[u] = nearestAttackPath;
					unitRetreatPath[u] = nearestRetreatPath;
				}
			}
		}
	}
	else
	{
		nearestAttackPath = unitAttackPath[unit.unit];
		nearestRetreatPath = unitRetreatPath[unit.unit];
	}

	int curFrame = Broodwar->getFrameCount();
	std::pair<int, Unit> target = assignTarget(unit, enemyAssign, unitRetreatInfo, 8, MutaliskHarassTac,
		Positions::None, set<BWAPI::Unit>(), encounteredEnemy);
	
	unit.enemyTarget = target.second;

	if (unit.unit->getType() == UnitTypes::Zerg_Mutalisk && unit.state != UnitState::ATTACK && unit.state != UnitState::RETREAT) {
		// not attacking for a long time
		bool flag1 = !isRetreat && Broodwar->getFrameCount() - unit.lastAttackingFrame >= BattleArmy::maximumNoneAttackDurtion;
		// exceed maximum harass time limit(if not attacking)
		bool flag2 = !isRetreat && Broodwar->getFrameCount() - unit.starHarassFrame >= BattleArmy::maximumMutaHarassDurtion;
		if(flag1 || flag2){
			unit.retreatFlag = 1;
		}
	}

	//log
	//if (unit.unit == BattleArmy::logUnit && curFrame % 24 == 0)
	//{
	//	bool hasTarget = unit.enemyTarget != nullptr;
	//	int targetType = 0;
	//	if (hasTarget)
	//		targetType = int(unit.enemyTarget->getType());
	//	std::string logstr = "unit id:" + to_string(unit.unit->getID()) + "|| unit state:" + to_string(int(unit.state))
	//		+ "|| unit retreat flag:" + to_string(unit.retreatFlag) + "|| unit has target:" + to_string(int(hasTarget))
	//		+ "|| target type: " + to_string(targetType) + "|| is retreat:" + to_string(isRetreat)
	//		+ "|| unit position:" + to_string(unit.unit->getTilePosition().x) + " " + to_string(unit.unit->getTilePosition().y);
	//	logInfo("MutaliskHarassTacticUnit", logstr);
	//}

	bool needStop = false;

	switch (unit.state)
	{
	case UnitState::MOVE:
	{
		if (unit.retreatFlag == 1)
		{
			unit.enemyTarget = nullptr;
			unit.state = UnitState::RETREAT;
			unit.keepMoveStartFrame = Broodwar->getFrameCount();

			if (unit.unit == BattleArmy::logUnit)
			{
				logInfo("MutaliskHarassTacticUnit", "muta change to retreat, current enemy:");
				//logDetail(unit, enemyAssign);
			}
			return;
		}
		if (target.second != nullptr)
		{
			unit.enemyTarget = target.second;
			unit.state = UnitState::ATTACK;

			if (unit.unit == BattleArmy::logUnit)
			{
				logInfo("MutaliskHarassTacticUnit", "muta change to attack, current enemy:");
				//logDetail(unit, enemyAssign);
			}
			return;
		}
		if (unit.seigeFlag) {
			unit.state = UnitState::SEIGE;
			unit.keepMoveStartFrame = Broodwar->getFrameCount();
			return;
		}
		if (needStop && curFrame - unit.stopStartFrame > 2 * BattleArmy::stopMoveDurtion)
		{
			unit.enemyTarget = nullptr;
			unit.stopStartFrame = Broodwar->getFrameCount();
			unit.state = UnitState::STOP;
		}
		move(unit, nearestAttackPath);
	}
	break;

	case UnitState::ATTACK:
	{
		unit.lastAttackingFrame = Broodwar->getFrameCount();
		if (unit.enemyTarget == nullptr || !unit.enemyTarget->exists() || unit.retreatFlag != 0)
		{
			unit.state = UnitState::MOVE;
		}
		else
		{
			//Broodwar->drawLineMap(unit.unit->getPosition(), unit.enemyTarget->getPosition(), Colors::Red);
			attack(unit, unit.enemyTarget);
		}
	}
	break;

	//if partial army encounter strong enemy, go to other mutas's position.
	//if decide do retreat, group and move backwards
	case UnitState::RETREAT:
	{
		if (curFrame - unit.keepMoveStartFrame < BattleArmy::retreatMoveDurtion)
		{
			move(unit, nearestRetreatPath);
		}
		else
		{
			unit.state = UnitState::MOVE;
		}
	}
	break;

	case UnitState::STOP:
	{
		if (curFrame - unit.stopStartFrame < BattleArmy::stopMoveDurtion && needStop)
		{
			unit.unit->stop();
		}
		else
		{
			unit.state = UnitState::MOVE;
		}
	}
	break;

	default:
		break;
	}
}


void BattleArmy::microUpdate(UnitState& unit, map<Unit, Position>& unitAttackPath, map<Unit, Position>& unitRetreatPath,
	map<Unit, int>& unitRetreatInfo, map<Unit, int>& enemyAssign, bool isRetreat, Position attackPosition,
	tacticType curTactic, Unit groundUnit, int groundArmyCount, set<BWAPI::Unit>& defendEnemy,
	map<UnitType, set<Unit>>& encounteredEnemy, map<Unit, Unit>& kiteAssign)
{
	int shareRadius = (isRetreat ? BattleArmy::retreatPathShareRadius : BattleArmy::pathShareRadius);
	if (unit.unit->isSelected())
	{
		Broodwar->drawCircleMap(unit.unit->getPosition(), shareRadius * 32, Colors::Red, false);
		if (unit.state == UnitState::MOVE)
		{
			Broodwar->drawCircleMap(unit.unit->getPosition(), 16, Colors::Green, true);
		}
		else if (unit.state == UnitState::ATTACK)
		{
			Broodwar->drawCircleMap(unit.unit->getPosition(), 16, Colors::Red, true);
		}
		else if (unit.state == UnitState::STOP)
		{
			Broodwar->drawCircleMap(unit.unit->getPosition(), 16, Colors::Purple, true);
		}
		else if (unit.state == UnitState::RETREAT)
		{
			Broodwar->drawCircleMap(unit.unit->getPosition(), 16, Colors::Black, true);
		}
		else if (unit.state == UnitState::SEIGE)
		{
			Broodwar->drawCircleMap(unit.unit->getPosition(), 16, Colors::Brown, true);
		}

		//Astar::Instance().getGroundPath(unit.unit, attackPosition, true);
	}

	if (unit.unit->getType().isFlyer()) {
		if (InformationManager::Instance().getEnemyInfluenceMap(TilePosition(unit.unit->getPosition()).x, TilePosition(unit.unit->getPosition()).y).psionicStormDamage > 0) {
			unit.unit->move(Position(Broodwar->self()->getStartLocation()));
			return;
		}
	}


	Position nearestAttackPath;
	Position nearestRetreatPath;
	if (unitAttackPath.find(unit.unit) == unitAttackPath.end())
	{
		//Broodwar->drawCircleMap(unit.unit->getPosition(), 20, Colors::Green, false);
		if (unit.unit->getType().isFlyer())
		{
			if (groundUnit == NULL || isRetreat)
			{
				nearestAttackPath = Astar::Instance().getAirPath(unit.unit, attackPosition);
				nearestRetreatPath = Astar::Instance().getAirPath(unit.unit, InformationManager::Instance().getRetreatDestination());
			}
			else
			{
				//nearestAttackPath = Astar::Instance().getGroundPath(unit.unit, groundUnit->getPosition());
				//nearestRetreatPath = Astar::Instance().getGroundPath(unit.unit, InformationManager::Instance().getRetreatDestination());
				//nearestRetreatPath = Astar::Instance().getGroundRetreatPath(unit.unit, 7);
				nearestAttackPath = groundUnit->getPosition();
				nearestRetreatPath = groundUnit->getPosition();
			}
			map<UnitType, set<Unit>> nearyByOurUnits =
				InformationManager::Instance().getUnitGridMap().GetUnits(unit.unit->getTilePosition(), shareRadius, Broodwar->self(), false);
			for (auto& army : nearyByOurUnits)
			{
				if (!army.first.isFlyer())
					continue;
				for (auto& u : army.second)
				{
					if (unitAttackPath.find(u) == unitAttackPath.end())
					{
						unitAttackPath[u] = nearestAttackPath;
						unitRetreatPath[u] = nearestRetreatPath;
					}
				}
			}
		}
		else
		{
			nearestAttackPath = Astar::Instance().getGroundPath(unit.unit, attackPosition);
			//nearestRetreatPath = Astar::Instance().getGroundPath(unit.unit, InformationManager::Instance().getRetreatDestination());
			nearestRetreatPath = Astar::Instance().getGroundRetreatPath(unit.unit, 7);
			map<UnitType, set<Unit>> nearyByOurUnits =
				InformationManager::Instance().getUnitGridMap().GetUnits(unit.unit->getTilePosition(), shareRadius, Broodwar->self(), false);
			for (auto& army : nearyByOurUnits)
			{
				if (army.first.isFlyer())
					continue;
				for (auto& u : army.second)
				{
					if (unitAttackPath.find(u) == unitAttackPath.end() &&
						BWEMMap.GetArea(u->getTilePosition()) == BWEMMap.GetArea(unit.unit->getTilePosition()))
					{
						unitAttackPath[u] = nearestAttackPath;
						unitRetreatPath[u] = nearestRetreatPath;
					}
				}
			}
		}
	}
	else
	{
		nearestAttackPath = unitAttackPath[unit.unit];
		nearestRetreatPath = unitRetreatPath[unit.unit];
	}

	int curFrame = Broodwar->getFrameCount();
	std::pair<int, Unit> target = assignTarget(unit, enemyAssign, unitRetreatInfo, 10,
		curTactic, attackPosition, defendEnemy, encounteredEnemy, kiteAssign);

	//if (target.second != nullptr) {
	//	Broodwar->drawLineMap(unit.unit->getPosition(), target.second->getPosition(), Colors::Blue);
	//}
	unit.retreatFlag = target.first;

	if (groundArmyCount > 100 || curTactic == tacticType::AirdropTac)
	{
		unit.retreatFlag = 0;
	}
	//keep state only trigger at attack time
	if (isRetreat && unit.retreatFlag == 2)
	{
		unit.retreatFlag = 1;
	}
	unit.enemyTarget = target.second;

	//Broodwar->drawTextMap(unit.unit->getPosition(), "%d:%d",unit.ETAtoTarget , unit.nonAttackingDuration);
	if (isRetreat || hasNearbyEnemy || curTactic == tacticType::DefendTactic || unit.seigeFlag || Broodwar->getFrameCount() <= 15 * 60 * 24) {
		needRegroupFlag = false;
	}

	switch (unit.state)
	{
	case UnitState::MOVE:
	{
		if (unit.retreatFlag == 1)
		{
			unit.enemyTarget = nullptr;
			unit.state = UnitState::RETREAT;
			unit.keepMoveStartFrame = Broodwar->getFrameCount();
			return;
		}
		if (target.second != nullptr)
		{
			unit.enemyTarget = target.second;
			unit.state = UnitState::ATTACK;
			return;
		}
		if (unit.seigeFlag) {
			unit.state = UnitState::SEIGE;
			return;
		}
		if (curFrame - unit.stopEndFrame > BattleArmy::stopMoveDurtion && needRegroupFlag)
		{
			unit.enemyTarget = nullptr;
			unit.stopStartFrame = Broodwar->getFrameCount();
			unit.state = UnitState::STOP;
		}
		move(unit, nearestAttackPath);
	}
	break;

	case UnitState::ATTACK:
	{
		if (unit.enemyTarget == nullptr ||
			!unit.enemyTarget->exists() ||
			unit.retreatFlag != 0)
		{
			unit.state = UnitState::MOVE;
		}
		else
		{
			attack(unit, unit.enemyTarget);
		}
	}
	break;

	//if partial army encounter strong enemy, go to other mutas's position.
	//if decide do retreat, group and move backwards
	case UnitState::RETREAT:
	{
		if (curFrame - unit.keepMoveStartFrame < BattleArmy::retreatMoveDurtion)
		{
			move(unit, nearestRetreatPath);
		}
		else
		{
			unit.state = UnitState::MOVE;
		}
	}
	break;

	case UnitState::STOP:
	{
		if (target.second != nullptr)
		{
			unit.stopEndFrame = curFrame;
			unit.enemyTarget = target.second;
			unit.state = UnitState::ATTACK;
			return;
		}
		if (curFrame - unit.stopStartFrame < BattleArmy::groupingDurtion && needRegroupFlag)
		{
			//unit.unit->stop();
			if (!unit.unit->getType().isFlyer() && regroupFrontLine != -1) {
				if (groundDistanceSet[unit.unit] > regroupFrontLine) {
					if (unit.unit->getDistance(regroupPosition) > 100) {
						BattleArmy::smartMove(unit.unit, regroupPosition);
					}
					else if (unit.unit->getDistance(regroupPosition) > 50) {
						unit.unit->stop();
					}
					else {
						BattleArmy::smartMove(unit.unit, InformationManager::Instance().getRetreatDestination());
					}
				}
				else {
					BattleArmy::smartMove(unit.unit, InformationManager::Instance().getRetreatDestination());
				}
			}
			else {
				if (unit.unit->getDistance(regroupPosition) > 50) {
					BattleArmy::smartMove(unit.unit, regroupPosition);
				}
				else {
					unit.unit->stop();
				}
			}

		}
		else
		{
			unit.stopEndFrame = curFrame;
			unit.state = UnitState::MOVE;
		}
		
	}
	break;

	default:
		break;
	}
}


void BattleArmy::move(UnitState& unit, BWAPI::Position targetPositio)
{
	BattleArmy::smartMove(unit.unit, targetPositio);
}


void BattleArmy::attack(UnitState& unit, BWAPI::Unit target)
{
	smartAttackUnit(unit.unit, target);
}


void BattleArmy::armyTypeSaveFunc(const Unit& u, const UnitType& curType, int count, map<string, std::pair<int, int>>& armyTypeScoreDict, bool hasDetector)
{
	//if (curType.isBuilding() && u != nullptr)
	//{
	//	if (curType.canAttack() && u->getPlayer() == Broodwar->self() && !u->isAttacking())
	//	{
	//		return;
	//	}
	//}
	if (curType.isFlyer())
	{
		if (curType == UnitTypes::Protoss_Interceptor)
		{
			return;
		}
		if (curType == UnitTypes::Protoss_Carrier)
		{
			armyTypeScoreDict["air_allWeapon"].first += calTypeScore(curType) * count;
			return;
		}

		if (curType.groundWeapon() != WeaponTypes::None)
		{
			if (curType.airWeapon() != WeaponTypes::None)
			{
				if (curType.isBuilding())
				{
					armyTypeScoreDict["air_allWeapon"].second += calTypeScore(curType) * count;
				}
				else
				{
					armyTypeScoreDict["air_allWeapon"].first += calTypeScore(curType) * count;
				}
			}
			else
			{
				if (curType.isBuilding())
				{
					armyTypeScoreDict["air_groundWeapon"].second += calTypeScore(curType) * count;
				}
				else
				{
					armyTypeScoreDict["air_groundWeapon"].first += calTypeScore(curType) * count;
				}
			}
		}
		else if (curType.airWeapon() != WeaponTypes::None)
		{
			if (curType.isBuilding())
			{
				armyTypeScoreDict["air_airWeapon"].second += calTypeScore(curType) * count;
			}
			else
			{
				armyTypeScoreDict["air_airWeapon"].first += calTypeScore(curType) * count;
			}
		}
		else
		{
			return;
		}
	}
	else
	{
		if (curType == UnitTypes::Zerg_Lurker && hasDetector == false)
		{
			armyTypeScoreDict["underGround_groundWeapon"].first += calTypeScore(curType) * count;
		}
		else
		{
			if (curType.groundWeapon() != WeaponTypes::None || curType == UnitTypes::Zerg_Lurker
				|| curType == UnitTypes::Terran_Bunker)
			{
				if (curType.airWeapon() != WeaponTypes::None || curType == UnitTypes::Terran_Bunker)
				{
					if (curType.isBuilding())
					{
						armyTypeScoreDict["ground_allWeapon"].second += calTypeScore(curType) * count;
					}
					else
					{
						armyTypeScoreDict["ground_allWeapon"].first += calTypeScore(curType) * count;
					}
				}
				else
				{
					if (curType.isBuilding())
					{
						armyTypeScoreDict["ground_groundWeapon"].second += calTypeScore(curType) * count;
					}
					else
					{
						armyTypeScoreDict["ground_groundWeapon"].first += calTypeScore(curType) * count;
					}
				}
			}
			else if (curType.airWeapon() != WeaponTypes::None)
			{
				if (curType.isBuilding())
				{
					armyTypeScoreDict["ground_airWeapon"].second += calTypeScore(curType) * count;
				}
				else
				{
					armyTypeScoreDict["ground_airWeapon"].first += calTypeScore(curType) * count;
				}
			}
			else
			{
				return;
			}
		}
	}
}


int BattleArmy::calTypeScore(UnitType u)
{
	if (u.isWorker()) {
		return 1;
	}
	if (u == UnitTypes::Zerg_Scourge)
	{
		return 2;
	}
	else if (u.isBuilding())
	{
		if (u == UnitTypes::Terran_Bunker)
			return 16;
		else
			return 10;
	}
	else
	{
		return u.supplyRequired();
	}
	

	/*
	if (u.isBuilding()) {
		if (u == UnitTypes::Terran_Bunker)
			return 800;
		else
			return 500;
	}
	else {
		int price = u.mineralPrice() + int(1 * u.gasPrice());
		return price;
	}
	*/
}


int BattleArmy::needRetreat(map<string, std::pair<int, int>> ourArmyTypeScoreDict, map<string, std::pair<int, int>> enemyArmyTypeScoreDict, 
	set<string>& overWhelmEnemyArmy, bool ourHasDetector, bool enemyHasDetector, tacticType curTactic)
{
	int ourTotalArmyForce = 0;
	for (auto& counterItems : ourArmyTypeScoreDict)
	{
		counterItems.second.first = counterItems.second.first + counterItems.second.second;
		if (Broodwar->getFrameCount() % 24 == 0 || BattleArmy::needLog)
		{
			logInfo("BattleArmy", "needRetreat our " + counterItems.first + " " + to_string(counterItems.second.first));
		}

		ourTotalArmyForce += counterItems.second.first;
	}
	if (ourTotalArmyForce == 0)
	{
		return 1;
	}

	map<string, std::pair<int, int>> ourArmyTypeScoreDictRaw = ourArmyTypeScoreDict;

	for (auto& counterItems : enemyArmyTypeScoreDict)
	{
		counterItems.second.first = counterItems.second.first + counterItems.second.second;
		if (Broodwar->getFrameCount() % 24 == 0 || BattleArmy::needLog)
		{
			logInfo("BattleArmy", "needRetreat enemy " + counterItems.first + " " + to_string(counterItems.second.first));
		}
	}
	map<string, std::pair<int, int>> enemyArmyTypeScoreDictRaw = enemyArmyTypeScoreDict;

	//calculate the counter army type result
	for (auto& ourItems : ourArmyTypeScoreDict)
	{
		for (auto& counterItem : armyTypeCounterDict[ourItems.first])
		{
			if (enemyArmyTypeScoreDict.find(counterItem) != enemyArmyTypeScoreDict.end())
			{
				int ourValue = std::max(0, ourItems.second.first - enemyArmyTypeScoreDict[counterItem].first);
				int enemyValue = std::max(0, enemyArmyTypeScoreDict[counterItem].first - ourItems.second.first);
				if (ourItems.first == "underGround_groundWeapon")
				{
					ourValue = std::max(0, ourItems.second.first - int(0.5 * enemyArmyTypeScoreDict[counterItem].first));
					enemyValue = std::max(0, enemyArmyTypeScoreDict[counterItem].first - int(1.5 * ourItems.second.first));
				}
				ourItems.second.first = ourValue;
				enemyArmyTypeScoreDict[counterItem].first = enemyValue;
			}
		}
	}

	//calculate the overwhelming army type result 
	int overWhelmFactor = 3;
	for (auto& enemyItems : enemyArmyTypeScoreDict)
	{
		for (auto& counterItem : armyTypeOverwhelmDict[enemyItems.first])
		{
			if (ourArmyTypeScoreDict.find(counterItem) != ourArmyTypeScoreDict.end())
			{
				if (counterItem == "air_airWeapon")
				{
					if (enemyItems.second.first > 0)
					{
						ourArmyTypeScoreDict[counterItem].first = 0;
					}
				}
				else
				{
					ourArmyTypeScoreDict[counterItem].first = std::max(0, ourArmyTypeScoreDict[counterItem].first - enemyItems.second.first * overWhelmFactor);
				}

			}
		}
	}

	//calculate our overWhelming army type
	overWhelmEnemyArmy.clear();
	for (auto& counterItems : ourArmyTypeScoreDict)
	{
		if (counterItems.first == "underGround_groundWeapon")
		{
			continue;
		}
		if (ourArmyTypeScoreDictRaw[counterItems.first].first > 0 && counterItems.second.first * 10 / ourArmyTypeScoreDictRaw[counterItems.first].first > 8)
		{
			int counterForce = 0;
			for (auto& enemyItem : armyTypeCounterDict[counterItems.first])
			{
				if (enemyArmyTypeScoreDictRaw.find(enemyItem) != enemyArmyTypeScoreDictRaw.end())
				{
					counterForce += enemyArmyTypeScoreDictRaw[enemyItem].first;
				}
			}

			if (counterForce * 10 / ourArmyTypeScoreDictRaw[counterItems.first].first < 2)
			{
				bool hasTarget = false;
				for (auto& enemyItem : armyTypeOverwhelmDict[counterItems.first])
				{
					if (enemyArmyTypeScoreDictRaw.find(enemyItem) != enemyArmyTypeScoreDictRaw.end())
					{
						hasTarget = true;
						break;
					}
				}
				if (hasTarget)
				{
					overWhelmEnemyArmy.insert(counterItems.first);
				}
			}
		}
	}

	//if the final result show that we not lose too much on current army type, do not retreat
	int ourTotalResidualArmyForce = 0;
	for (auto& counterItems : ourArmyTypeScoreDict)
	{
		ourTotalResidualArmyForce += counterItems.second.first;
	}
	int retreatThreshold = 10;
	if (curTactic == DefendTactic)
	{
		retreatThreshold = 1;
	}
	int ourTotalArmyResidualPercent = ourTotalResidualArmyForce * 100 / ourTotalArmyForce;
	if (ourTotalArmyResidualPercent >= retreatThreshold)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}


TilePosition BattleArmy::calNextMovePosition(UnitState& u, Position destination, Position armyCenterPosition, int moveMode)
{
	//getPosition represent the center of the unit, while getTileposition represent the top left
	int weaponRange = u.unit->getType().airWeapon().maxRange();
	TilePosition unitPosition = TilePosition(u.unit->getPosition());
	UnitType curType = u.unit->getType();
	TilePosition bestPosition(-1, -1);
	double minScore = 99999;
	map<UnitType, set<Unit>> enemyUnits;
	vector<std::pair<int, int>>	oldMoveDirections = moveDirections;
	if (moveMode == 6) {
		enemyUnits = InformationManager::Instance().getUnitGridMap().GetUnits(u.unit->getTilePosition(), 12, Broodwar->enemy(), false);
		moveDirections.clear();
		for (int x = -2; x <= 2; x++)
		{
			for (int y = -2; y <= 2; y++)
			{
				if (x != -2 && x != 2 && y != -2 && y != 2)
					continue;
				moveDirections.push_back(std::pair<int, int>(x, y));
			}
		}
	}
	for (auto const& direction : moveDirections)
	{
		TilePosition tmp(unitPosition.x + direction.first, unitPosition.y + direction.second);
		if (isTilePositionValid(tmp))
		{
			double altitude = BWEMMap.GetMiniTile(WalkPosition(tmp)).Altitude() / 10;
			double destScore = tmp.getDistance(TilePosition(destination));
			double cohesionScore = tmp.getDistance(TilePosition(armyCenterPosition));
			double cohesionWeight = 1;
			double IMScore = InformationManager::Instance().getEnemyInfluenceMap(tmp.x, tmp.y).groundForce;
			double IMArmyScore = InformationManager::Instance().getEnemyInfluenceMap(tmp.x, tmp.y).enemyUnitGroundForce;
			if (curType.isFlyer())
			{
				IMScore = InformationManager::Instance().getEnemyInfluenceMap(tmp.x, tmp.y).airForce;
				IMArmyScore = InformationManager::Instance().getEnemyInfluenceMap(tmp.x, tmp.y).enemyUnitAirForce;
				if (cohesionScore > 5)
				{
					cohesionWeight = 2;
				}
			}
			else
			{
				if (BWEMMap.GetArea(tmp) == NULL || BWEMMap.GetArea(unitPosition) == NULL || !BWEMMap.GetArea(unitPosition)->AccessibleFrom(BWEMMap.GetArea(tmp)))
					continue;

				cohesionWeight = 0.5;
				if (cohesionScore > 10)
				{
					cohesionWeight = 5;
				}
			}

			double curScore = 9999999.0;
			//normal move, keep together
			if (moveMode == 1)
			{
				curScore = destScore + cohesionScore * cohesionWeight;
			}
			//avoid cannon damage while looking for potential path to destination
			else if (moveMode == 2)
			{
				curScore = IMScore + destScore;
			}
			//away from damage and enemy
			else if (moveMode == 3)
			{
				curScore = IMScore + IMArmyScore - destScore;
			}
			//away from damage and close to dest
			else if (moveMode == 4)
			{
				curScore = IMScore + IMArmyScore + destScore;
			}

			else if (moveMode == 5)
			{
				curScore = destScore + cohesionScore * cohesionWeight + IMScore + IMArmyScore;
			}

			// away from all enemies
			else if (moveMode == 6)
			{
				if (cohesionScore > 3) {
					curScore = cohesionScore * 100;
				}
				else {
					curScore = 0;
				}
				//curScore = 0;
				//curScore = IMScore + IMArmyScore;
				double totalDistanceScore = 0.0;

				/*
				for (auto const unit : Broodwar->getAllUnits()) {
					if (unit->getPlayer() == Broodwar->enemy()) {
						if (curType.isFlyer() && unit->getType().airWeapon() != BWAPI::WeaponTypes::None) {
							//Broodwar->drawCircleMap(unit->getPosition(), unit->getType().airWeapon().maxRange(), Colors::Red, false);
							double distance = tmp.getDistance(TilePosition(unit->getPosition()));
							if (distance < unit->getType().airWeapon().maxRange() / 32 + 10) {
								totalDistanceScore += distance;
							}
						}
						else if (!curType.isFlyer() && unit->getType().groundWeapon() != BWAPI::WeaponTypes::None) {
							double distance = tmp.getDistance(TilePosition(unit->getPosition()));
							if (distance < unit->getType().groundWeapon().maxRange() / 32 + 10) {
								totalDistanceScore += distance;
							}
						}
					}
				}
				curScore -= totalDistanceScore;
				*/
				
				for (auto& u : enemyUnits) {
					if (u.first.canAttack()) {
						if ((curType.isFlyer() && u.first.airWeapon() != BWAPI::WeaponTypes::None) || (!curType.isFlyer() && u.first.groundWeapon() != BWAPI::WeaponTypes::None)) {
							for (auto const& unit : u.second) {
								double distance = tmp.getDistance(TilePosition(unit->getPosition()));
								curScore -= distance;
							}
						}
					}
				}

				int stormDamage = InformationManager::Instance().getEnemyInfluenceMap(tmp.x, tmp.y).psionicStormDamage;
				stormDamage += int(InformationManager::Instance().getEnemyInfluenceMap(tmp.x, tmp.y).strongAirForce);
				curScore += stormDamage * 10000;
			}

			if (u.unit->isSelected())
			{
				Broodwar->drawTextMap(Position(tmp), "%d", int(curScore));
			}
			if (curScore < minScore)
			{
				minScore = curScore;
				bestPosition = tmp;//TilePosition(unitPosition.x + direction.first * 3, unitPosition.y + direction.second * 3);
			}
		}
	}

	if (u.unit->isSelected())
	{
		Broodwar->drawCircleMap(Position(bestPosition), 16, Colors::Green, false);
	}

	//all position has visited
	if (bestPosition != TilePosition(-1, -1))
	{
		u.nextMovePosition = bestPosition;
	}
	if (moveMode == 6) {
		moveDirections = oldMoveDirections;
	}
	return bestPosition;
}


void BattleArmy::smartAttackUnit(BWAPI::Unit attacker, BWAPI::Unit target)
{
	if (attacker == NULL || target == NULL)
		return;

	// if we have issued a command to this unit already this frame, ignore this one
	//if (attacker->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount() - 3)
	//{
	//	return;
	//}

	// get the unit's current command
	BWAPI::UnitCommand currentCommand(attacker->getLastCommand());

	// for
	if (currentCommand.getType() == BWAPI::UnitCommandTypes::Attack_Unit &&	currentCommand.getTarget() == target
		&& (BWAPI::Broodwar->getFrameCount() - attacker->getLastCommandFrame() < 24))
	{
		return;
	}

	// if nothing prevents it, attack the target
	bool re = attacker->attack(target);
	BWAPI::Color c;
	if (re)
	{
		c = BWAPI::Colors::Red;
	}
	else
	{
		c = BWAPI::Colors::Blue;
	}

	//if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawLineMap(attacker->getPosition().x, attacker->getPosition().y,
	//	target->getPosition().x, target->getPosition().y, c);

}

void BattleArmy::smartAttackMove(BWAPI::Unit attacker, BWAPI::Position targetPosition)
{
	assert(attacker);

	// if we have issued a command to this unit already this frame, ignore this one
	//if (attacker->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount() - 3)
	//{
	//	return;
	//}

	// get the unit's current command
	BWAPI::UnitCommand currentCommand(attacker->getLastCommand());

	// if we've recently already told this unit to attack this target, ignore
	if (currentCommand.getType() == BWAPI::UnitCommandTypes::Attack_Move &&	currentCommand.getTargetPosition() == targetPosition
		&& (BWAPI::Broodwar->getFrameCount() - attacker->getLastCommandFrame() < 24))
	{
		return;
	}

	// if nothing prevents it, attack the target
	attacker->attack(targetPosition);

	if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawLineMap(attacker->getPosition().x, attacker->getPosition().y,
		targetPosition.x, targetPosition.y,
		BWAPI::Colors::Orange);
}

void BattleArmy::smartMove(BWAPI::Unit attacker, BWAPI::Position targetPosition)
{
	assert(attacker);

	// if we have issued a command to this unit already this frame, ignore this one
	if (attacker->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount() - 5)
	{
		return;
	}

	// get the unit's current command
	BWAPI::UnitCommand currentCommand(attacker->getLastCommand());

	// if we've already told this unit to attack this target, ignore this command
	if ((currentCommand.getType() == BWAPI::UnitCommandTypes::Move)
		&& (currentCommand.getTargetPosition() == targetPosition)
		&& (BWAPI::Broodwar->getFrameCount() - attacker->getLastCommandFrame() < 24)
		&& (attacker->isMoving()))
	{
		return;
	}

	// if nothing prevents it, attack the target
	attacker->move(targetPosition);

	//if (Options::Debug::DRAW_UALBERTABOT_DEBUG)
	//{
	//	BWAPI::Broodwar->drawLineMap(attacker->getPosition().x, attacker->getPosition().y, targetPosition.x, targetPosition.y, BWAPI::Colors::Orange);
	//	BWAPI::Broodwar->drawLineMap(attacker->getPosition().x, attacker->getPosition().y, attacker->getOrderTargetPosition().x, attacker->getOrderTargetPosition().y, BWAPI::Colors::Orange);
	//}
}

void BattleArmy::setRegroupPosition(Position newRegroupPosition) {
	regroupPosition = newRegroupPosition;
	Broodwar->drawCircleMap(regroupPosition, 20, Colors::Red, false);
}
