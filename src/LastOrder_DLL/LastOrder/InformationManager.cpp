#include "InformationManager.h"

#define SELF_INDEX 0
#define ENEMY_INDEX 1

// constructor
InformationManager::InformationManager() : unitGM(&BWEMMap), IMGM(&BWEMMap)
{
	selfNaturalBaseLocation = BWAPI::TilePositions::None;
	selfStartBaseLocation = BWAPI::Broodwar->self()->getStartLocation();
	for(auto u : BWAPI::Broodwar->self()->getUnits())
	{
		if (u->getType().isResourceDepot())
			selfAllBase.insert(u);
	}
	enemyStartBaseLocation = BWAPI::TilePositions::None;
	enemyNaturalBaseLocation = BWAPI::TilePositions::None;
	enemyBaseChoke = BWAPI::TilePositions::None;
	enemyEverBlockingBase = false;
	enemyBlockingBase = false;
	enemyBlockingNatural = false;

	for(auto u : selfAllBase)
	{
		if (u->getTilePosition() == BWAPI::Broodwar->self()->getStartLocation())
		{
			selfBaseUnit = u;
			break;
		}
	}

	selfNaturalChokePoint = Positions::None;
}

BWAPI::TilePosition	InformationManager::getSunkenBuildingPosition(std::string sunkenArea)
{
	//generate selfNaturalChokePoint
	getOurNatrualLocation();

	if (sunkenArea == "start")
	{
		double2 direc = selfNaturalChokePoint - BWAPI::Position(selfStartBaseLocation);
		double2 direcNormal = direc / direc.len();
		int targetx = BWAPI::Position(selfStartBaseLocation).x + int(direcNormal.x * 32 * 8);
		int targety = BWAPI::Position(selfStartBaseLocation).y + int(direcNormal.y * 32 * 8);
		baseSunkenBuildingPosition = BWAPI::TilePosition(targetx / 32, targety / 32);
		return baseSunkenBuildingPosition;
	}
	if (sunkenArea == "natural")
	{
		
		Position center = Position((selfNaturalBaseLocation.x + 2) * 32, (selfNaturalBaseLocation.y + 2) * 32);
		double2 direc = selfNaturalToEnemyChoke - center;
		double2 direcNormal = direc / direc.len();
		int targetx = center.x + int(direcNormal.x * 32 * 3);
		int targety = center.y + int(direcNormal.y * 32 * 3);
		natrualSunkenBuildingPosition = BWAPI::TilePosition(targetx / 32, targety / 32);
		return natrualSunkenBuildingPosition;
		
	}

	return TilePositions::None;
}


bool InformationManager::hasAttackArmy()
{
	for (auto& a : selfAllBattleUnit)
	{
		if (a.first.canAttack() && !a.first.isWorker() && a.second.size() > 0)
		{
			return true;
		}
	}
	return false;
}


int	InformationManager::getEnemyTotalAntiGroundBattleForce()
{
	int totalSupply = 0;
	for (auto& unitCategory : enemyAllBattleUnit)
	{
		if (unitCategory.first.groundWeapon() != BWAPI::WeaponTypes::None && !unitCategory.first.isWorker())
		{
			totalSupply += unitCategory.first.supplyRequired() * unitCategory.second.size();
		}
	}
	return totalSupply;
}


int	InformationManager::getOurTotalBattleForce()
{
	int totalSupply = 0;
	for (auto unitCategory : selfAllBattleUnit)
	{
		if (!unitCategory.first.isWorker())
		{
			totalSupply += unitCategory.first.supplyRequired() * unitCategory.second.size();
		}
	}
	return totalSupply;
}


std::string InformationManager::getEnemyRace()
{
	std::string enemyRace;
	if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Zerg)
	{
		enemyRace = "z";
	}
	else if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Terran)
	{
		enemyRace = "T";
	}
	else if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Protoss)
	{
		enemyRace = "p";
	}
	else
		enemyRace = "unknow";

	return enemyRace;
}


Position InformationManager::getRetreatDestination()
{
	if (isHasNatrualBase())
	{
		return Position(getOurNatrualLocation());
	}
	else
	{
		return Position(BWAPI::Broodwar->self()->getStartLocation());
	}
}


void InformationManager::updateDefendInfo()
{
	enemyUnitsInRegion.clear();
	enemyUnitsTargetRegion.clear();
	std::map<const Area*, TilePosition>&	 myRegion = getBaseOccupiedRegions(BWAPI::Broodwar->self());
	std::map<const Area*, TilePosition>&	 enemyRegion = getBaseOccupiedRegions(BWAPI::Broodwar->enemy());
	std::set<BWAPI::Unit> ourAllBases = getOurAllBaseUnit();

	BWAPI::Position curP = BWAPI::Positions::None;
	for (auto enemyUnit : BWAPI::Broodwar->enemy()->getUnits())
	{
		if (!enemyUnit->isDetected() && enemyUnit->isVisible())
			continue;
		if (!enemyUnit->exists())
			continue;
		if (enemyRegion.find(BWEMMap.GetArea(enemyUnit->getTilePosition())) != enemyRegion.end())
			continue;

		const Area* enemyArea = BWEMMap.GetArea(enemyUnit->getTilePosition());
		if (enemyArea == NULL)
		{
			enemyArea = BWEMMap.GetNearestArea(enemyUnit->getTilePosition());
		}
		enemyUnitsInRegion[enemyArea].insert(enemyUnit);
	}

	//get grouped enemy info
	for (auto const& enemyArea : enemyUnitsInRegion)
	{
		bool isAllFlyer = true;
		TilePosition groundUnitPosition;
		for (auto const& e : enemyArea.second)
		{
			if (!e->getType().isFlyer())
			{
				isAllFlyer = false;
				groundUnitPosition = e->getTilePosition();
				break;
			}
		}
		
		TilePosition enemyAreaPosition;
		if (isAllFlyer == false)
		{
			enemyAreaPosition = groundUnitPosition;
		}
		else
		{
			enemyAreaPosition = (*enemyArea.second.begin())->getTilePosition();
		}
		const Area* belongedArea = getNearstRegion(enemyAreaPosition, isAllFlyer, false);
		if (belongedArea != nullptr)
		{
			if (belongedArea == BWEMMap.GetArea(selfStartBaseLocation) ||
				belongedArea == BWEMMap.GetArea(selfNaturalBaseLocation))
			{
				TilePosition targetPosition = belongedArea == BWEMMap.GetArea(selfStartBaseLocation) ? selfStartBaseLocation : selfNaturalBaseLocation;
				for (auto& eU : enemyArea.second)
				{
					if (TilePosition(eU->getPosition()).getDistance(targetPosition) < 30)
					{
						enemyUnitsTargetRegion[belongedArea].insert(eU);
					}
				}
			}
			else
			{
				enemyUnitsTargetRegion[belongedArea].insert(enemyArea.second.begin(), enemyArea.second.end());
			}
		}
	}
}


const Area*	InformationManager::getNearstRegion(TilePosition p, bool isFlyer, bool checkEnemy)
{
	std::map<const Area*, TilePosition>	 myRegion = getBaseOccupiedRegions(BWAPI::Broodwar->self());
	if (checkEnemy)
		myRegion = getBaseOccupiedRegions(BWAPI::Broodwar->enemy());

	if (isFlyer)
	{
		double minDistance = 30;
		const Area* minTarget = nullptr;
		for (auto myR : myRegion)
		{
			if (myR.second.getDistance(p) < minDistance)
			{
				minDistance = myR.second.getDistance(p);
				minTarget = myR.first;
			}
			if (BWEMMap.GetArea(p) == myR.first )
			{
				minTarget = myR.first;
				return minTarget;
			}
		}
		return minTarget;
	}
	else
	{
		int minDistance = 99999;
		const Area* minTarget = nullptr;
		for (auto& myR : myRegion)
		{
			int pathL = 0;
			CPPath path = BWEMMap.GetPath(Position(p), Position(myR.second), &pathL);
			//if enemy is close to our region
			if (pathL != -1 && path.size() <= 2 && pathL < minDistance)
			{
				if ((selfStartBaseLocation != BWAPI::TilePositions::None && myR.first == BWEMMap.GetArea(selfStartBaseLocation)) ||
					(selfNaturalBaseLocation != BWAPI::TilePositions::None && myR.first == BWEMMap.GetArea(selfNaturalBaseLocation)))
				{
					if (path.size() <= 2)
					{
						minDistance = pathL;
						minTarget = myR.first;
					}
				}
				else
				{
					if (path.size() == 0)
					{
						minDistance = pathL;
						minTarget = myR.first;
					}
				}
			}
		}
		
		return minTarget;
	}
}



void InformationManager::update()
{
	updateAllUnit();
	//for enemy buildings not destroy by us
	checkOccupiedDetail();
	updateDefendInfo();
	updateEnemyBlockInfo();

	//updateEnemyLocationInfo();

	/*
	if (Broodwar->getFrameCount() % 24 * 5 == 0)
	{
		for (auto& b : selfAllBuilding)
		{
			logInfo("InformationManager", "our building " + to_string(int(b.first)) + " count " + to_string(b.second.size()));
		}
		for (auto& u : selfAllBattleUnit)
		{
			logInfo("InformationManager", "our army " + to_string(int(u.first)) + " count " + to_string(u.second.size()));
		}
	}
	*/

}


void InformationManager::updateEnemyLocationInfo()
{
	std::map<const Area*, TilePosition>& myRegion = getBaseOccupiedRegions(BWAPI::Broodwar->self());
	std::map<const Area*, TilePosition>& enemyRegion = getBaseOccupiedRegions(BWAPI::Broodwar->enemy());

	int curFrame = Broodwar->getFrameCount();
	for (auto& army : enemyAllBattleUnit)
	{
		for (auto& unit : army.second)
		{
			//unit's position is valid while last update frame less than 10 seconds
			if (curFrame - unit.second.latestUpdateFrame < 24 * 10)
			{
				const Area* nearOurArea = getNearstRegion(unit.second.latestPosition, army.first.isFlyer(), false);
				if (nearOurArea != nullptr)
				{
					enemyNearOurBaseInfo[nearOurArea][army.first][unit.first] = unit.second;
				}
				const Area* nearEnemyArea = getNearstRegion(unit.second.latestPosition, army.first.isFlyer(), true);
				if (nearEnemyArea != nullptr)
				{
					enemyNearEnemyBaseInfo[nearEnemyArea][army.first][unit.first] = unit.second;
				}
			}
		}
	}


}



void InformationManager::checkOccupiedDetail()
{
	for (auto it = enemyOccupiedDetail.begin(); it != enemyOccupiedDetail.end();)
	{
		for (auto detailIt = it->second.begin(); detailIt != it->second.end();)
		{
			if (BWAPI::Broodwar->isVisible(BWAPI::TilePosition(detailIt->second.initPosition)))
			{
				bool isExist = false;
				for (auto u : BWAPI::Broodwar->getUnitsOnTile(detailIt->second.initPosition))
				{
					if (u->getPlayer() == BWAPI::Broodwar->enemy() && u->getType() == detailIt->second.unitType)
					{
						isExist = true;
						break;
					}
				}
				if (isExist == false)
				{
					unitGM.RemoveBuilding(detailIt->first, detailIt->second.initPosition);
					it->second.erase(detailIt++);
				}
				else
				{
					detailIt++;
				}
			}
			else
				detailIt++;
		}
		if (it->second.empty())
		{
			logInfo("InformationManager", "erase region " + to_string(occupiedRegions[1][it->first].x) + " " + to_string(occupiedRegions[1][it->first].y));
			occupiedRegions[1].erase(it->first);
			enemyOccupiedDetail.erase(it++);
		}
		else
			it++;
	}
}


BWAPI::TilePosition	InformationManager::GetNextExpandLocation()
{
	std::set<const Area*> enemyRegions;
	//for (auto u : BWAPI::Broodwar->getAllUnits())
	//{
	//	if (u->getPlayer() == BWAPI::Broodwar->enemy())
	//	{
	//		if (BWEMMap.GetArea(u->getTilePosition()) != NULL)
	//			enemyRegions.insert(BWEMMap.GetArea(u->getTilePosition()));
	//	}
	//}

	double gasClosest = 999999999;
	double closest = 999999999;
	BWAPI::TilePosition nextGasBase = BWAPI::TilePositions::None;
	BWAPI::TilePosition nextBase = BWAPI::TilePositions::None;
	for (auto const& area : BWEMMap.Areas())
	{
		if (occupiedRegions[0].find(&area) != occupiedRegions[0].end() || occupiedRegions[1].find(&area) != occupiedRegions[1].end())
		{
			continue;
		}
		if (enemyRegions.find(&area) != enemyRegions.end())
		{
			continue;
		}

		for (auto const& base : area.Bases())
		{
			int length = 0;
			BWEMMap.GetPath(Position(selfStartBaseLocation), Position(base.Location()), &length);
			if (length != -1)
			{
				if (length < gasClosest && !base.Geysers().empty() && !base.Minerals().empty())
				{
					gasClosest = length;
					nextGasBase = base.Location();
				}

				if (length < closest && !base.Minerals().empty())
				{
					closest = length;
					nextBase = base.Location();
				}
			}
		}
	}

	if (nextGasBase != TilePositions::None)
	{
		return nextGasBase;
	}
	else
	{
		return nextBase;
	}
}


bool InformationManager::isHasNatrualBase()
{
	if (occupiedRegions[0].find(BWEMMap.GetArea(selfNaturalBaseLocation)) != occupiedRegions[0].end())
	{
		return true;
		//for (auto& b : selfAllBase)
		//{
		//	if (BWEMMap.GetArea(b->getTilePosition()) == BWEMMap.GetArea(selfNaturalBaseLocation))
		//	{
		//		if (b->isCompleted())
		//		{
		//			return true;
		//		}
		//		else
		//		{
		//			return false;
		//		}
		//	}
		//}
		//return false;
	}
	else
	{
		return false;
	}
}


BWAPI::TilePosition	InformationManager::getOurNatrualLocation()
{
	if (selfNaturalBaseLocation != BWAPI::TilePositions::None)
		return selfNaturalBaseLocation;

	double closest = 999999;
	for (auto const& a : BWEMMap.Areas())
	{
		for (auto const& b : a.Bases())
		{
			if (BWEMMap.GetArea(b.Location()) == BWEMMap.GetArea(selfStartBaseLocation))
			{
				continue;
			}
			if (b.Geysers().empty())
			{
				continue;
			}
			int length;
			const CPPath& Path = BWEMMap.GetPath(Position(selfStartBaseLocation), Position(b.Location()), &length);
			if (length != -1 && length < closest)
			{
				closest = length;
				selfNaturalBaseLocation = b.Location();
				selfNaturalChokePoint = Position(Path.front()->Center());
			}
		}
	}

	int maxLength = 0;
	for (auto& c : BWEMMap.GetArea(selfNaturalBaseLocation)->ChokePoints())
	{
		int length;
		BWEMMap.GetPath(Position(selfStartBaseLocation), Position(c->Center()), &length);
		if (length > maxLength)
		{
			maxLength = length;
			selfNaturalToEnemyChoke = Position(c->Center());
		}
	}

	return selfNaturalBaseLocation;
}


Position InformationManager::getOurBaseToNatrualChokePosition()
{
	if (selfNaturalChokePoint == Positions::None)
	{
		getOurNatrualLocation();
	}
	return selfNaturalChokePoint;
}


std::map<const Area*, int>& InformationManager::getBaseGroudDistance()
{
	if (!baseGroundDistance.empty())
		return baseGroundDistance;

	for (auto const& b : BWEMMap.StartingLocations())
	{
		if (BWEMMap.GetArea(b) == BWEMMap.GetArea(selfStartBaseLocation))
			continue;
		int length = 0;
		BWEMMap.GetPath(Position(selfStartBaseLocation), Position(b), &length);
		baseGroundDistance[BWEMMap.GetArea(b)] = length / 32;
	}
	return baseGroundDistance;
}


std::map<const Area*, int>& InformationManager::getBaseAirDistance()
{
	if (!baseAirDistance.empty())
		return baseAirDistance;

	for (auto const& b : BWEMMap.StartingLocations())
	{
		if (BWEMMap.GetArea(b) == BWEMMap.GetArea(selfStartBaseLocation))
			continue;
		int length = int(selfStartBaseLocation.getDistance(b));
		baseAirDistance[BWEMMap.GetArea(b)] = length;
	}
	return baseAirDistance;
}


void InformationManager::setLocationEnemyBase(BWAPI::TilePosition Here)
{
	enemyStartBaseLocation = Here;
	//for we only detect enemy natural, we also update enemy base info
	updateOccupiedRegions(Here, BWAPI::Broodwar->enemy());

	double closest = 999999;
	for (auto const& a : BWEMMap.Areas())
	{
		for (auto const& b : a.Bases())
		{
			if (BWEMMap.GetArea(b.Location()) == BWEMMap.GetArea(enemyStartBaseLocation))
			{
				continue;
			}
			int length;
			const CPPath& Path = BWEMMap.GetPath(Position(enemyStartBaseLocation), Position(b.Location()), &length);
			if (length != -1 && length < closest)
			{
				closest = length;
				enemyNaturalBaseLocation = b.Location();
			}
		}
	}
}


void InformationManager::onUnitShow(BWAPI::Unit unit)
{
	updateUnit(unit);
}

void InformationManager::onUnitMorph(BWAPI::Unit unit)
{
 	updateUnit(unit);
}


map<string, std::pair<int, int>> InformationManager::getMaxInfluenceMapValue(set<TilePosition>& unitPositions)
{
	TilePosition maxPosition;
	int maxCount = 0;
	for (auto& p : unitPositions)
	{
		if (!IMGM.GetCell(p).im.buildingScore.empty())
		{
			int tmpCount = 0;
			map<string, std::pair<int, int>>& tmp = IMGM.GetCell(p).im.buildingScore;
			for (auto& b : tmp)
			{
				tmpCount += b.second.second;
			}
			if (tmpCount > maxCount)
			{
				maxPosition = p;
				maxCount = tmpCount;
			}
		}
	}
	return IMGM.GetCell(maxPosition).im.buildingScore;
}


map<string, std::pair<int, int>> InformationManager::getMaxInfluenceMapValue(TilePosition p, int range)
{
	double2 centerPoint(p.x, p.y);
	int maxCount = 0;
	TilePosition maxPosition;
	map<string, std::pair<int, int>> maxBuildingDetails;
	int xStartPosition = int(centerPoint.x - range < 0 ? 0 : centerPoint.x - range);
	int xEndPosition = int(centerPoint.x + range > BWAPI::Broodwar->mapWidth() - 1 ? BWAPI::Broodwar->mapWidth() - 1 : centerPoint.x + range);
	int yStartPosition = int(centerPoint.y - range < 0 ? 0 : centerPoint.y - range);
	int yEndPosition = int(centerPoint.y + range > BWAPI::Broodwar->mapHeight() - 1 ? BWAPI::Broodwar->mapHeight() - 1 : centerPoint.y + range);

	for (int x = xStartPosition; x <= xEndPosition; x++)
	{
		for (int y = yStartPosition; y <= yEndPosition; y++)
		{
			if (!IMGM.GetCell(TilePosition(x, y)).im.buildingScore.empty())
			{
				int tmpCount = 0;
				map<string, std::pair<int, int>>& tmp = IMGM.GetCell(TilePosition(x, y)).im.buildingScore;
				for (auto& b : tmp)
				{
					tmpCount += b.second.second;
				}
				if (tmpCount > maxCount)
				{
					maxPosition = TilePosition(x, y);
					maxCount = tmpCount;
				}
			}
		}
	}
	return IMGM.GetCell(maxPosition).im.buildingScore;
}


void InformationManager::updateAllUnit()
{
	//clear unit info
	int width = BWAPI::Broodwar->mapWidth();
	int height = BWAPI::Broodwar->mapHeight();
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			BWAPI::TilePosition tilePos(i, j);
			IMGM.GetCell(tilePos).im.enemyUnitAirForce = 0;
			IMGM.GetCell(tilePos).im.enemyUnitGroundForce = 0;
			IMGM.GetCell(tilePos).im.psionicStormDamage = 0;
			IMGM.GetCell(tilePos).im.walkableArea = 0;
			IMGM.GetCell(tilePos).im.unavailable = 0;
			IMGM.GetCell(tilePos).im.buildingOnTile = nullptr;

			int areaID = BWEMMap.GetTile(tilePos).AreaId();
			if (areaID == 0) {
				IMGM.GetCell(tilePos).im.walkableArea = 0;
			}
			else if(areaID == -1) {
				IMGM.GetCell(tilePos).im.walkableArea = 1;
			}
			else if(areaID > 0){
				//bool tileEntireWalkable = true;
				//for (int s = 0; s < 4; s++) {
				//	if (!tileEntireWalkable)
				//		break;
				//	for (int t = 0; t < 4; t++) {
				//		WalkPosition cur(i * 4 + s, j * 4 + t);
				//		if (!BWEMMap.GetMiniTile(cur).Walkable()) {
				//			tileEntireWalkable = false;
				//			break;
				//		}
				//	}
				//}
				//if (!tileEntireWalkable) {
				//	IMGM.GetCell(tilePos).im.walkableArea = 0;
				//}
				if (BWEMMap.GetMiniTile(WalkPosition(tilePos) + WalkPosition(1, 1)).Altitude() == 0 ||
					BWEMMap.GetMiniTile(WalkPosition(tilePos) + WalkPosition(1, 2)).Altitude() == 0 ||
					BWEMMap.GetMiniTile(WalkPosition(tilePos) + WalkPosition(2, 1)).Altitude() == 0 ||
					BWEMMap.GetMiniTile(WalkPosition(tilePos) + WalkPosition(2, 2)).Altitude() == 0) {
					IMGM.GetCell(tilePos).im.walkableArea = 0;
				}
				else if (BWEMMap.GetArea(tilePos)->ChokePoints().size() > 0) {
					IMGM.GetCell(tilePos).im.walkableArea = areaID + 1;
				}
				else {
					IMGM.GetCell(tilePos).im.walkableArea = 0;
				}
			}
			else {
				IMGM.GetCell(tilePos).im.walkableArea = 0;
			}
			unitGM.GetCell(tilePos).clearUnits();
			//if (int(IMGM.GetCell(TilePosition(i, j)).im.airForce) != 0)
			//{
			//	BWAPI::Broodwar->drawTextMap(i * 32, j * 32, "%d", int(IMGM.GetCell(TilePosition(i, j)).im.airForce));
			//}
			//if (int(IMGM.GetCell(TilePosition(i, j)).im.enemyUnitAirForce) != 0)
			//{
			//	BWAPI::Broodwar->drawTextMap(i * 32, j * 32, "%d", int(IMGM.GetCell(TilePosition(i, j)).im.enemyUnitAirForce));
			//}
		}
	}

	for(auto const& enemy : BWAPI::Broodwar->getAllUnits())
	{
		if (enemy->getPlayer() == Broodwar->enemy() || enemy->getPlayer() == Broodwar->self())
		{
			if (enemy->getType() != UnitTypes::Zerg_Larva && enemy->getType() != UnitTypes::Zerg_Egg && enemy->getType() != UnitTypes::Zerg_Lurker_Egg
				&& enemy->getType() != UnitTypes::Zerg_Cocoon)
			{
				unitGM.Add(enemy);
				if (enemy->getPlayer() == Broodwar->enemy() && !enemy->getType().isBuilding())
				{
					enemyAllBattleUnit[enemy->getType()][enemy] = unitInfo(enemy);
				}
			}
		}
		
		if (enemy->getPlayer() == Broodwar->enemy() && !enemy->getType().isBuilding() && enemy->getType().canAttack())
		{
			int attackRange = 0;
			if (enemy->getType() == Broodwar->enemy()->getRace().getWorker())
				attackRange = ((enemy->getType().groundWeapon().maxRange() + 31) / 32);
			else if (enemy->getType().groundWeapon() != BWAPI::WeaponTypes::None)
				attackRange = ((enemy->getType().groundWeapon().maxRange() + 31) / 32) + 1;
			else
				attackRange = ((enemy->getType().airWeapon().maxRange() + 31) / 32) + 1;
			
			int y_start = enemy->getTilePosition().y - attackRange > 0 ? enemy->getTilePosition().y - attackRange : 0;
			int y_end = enemy->getTilePosition().y + attackRange + 1 > BWAPI::Broodwar->mapHeight() - 1 ? BWAPI::Broodwar->mapHeight() - 1 : enemy->getTilePosition().y + attackRange + 1;
			
			int x_start = enemy->getTilePosition().x - attackRange > 0 ? enemy->getTilePosition().x - attackRange : 0;
			int x_end = enemy->getTilePosition().x + attackRange + 1 > BWAPI::Broodwar->mapWidth() - 1 ? BWAPI::Broodwar->mapWidth() - 1 : enemy->getTilePosition().x + attackRange + 1;
			
			for (int i = x_start; i <= x_end; i++)
			{
				for (int j = y_start; j <= y_end; j++)
				{
					if (enemy->getType().groundWeapon() != BWAPI::WeaponTypes::None)
						IMGM.GetCell(TilePosition(i, j)).im.enemyUnitGroundForce += enemy->getType().groundWeapon().damageAmount();
					if (enemy->getType().airWeapon() != BWAPI::WeaponTypes::None)
						IMGM.GetCell(TilePosition(i, j)).im.enemyUnitAirForce += enemy->getType().airWeapon().damageAmount();
				}
			}
		}

		if (enemy->getType().isBuilding() || enemy->getType()==UnitTypes::Zerg_Egg || enemy->getType() == UnitTypes::Zerg_Lurker_Egg) {
			if (!enemy->isFlying()) {
				BWAPI::TilePosition buildingPosition = enemy->getTilePosition();
				BWAPI::TilePosition buildingSize = enemy->getType().tileSize();
				for (int i = 0; i < buildingSize.x; i++) {
					for (int j = 0; j < buildingSize.y; j++) {
						if (buildingPosition.x + i >= 0 && buildingPosition.x + i <= width - 1 && buildingPosition.y + j >= 0 && buildingPosition.y + j <= height - 1) {
							IMGM.GetCell(TilePosition(buildingPosition.x + i, buildingPosition.y + j)).im.walkableArea = 0;
							IMGM.GetCell(TilePosition(buildingPosition.x + i, buildingPosition.y + j)).im.buildingOnTile = enemy;
						}
					}
				}
			}
		}
	}

	for (auto const& bullet : Broodwar->getBullets()) {
		if (bullet->getType() == BWAPI::BulletTypes::Psionic_Storm) {
			TilePosition centerPosition = TilePosition(bullet->getPosition());
			int radialRange = 7;
			int y_start = centerPosition.y - radialRange > 0 ? centerPosition.y - radialRange : 0;
			int y_end = centerPosition.y + radialRange > BWAPI::Broodwar->mapHeight() - 1 ? BWAPI::Broodwar->mapHeight() - 1 : centerPosition.y + radialRange;

			int x_start = centerPosition.x - radialRange > 0 ? centerPosition.x - radialRange : 0;
			int x_end = centerPosition.x + radialRange > BWAPI::Broodwar->mapWidth() - 1 ? BWAPI::Broodwar->mapWidth() - 1 : centerPosition.x + radialRange;

			for (int i = x_start; i <= x_end; i++)
			{
				for (int j = y_start; j <= y_end; j++)
				{
					if (TilePosition(i, j).getDistance(centerPosition) <= radialRange) {
						IMGM.GetCell(TilePosition(i, j)).im.psionicStormDamage = 1;
					}
				}
			}
		}
	}
	
	for (auto const& mineral : BWEMMap.Minerals()) {
		BWAPI::TilePosition mineralPosition = mineral->TopLeft();
		int mineralSizeX = 2, mineralSizeY = 1;
		for (int i = 0; i < mineralSizeX; i++) {
			for (int j = 0; j < mineralSizeY; j++) {
				if (mineralPosition.x + i >= 0 && mineralPosition.x + i <= width - 1 && mineralPosition.y + j >= 0 && mineralPosition.y + j <= height - 1)
					IMGM.GetCell(TilePosition(mineralPosition.x + i, mineralPosition.y + j)).im.walkableArea = 0;
			}
		}
	}
	for (auto const& geysers : BWEMMap.Geysers()) {
		BWAPI::TilePosition geysersPosition = geysers->TopLeft();
		int geysersSizeX = 4, geysersSizeY = 2;
		for (int i = 0 ; i < geysersSizeX; i++) {
			for (int j = 0; j < geysersSizeY; j++) {
				if (geysersPosition.x + i >= 0 && geysersPosition.x + i <= width - 1 && geysersPosition.y + j >= 0 && geysersPosition.y + j <= height - 1)
					IMGM.GetCell(TilePosition(geysersPosition.x + i, geysersPosition.y + j)).im.walkableArea = 0;
			}
		}
	}
	
	
	//for (int i = 0; i < BWAPI::Broodwar->mapWidth(); i++)
	//{
	//	for (int j = 0; j < BWAPI::Broodwar->mapHeight(); j++)
	//	{
	//		if (int(IMGM.GetCell(TilePosition(i, j)).im.strongAirForce) != 0)
	//		{
	//			BWAPI::Broodwar->drawTextMap(i * 32, j * 32, "%d", int(IMGM.GetCell(TilePosition(i, j)).im.strongAirForce));
	//		}
	//	}
	//}
	
	/*
	for (int i = 0; i < BWAPI::Broodwar->mapWidth(); i++)
	{
		for (int j = 0; j < BWAPI::Broodwar->mapHeight(); j++)
		{
			if (int(IMGM.GetCell(TilePosition(i, j)).im.psionicStormDamage) != 0)
			{
				BWAPI::Broodwar->drawTextMap(i * 32, j * 32, "%d", int(IMGM.GetCell(TilePosition(i, j)).im.psionicStormDamage));
			}
		}
	}
	*/
	/*
	for (int i = 0; i < BWAPI::Broodwar->mapWidth(); i++)
	{
		for (int j = 0; j < BWAPI::Broodwar->mapHeight(); j++)
		{
			if (int(IMGM.GetCell(TilePosition(i, j)).im.walkableArea) != 0)
			{
				BWAPI::Broodwar->drawTextMap(i * 32, j * 32, "%d", int(IMGM.GetCell(TilePosition(i, j)).im.walkableArea));
			}
		}
	}
	*/
}


void InformationManager::setInfluenceMap(BWAPI::Position initPosition, int attackRange, int exactAttackRange, int groundDamage, int airDamage, bool addOrdestroy, UnitType type)
{
	// get the approximate center of the building
	double2 normalLength = double2(1, 0);
	std::set<BWAPI::TilePosition> alreadySetPosition;
	int startDegree = 0;
	while (startDegree < 360)
	{
		double2 rotateNormal(normalLength.rotateReturn(startDegree));
		for (int length = 0; length <= attackRange; length++)
		{
			double2 rotateVector(rotateNormal * length + initPosition);
			BWAPI::TilePosition tmp(int(rotateVector.x), int(rotateVector.y));
			if (int(tmp.x) >= 0 && int(tmp.x) < BWAPI::Broodwar->mapWidth() && int(tmp.y) >= 0 && int(tmp.y) < BWAPI::Broodwar->mapHeight()
				&& alreadySetPosition.find(tmp) == alreadySetPosition.end())
			{
				alreadySetPosition.insert(tmp);
				gridMapCell& cell = IMGM.GetCell(tmp);
				if (addOrdestroy)
				{
					cell.im.groundForce += groundDamage;
					cell.im.airForce += airDamage;
					if (length <= exactAttackRange) {
						cell.im.strongAirForce += airDamage;
					}
					cell.im.buildingDetails[type] += 1;
				}
				else
				{
					cell.im.groundForce -= groundDamage;
					cell.im.airForce -= airDamage;
					if (length <= exactAttackRange) {
						cell.im.strongAirForce -= airDamage;
					}
					cell.im.buildingDetails[type] -= 1;
					if (cell.im.buildingDetails[type] == 0)
					{
						cell.im.buildingDetails.erase(type);
					}
				}
				cell.im.buildingScore.clear();
				for (const auto& item : cell.im.buildingDetails)
				{
					BattleArmy::armyTypeSaveFunc(nullptr, item.first, item.second, cell.im.buildingScore, false);
				}

			}
		}
		startDegree += 3;
	}
}


void InformationManager::addUnitInfluenceMap(BWAPI::Unit unit, bool addOrdestroy)
{
	if (addOrdestroy && enemyAllBuilding[unit->getType()].find(unit) != enemyAllBuilding[unit->getType()].end())
		return;
	if (!addOrdestroy && enemyAllBuilding[unit->getType()].find(unit) == enemyAllBuilding[unit->getType()].end())
		return;

	if (unit->getType() == BWAPI::UnitTypes::Protoss_Photon_Cannon
		|| unit->getType() == BWAPI::UnitTypes::Zerg_Spore_Colony
		|| unit->getType() == BWAPI::UnitTypes::Terran_Missile_Turret
		|| unit->getType() == BWAPI::UnitTypes::Zerg_Sunken_Colony
		|| unit->getType() == BWAPI::UnitTypes::Terran_Bunker)
	{		
		int attackRange = 0;
		int exactAttackRange = 0;
		int airDamage = 0;
		int groundDamage = 0;
		int buildingWidth = unit->getType().tileWidth();
		int buildingHeight = unit->getType().tileHeight();
		int maxSize = buildingWidth > buildingHeight ? buildingWidth / 2 : buildingHeight / 2;

		if (unit->getType() == BWAPI::UnitTypes::Terran_Bunker)
		{
			attackRange = 5 + maxSize + 3;
			exactAttackRange = (UnitTypes::Terran_Marine.groundWeapon().maxRange() + 31) / 32 + 1;
			groundDamage = BWAPI::UnitTypes::Terran_Marine.groundWeapon().damageAmount() * 4;
			airDamage = BWAPI::UnitTypes::Terran_Marine.airWeapon().damageAmount() * 4;
		}

		if (unit->getType().groundWeapon() != BWAPI::WeaponTypes::None)
		{
			attackRange = ((unit->getType().groundWeapon().maxRange() + 31) / 32) + 3;
			exactAttackRange = (unit->getType().groundWeapon().maxRange() + 31) / 32 + 1;
			groundDamage = unit->getType().groundWeapon().damageAmount();
		}
		if (unit->getType().airWeapon() != BWAPI::WeaponTypes::None)
		{
			attackRange = ((unit->getType().airWeapon().maxRange() + 31) / 32) + 3;
			exactAttackRange = (unit->getType().airWeapon().maxRange() + 31) / 32 + 1;
			airDamage = unit->getType().airWeapon().damageAmount();
		}

		double2 initPosition(unit->getTilePosition().x + buildingWidth / 2, unit->getTilePosition().y + buildingHeight / 2);
		setInfluenceMap(initPosition, attackRange, exactAttackRange, groundDamage, airDamage, addOrdestroy, unit->getType());
	}
} 


void InformationManager::updateUnit(BWAPI::Unit unit)
{
	if (unit->getPlayer() == BWAPI::Broodwar->enemy())
	{
		if (unit->getType().isBuilding())
		{
			addUnitInfluenceMap(unit, true);
			addOccupiedRegionsDetail(BWEMMap.GetArea(unit->getTilePosition()), BWAPI::Broodwar->enemy(), unit);

			if (unit->getType().isResourceDepot())
			{
				for (auto const& a : BWEMMap.Areas())
				{
					for (auto const& b : a.Bases())
					{
						if (b.Location().getDistance(unit->getTilePosition()) < 3)
						{
							enemyAllBase.insert(unit);
							updateOccupiedRegions(unit->getTilePosition(), BWAPI::Broodwar->enemy());
							break;
						}
					}
				}
			}

			enemyAllBuilding[unit->getType()][unit] = buildingInfo(unit);
		}
		else
		{
			enemyAllBattleUnit[unit->getType()][unit] = unitInfo(unit);
		}
	}
	else if (unit->getPlayer() == BWAPI::Broodwar->self())
	{
		if (unit->getType().isBuilding())
		{
			selfAllBattleUnit[BWAPI::UnitTypes::Zerg_Drone].erase(unit);

			addOccupiedRegionsDetail(BWEMMap.GetArea(unit->getTilePosition()), BWAPI::Broodwar->self(), unit);
			if (unit->getType().isResourceDepot())
			{
				for (auto const& a : BWEMMap.Areas())
				{
					for (auto const& b : a.Bases())
					{
						if (b.Location().getDistance(unit->getTilePosition()) < 3)
						{
							selfAllBase.insert(unit);
							updateOccupiedRegions(unit->getTilePosition(), BWAPI::Broodwar->self());
							break;
						}
					}
				}
			}

			selfAllBuilding[unit->getType()].insert(unit);
		}
		else
		{
			selfAllBattleUnit[unit->getType()].insert(unit);
		}
	}
}


bool InformationManager::isEnemyHasInvisibleUnit()
{
	for (auto eU : enemyAllBattleUnit)
	{
		if (eU.first.hasPermanentCloak() 
			|| eU.first == BWAPI::UnitTypes::Zerg_Lurker
			|| eU.first == BWAPI::UnitTypes::Terran_Wraith)
		{
			return true;
		}
	}
	return false;
}


bool InformationManager::isEnemyHasFlyerAttacker()
{
	for (auto eU : enemyAllBattleUnit)
	{
		if (eU.first.isFlyer() && 
			(eU.first.airWeapon() != BWAPI::WeaponTypes::None))
		{
			return true;
		}
	}
	return false;
}


void InformationManager::onLurkerMorph(Unit unit)
{
	if (selfAllBattleUnit.find(UnitTypes::Zerg_Hydralisk) != selfAllBattleUnit.end())
	{
		for (std::set<Unit>::iterator it = selfAllBattleUnit[UnitTypes::Zerg_Hydralisk].begin(); it != selfAllBattleUnit[UnitTypes::Zerg_Hydralisk].end(); it++)
		{
			if ((*it)->isMorphing())
			{
				selfAllBattleUnit[UnitTypes::Zerg_Hydralisk].erase(it);
				break;
			}
		}
	}
}


void InformationManager::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit->getPlayer() == BWAPI::Broodwar->enemy())
	{
		killedEnemyCount[unit->getType()] += 1;

		if (unit->getType().isBuilding())
		{
			addUnitInfluenceMap(unit, false);
			destroyOccupiedRegionsDetail(BWEMMap.GetArea(unit->getTilePosition()), BWAPI::Broodwar->enemy(), unit);

			if (unit->getType().isResourceDepot())
			{
				for (auto const& a : BWEMMap.Areas())
				{
					for (auto const& b : a.Bases())
					{
						if (b.Location().getDistance(unit->getTilePosition()) < 3)
						{
							enemyAllBase.erase(unit);
							break;
						}
					}
				}
			}
			if (enemyAllBuilding.find(unit->getType()) != enemyAllBuilding.end())
				enemyAllBuilding[unit->getType()].erase(unit);
		}
		else
		{
			if (enemyAllBattleUnit.find(unit->getType()) != enemyAllBattleUnit.end())
			{
				enemyAllBattleUnit[unit->getType()].erase(unit);
				//logInfo("InformationManager", "onUnitDestroy " + to_string(unit->getType()), "InformationManager");
			}
		}
	}
	else if (unit->getPlayer() == BWAPI::Broodwar->self())
	{
		killedOurCount[unit->getType()] += 1;

		if (unit->getType().isBuilding())
		{
			destroyOccupiedRegionsDetail(BWEMMap.GetArea(unit->getTilePosition()), BWAPI::Broodwar->self(), unit);

			if (unit->getType().isResourceDepot())
			{
				for (auto const& a : BWEMMap.Areas())
				{
					for (auto const& b : a.Bases())
					{
						if (b.Location().getDistance(unit->getTilePosition()) < 3)
						{
							selfAllBase.erase(unit);
							break;
						}
					}
				}
			}

			if (selfAllBuilding.find(unit->getType()) != selfAllBuilding.end())
				selfAllBuilding[unit->getType()].erase(unit);
		}
		else
		{
			if (selfAllBattleUnit.find(unit->getType()) != selfAllBattleUnit.end())
				selfAllBattleUnit[unit->getType()].erase(unit);
		}
	}
}


BWAPI::Unit InformationManager::GetOurBaseUnit()
{
	BWAPI::Unit minBase = nullptr;
	int minDistance = 9999;
	for(auto u : BWAPI::Broodwar->self()->getUnits())
	{
		if (u->getType() == BWAPI::UnitTypes::Zerg_Lair
			|| u->getType() == BWAPI::UnitTypes::Zerg_Hive)
		{
			minBase = u;
			break;
		}

		if (u->getType() == BWAPI::UnitTypes::Zerg_Hatchery && u->isCompleted())
		{
			if (u->getDistance(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation())) < minDistance)
			{
				minDistance = u->getDistance(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()));
				minBase = u;
			}
		}
	}
	return minBase;

}

BWAPI::Position InformationManager::GetEnemyBasePosition()
{
	if (enemyStartBaseLocation == BWAPI::TilePositions::None)
		return BWAPI::Positions::None;
	else
		return BWAPI::Position(enemyStartBaseLocation);
}

BWAPI::Position	InformationManager::GetEnemyNaturalPosition()
{
	if (enemyNaturalBaseLocation == BWAPI::TilePositions::None)
		return BWAPI::Positions::None;
	else
		return BWAPI::Position(enemyNaturalBaseLocation);
}


InformationManager& InformationManager::Instance()
{
	static InformationManager instance;
	return instance;
}


void InformationManager::updateOccupiedRegions(TilePosition basePosition, BWAPI::Player player)
{
	// if the region is valid (flying buildings may be in NULL regions)
	if (BWEMMap.GetArea(basePosition) != nullptr)
	{
		// add it to the list of occupied regions
		if (player == BWAPI::Broodwar->self())
			occupiedRegions[0][BWEMMap.GetArea(basePosition)] = basePosition;
		else
			occupiedRegions[1][BWEMMap.GetArea(basePosition)] = basePosition;
	}
}


void InformationManager::addOccupiedRegionsDetail(const Area * region, BWAPI::Player player, BWAPI::Unit building)
{
	if (region)
	{
		unitGM.Add(building);
		if (player == BWAPI::Broodwar->self())
		{
			selfOccupiedDetail[region][building] = buildingInfo(building);
		}
		else
		{
			enemyOccupiedDetail[region][building] = buildingInfo(building);
		}
	}
}


void InformationManager::destroyOccupiedRegionsDetail(const Area * region, BWAPI::Player player, BWAPI::Unit building)
{
	if (region)
	{
		unitGM.RemoveBuilding(building, building->getTilePosition());

		if (player == BWAPI::Broodwar->self())
		{
			if (selfOccupiedDetail.find(region) != selfOccupiedDetail.end())
			{	
				selfOccupiedDetail[region].erase(building);
				if (selfOccupiedDetail[region].empty())
				{
					occupiedRegions[0].erase(region);
					selfOccupiedDetail.erase(region);
				}
			}
		}
		else
		{
			if (enemyOccupiedDetail.find(region) != enemyOccupiedDetail.end())
			{
				enemyOccupiedDetail[region].erase(building);
				if (enemyOccupiedDetail[region].empty())
				{
					logInfo("InformationManager", "erase region " + to_string(occupiedRegions[1][region].x) + " " + to_string(occupiedRegions[1][region].y));
					occupiedRegions[1].erase(region);
					enemyOccupiedDetail.erase(region);
				} 
			}
		}
	}
}


std::map<const Area*, TilePosition>& InformationManager::getBaseOccupiedRegions(BWAPI::Player player)
{
	if (player == BWAPI::Broodwar->self())
	{
		return occupiedRegions[0];
	}
		
	else
		return occupiedRegions[1];
}


void InformationManager::updateEnemyBlockInfo() {
	if (enemyStartBaseLocation == BWAPI::TilePositions::None)
		return;
	if (enemyNaturalBaseLocation == BWAPI::TilePositions::None)
		return;
	const Area* enemyBaseArea = BWEMMap.GetArea(enemyStartBaseLocation);
	const Area* enemyNaturalArea = BWEMMap.GetArea(enemyNaturalBaseLocation);
	const CPPath& basepath = BWEMMap.GetPath(Position(enemyStartBaseLocation), Position(enemyNaturalBaseLocation));
	const ChokePoint* baseCP = basepath[0];
	if (enemyNaturaltoBaseAreas.size() == 0) {
		for (auto const& cp : basepath) {
			const Area* area1 = cp->GetAreas().first;
			const Area* area2 = cp->GetAreas().second;
			enemyNaturaltoBaseAreas.insert(area1);
			enemyNaturaltoBaseAreas.insert(area2);
		}
		for (int i = 0; i < Broodwar->mapWidth(); i++) {
			for (int j = 0; j < Broodwar->mapHeight(); j++) {
				TilePosition curTile(i, j);
				const Area* curArea = BWEMMap.GetArea(curTile);
				if(enemyNaturaltoBaseAreas.find(curArea)!= enemyNaturaltoBaseAreas.end()){
					IMGM.GetCell(curTile).im.isInEnemyHome = true;
				}
			}
		}
	}
	const ChokePoint* naturalCP = BWEMMap.GetPath(Position(enemyNaturalBaseLocation), Position(Broodwar->self()->getStartLocation()))[0];
	enemyBaseChoke = TilePosition(baseCP->Center());
	enemyNaturalChoke = TilePosition(naturalCP->Center());
	//Broodwar->drawCircleMap(Position(enemyStartBaseLocation), 20, Colors::Red, true);
	//Broodwar->drawCircleMap(Position(enemyNaturalBaseLocation), 20, Colors::Blue, true);
	//Broodwar->drawCircleMap(Position(baseCP->Center()), 20, Colors::Cyan, true);
	//Broodwar->drawCircleMap(Position(naturalCP->Center()), 20, Colors::Yellow, true);

	Position enemyNaturalEntrance = getAreaEntrance(naturalCP, enemyNaturalArea);
	Position enemyBaseEntrance = getAreaEntrance(baseCP, enemyBaseArea);

	if (enemyNaturalEntrance != Positions::None && Broodwar->isVisible(TilePosition(enemyNaturalEntrance))) {
		enemyBlockingNatural = Astar::Instance().bfsTargetArea(enemyNaturalChoke, enemyNaturalBaseLocation);
		if (enemyBlockingNatural) {
			enemyEverBlockingNatural = true;
			return;
		}
	}
	if (enemyBaseEntrance != Positions::None && Broodwar->isVisible(TilePosition(enemyBaseEntrance))) {
		enemyBlockingBase = Astar::Instance().bfsTargetArea(enemyBaseChoke, enemyStartBaseLocation);
		if (enemyBlockingBase) {
			enemyEverBlockingBase = true;
		}
	}
}


Position InformationManager::getAreaEntrance(const ChokePoint* cp, const Area* target) {
	double2 p1 = Position(cp->Center());
	double2 p2 = Position(cp->Pos(ChokePoint::end1));
	double2 p3 = Position(cp->Pos(ChokePoint::end2));
	p2 = p1 + (p2 - p1) * 96 / (p2 - p1).len();
	p3 = p1 + (p3 - p1) * 96 / (p3 - p1).len();
	double2 res1 = (p2 + p3) / 2.0;
	double eps = 1e-8;
	if ((res1 - p1).len() < eps) {
		res1 = p1 + double2(p3.y - p1.y, -(p3.x - p1.x));
	}
	double2 res2 = p1 + (p1 - res1);
	res1 = p1 + (res1 - p1) * 96 / (res1 - p1).len();
	res2 = p1 + (res2 - p1) * 96 / (res2 - p1).len();
	Position pres1 = Position(res1);
	Position pres2 = Position(res2);
	if (BWEMMap.GetArea(WalkPosition(pres1)) == target) {
		return pres1;
	}
	if (BWEMMap.GetArea(WalkPosition(pres2)) == target) {
		return pres2;
	}
	return Positions::None;
}
