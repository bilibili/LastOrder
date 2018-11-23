#include "AstarPath.h"
#include "InformationManager.h"


Astar& Astar::Instance()
{
	static Astar instance;
	return instance;
}

Astar::Astar()
{
	directions.reserve(8);
	double2 direct1(1, 0);
	directions.push_back(direct1);
	double2 direct2(1, 1);
	directions.push_back(direct2);
	double2 direct3(0, 1);
	directions.push_back(direct3);
	double2 direct4(-1, 1);
	directions.push_back(direct4);

	double2 direct5(-1, 0);
	directions.push_back(direct5);
	double2 direct6(-1, -1);
	directions.push_back(direct6);
	double2 direct7(0, -1);
	directions.push_back(direct7);
	double2 direct8(1, -1);
	directions.push_back(direct8);

	int width = BWAPI::Broodwar->mapWidth();
	int heigth = BWAPI::Broodwar->mapHeight();

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < heigth; j++) {
			openListIndex[i][j] = -1.0;
			closeListIndex[i][j] = false;
			groundBlockTile[i][j] = true;
			backtraceList[i][j] = -1;
		}
	}

	minimumDistance = 5;

}

double Astar::diag_distance(double2 pos)
{
	double d = std::abs(pos.x) > std::abs(pos.y) ? std::abs(pos.y) : std::abs(pos.x);
	double s = std::abs(pos.x) + std::abs(pos.y);
	return d * 1.4142135623730950488016887242097 + (s - d * 2);
	
}


// If searchByArea is true, then searchRange is AreaID, else searchRange is rectangle range near starPosition
void Astar::makeGroundBlockTiles(BWAPI::TilePosition startPosition, bool searchByArea, int searchRange) {
	int width = BWAPI::Broodwar->mapWidth();
	int heigth = BWAPI::Broodwar->mapHeight();
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < heigth; j++) {
			BWAPI::TilePosition tilePos(i, j);
			int walkable = InformationManager::Instance().getEnemyInfluenceMap(i, j).walkableArea;
			groundBlockTile[i][j] = (walkable > 0);
			if (searchByArea == true) {
				if (walkable != 1 && BWEMMap.GetArea(tilePos) != BWEMMap.GetArea(searchRange)) {
					groundBlockTile[i][j] = false;
				}
			}
			else {
				if (abs(int(i) - startPosition.x) > searchRange || abs(int(j) - startPosition.y) > searchRange) {
					groundBlockTile[i][j] = false;
				}
			}
		}
	}
	/*
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < heigth; j++) {
			BWAPI::TilePosition tilePos(i, j);
			int walkable = InformationManager::Instance().getEnemyInfluenceMap(i, j).walkableArea;
			if (searchByArea == true) {
				if (walkable == 1 || BWEMMap.GetArea(tilePos) == BWEMMap.GetArea(searchRange)) {
					BWAPI::Broodwar->drawTextMap(i * 32, j * 32, "%d", int(groundBlockTile[i][j]));
				}
			}
			else {
				if (abs(i - startPosition.x) <= searchRange + 1 && abs(j - startPosition.y) <= searchRange + 1) {
					BWAPI::Broodwar->drawTextMap(i * 32, j * 32, "%d", int(groundBlockTile[i][j]));
				}
			}
		}
	}
	*/
}


std::list<BWAPI::TilePosition> Astar::aStarPathFinding(BWAPI::TilePosition startPosition,
	BWAPI::TilePosition endPosition, bool nearEndPosition, bool isGroundPathFinding, bool searchByArea, bool usingIM, int expandRate, int minimumDistance, int searchRange)
{
	int width = BWAPI::Broodwar->mapWidth();
	int heigth = BWAPI::Broodwar->mapHeight();
	//init
	std::priority_queue<fValueGridPoint> openList;
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < heigth; j++) {
			openListIndex[i][j] = -1.0;
			closeListIndex[i][j] = false;
			//groundBlockTile[i][j] = true;   //it has been initialized in retreat logic
			backtraceList[i][j] = -1;
		}
	}

	/**********modified start**********/
	if (isGroundPathFinding) {
		// if searchByArea is true, the map has already been calculated
		if (searchByArea == true) {
			makeGroundBlockTiles(startPosition, searchByArea, searchRange);
		}
	}
	/**********modified   end**********/

	double hValue = diag_distance(double2(endPosition.x - startPosition.x, endPosition.y - startPosition.y));
	openList.push(fValueGridPoint(startPosition.x, startPosition.y, hValue));
	openListIndex[startPosition.x][startPosition.y] = 1;
	backtraceList[startPosition.x][startPosition.y] = startPosition.x*heigth + startPosition.y;

	BWAPI::TilePosition backPosition = BWAPI::TilePositions::None;
	int canNotFind = 0;
	int loop1 = 0;
	int loop2 = 0;
	double previousPathCost = 0;

	TimerManager::Instance().startTimer(TimerManager::Astar1);
	// if we expand the end, we find the optimal path
	while (openList.size() > 0)
	{
		TimerManager::Instance().stopTimer(TimerManager::Astar1);
		double elapseTime = TimerManager::Instance().getElapseTime(TimerManager::Astar1);
		loop1++;

		fValueGridPoint current = openList.top();
		BWAPI::TilePosition currentPosition(current.x, current.y);
		closeListIndex[currentPosition.x][currentPosition.y] = true; //openListIndex[current.position.x][current.position.y];
		openList.pop();
		previousPathCost = openListIndex[currentPosition.x][currentPosition.y];

		//openListIndex[currentPosition.x][currentPosition.y] = -1;

		if (!nearEndPosition)
		{
			if (BWEMMap.GetArea(currentPosition) == BWEMMap.GetArea(endPosition))
			{
				backPosition = currentPosition;
				break;
			}
		}
		else
		{
			if (isGroundPathFinding) {
				if (searchByArea) {
					if (currentPosition.getDistance(endPosition) < 2)
					{
						backPosition = currentPosition;
						break;
					}
				}
				else {
					if (currentPosition.getDistance(endPosition) < 3)
					{
						backPosition = currentPosition;
						break;
					}
				}
			}
			else {
				if (currentPosition.getDistance(endPosition) < 10)
				{
					backPosition = currentPosition;
					break;
				}
			}
		}

		for (int i = 0; i < int(directions.size()); i++)
		{
			BWAPI::TilePosition nextTilePosition(currentPosition.x + int(directions[i].x * expandRate), currentPosition.y + int(directions[i].y * expandRate));
			if (nextTilePosition.x > width - 1 || nextTilePosition.x < 0 || nextTilePosition.y > heigth - 1 || nextTilePosition.y < 0)
			{
				continue;
			}
			if (closeListIndex[nextTilePosition.x][nextTilePosition.y] == true)
				continue;

			if (isGroundPathFinding)
			{
				bool notWalkable = false;
				for (int j = 1; j <= expandRate; j++)
				{
					/**********modified start**********/
					BWAPI::TilePosition nextTmpTilePosition(currentPosition.x + int(directions[i].x * j), currentPosition.y + int(directions[i].y * j));
					if (groundBlockTile[nextTmpTilePosition.x][nextTmpTilePosition.y] == false) {
						notWalkable = true;
						break;
					}
					/**********modified   end**********/
				}
				if (notWalkable)
				{
					continue;
				}
			}

			gridInfo accumulatedIM;
			for (int k = 1; k <= expandRate; k++)
			{
				TilePosition tempTilePosition(currentPosition.x + int(directions[i].x * k), currentPosition.y + int(directions[i].y * k));
				gridInfo& tempIM = InformationManager::Instance().getEnemyInfluenceMap(tempTilePosition.x, tempTilePosition.y);
				accumulatedIM.groundForce += tempIM.groundForce;
				accumulatedIM.airForce += tempIM.airForce;
				accumulatedIM.enemyUnitGroundForce += tempIM.enemyUnitGroundForce;
				accumulatedIM.enemyUnitAirForce += tempIM.enemyUnitAirForce;
			}

			//gridInfo& accumulatedIM = InformationManager::Instance().getEnemyInfluenceMap(nextTilePosition.x, nextTilePosition.y);
			//double2 newDirection(nextTilePosition.x - current.position.x, nextTilePosition.y - current.position.y);
			//double degreeChangeCost = newDirection.getVectorDegree(preDirection);
			double newCost = 0;
			if (usingIM) {
				double moveDistance = 0;
				if (abs(directions[i].x) + abs(directions[i].y) == 2)
					moveDistance = expandRate * 1.4142135623730950488016887242097;
				else
					moveDistance = expandRate;
				if (isGroundPathFinding)
				{
					newCost = previousPathCost + moveDistance + accumulatedIM.groundForce * moveDistance * 30 + accumulatedIM.enemyUnitGroundForce * moveDistance * 30;
				}
				else
				{
					newCost = previousPathCost + moveDistance + accumulatedIM.airForce * moveDistance * 30 + accumulatedIM.enemyUnitAirForce * moveDistance * 30;
				}
			}
			/**********modified   end**********/

			//if point have already been expand(if our heuristic function is admissible, this will not happen)
			//if (closeListIndex[nextTilePosition.x][nextTilePosition.y] != -1 && closeListIndex[nextTilePosition.x][nextTilePosition.y] > newCost)
			//closeListIndex[nextTilePosition.x][nextTilePosition.y] = -1;

			// if point have already in open point, check the cost value
			// since std::priority_queue do not have increase-priority interface, so we push the same position twice into the open list 
			if (openListIndex[nextTilePosition.x][nextTilePosition.y] != -1 && openListIndex[nextTilePosition.x][nextTilePosition.y] > newCost)
				openListIndex[nextTilePosition.x][nextTilePosition.y] = -1;

			if (openListIndex[nextTilePosition.x][nextTilePosition.y] == -1 && closeListIndex[nextTilePosition.x][nextTilePosition.y] == false)
			{
				loop2++;
				double fvalue;
				fvalue = newCost + diag_distance(double2(endPosition.x - nextTilePosition.x, endPosition.y - nextTilePosition.y));

				openList.push(fValueGridPoint(nextTilePosition.x, nextTilePosition.y, fvalue));
				openListIndex[nextTilePosition.x][nextTilePosition.y] = newCost;
				backtraceList[nextTilePosition.x][nextTilePosition.y] = currentPosition.x*heigth + currentPosition.y;
			}
		}

		//can not find the solution path
		if (openList.size() == 0)
		{
			//BWAPI::Broodwar->printf("WARNING: can not find path!!!");
			canNotFind = 1;
			std::list<BWAPI::TilePosition> pathFind;
			pathFind.push_back(endPosition);
			return pathFind;
		}
	}

	int count = 0;
	std::list<BWAPI::TilePosition> pathFind;
	pathFind.push_back(endPosition);
	/**********modified start**********/
	if (backPosition.getDistance(startPosition) > minimumDistance)
	{
		//BWAPI::Broodwar->drawLineMap(BWAPI::Position(backPosition).x, BWAPI::Position(backPosition).y, BWAPI::Position(endPosition).x, BWAPI::Position(endPosition).y, BWAPI::Colors::Purple);
		pathFind.push_front(backPosition);
		while (backPosition != startPosition && backPosition.getDistance(startPosition) > minimumDistance)
		{
			count += 1;
			if (count >= 1000)
			{
				std::list<BWAPI::TilePosition> pathFind;
				pathFind.push_back(endPosition);
				logInfo("Astar", "backPosition dead lock ", "BIG_ERROR_Astar");
				return pathFind;
			}
			int backID = backtraceList[backPosition.x][backPosition.y];
			int backX = backID / heigth, backY = backID % heigth;
			//BWAPI::Broodwar->drawLineMap(BWAPI::Position(backPosition).x, BWAPI::Position(backPosition).y, Position(TilePosition(backX, backY)).x, Position(TilePosition(backX, backY)).y, BWAPI::Colors::Purple);
			TilePosition back(backX, backY);
			pathFind.push_front(back);
			backPosition = back;
		}
	}
	//BWAPI::Broodwar->drawLineMap(BWAPI::Position(startPosition).x, BWAPI::Position(startPosition).y, BWAPI::Position(pathFind.front()).x, BWAPI::Position(pathFind.front()).y, BWAPI::Colors::Purple);
	/**********modified   end**********/
	return pathFind;
}



Position Astar::getNextMovePosition(Unit unit, Position des, bool plotPath)
{
	int pLongth = 0;
	const CPPath& path = BWEMMap.GetPath(unit->getPosition(), des, &pLongth);
	if (path.size() == 0)
	{
		return des;
	}

	//Position cur = unit->getPosition();
	//for (auto const& p : path) {
	//	Broodwar->drawCircleMap(Position(p->Pos(ChokePoint::middle)), 5, Colors::Red, true);
	//	double2 p1 = Position(p->Center());
	//	double2 p2= Position(p->Pos(ChokePoint::end1));
	//	double2 p3 = Position(p->Pos(ChokePoint::end2));
	//	p2 = p1 + (p2 - p1) * 96 / (p2 - p1).len();
	//	p3 = p1 + (p3 - p1) * 96 / (p3 - p1).len();
	//
	//	Broodwar->drawCircleMap(Position(p2), 5, Colors::Orange, true);
	//	Broodwar->drawCircleMap(Position(p3), 5, Colors::Orange, true);
	//
	//	double2 res1 = (p2 + p3) / 2.0;
	//	double eps = 1e-8;
	//	if ((res1 - p1).len() < eps) {
	//		res1 = p1 + double2(p3.y - p1.y, -(p3.x - p1.x));
	//	}
	//	double2 res2 = p1 + (p1 - res1);
	//	res1 = p1 + (res1 - p1) * 96 / (res1 - p1).len();
	//	res2 = p1 + (res2 - p1) * 96 / (res2 - p1).len();
	//	Broodwar->drawCircleMap(Position(res1), 5, Colors::Cyan, true);
	//	Broodwar->drawCircleMap(Position(res2), 5, Colors::Cyan, true);
	//	Broodwar->drawLineMap(cur, Position(p->Center()),Colors::Blue);
	//	cur = Position(p->Center());
	//}

	double2 p1 = Position(path[0]->Center());
	double2 p2 = Position(path[0]->Pos(ChokePoint::end1));
	double2 p3 = Position(path[0]->Pos(ChokePoint::end2));
	bool longChoke = ((p2 - p1).len() > 144 || (p3 - p1).len() > 144);
	bool verylongChoke = ((p2 - p1).len() > 240 || (p3 - p1).len() > 240);
	p2 = p1 + (p2 - p1) * 96 / (p2 - p1).len();
	p3 = p1 + (p3 - p1) * 96 / (p3 - p1).len();
	double2 p4 = p1 + (p2 - p1) * 192 / (p2 - p1).len();
	double2 p5 = p1 + (p3 - p1) * 192 / (p3 - p1).len();
	double2 p6 = p1 + (p2 - p1) * 288 / (p2 - p1).len();
	double2 p7 = p1 + (p3 - p1) * 288 / (p3 - p1).len();
	//Broodwar->drawCircleMap(p1, 20, Colors::Orange, true);
	//Broodwar->drawCircleMap(p1, 3 * 32, Colors::Red);
	//Broodwar->drawCircleMap(p2, 3 * 32, Colors::Red);
	//Broodwar->drawCircleMap(p3, 3 * 32, Colors::Red);
	//if (longChoke) {
	//	Broodwar->drawCircleMap(p1, 20, Colors::Blue, true);
	//	Broodwar->drawCircleMap(p4, 3 * 32, Colors::Red);
	//	Broodwar->drawCircleMap(p5, 3 * 32, Colors::Red);
	//}
	//if (verylongChoke) {
	//	Broodwar->drawCircleMap(p1, 20, Colors::Red, true);
	//	Broodwar->drawCircleMap(p6, 3 * 32, Colors::Red);
	//	Broodwar->drawCircleMap(p7, 3 * 32, Colors::Red);
	//}
	if (unit->getPosition().getDistance(Position(p1)) < 3 * 32 ||
		unit->getPosition().getDistance(Position(p2)) < 3 * 32 || 
		unit->getPosition().getDistance(Position(p3)) < 3 * 32 ||
		(longChoke && (unit->getPosition().getDistance(Position(p4)) < 3 * 32 || unit->getPosition().getDistance(Position(p5)) < 3 * 32)) ||
		(verylongChoke && (unit->getPosition().getDistance(Position(p6)) < 3 * 32 || unit->getPosition().getDistance(Position(p7)) < 3 * 32))) {
		const Area* nextArea = nullptr;
		if (path.size() > 1) {
			std::pair<const Area *, const Area *> area1 = path[0]->GetAreas();
			std::pair<const Area *, const Area *> area2 = path[1]->GetAreas();
			if (area1.first == area2.first || area1.first == area2.second)
				nextArea = area1.first;
			if (area1.second == area2.first || area1.second == area2.second)
				nextArea = area1.second;
		}
		else {
			nextArea = BWEMMap.GetArea(TilePosition(des));
		}
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
		if (isPositionValid(pres1) && BWEMMap.GetArea(TilePosition(pres1)) == nextArea) {
			return pres1;
		}
		else if (isPositionValid(pres2) && BWEMMap.GetArea(TilePosition(pres2)) == nextArea) {
			return pres2;
		}
		else {
			if (path.size() > 1)
				return Position(path[1]->Center());
			else
				return des;
		}
	}
	else
	{
		Position curDes = Position(path[0]->Center());
		return curDes;
	}
}



Position Astar::getAirPath(Unit unit, Position des, bool plotPath)
{
	list<BWAPI::TilePosition> attackPath = Astar::Instance().aStarPathFinding(unit->getTilePosition(), TilePosition(des), true,
		false, false, true, 4, 10, -1);
	while (attackPath.size() > 1 && attackPath.front().getDistance(unit->getTilePosition()) < 3)
	{
		attackPath.pop_front();
	}
	if (plotPath)
	{
		for (auto p = attackPath.begin(); p != attackPath.end(); p++)
		{
			auto nx = std::next(p, 1);
			if (nx == attackPath.end())
			{
				continue;
			}
			Broodwar->drawLineMap(Position(*p), Position(*nx), Colors::Red);
		}
	}

	return Position(attackPath.front());
}


Position Astar::getGroundPath(Unit unit, Position des, bool plotPath)
{
	const Area * unitArea = BWEMMap.GetArea(unit->getTilePosition());
	Position unitPosition = unit->getPosition();
	//if (unitArea == NULL)
	//{
	//	unitArea = BWEMMap.GetNearestArea(unit->getTilePosition());
	//	unitPosition = Position(unitArea->Top());
	//}

	int pLongth = 0;
	const CPPath& path = BWEMMap.GetPath(unitPosition, des, &pLongth);
	Position tmpTarget;
	double pathLength = 0;
	Position thisPosition = unitPosition;
	for (size_t i = 0; i <= path.size(); i++)
	{
		if (i == path.size())
		{
			tmpTarget = des;
			break;
		}
		pathLength += Position(path[i]->Center()).getDistance(thisPosition);
		thisPosition = Position(path[i]->Center());
		if (pathLength > 10 * 32)
		{
			if (plotPath)
			{
				for (size_t j = i; j < path.size(); j++)
				{
					int t = j + 1;
					if (t == path.size())
					{
						break;
					}
					Broodwar->drawLineMap(Position(path[j]->Center()), Position(path[t]->Center()), Colors::Blue);
				}
			}

			tmpTarget = thisPosition;
			break;
		}
	}
	/*
	if (path.size() > 0) {
		int areaId = BWEMMap.GetMiniTile(WalkPosition(unitPosition)).AreaId();
		list<BWAPI::TilePosition> attackPath = Astar::Instance().aStarPathFinding(TilePosition(unitPosition), TilePosition(path[0]->Center()), true, true, true, false, 1, 3, areaId);
		Position previous_pos = unitPosition;
		for (auto t : attackPath) {
			BWAPI::Broodwar->drawLineMap(previous_pos.x, previous_pos.y, BWAPI::Position(t).x, BWAPI::Position(t).y, BWAPI::Colors::Purple);
			previous_pos = Position(t);
		}
	}
	*/
	return tmpTarget;
}


Position Astar::getGroundRetreatPath(Unit unit, int searchRange, bool plotPath) {
	const Area * unitArea = BWEMMap.GetArea(unit->getTilePosition());
	Position unitPosition = unit->getPosition();
	//if (unitArea == NULL)
	//{
	//	unitArea = BWEMMap.GetNearestArea(unit->getTilePosition());
	//	unitPosition = Position(unitArea->Top());
	//}
	Position des = InformationManager::Instance().getRetreatDestination();
	/*
	int pLongth = 0;
	const CPPath& path = BWEMMap.GetPath(unitPosition, des, &pLongth);
	Position tmpTarget;
	double pathLength = 0;
	Position thisPosition = unitPosition;
	for (size_t i = 0; i <= path.size(); i++)
	{
		if (i == path.size())
		{
			tmpTarget = des;
			break;
		}
		pathLength += Position(path[i]->Center()).getDistance(thisPosition);
		thisPosition = Position(path[i]->Center());
		if (pathLength > 10 * 32)
		{
			tmpTarget = thisPosition;
			break;
		}
	}
	*/
	Position tmpTarget = getNextMovePosition(unit,des);

	BWAPI::TilePosition curPosition = BWAPI::TilePosition(unit->getPosition());
	vector<std::pair<int, int>> moveDirections;
	for (int x = -1 * searchRange; x <= searchRange; x++)
	{
		for (int y = -1 * searchRange; y <= searchRange; y++)
		{
			if (x != -1 * searchRange && x != searchRange && y != -1 * searchRange && y != searchRange)
				continue;
			moveDirections.push_back(std::make_pair(x, y));
		}
	}
	TilePosition retreatDestination = TilePosition(tmpTarget);

	makeGroundBlockTiles(curPosition, false, searchRange);

	TilePosition currentDestination = TilePositions::None;
	double minimumDistance = 999999.0;
	for (auto& direction : moveDirections) {
		TilePosition tmp = curPosition + TilePosition(direction.first,direction.second);
		if (isTilePositionValid(tmp) == false)
			continue;
		gridInfo& curInfo = InformationManager::Instance().getEnemyInfluenceMap(tmp.x, tmp.y);
		if (!BWEMMap.GetMiniTile(WalkPosition(tmp)).Walkable())
			continue;
		if (groundBlockTile[tmp.x][tmp.y] == false) 
			continue;
		double score = tmp.getDistance(retreatDestination) + std::min((int(curInfo.enemyUnitGroundForce)) / 5, 10);
		if (score < minimumDistance) {
			minimumDistance = score;
			currentDestination = tmp;
		}
	}
	if (currentDestination == TilePositions::None) {
		return Position(retreatDestination);
	}
	//Broodwar->drawCircleMap(Position(curPosition), 16, Colors::Blue, false);
	//Broodwar->drawCircleMap(Position(currentDestination), 16, Colors::Green, false);

	std::list<BWAPI::TilePosition> astarpath = Astar::Instance().aStarPathFinding(curPosition, currentDestination, true, true, false, true, 1, 6, searchRange);
	BWAPI::Position returnPosition = BWAPI::Position(astarpath.front());
	//BWAPI::Position returnPosition = BWAPI::Position(currentDestination);
	BWAPI::Position compensation(16, 16);
	Position endPosition = returnPosition + compensation;
	Position P_curPosition = Position(curPosition);
	
	int maxDistance = 10;
	Position newEndPosition = P_curPosition + ((endPosition - P_curPosition) * maxDistance * 32 / int((endPosition - P_curPosition).getLength()));
	if (isPositionValid(newEndPosition) == false)
	{
		return Position(retreatDestination);
	}
	while (maxDistance > 5 && InformationManager::Instance().getEnemyInfluenceMap(TilePosition(newEndPosition).x, TilePosition(newEndPosition).y).walkableArea > 0) {
		maxDistance--;
		Position tmp = newEndPosition;
		newEndPosition = P_curPosition + ((endPosition - P_curPosition) * maxDistance * 32 / int((endPosition - P_curPosition).getLength()));
		if (isPositionValid(newEndPosition) == false)
		{
			return Position(retreatDestination);
		}
	}
	//newEndPosition = P_curPosition + ((endPosition - P_curPosition) * (maxDistance - 1) * 32 / (endPosition - P_curPosition).getLength());
	Broodwar->drawCircleMap(newEndPosition, 4, Colors::Green, true);
	return newEndPosition;
}


bool Astar::bfsTargetArea(TilePosition startPosition, TilePosition targetPosition) {
	memset(bfsCanWalk, 0, sizeof(bfsCanWalk));
	int dir[4][2] = { { -1,0 },{ 1,0 },{ 0,1 },{ 0,-1 } };
	int totCanWalkTiles = 0;
	//int range = 12;
	//int xStart = (startPosition.x - range) < 0 ? 0 : (startPosition.x - range);
	//int xEnd = (startPosition.x + range) >= Broodwar->mapWidth() ? (Broodwar->mapWidth() - 1) : (startPosition.x + range);
	//int yStart = (startPosition.y - range) < 0 ? 0 : (startPosition.y - range);
	//int yEnd = (startPosition.y + range) >= Broodwar->mapHeight() ? (Broodwar->mapHeight() - 1) : (startPosition.y + range);
	//for (int i = xStart; i <= xEnd; i++) {
	//	for (int j = yStart; j <= yEnd; j++) {
	//		TilePosition curTile(i, j);
	//		if (curTile.getDistance(startPosition) > 12) {
	//			bfsCanWalk[i][j] = 0;
	//		}
	//		else if (InformationManager::Instance().getEnemyInfluenceMap(i, j).walkableArea == 0) {
	//			bfsCanWalk[i][j] = 0;
	//		}
	//		else if (BWEMMap.GetArea(curTile) != BWEMMap.GetArea(targetPosition)) {
	//			bfsCanWalk[i][j] = 0;
	//		}
	//		else {
	//			bfsCanWalk[i][j] = 1;
	//			totCanWalkTiles++;
	//		}
	//	}
	//}
	for (int i = 0; i < Broodwar->mapWidth(); i++) {
		for (int j = 0; j < Broodwar->mapHeight(); j++) {
			TilePosition curTile(i, j);
			if (BWEMMap.GetTile(curTile).AreaId() == -1) {
				bfsCanWalk[i][j] = 1;
			}
			else if (InformationManager::Instance().getEnemyInfluenceMap(i, j).walkableArea == 0) {
				bfsCanWalk[i][j] = 0;
			}
			else if (BWEMMap.GetArea(curTile) != BWEMMap.GetArea(targetPosition)) {
				bfsCanWalk[i][j] = 0;
			}
			else {
				bfsCanWalk[i][j] = 1;
				totCanWalkTiles++;
			}
		}
	}
	//for (int i = 0; i < Broodwar->mapWidth(); i++) {
	//	for (int j = 0; j < Broodwar->mapHeight(); j++) {
	//		if (bfsCanWalk[i][j]) {
	//			Broodwar->drawTextMap(i * 32, j * 32, "1");
	//		}
	//	}
	//}
	int exactCanWalkTiles = 0;
	queue<TilePosition> Q;
	Q.push(startPosition);
	while (!Q.empty()) {
		TilePosition cur = Q.front();
		//Broodwar->drawCircleMap(Position(cur)+Position(16,16),3,Colors::Red,false);
		if(BWEMMap.GetTile(cur).AreaId() != -1)
			exactCanWalkTiles++;
		Q.pop();
		for (int t = 0; t < 4; t++) {
			TilePosition tmp(cur.x + dir[t][0], cur.y + dir[t][1]);
			if (!isTilePositionValid(tmp) || bfsCanWalk[tmp.x][tmp.y]==0)
				continue;
			Q.push(tmp);
			bfsCanWalk[tmp.x][tmp.y] = 0;
		}
	}
	//for (int i = 0; i < Broodwar->mapWidth(); i++) {
	//	for (int j = 0; j < Broodwar->mapHeight(); j++) {
	//		if (bfsCanWalk[i][j]) {
	//			Broodwar->drawTextMap(i * 32, j * 32, "0");
	//		}
	//	}
	//}
	//exactCanWalkTiles--; // remove startPosition
	//Broodwar->printf("exact:%d tot:%d",exactCanWalkTiles,totCanWalkTiles);
	// less 70% of the tiles are available
	bool result;
	if (exactCanWalkTiles < int(0.7*totCanWalkTiles))
		result=true;
	else
		result=false;
	if (result) {
		for (int i = 0; i < Broodwar->mapWidth(); i++) {
			for (int j = 0; j < Broodwar->mapHeight(); j++) {
				TilePosition curTile(i, j);
				const Area* curArea = BWEMMap.GetArea(curTile);
				if ((bfsCanWalk[i][j] && InformationManager::Instance().getEnemyInfluenceMap(i, j).walkableArea > 1) ||
					(InformationManager::Instance().getEnemyInfluenceMap(i, j).buildingOnTile != nullptr && curArea == BWEMMap.GetArea(targetPosition))) {
					InformationManager::Instance().setUnavailableTile(curTile);
				}
				else {
					if (targetPosition == TilePosition(InformationManager::Instance().GetEnemyNaturalPosition())) {
						if (curArea != BWEMMap.GetArea(TilePosition(InformationManager::Instance().GetEnemyNaturalPosition())) &&
							InformationManager::Instance().getEnemyInfluenceMap(i, j).isInEnemyHome) {
							InformationManager::Instance().setUnavailableTile(curTile);
						}
					}
				}
			}
		}
	}
	return result;
}

