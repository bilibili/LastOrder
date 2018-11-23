#pragma once
#include "ScoutTactic.h"
#include "InformationManager.h"


ScoutTactic::ScoutTactic()
{

	if (InformationManager::Instance().GetEnemyBasePosition() == BWAPI::Positions::None)
		state = scoutForEnemyBase;
	else
		state = scoutForEnemyInfo;

	if (state == scoutForEnemyBase)
	{
		/***********modify begin***********/
		/*
		for(auto const& startLocation : BWAPI::Broodwar->getStartLocations())
		{
			if (startLocation != BWAPI::Broodwar->self()->getStartLocation()){
				scoutLocation.push_back(scoutTarget(startLocation, 0 - int(startLocation.getDistance(BWAPI::Broodwar->self()->getStartLocation()))));
			}
		}
		*/
		// instead of using the priority of distance to base, use the distance to current position
		auto currentLocation = BWAPI::Broodwar->self()->getStartLocation();
		std::map<TilePosition, bool> closedList;
		for (auto const& startLocation : BWAPI::Broodwar->getStartLocations())
		{
			closedList[startLocation] = false;
		}
		unsigned closedVisitedIndex = 0;
		closedList[currentLocation] = true;
		closedVisitedIndex++;
		while (closedVisitedIndex < BWAPI::Broodwar->getStartLocations().size()) {
			double minDistance = 9999999;
			BWAPI::TilePosition nextLocation = BWAPI::TilePositions::None;
			for (auto const& startLocation : BWAPI::Broodwar->getStartLocations())
			{
				if (closedList[startLocation]==false) {
					double distanceToCurrent = currentLocation.getDistance(startLocation);
					if (distanceToCurrent < minDistance) {
						minDistance = distanceToCurrent;
						nextLocation = startLocation;
					}
				}
			}
			scoutLocation.push_back(scoutTarget(nextLocation, -int(closedVisitedIndex)));

			currentLocation = nextLocation;
			closedList[currentLocation] = true;
			closedVisitedIndex++;
		}
		/***********modify end***********/

		if (scoutLocation.size() == 1)
		{
			InformationManager::Instance().setLocationEnemyBase(scoutLocation.front().location);
			scoutLocation.clear();
		}
	}

	endFlag = false;

	double2 direct1(1, 0);
	moveDirections.push_back(direct1);
	double2 direct2(1, 1);
	moveDirections.push_back(direct2);
	double2 direct3(0, 1);
	moveDirections.push_back(direct3);
	double2 direct4(-1, 1);
	moveDirections.push_back(direct4);

	double2 direct5(-1, 0);
	moveDirections.push_back(direct5);
	double2 direct6(-1, -1);
	moveDirections.push_back(direct6);
	double2 direct7(0, -1);
	moveDirections.push_back(direct7);
	double2 direct8(1, -1);
	moveDirections.push_back(direct8);

	double2 stay(0, 0);

	//set different type of unit to the same scout array
	for (auto army : tacticArmy)
	{
		if (army.second->getUnits().size() > 0)
		{
			for (auto armyUnit : army.second->getUnits())
			{
				overLordIdle.push_back(Scout(armyUnit.unit));
			}
		}
	}

	safe_count = 24 * 3; 

	double2 position1(0, 0);
	enemyBaseSquarePoints.push_back(position1);
	double2 position2(10 * 32, -10 * 32);
	enemyBaseSquarePoints.push_back(position2);
	double2 position3(10 * 32, 10 * 32);
	enemyBaseSquarePoints.push_back(position3);
	double2 position4(-10 * 32, 10 * 32);
	enemyBaseSquarePoints.push_back(position4);
	double2 position5(-10 * 32, -10 * 32);
	enemyBaseSquarePoints.push_back(position5);
	wanderingID = 0;

	enemyBaseSquareAstar.push_back(std::make_pair(1, -1));
	enemyBaseSquareAstar.push_back(std::make_pair(1, 1));
	enemyBaseSquareAstar.push_back(std::make_pair(-1, 1));
	enemyBaseSquareAstar.push_back(std::make_pair(-1, -1));
	

	for (int i = 0; i < 9; i++) {
		blockDirection.push_back(0);
	}
}


BWAPI::TilePosition	ScoutTactic::getNextScoutLocation()
{
	if (zerglingScoutLocation.size() == 0)
	{
		std::map<const Area*, TilePosition>	 myRegion = InformationManager::Instance().getBaseOccupiedRegions(BWAPI::Broodwar->self());

		for (auto const& a : BWEMMap.Areas())
		{
			if (a.AccessibleNeighbours().empty())
			{
				continue;
			}
			if (myRegion.find(&a) == myRegion.end())
			{
				for (auto const& b : a.Bases())
				{
					int pathLong = 0;
					BWEMMap.GetPath(Position(b.Location()), Position(Broodwar->self()->getStartLocation()), &pathLong);
					zerglingScoutLocation.push_back(scoutTarget(b.Location(), pathLong));
				}
			}
		}
		std::sort(zerglingScoutLocation.begin(), zerglingScoutLocation.end());
	}

	if (zerglingScoutLocation.size() > 0)
	{
		BWAPI::TilePosition returnValue = zerglingScoutLocation.back().location;
		zerglingScoutLocation.pop_back();
		return returnValue;
	}
	else
	{
		return BWAPI::TilePositions::None;
	}

}

int cloestDirection_r(BWAPI::Position myPosition, BWAPI::Position enemyPosition)
{
	/*ID
	1 : (1, 0)(right)
	2 : (1, 1)
	3 : (0, 1)(down)
	4 : (-1, 1)
	5 : (-1, 0)
	6 : (-1, -1)
	7 : (0, -1)
	8 : (1, -1)*/
	double eps = 1e-8;
	double PI = acos(-1.0);
	double2 v = enemyPosition - myPosition;
	if (abs(v.x) < eps && abs(v.y) < eps)
	{
		// same position, move right
		double2 enemyDirection(0, 0);
		double2 reverseDirection(1, 0);
		int id = 1;
		return id;
	}
	if (v.x > 0 && abs(v.y) < eps) {
		double2 enemyDirection(1, 0);
		double2 reverseDirection(-1, 0);
		int id = 5;
		return id;
	}
	if (v.x < 0 && abs(v.y) < eps) {
		double2 enemyDirection(-1, 0);
		double2 reverseDirection(1, 0);
		int id = 1;
		return id;
	}
	if (abs(v.x) < eps && v.y > 0) {
		double2 enemyDirection(0, 1);
		double2 reverseDirection(0, -1);
		int id = 7;
		return id;
	}
	if (abs(v.x) < eps && v.y < 0) {
		double2 enemyDirection(0, -1);
		double2 reverseDirection(0, 1);
		int id = 3;
		return id;
	}
	/*
	double degree = atan(v.y / v.x) * 180.0 / PI;
	if (v.x < 0) {
		degree += 180.0;
	}
	else if (v.y < 0) {
		degree += 360.0;
	}
	*/
	double degree = atan2(v.y, v.x) * 180.0 / PI;
	if (degree < 0) {
		degree += 360.0;
	}

	if (degree >= 22.5 && degree < 67.5) {
		double2 enemyDirection(1, 1);
		double2 reverseDirection(-1, -1);
		int id = 6;
		return id;
	}
	else if (degree >= 67.5 && degree < 112.5) {
		double2 enemyDirection(0, 1);
		double2 reverseDirection(0, -1);
		int id = 7;
		return id;
	}
	else if (degree >= 112.5 && degree < 157.5) {
		double2 enemyDirection(-1, 1);
		double2 reverseDirection(1, -1);
		int id = 8;
		return id;
	}
	else if (degree >= 157.5 && degree < 202.5) {
		double2 enemyDirection(-1, 0);
		double2 reverseDirection(1, 0);
		int id = 1;
		return id;
	}
	else if (degree >= 202.5 && degree < 247.5) {
		double2 enemyDirection(-1, -1);
		double2 reverseDirection(1, 1);
		int id = 2;
		return id;
	}
	else if (degree >= 247.5 && degree < 292.5) {
		double2 enemyDirection(0, -1);
		double2 reverseDirection(0, 1);
		int id = 3;
		return id;
	}
	else if (degree >= 292.5 && degree < 337.5) {
		double2 enemyDirection(1, -1);
		double2 reverseDirection(-1, 1);
		int id = 4;
		return id;
	}
	else {
		double2 enemyDirection(1, 0);
		double2 reverseDirection(-1, 0);
		int id = 5;
		return id;
	}
}


bool inEnemyBaseArea(BWAPI::Position distination) {
	if (BWEMMap.GetArea(BWAPI::TilePosition(distination)) != NULL &&
		BWEMMap.GetArea(BWAPI::TilePosition(distination)) != BWEMMap.GetArea(BWAPI::TilePosition(InformationManager::Instance().GetEnemyBasePosition()))) {
		return false;
	}
	return true;
}

bool reachableInEnemyBase(BWAPI::TilePosition myPosition, BWAPI::TilePosition distination, int wrap = 0) {
	// if out of map
	if (distination.x < 0 || distination.x >= BWAPI::Broodwar->mapWidth() || distination.y < 0 || distination.y >= BWAPI::Broodwar->mapHeight()) {
		return false;
	}
	/*
	// if unreachable
	if (BWEMMap.GetArea(distination) == NULL) {
		return false;
	}
	if (!BWAPI::Broodwar->hasPath(BWAPI::Position(myPosition), BWAPI::Position(distination))) {
		return false;
	}
	// if self is in enemy base area but distination is out of enemy base area
	if (BWEMMap.GetArea(myPosition) == BWEMMap.GetArea(BWAPI::TilePosition(InformationManager::Instance().GetEnemyBasePosition()))) {
		if (BWEMMap.GetArea(distination) != BWEMMap.GetArea(BWAPI::TilePosition(InformationManager::Instance().GetEnemyBasePosition()))) {
			return false;
		}
	}
	
	// if altitude is less than 10
	//if (BWEMMap.GetMiniTile(WalkPosition(distination)).Altitude() < 10) {
	//	return false;
	//}

	const Area* enemyBaseArea = BWEMMap.GetArea(BWAPI::TilePosition(InformationManager::Instance().GetEnemyBasePosition()));
	// if mine or gas
	for (auto const& mineral : enemyBaseArea->Minerals()) {
		BWAPI::TilePosition mineralPosition = mineral->TopLeft();
		int mineralSizeX = 2, mineralSizeY = 1;
		if ((distination.x >= mineralPosition.x - wrap && distination.x < mineralPosition.x + mineralSizeX + wrap)
			&& (distination.y >= mineralPosition.y - wrap && distination.y < mineralPosition.y + mineralSizeY + wrap)) {
			return false;
		}
	}
	for (auto const& geysers : enemyBaseArea->Geysers()) {
		BWAPI::TilePosition geysersPosition = geysers->TopLeft();
		int geysersSizeX = 2, geysersSizeY = 1;
		if ((distination.x >= geysersPosition.x - wrap && distination.x < geysersPosition.x + geysersSizeX + wrap)
			&& (distination.y >= geysersPosition.y - wrap && distination.y < geysersPosition.y + geysersSizeY + wrap)) {
			return false;
		}
	}
	// if there is a building blocking the way
	for (auto const& building : BWAPI::Broodwar->getAllUnits()) {
		if (building->getType().isBuilding())
		{
			BWAPI::TilePosition buildingPosition = building->getTilePosition();
			BWAPI::TilePosition buildingSize = building->getType().tileSize();
			if ((distination.x >= buildingPosition.x - wrap && distination.x < buildingPosition.x + buildingSize.x + wrap)
				&& (distination.y >= buildingPosition.y - wrap && distination.y < buildingPosition.y + buildingSize.y + wrap)) {
				return false;
			}
			// add fort logic: if distination in shooting range
			if (building->getType().canAttack()) {
				int range = -1;
				if (building->getType().airWeapon()) {
					range = (building->getType().airWeapon().maxRange() + 31) / 32;
				}
				else {
					range = (building->getType().groundWeapon().maxRange() + 31) / 32;
				}
				double distance = myPosition.getDistance(buildingPosition + buildingSize / 2);
				if (distance < range + 2) {
					return false;
				}
			}
		}
	}
	return true;
	*/
	if (BWEMMap.GetArea(distination) != BWEMMap.GetArea(BWAPI::TilePosition(InformationManager::Instance().GetEnemyBasePosition()))) {
		return false;
	}
	if (InformationManager::Instance().getEnemyInfluenceMap(distination.x, distination.y).walkableArea==0) {
		return false;
	}
	return true;
}


BWAPI::TilePosition ScoutTactic::droneDodge(Scout& drone) {
	//BWAPI::Broodwar->printf("safe count %d",safe_count);

	std::vector<double2> droneMoveDirections;
	double2 stay(0, 0);
	droneMoveDirections.push_back(stay);
	for (int i = 0; i < 8; i++) {
		droneMoveDirections.push_back(moveDirections[i]);
	}

	vector<int> potentialScore(9);  // moving to this direction can avoid how much damage
	vector<int> directionID(9);

	vector<bool> directionIDUsed(9); // enemies on one direction only count once

	for (int i = 0; i < 9; i++) {
		potentialScore[i] = 0;
		directionID[i] = i;
		directionIDUsed[i] = false;
	}


	BWAPI::Position curPosition = drone.scoutUnit->getPosition();
	bool dangereous = false;
	BWAPI::Position enemyBasePosition = InformationManager::Instance().GetEnemyBasePosition();
	BWAPI::Position enemyBaseSize = BWAPI::Position(BWAPI::Broodwar->enemy()->getRace().getResourceDepot().tileSize());
	enemyBasePosition += enemyBaseSize / 2;

	if (BWEMMap.GetArea(BWAPI::TilePosition(curPosition)) != BWEMMap.GetArea(BWAPI::TilePosition(InformationManager::Instance().GetEnemyBasePosition()))) {
		int reverseDirID_BASE = cloestDirection_r(curPosition, enemyBasePosition);
		
		potentialScore[reverseDirID_BASE] += -6;
		potentialScore[reverseDirID_BASE + 1 > 8 ? reverseDirID_BASE + 1 - 8 : reverseDirID_BASE + 1] += -4;
		potentialScore[reverseDirID_BASE - 1 < 1 ? reverseDirID_BASE - 1 + 8 : reverseDirID_BASE - 1] += -4;
		potentialScore[reverseDirID_BASE + 2 > 8 ? reverseDirID_BASE + 2 - 8 : reverseDirID_BASE + 2] += 1;
		potentialScore[reverseDirID_BASE - 2 < 1 ? reverseDirID_BASE - 2 + 8 : reverseDirID_BASE - 2] += 1;
		potentialScore[reverseDirID_BASE + 3 > 8 ? reverseDirID_BASE + 3 - 8 : reverseDirID_BASE + 3] += 2;
		potentialScore[reverseDirID_BASE - 3 < 1 ? reverseDirID_BASE - 3 + 8 : reverseDirID_BASE - 3] += 2;
		potentialScore[reverseDirID_BASE + 4 > 8 ? reverseDirID_BASE + 4 - 8 : reverseDirID_BASE + 4] += 3;
	}

	for (auto const& enemy : BWAPI::Broodwar->getAllUnits()) 
	{
		if (enemy->getPlayer() == Broodwar->enemy() && enemy->getType().canAttack() && enemy->getType() != BWAPI::Broodwar->enemy()->getRace().getWorker())
		{
			BWAPI::Position enemyPosition = BWAPI::Position(unitInfo(enemy).latestPosition);
			int reverseDirID = cloestDirection_r(curPosition, enemyPosition);

			double distance = enemyPosition.getDistance(curPosition);
			BWAPI::Position enemySize = BWAPI::Position(enemy->getType().tileSize());
			distance -= max(enemySize.x, enemySize.y) + 32;

			double enemySpeed = enemy->getType().topSpeed();
			bool hasAirWeapon = enemy->getType().airWeapon() != WeaponTypes::Enum::None;
			bool hasGroundWeapon = enemy->getType().groundWeapon() != WeaponTypes::Enum::None;
			if (hasAirWeapon) {
				double range = (double)(enemy->getType().airWeapon().maxRange());
				int damage = enemy->getType().airWeapon().damageAmount();
				if (distance <= range + enemySpeed * 24 * 3 + 5 * 32) {
					dangereous = true;
					if (directionIDUsed[reverseDirID] == false) {
						if (enemy->getType().isBuilding()) {
							potentialScore[reverseDirID] += 10000;
							potentialScore[reverseDirID + 1 > 8 ? reverseDirID + 1 - 8 : reverseDirID + 1] += 15000;
							potentialScore[reverseDirID - 1 < 1 ? reverseDirID - 1 + 8 : reverseDirID - 1] += 15000;
							potentialScore[reverseDirID + 2 > 8 ? reverseDirID + 2 - 8 : reverseDirID + 2] += 12000;
							potentialScore[reverseDirID - 2 < 1 ? reverseDirID - 2 + 8 : reverseDirID - 2] += 12000;
							potentialScore[reverseDirID + 3 > 8 ? reverseDirID + 3 - 8 : reverseDirID + 3] += -10000;
							potentialScore[reverseDirID - 3 < 1 ? reverseDirID - 3 + 8 : reverseDirID - 3] += -10000;
							potentialScore[reverseDirID + 4 > 8 ? reverseDirID + 4 - 8 : reverseDirID + 4] += -10000;
						}
						else {
							potentialScore[reverseDirID] += 10;
							potentialScore[reverseDirID + 1 > 8 ? reverseDirID + 1 - 8 : reverseDirID + 1] += 7;
							potentialScore[reverseDirID - 1 < 1 ? reverseDirID - 1 + 8 : reverseDirID - 1] += 7;
							potentialScore[reverseDirID + 2 > 8 ? reverseDirID + 2 - 8 : reverseDirID + 2] += 1;
							potentialScore[reverseDirID - 2 < 1 ? reverseDirID - 2 + 8 : reverseDirID - 2] += 1;
							potentialScore[reverseDirID + 3 > 8 ? reverseDirID + 3 - 8 : reverseDirID + 3] += -7;
							potentialScore[reverseDirID - 3 < 1 ? reverseDirID - 3 + 8 : reverseDirID - 3] += -7;
							potentialScore[reverseDirID + 4 > 8 ? reverseDirID + 4 - 8 : reverseDirID + 4] += -10;
						}
						directionIDUsed[reverseDirID] = true;
					}
				}
			}
			else if (hasGroundWeapon) {
				double range = (double)(enemy->getType().groundWeapon().maxRange());
				int damage = enemy->getType().groundWeapon().damageAmount();
				//BWAPI::Broodwar->printf("enemy with ground weapon range %f, damage %d", range, damage);
				if (distance <= range + enemySpeed * 24 * 3 + 5 * 32) {
					dangereous = true;
					if (directionIDUsed[reverseDirID] == false) {
						if (enemy->getType().isBuilding()) {
							potentialScore[reverseDirID] += 10000;
							potentialScore[reverseDirID + 1 > 8 ? reverseDirID + 1 - 8 : reverseDirID + 1] += 15000;
							potentialScore[reverseDirID - 1 < 1 ? reverseDirID - 1 + 8 : reverseDirID - 1] += 15000;
							potentialScore[reverseDirID + 2 > 8 ? reverseDirID + 2 - 8 : reverseDirID + 2] += 12000;
							potentialScore[reverseDirID - 2 < 1 ? reverseDirID - 2 + 8 : reverseDirID - 2] += 12000;
							potentialScore[reverseDirID + 3 > 8 ? reverseDirID + 3 - 8 : reverseDirID + 3] += -10000;
							potentialScore[reverseDirID - 3 < 1 ? reverseDirID - 3 + 8 : reverseDirID - 3] += -10000;
							potentialScore[reverseDirID + 4 > 8 ? reverseDirID + 4 - 8 : reverseDirID + 4] += -10000;
						}
						else {
							potentialScore[reverseDirID] += 10;
							potentialScore[reverseDirID + 1 > 8 ? reverseDirID + 1 - 8 : reverseDirID + 1] += 7;
							potentialScore[reverseDirID - 1 < 1 ? reverseDirID - 1 + 8 : reverseDirID - 1] += 7;
							potentialScore[reverseDirID + 2 > 8 ? reverseDirID + 2 - 8 : reverseDirID + 2] += 1;
							potentialScore[reverseDirID - 2 < 1 ? reverseDirID - 2 + 8 : reverseDirID - 2] += 1;
							potentialScore[reverseDirID + 3 > 8 ? reverseDirID + 3 - 8 : reverseDirID + 3] += -7;
							potentialScore[reverseDirID - 3 < 1 ? reverseDirID - 3 + 8 : reverseDirID - 3] += -7;
							potentialScore[reverseDirID + 4 > 8 ? reverseDirID + 4 - 8 : reverseDirID + 4] += -10;
						}
						directionIDUsed[reverseDirID] = true;
					}
				}
			}
		}
	}

	
	srand(time(NULL));
	for (int i = 0; i < 9; i++) {
		potentialScore[i] += rand() % 2;
	}
	
	

	string output = "";
	for (int i = 1; i < 9; i++) {
		output += std::to_string(potentialScore[i]) + " ";
	}
	if (!dangereous) {
		// if safe for 7 seconds, move to enemy base
		if (safe_count == 24 * 3) {
			enemyBasePosition = InformationManager::Instance().GetEnemyBasePosition();
			enemyBaseSize = BWAPI::Position(BWAPI::Broodwar->enemy()->getRace().getResourceDepot().tileSize());
			enemyBasePosition += enemyBaseSize / 2;
			BWAPI::TilePosition nextPosition = BWAPI::TilePosition(enemyBasePosition + enemyBaseSquarePoints[wanderingID]);
			nextPosition.x = std::max(nextPosition.x, 0);
			nextPosition.x = std::min(nextPosition.x, BWAPI::Broodwar->mapWidth() - 1);
			nextPosition.y = std::max(nextPosition.y, 0);
			nextPosition.y = std::min(nextPosition.y, BWAPI::Broodwar->mapHeight() - 1);
			//if (BWAPI::Broodwar->isVisible(nextPosition) || !inEnemyBaseArea(BWAPI::Position(nextPosition))) {
			if (curPosition.getDistance(BWAPI::Position(nextPosition)) < 32 * 7 || !inEnemyBaseArea(BWAPI::Position(nextPosition))) {
				wanderingID++;
				if (wanderingID == 5) {
					wanderingID = 1;
				}
			}
			return nextPosition;
		}
		else {
			safe_count++; 
			potentialScore[0] += 99999999;
		}
	}
	else {
		safe_count = 0;
		wanderingID++;
		if (wanderingID == 5) {
			wanderingID = 1;
		}
	}
	BWAPI::Broodwar->printf(output.c_str());
	potentialScore[0] = -9999999;
	std::sort(directionID.begin(), directionID.end(), [&potentialScore](int x, int y) {return potentialScore[x] > potentialScore[y]; });
	switch (directionID[0])
	{
	case 0:
		BWAPI::Broodwar->printf("DIRECTION: stay");
		break;
	case 1:
		BWAPI::Broodwar->printf("DIRECTION: right");
		break;
	case 2:
		BWAPI::Broodwar->printf("DIRECTION: down right");
		break;
	case 3:
		BWAPI::Broodwar->printf("DIRECTION: down");
		break;
	case 4:
		BWAPI::Broodwar->printf("DIRECTION: down left");
		break;
	case 5:
		BWAPI::Broodwar->printf("DIRECTION: left");
		break;
	case 6:
		BWAPI::Broodwar->printf("DIRECTION: upper left");
		break;
	case 7:
		BWAPI::Broodwar->printf("DIRECTION: upper");
		break;
	case 8:
		BWAPI::Broodwar->printf("DIRECTION: upper right");
		break;
	default:
		break;
	}

	int DIRID = 0;
	BWAPI::Position returnPosition = curPosition + droneMoveDirections[directionID[DIRID]] * 2 * 32;
	while (!reachableInEnemyBase(BWAPI::TilePosition(curPosition), BWAPI::TilePosition(returnPosition)) || blockDirection[directionID[DIRID]] > 0) {
		if (blockDirection[directionID[DIRID]] == 0)
			blockDirection[directionID[DIRID]] = 24 * 1;
		DIRID++;
		if (DIRID >= 8)
			break;
		returnPosition = curPosition + droneMoveDirections[directionID[DIRID]] * 2 * 32;
	}

	if (DIRID != 0) {
		//BWAPI::Broodwar->printf("Turing to another direction");
		switch (directionID[DIRID])
		{
		case 0:
			BWAPI::Broodwar->printf("DIRECTION: stay");
			break;
		case 1:
			BWAPI::Broodwar->printf("DIRECTION: right");
			break;
		case 2:
			BWAPI::Broodwar->printf("DIRECTION: down right");
			break;
		case 3:
			BWAPI::Broodwar->printf("DIRECTION: down");
			break;
		case 4:
			BWAPI::Broodwar->printf("DIRECTION: down left");
			break;
		case 5:
			BWAPI::Broodwar->printf("DIRECTION: left");
			break;
		case 6:
			BWAPI::Broodwar->printf("DIRECTION: upper left");
			break;
		case 7:
			BWAPI::Broodwar->printf("DIRECTION: upper");
			break;
		case 8:
			BWAPI::Broodwar->printf("DIRECTION: upper right");
			break;
		default:
			break;
		}
	}
	return BWAPI::TilePosition(returnPosition);
}

BWAPI::Position ScoutTactic::astarSearch(Scout& drone) {
	BWAPI::TilePosition curPosition = BWAPI::TilePosition(drone.scoutUnit->getPosition());
	//curPosition.x++;
	//curPosition.y++;
	BWAPI::TilePosition enemyBasePosition = BWAPI::TilePosition(InformationManager::Instance().GetEnemyBasePosition());
	if (BWEMMap.GetArea(curPosition) != BWEMMap.GetArea(enemyBasePosition)) {
		return InformationManager::Instance().GetEnemyBasePosition();
	}
	else {
		std::pair<int, int> dir = enemyBaseSquareAstar[wanderingID];
		int expand = 12;
		BWAPI::TilePosition comp(dir.first*expand, dir.second*expand);
		BWAPI::TilePosition baseSize = BWAPI::Broodwar->enemy()->getRace().getResourceDepot().tileSize();
		while (expand >= 0 && !reachableInEnemyBase(curPosition, enemyBasePosition + comp + baseSize / 2, 0)) {
			expand--;
			comp.x = dir.first*expand;
			comp.y = dir.second*expand;
		}
		BWAPI::TilePosition scoutTarget = enemyBasePosition + comp + baseSize / 2;
		if (expand < 0 || curPosition.getDistance(scoutTarget) < 5) {
			wanderingID++;
			if (wanderingID >= 4) {
				wanderingID = 0;
			}
		}
		//BWAPI::Broodwar->drawBoxMap(Position(curPosition), Position(curPosition+drone.scoutUnit->getType().tileSize()), BWAPI::Colors::Blue);
		//BWAPI::Broodwar->drawBoxMap(scoutTarget.x, scoutTarget.y, scoutTarget.x + 1, scoutTarget.y + 1, BWAPI::Colors::Blue, true);
		//BWAPI::TilePosition comp1(-7,-7);
		//BWAPI::TilePosition comp2(7, 7);
		//std::list<BWAPI::TilePosition> path = Astar::Instance().aStarPathFinding(enemyBasePosition + comp2, enemyBasePosition + comp1, true, true, 1);
		int enemyBaseAreaId = BWEMMap.GetArea(BWAPI::TilePosition(InformationManager::Instance().GetEnemyBasePosition()))->Id();
		std::list<BWAPI::TilePosition> path = Astar::Instance().aStarPathFinding(curPosition, scoutTarget, true, true, true, true, 1, 3, enemyBaseAreaId);
		BWAPI::Position returnPosition = BWAPI::Position(path.front());
		BWAPI::Position compensation(16,16);


		//returnPosition = curPosition + (returnPosition - curPosition) / curPosition.getDistance(returnPosition) * 3;
		//while (path.size()>0 && !reachableInEnemyBase(curPosition, returnPosition, 0)) {
		//	returnPosition = path.front();
		//	path.pop_front();
		//	returnPosition = curPosition + (returnPosition - curPosition) / curPosition.getDistance(returnPosition) * 3;
		//}
		return returnPosition + compensation;
	}
}

BWAPI::Position	ScoutTactic::moveAroundEnemyBase(Scout& drone) {
	for (int i = 0; i < 9; i++) {
		blockDirection[i]--;
		if (blockDirection[i] < 0) {
			blockDirection[i] = 0;
		}
	}
	//BWAPI::Broodwar->printf("moving around enemy base");
	//BWAPI::TilePosition dodgePosition = droneDodge(drone);
	BWAPI::Position dodgePosition = astarSearch(drone);
	return dodgePosition;
}


void ScoutTactic::onUnitDestroy(BWAPI::Unit unit)
{
	for (std::vector<Scout>::iterator it = overLordIdle.begin(); it != overLordIdle.end(); it++)
	{
		if (it->scoutUnit == unit)
		{
			//int livingTime = Broodwar->getFrameCount() / 24;
			//Broodwar->printf("First Scout lives for %d seconds.",livingTime);
			//stringstream ss;
			//ss << livingTime;
			//if (Broodwar->enemy()->getRace() == BWAPI::Races::Terran && livingTime < 150) {
			//	logInfo("Scout", "First scout unit lives for " + ss.str() + " second(s), died within 2.5 minutes. Enemy race is " + InformationManager::Instance().getEnemyRace(), "BIG_ERROR_Scout");
			//}
			//else if (Broodwar->enemy()->getRace() != BWAPI::Races::Terran && livingTime < 240) {
			//	logInfo("Scout", "First scout unit lives for " + ss.str() + " second(s), died within 4 minutes. Enemy race is " + InformationManager::Instance().getEnemyRace(), "BIG_ERROR_Scout");
			//}
			overLordIdle.erase(it);
			break;
		}
	}

	for (std::vector<Scout>::iterator it = overLordScouts.begin(); it != overLordScouts.end(); it++)
	{
		if (it->scoutUnit == unit)
		{
			overLordScouts.erase(it);
			break;
		}
	}
}

bool ScoutTactic::HasZergling()
{
	for (auto u : overLordIdle)
	{
		if (u.scoutUnit->getType() == BWAPI::UnitTypes::Zerg_Zergling
			|| u.scoutUnit->getType() == BWAPI::UnitTypes::Zerg_Drone)
		{
			return true;
		}
	}

	for (auto u : overLordScouts)
	{
		if (u.scoutUnit->getType() == BWAPI::UnitTypes::Zerg_Zergling
			|| u.scoutUnit->getType() == BWAPI::UnitTypes::Zerg_Drone)
		{
			return true;
		}
	}

	return false;
}



void ScoutTactic::assignScoutWork()
{
	if (state == scoutForEnemyInfo && overLordIdle.size() > 0)
	{
		for (std::vector<Scout>::iterator it = overLordIdle.begin(); it != overLordIdle.end();)
		{
			//arrange zergling to scout for other base
			if (it->scoutUnit->getType() == BWAPI::UnitTypes::Zerg_Zergling)
			{
				it->TileTarget = getNextScoutLocation();
				if (it->TileTarget != BWAPI::TilePositions::None)
				{
					overLordScouts.push_back(*it);
					it = overLordIdle.erase(it);
				}
				else
				{
					it++;
				}
			}
			//arrange scout drone to move around enemy base 
			else if (it->scoutUnit->getType() == BWAPI::UnitTypes::Zerg_Drone) 
			{
				// If time is within 10 minutes, moving around enemy base, else looking for enemy sub-mineral
				if (Broodwar->getFrameCount() <= 24 * 10 * 60 || it->refinedTarget != BWAPI::Positions::None) {
					it->refinedTarget = moveAroundEnemyBase(*it);
					if (it->refinedTarget != BWAPI::Positions::None)
					{
						overLordScouts.push_back(*it);
						it = overLordIdle.erase(it);
					}
					else
					{
						it++;
					}
				}
				else {
					it->TileTarget = getNextScoutLocation();
					if (it->TileTarget != BWAPI::TilePositions::None)
					{
						overLordScouts.push_back(*it);
						it = overLordIdle.erase(it);
					}
					else
					{
						it++;
					}
				}
			}
			else if (it->scoutUnit->getType() == BWAPI::UnitTypes::Zerg_Overlord)
			{
				it->TileTarget = TilePosition(InformationManager::Instance().GetEnemyBasePosition());
				if (it->TileTarget != BWAPI::TilePositions::None)
				{
					overLordScouts.push_back(*it);
					it = overLordIdle.erase(it);
				}
				else
				{
					it++;
				}
			}
			else {
				it++;
			}
		}
	}

	if (scoutLocation.size() == 0 || overLordIdle.size() == 0)
		return;

	
	while (scoutLocation.size() > 0)
	{
		if (overLordIdle.size() == 0)
			break;

		std::sort(scoutLocation.begin(), scoutLocation.end());
		BWAPI::TilePosition maxPriorityLocation = scoutLocation.back().location;
		BWAPI::TilePosition minTile = scoutLocation.back().location;
		scoutLocation.pop_back();

		int minDistance = 999999;
		std::vector<Scout>::iterator minOverlord;
		for (std::vector<Scout>::iterator it = overLordIdle.begin(); it != overLordIdle.end(); it++)
		{
			if (it->scoutUnit->getDistance(BWAPI::Position(maxPriorityLocation)) < minDistance || minDistance == 999999)
			{
				minDistance = it->scoutUnit->getDistance(BWAPI::Position(maxPriorityLocation));
				minOverlord = it;
			}
		}
		minOverlord->TileTarget = maxPriorityLocation;

		//for natural detect at scoutForEnemyBase stage
		double closest = 999999999;
		for (auto const& r : BWEMMap.Areas())
		{
			if (&r == BWEMMap.GetArea(minTile))
			{
				continue;
			}
			for (auto const& b : r.Bases())
			{
				int pathLong = 0;
				BWEMMap.GetPath(Position(minTile), Position(b.Location()), &pathLong);
				if (pathLong != -1 && pathLong < closest)
				{
					closest = pathLong;
					minOverlord->naturalTileTarget = b.Location();
				}
			}
		}

		overLordScouts.push_back(*minOverlord);
		overLordIdle.erase(minOverlord);
	}
}


void ScoutTactic::update()
{
	/*
	if (state == scoutForEnemyBase) {
		BWAPI::Broodwar->printf("searching for enemy base");
		BWAPI::Broodwar->printf("scout: %d, free: %d", overLordScouts.size(), overLordIdle.size());
	}
	else {
		BWAPI::Broodwar->printf("searching for enemy information");
		BWAPI::Broodwar->printf("scout: %d, free: %d", overLordScouts.size(), overLordIdle.size());
	}
	*/
	
	

	std::map<const Area*, std::map<BWAPI::Unit, buildingInfo>>& enemyBuildings = InformationManager::Instance().getEnemyOccupiedDetail();

	switch (state)
	{
	case scoutForEnemyBase:
	{
		if (InformationManager::Instance().GetEnemyBasePosition() == BWAPI::Positions::None)
		{
			assignScoutWork();

			// update for scout info
			for (std::vector<Scout>::iterator it = overLordScouts.begin(); it != overLordScouts.end();)
			{
				//if (Broodwar->getFrameCount() % 10 == 0)
				//{
				//	Position nextP = Astar::Instance().getNextMovePosition(it->scoutUnit, Position(it->TileTarget));
				//	it->scoutUnit->move(nextP);
				//}
				Position nextP = Astar::Instance().getNextMovePosition(it->scoutUnit, Position(it->TileTarget));
				map<UnitType, set<Unit>> nearyByEnemyUnits =
					InformationManager::Instance().getUnitGridMap().GetUnits(it->scoutUnit->getTilePosition(), 7, Broodwar->enemy(), false);
				if (nearyByEnemyUnits.size() == 0) {
					it->scoutUnit->move(nextP);
				}
				else {
					TilePosition curPosition = TilePosition(it->scoutUnit->getPosition());
					int curAreaId = BWEMMap.GetTile(curPosition).AreaId();
					std::list<BWAPI::TilePosition> path = Astar::Instance().aStarPathFinding(curPosition, TilePosition(nextP), true, true, true, true, 2, 4, curAreaId);
					BWAPI::Position returnPosition = BWAPI::Position(path.front());
					BWAPI::Position compensation(16, 16);
					it->scoutUnit->move(returnPosition + compensation);
				}



				bool depot = false;
				for(auto const& u : BWAPI::Broodwar->enemy()->getUnits())
				{
					if (u->getType().isBuilding() && ((BWEMMap.GetArea(u->getTilePosition()) == BWEMMap.GetArea(it->TileTarget))
						|| (BWEMMap.GetArea(u->getTilePosition()) == BWEMMap.GetArea(it->naturalTileTarget))))
						depot = true;
				}
				if (depot)
				{
					InformationManager::Instance().setLocationEnemyBase(it->TileTarget);
					//find the enemy start base
					scoutLocation.clear();
					break;
				}
				else if (BWAPI::Broodwar->isVisible(it->TileTarget) && !depot)
				{
					overLordIdle.push_back(*it);
					it = overLordScouts.erase(it);

					//unknown base is one, set it to enemy
					if ((scoutLocation.size() + overLordScouts.size()) == 1)
					{
						BWAPI::TilePosition t = scoutLocation.size() == 1 ? scoutLocation[0].location : overLordScouts[0].TileTarget;
						InformationManager::Instance().setLocationEnemyBase(t);
						scoutLocation.clear();
						break;
					}
				}
				else
					it++;
			}
		}
		else
		{
			overLordIdle.insert(overLordIdle.end(), overLordScouts.begin(), overLordScouts.end());
			overLordScouts.clear();
			for(auto& u : overLordIdle)
			{
				u.scoutUnit->move(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()));
				//u.scoutUnit->move(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()));
			}

			/***********modify begin***********/
			/*
			for (std::vector<Scout>::iterator it = overLordIdle.begin(); it != overLordIdle.end();)
			{
				if (it->scoutUnit->getType() == BWAPI::UnitTypes::Zerg_Drone)
				{
					WorkerManager::Instance().finishedWithWorker(it->scoutUnit);
					overLordIdle.erase(it);
					break;
				}
				else
				{
					it++;
				}
			}
			*/
			/***********modify end***********/
			
			generateScoutLocation();
			//endFlag = true;
			state = scoutForEnemyInfo;
		}
	
		break;
	}
	case scoutForEnemyInfo:
	{
		assignScoutWork();
		//BWAPI::Broodwar->printf("scout: %d, free: %d", overLordScouts.size(), overLordIdle.size());

		for (std::vector<Scout>::iterator it = overLordScouts.begin(); it != overLordScouts.end();)
		{
			if (it->scoutUnit->getType() == BWAPI::UnitTypes::Zerg_Zergling
				|| it->scoutUnit->getType() == BWAPI::UnitTypes::Zerg_Drone)
			{
				if (it->refinedTarget != BWAPI::Positions::None) {
					if (BWEMMap.GetArea(TilePosition(it->scoutUnit->getPosition())) == BWEMMap.GetArea(TilePosition(it->refinedTarget))) {
						BattleArmy::smartMove(it->scoutUnit, it->refinedTarget);
					}
					else {
						Position nextP = Astar::Instance().getNextMovePosition(it->scoutUnit, it->refinedTarget);
						map<UnitType, set<Unit>> nearyByEnemyUnits =
							InformationManager::Instance().getUnitGridMap().GetUnits(it->scoutUnit->getTilePosition(), 7, Broodwar->enemy(), false);
						if (nearyByEnemyUnits.size() == 0) {
							it->scoutUnit->move(nextP);
						}
						else {
							TilePosition curPosition = TilePosition(it->scoutUnit->getPosition());
							int curAreaId = BWEMMap.GetTile(curPosition).AreaId();
							std::list<BWAPI::TilePosition> path = Astar::Instance().aStarPathFinding(curPosition, TilePosition(nextP), true, true, true, true, 2, 4, curAreaId);
							BWAPI::Position returnPosition = BWAPI::Position(path.front());
							BWAPI::Position compensation(16, 16);
							it->scoutUnit->move(returnPosition + compensation);
						}
					}
				}
				else {
					//BattleArmy::smartMove(it->scoutUnit, BWAPI::Position(it->TileTarget));
					Position nextP = Astar::Instance().getNextMovePosition(it->scoutUnit, Position(it->TileTarget));
					map<UnitType, set<Unit>> nearyByEnemyUnits =
						InformationManager::Instance().getUnitGridMap().GetUnits(it->scoutUnit->getTilePosition(), 7, Broodwar->enemy(), false);
					if (nearyByEnemyUnits.size() == 0) {
						it->scoutUnit->move(nextP);
					}
					else {
						TilePosition curPosition = TilePosition(it->scoutUnit->getPosition());
						int curAreaId = BWEMMap.GetTile(curPosition).AreaId();
						std::list<BWAPI::TilePosition> path = Astar::Instance().aStarPathFinding(curPosition, TilePosition(nextP), true, true, true, true, 2, 4, curAreaId);
						BWAPI::Position returnPosition = BWAPI::Position(path.front());
						BWAPI::Position compensation(16, 16);
						it->scoutUnit->move(returnPosition + compensation);
					}

				}
				//*******If this is a drone and is moving around enemy base, it may change target at each frame************
				if (it->scoutUnit->getType() == BWAPI::UnitTypes::Zerg_Drone && Broodwar->getFrameCount() <= 24 * 10 * 60) {
					overLordIdle.push_back(*it);
					it = overLordScouts.erase(it);
				}
				else if (it->scoutUnit->getDistance(BWAPI::Position(it->TileTarget)) < 32 * 5 || BWAPI::Broodwar->isVisible(it->TileTarget))
				{
					overLordIdle.push_back(*it);
					it = overLordScouts.erase(it);
				}
				else
				{
					it++;
				}
			}
			else
			{
				overlordMove(*it);
				it++;
			}
		}

		break;
	}
	}
}

void ScoutTactic::generateScoutLocation()
{
	const Area* nextBase = BWEMMap.GetArea(InformationManager::Instance().getOurNatrualLocation());
	if (nextBase == NULL)
	{
		nextBase = BWEMMap.GetNearestArea(InformationManager::Instance().getOurNatrualLocation());
	}
	double maxDist = 0;
	BWAPI::Position maxChokeCenter;
	for (auto const& c : nextBase->ChokePoints())
	{
		if (BWAPI::Broodwar->self()->getStartLocation().getDistance(BWAPI::TilePosition(c->Center())) > maxDist)
		{
			maxDist = BWAPI::Broodwar->self()->getStartLocation().getDistance(BWAPI::TilePosition(c->Center()));
			maxChokeCenter = Position(c->Center());
		}
	}

	if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Zerg)
	{
		//scoutLocation.push_back(BWAPI::TilePosition(InformationManager::Instance().GetEnemyNaturalPosition()));
		scoutLocation.push_back(scoutTarget(BWAPI::TilePosition(InformationManager::Instance().GetEnemyBasePosition()), 100));

		//if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Zerg)
		//scoutLocation.push_back(scoutTarget(BWAPI::TilePosition(reachRegion->getCenter()), 80));
	}

	//std::map<const Area*, TilePosition> & ourRegions = InformationManager::Instance().getBaseOccupiedRegions(BWAPI::Broodwar->self());
	//if (ourRegions.find(nextBase) != ourRegions.end())
	//{
	//	if (BWAPI::Broodwar->enemy()->getRace() != BWAPI::Races::Terran)
	//	{
	//		scoutLocation.push_back(scoutTarget(BWAPI::TilePosition(maxChokeCenter), 50));
	//	}
	//	scoutLocation.push_back(scoutTarget(InformationManager::Instance().getOurNatrualLocation(), 30));
	//}
}


void ScoutTactic::overlordMove(Scout& overlord)
{
	//select the move target every 4 position step
	if (overlord.nextMovePosition == BWAPI::TilePositions::None)
	{
		int walkRate = 4;
		int lowestAirForce = 99999;
		int cloest = 99999;
		BWAPI::TilePosition nextMovePosition = BWAPI::TilePositions::None;
		for (int i = 0; i < int(moveDirections.size()); i++)
		{
			BWAPI::TilePosition nextTilePosition(overlord.scoutUnit->getTilePosition().x + int(moveDirections[i].x * walkRate), overlord.scoutUnit->getTilePosition().y + int(moveDirections[i].y * walkRate));
			if (nextTilePosition.x > BWAPI::Broodwar->mapWidth() - 1 || nextTilePosition.x < 0
				|| nextTilePosition.y > BWAPI::Broodwar->mapHeight() - 1 || nextTilePosition.y < 0)
			{
				continue;
			}
			int nextAirForce = int(InformationManager::Instance().getEnemyInfluenceMap(nextTilePosition.x, nextTilePosition.y).airForce);
			if (nextAirForce < lowestAirForce || (nextAirForce == lowestAirForce && nextTilePosition.getDistance(overlord.TileTarget) < cloest))
			{
				nextMovePosition = nextTilePosition;
				lowestAirForce = nextAirForce;
				cloest = int(nextTilePosition.getDistance(overlord.TileTarget));
			}
		}
		overlord.nextMovePosition = nextMovePosition;
	}
	else
	{
		BattleArmy::smartMove(overlord.scoutUnit, BWAPI::Position(overlord.nextMovePosition));
		if (overlord.scoutUnit->getDistance(BWAPI::Position(overlord.nextMovePosition)) < 32)
		{
			overlord.nextMovePosition = BWAPI::TilePositions::None;
		}
	}
}



bool ScoutTactic::isTacticEnd()
{
	//if (overLordIdle.size() == 0 && overLordScouts.size() == 0)
	//{
	//	return true;
	//}

	if (endFlag)
	{
		for (auto u : overLordIdle)
		{
			tacticArmy[BWAPI::UnitTypes::Zerg_Overlord]->addUnit(u.scoutUnit);
		}

		for (auto u : overLordScouts)
		{
			tacticArmy[BWAPI::UnitTypes::Zerg_Overlord]->addUnit(u.scoutUnit);
		}
		return true;
	}
	else
	{
		return false;
	}
}




