#include "BuildingManager.h"


BuildingManager::BuildingManager()
	: debugMode(false)
	, reservedMinerals(0)
	, reservedGas(0)
	, buildingSpace(1)
	, totalBuildTasks(0)
{
	latestIssueBuildingFrame = 0;
	InformationManager::Instance().resetBuildingReserve();
}


void BuildingManager::update()
{
	//for (int i = 0; i < Broodwar->mapWidth(); i++) {
	//	for (int j = 0; j < Broodwar->mapHeight(); j++) {
	//		Broodwar->drawTextMap(i * 32, j * 32, "%d", int(InformationManager::Instance().getBuildingReserve(i, j)));
	//	}
	//}
	int i = 0;
	for (vector<Building>::iterator it = buildingData.begin(); it != buildingData.end();)
	{
		//if (Broodwar->getFrameCount() % 24 * 60 * 5 == 0 && 
		//	Broodwar->getFrameCount() - it->startBuildingFrame > 24 * 60 * 5)
		//{
		//	int producerType = -1;
		//	if (it->builderUnit != NULL)
		//	{
		//		producerType = it->builderUnit->getType();
		//	}
		//	logInfo("BuildingManager", "120 seconds building not build: building type " +  to_string(it->type) + 
		//		" builder type " + to_string(producerType) +
		//		" item mineral " + to_string(it->type.mineralPrice()) +
		//		" item gas " + to_string(it->type.gasPrice()) +
		//		" current mineral " + to_string(Broodwar->self()->minerals()) +
		//		" current gas " + to_string(Broodwar->self()->gas()) +
		//		" building reserved mineral " + to_string(reservedMinerals) +
		//		" building reserved gas " + to_string(reservedGas), "BIG_ERROR_BuildingManager");
		//}

		if (it->nextTryBuildTime > Broodwar->getFrameCount())
		{
			it++;
			continue;
		}

		if (it->assignPositionTime != -1 && Broodwar->getFrameCount() - it->assignPositionTime > 5 * 60 * 24)
		{
			it->buildingState = Building::end;
			it->buildFail = true;
			logInfo("BuildingManager", "building " + to_string(it->type) + " timeout!", "BIG_ERROR_BuildingManager");
		}

		switch (it->buildingState)
		{
		case Building::initBuilderAndLocation:
		{
			assignWorkersToUnassignedBuildings(*it);
		}
			break;
		case Building::exploreMove:
		{
			exploreUnseenPosition(*it);
		}
			break;
		case Building::issueBuildOrder:
		{
			constructAssignedBuildings(*it);
		}
			break;
		case Building::waitLatency:
		{
			if (Broodwar->getFrameCount() > it->checkBuilderStatusTime)
			{
				if (it->type.isRefinery())
				{
					it->buildingState = Building::refinerySpecial;
				}
				else
				{
					it->buildingState = Building::buildingOrderCheck;
				}
				logInfo("BuildingManager", "check builder status " + std::to_string(int(it->type)));
			}
		}
		break;

		case Building::refinerySpecial:
		{
			//for refinery bug...
			if (!it->builderUnit->exists())
			{
				it->buildingState = Building::end;
				return;
			}

			if (!it->builderUnit->isConstructing())
			{
				it->buildingState = Building::initBuilderAndLocation;
			}
			// issue the build order!
			//it->builderUnit->build(it->finalPosition, it->type);
		}
		break;
		case Building::buildingOrderCheck:
		{
			buildingOrderCheck(*it);
		}
			break;
		default:
			break;
		}

		if (it->buildingState == Building::end)
		{
			cancelBuildingReserve(*it);
			if (it->buildFail == false)
			{
				logInfo("BuildingManager", "build success " + std::to_string(int(it->type)) +
				" reserved mineral " + to_string(reservedMinerals) + 
				" reserved gas " + to_string(reservedGas));
				ProductionManager::Instance().buildingCallback(it->builderUnit, it->waitingBuildType);
			}
			else
			{
				//build fail
				if (!it->waitingBuildType.empty() && it->type != UnitTypes::Zerg_Creep_Colony)
					StrategyManager::Instance().buildingFinish(it->waitingBuildType.front().unitType);
				logInfo("BuildingManager", "builder dead, building fail, discard " + std::to_string(int(it->type)));
			}
			reservedMinerals -= it->type.mineralPrice();
			reservedGas -= it->type.gasPrice();
			it = buildingData.erase(it);
		}
		else
			it++;
	}
}


void BuildingManager::buildingFail(Building & b)
{
	b.nextTryBuildTime = Broodwar->getFrameCount() + 2 * 24;
	b.tryBuildTimes += 1;
	cancelBuildingReserve(b);

	if (b.tryBuildTimes >= 5)
	{
		logInfo("BuildingManager", "buildingOrderCheck fail " + std::to_string(int(b.type)), "BIG_ERROR_BuildingManager");
		b.buildingState = Building::end;
		WorkerManager::Instance().finishedWithWorker(b.builderUnit);
		b.buildFail = true;
	}
	else
	{
		b.buildingState = Building::initBuilderAndLocation;
		logInfo("BuildingManager", "building issue fail " + std::to_string(int(b.type)));
	}
}


void BuildingManager::assignWorkersToUnassignedBuildings(Building& b)
{
	b.finalPosition = b.desiredPosition;
	if (b.builderUnit == NULL || !b.builderUnit->exists())
	{
		BWAPI::Unit workerToAssign = WorkerManager::Instance().getBuilder(b);
		b.builderUnit = workerToAssign;
	}

	if (b.builderUnit != NULL)
	{
		BWAPI::TilePosition testLocation = getBuildingLocation(b);

		if (b.type == UnitTypes::Zerg_Hatchery && BWEMMap.GetArea(b.desiredPosition) != BWEMMap.GetArea(Broodwar->self()->getStartLocation()) )
		{
			if (testLocation != b.desiredPosition)
			{
				logInfo("BuildingManager", "expand position shift!! ");
			}
			testLocation = b.desiredPosition;
		}

		// if we can't find a valid build location, do not build at this frame
		if (!testLocation.isValid())
		{
			b.finalPosition = TilePositions::None;
			buildingFail(b);
			return;
		}
		b.assignPositionTime = Broodwar->getFrameCount();
		// set the final position of the building as this location
		b.finalPosition = testLocation;
		//logInfo("BuildingManager", "building " + std::to_string(int(b.type)) + " find location " + to_string(testLocation.x) + " " + to_string(testLocation.y));

		reserveBuilding(b);

		if (!isBuildingPositionExplored(b))
		{
			b.buildingState = Building::exploreMove;
		}
		else
		{
			if (b.type.isRefinery())
			{
				if (BWEMMap.GetArea(b.builderUnit->getTilePosition()) == BWEMMap.GetArea(b.finalPosition))
				{
					b.buildingState = Building::waitLatency;
					b.builderUnit->build(b.type, b.finalPosition);
					b.checkBuilderStatusTime = Broodwar->getFrameCount() + 24 * 2;
				}
				else
				{
					Position nextP = Astar::Instance().getNextMovePosition(b.builderUnit, Position(b.finalPosition));
					b.builderUnit->move(nextP);
				}
			}
			else
			{
				b.buildingState = Building::issueBuildOrder;
			}
		}
	}
}


bool BuildingManager::checkBuilder(Building& b)
{
	if (!b.builderUnit->exists())
	{
		logInfo("BuildingManager", "builder not exist, build fail " + std::to_string(int(b.type)));
		b.buildingState = Building::end;
		b.buildFail = true;
		return true;
	}
	return false;
}


void BuildingManager::exploreUnseenPosition(Building& b)
{
	if (checkBuilder(b))
	{
		return;
	}

	if (!isBuildingPositionExplored(b))
	{
		Position nextP = Astar::Instance().getNextMovePosition(b.builderUnit, Position(b.finalPosition));
		b.builderUnit->move(nextP);
	}
	else
	{
		b.buildingState = Building::issueBuildOrder;
	}
}


void BuildingManager::constructAssignedBuildings(Building& b)
{
	if (checkBuilder(b))
	{
		return;
	}

	if (BWEMMap.GetArea(b.builderUnit->getTilePosition()) == BWEMMap.GetArea(b.finalPosition))
	{
		//if (Broodwar->getFrameCount() - latestIssueBuildingFrame < 3 * 24)
		//{
		//	return;
		//}

		// issue the build order!
		bool canBuild = b.builderUnit->build(b.type, b.finalPosition);
		if (canBuild)
		{
			//latestIssueBuildingFrame = Broodwar->getFrameCount();
			b.buildingState = Building::waitLatency;
			logInfo("BuildingManager", "building issue build order " + std::to_string(int(b.type)));
			b.checkBuilderStatusTime = Broodwar->getFrameCount() + 24 * 2;
		}
		else
		{
			buildingFail(b);
		}
	}
	else
	{
		Position nextP = Astar::Instance().getNextMovePosition(b.builderUnit, Position(b.finalPosition));
		b.builderUnit->move(nextP);
	}
}


void BuildingManager::buildingOrderCheck(Building& b)
{
	if (checkBuilder(b))
	{
		return;
	}

	if (b.builderUnit->isMorphing())
	{
		logInfo("BuildingManager", "building morph begin " + std::to_string(int(b.type)));
		b.buildingState = Building::end;
	}
	//if build command fail, something is wrong
	else if (!b.builderUnit->isConstructing())
	{
		buildingFail(b);
	}
}


BWAPI::TilePosition BuildingManager::getBuildingLocation( Building & b)
{
	BWAPI::TilePosition testLocation = BWAPI::TilePositions::None;

	int numPylons = BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Pylon);

	if (b.type.isRefinery())
	{
		return getRefineryPosition();
	}

	if (b.type == UnitTypes::Zerg_Creep_Colony)
	{
		testLocation = getBuildLocationNear(b, 0, false);
	}
	else
		testLocation = getBuildLocationNear(b, 1, false);

	// send back the location
	return testLocation;
}


BWAPI::TilePosition BuildingManager::getRefineryPosition()
{
	int closest = 999999;
	BWAPI::TilePosition position = BWAPI::TilePositions::None;
	std::map<const Area*, TilePosition> & myRegions = InformationManager::Instance().getBaseOccupiedRegions(BWAPI::Broodwar->self());

	//BWAPI::Broodwar->getGeysers() return all accessable gaysers that can not build extractor
	for(auto geyser : BWAPI::Broodwar->getGeysers())
	{	
		//not my region
		if (!geyser->exists())
		{
			continue;
		}
		if (myRegions.find(BWEMMap.GetArea(geyser->getTilePosition())) == myRegions.end())
		{
			continue;
		}

		if (geyser->getDistance(Position(BWAPI::Broodwar->self()->getStartLocation())) < closest)
		{
			closest = geyser->getDistance(Position(BWAPI::Broodwar->self()->getStartLocation()));
			position = geyser->getTilePosition();
		}
	}
	return position;
}


BWAPI::TilePosition BuildingManager::getBuildLocationNear(Building & b, int buildDist, bool inRegionPriority, bool horizontalOnly) const
{
	set<const Area*> adjacentAreas;
	const Area* curArea = BWEMMap.GetArea(b.desiredPosition);
	if (curArea == NULL)
	{
		curArea = BWEMMap.GetNearestArea(b.desiredPosition);
	}
	adjacentAreas.insert(curArea);
	for (auto& area : curArea->AccessibleNeighbours())
	{
		adjacentAreas.insert(area);
	}

	int minX = 99999;
	int minY = 99999;
	int maxX = -99999;
	int maxY = -99999;
	for (auto& a : adjacentAreas)
	{
		if (a->TopLeft().x < minX)
			minX = a->TopLeft().x;
		if (a->TopLeft().y < minY)
			minY = a->TopLeft().y;
		if (a->BottomRight().x > maxX)
			maxX = a->BottomRight().x;
		if (a->BottomRight().y > maxY)
			maxY = a->BottomRight().y;
	}

	//initial desired position is base position of area
	TilePosition initialPosition = b.desiredPosition;

	//Zerg_Sunken_Colony initial location
	vector<TilePosition> areaMineralGasBase;
	TilePosition basePosition;
	if (b.type == UnitTypes::Zerg_Creep_Colony && b.waitingBuildType.size() > 0)
	{
		areaMineralGasBase.reserve(100);
		std::map<const Area*, std::map<BWAPI::Unit, buildingInfo>>& ourBuildings = InformationManager::Instance().getSelfOccupiedDetail();
		if (ourBuildings.find(BWEMMap.GetArea(b.desiredPosition)) != ourBuildings.end())
		{
			TilePosition minPosition = b.desiredPosition;
			basePosition = BWEMMap.GetArea(b.desiredPosition)->Bases().front().Location();
			basePosition = TilePosition(basePosition.x + 2, basePosition.y + 2);
			for (auto& mineral : BWEMMap.GetArea(b.desiredPosition)->Minerals())
			{
				areaMineralGasBase.push_back(TilePosition(mineral->Pos()));
			}
			for (auto& gas : BWEMMap.GetArea(b.desiredPosition)->Geysers())
			{
				areaMineralGasBase.push_back(TilePosition(gas->Pos()));
			}
			b.desiredPosition = minPosition;
		}
	}

	//returns a valid build location near the specified tile position.
	//searches outward in a spiral.
	int x = b.desiredPosition.x;
	int y = b.desiredPosition.y;

	int length = 1;
	int j = 0;
	bool first = true;
	int dx = 0;
	int dy = 1;

	int iter = 0;
	int totalIter = 1000; //(maxX - minX) * (maxY - minY);
	//distance from the nearest non walkable position
	int altitude = 32 * 2;
	if (b.type == UnitTypes::Zerg_Spawning_Pool && Broodwar->self()->supplyUsed() <= 20 * 2)
	{
		altitude = 32 * 4;
	}
	if (b.type == UnitTypes::Zerg_Creep_Colony && b.waitingBuildType.size() > 0
		&& b.waitingBuildType.front().unitType == UnitTypes::Zerg_Spore_Colony)
	{
		altitude = 32 * 4;
	}
	int minAwayDistance = 4;
	while (iter < totalIter)
	{
		iter++;

		if (isTilePositionValid(TilePosition(x, y)) && BWEMMap.GetMiniTile(WalkPosition(x * 4, y * 4)).Altitude() >= altitude)
		{
			bool tooClose = false;
			
			for (auto& block : areaMineralGasBase)
			{
				if (TilePosition(x + 1, y + 1).getDistance(block) < minAwayDistance)
				{
					tooClose = true;
					break;
				}
			}
			if (b.type == UnitTypes::Zerg_Creep_Colony && b.waitingBuildType.size() > 0)
			{
				if (b.waitingBuildType.front().unitType == UnitTypes::Zerg_Sunken_Colony)
				{
					if (TilePosition(x + 1, y + 1).getDistance(basePosition) < 3)
					{
						tooClose = true;
					}
				}
			}

			if (tooClose == false)
			{
				// can we build this building at this location
				bool canBuild = this->canBuildHereWithSpace(BWAPI::TilePosition(x, y), b, buildDist, horizontalOnly);
				//logInfo("BuildingManager", "cal building location " + to_string(x) + " " + to_string(y));

				if (canBuild)
				{
					logInfo("BuildingManager", "cal building location " + to_string(iter) + " minX " + to_string(minX)
						+ " maxX " + to_string(maxX) + " minY " + to_string(minY) + " maxY " + to_string(maxY));
					return BWAPI::TilePosition(x, y);
				}
			}
		}
		
		//otherwise, move to another position
		x = x + dx;
		y = y + dy;
		//count how many steps we take in this direction
		j++;
		if (j == length) //if we've reached the end, its time to turn
		{
			//reset step counter
			j = 0;

			//Spiral out. Keep going.
			if (!first)
				length++; //increment step counter if needed

			//first=true for every other turn so we spiral out at the right rate
			first = !first;

			//turn counter clockwise 90 degrees:
			if (dx == 0)
			{
				dx = dy;
				dy = 0;
			}
			else
			{
				dy = -dx;
				dx = 0;
			}
		}
		//Spiral out. Keep going.
	}
	logInfo("BuildingManager", "cal building location " + to_string(iter) + " minX " + to_string(minX)
		+ " maxX " + to_string(maxX) + " minY " + to_string(minY) + " maxY " + to_string(maxY));

	return BWAPI::TilePositions::None;
}


//returns true if we can build this type of unit here with the specified amount of space.
//space value is stored in this->buildDistance.
bool BuildingManager::canBuildHereWithSpace(BWAPI::TilePosition position, const Building & b, int buildDist, bool horizontalOnly) const
{
	
	//if we can't build here, we of course can't build here with space
	if (!BWAPI::Broodwar->canBuildHere(position, b.type, b.builderUnit))
	{
		return false;
	}

	// height and width of the building
	int width(b.type.tileWidth());
	int height(b.type.tileHeight());

	// define the rectangle of the building spot
	int startx = position.x - buildDist;
	int starty = position.y - buildDist;
	int endx = position.x + width;
	int endy = position.y + height;


	// if this rectangle doesn't fit on the map we can't build here
	if (startx < 0 || starty < 0 || endx > BWAPI::Broodwar->mapWidth() - 1 || endy > BWAPI::Broodwar->mapHeight() - 1)
	{
		return false;
	}

	// if we can't build here, or space is reserved, or it's in the resource box, we can't build here
	for (int x = startx; x < endx; x++)
	{
		for (int y = starty; y < endy; y++)
		{
			if (!b.type.isRefinery())
			{
				if (!buildable(x, y, b))
				{
					return false;
				}
			}
		}
	}

	return true;
}



bool BuildingManager::buildable(int x, int y, const Building & b) const
{
	//returns true if this tile is currently buildable, takes into account units on tile
	if (!BWAPI::Broodwar->isBuildable(x, y)) // &&|| b.type == BWAPI::UnitTypes::Zerg_Hatchery
	{
		return false;
	}

	if (!(BWAPI::Broodwar->hasCreep(x, y) || b.type == BWAPI::UnitTypes::Zerg_Hatchery))
	{
		return false;
	}

	// if this position is reserved
	if (InformationManager::Instance().getBuildingReserve(x, y)) {
		return false;
	}

	for(auto const& unit : Broodwar->getUnitsOnTile(x, y))
	{
		if (!unit->getType().isFlyer())
		{
			return false;
		}
	}

	return true;
}



// add a new building to be constructed
void BuildingManager::addBuildingTask(BWAPI::UnitType type, BWAPI::TilePosition desiredLocation, 
	std::vector<MetaType> waitingBuildType, Unit producer)
{

	if (debugMode) { BWAPI::Broodwar->printf("Issuing addBuildingTask: %s", type.getName().c_str()); }

	totalBuildTasks++;

	// reserve the resources for this building
	reservedMinerals += type.mineralPrice();
	reservedGas += type.gasPrice();

	// set it up to receive a worker
	//buildingData.addBuilding(ConstructionData::Unassigned, Building(type, desiredLocation));
	Building b = Building(type, desiredLocation, waitingBuildType);
	b.builderUnit = producer;
	WorkerManager::Instance().setBuildWorker(producer);
	buildingData.push_back(b);
	logInfo("BuildingManager", "add building task " + std::to_string(int(type)) + " location " + to_string(desiredLocation.x) + " " + to_string(desiredLocation.y));
}


bool BuildingManager::isBuildingPositionExplored(const Building & b) const
{
	BWAPI::TilePosition tile = b.finalPosition;

	// for each tile where the building will be built
	for (int x = 0; x < b.type.tileWidth(); ++x)
	{
		for (int y = 0; y < b.type.tileHeight(); ++y)
		{
			if (!BWAPI::Broodwar->isExplored(tile.x + x, tile.y + y))
			{
				return false;
			}
		}
	}

	return true;
}


int BuildingManager::getReservedMinerals() {
	return reservedMinerals;
}

int BuildingManager::getReservedGas() {
	return reservedGas;
}

BuildingManager & BuildingManager::Instance()
{
	static BuildingManager instance;
	return instance;
}


void BuildingManager::reserveBuilding(const Building & b) {
	TilePosition bpos = b.finalPosition;
	TilePosition bsize = b.type.tileSize();
	for (int i = bpos.x; i < bpos.x + bsize.x; i++) {
		for (int j = bpos.y; j < bpos.y + bsize.y; j++) {
			if (i < 0 || i >= Broodwar->mapWidth() || j < 0 || j >= Broodwar->mapHeight())
				continue;
			InformationManager::Instance().setBuildingReserve(i, j, true);
		}
	}
}


void BuildingManager::cancelBuildingReserve(const Building & b) {
	if (b.finalPosition == TilePositions::None)
		return;

	TilePosition bpos = b.finalPosition;
	TilePosition bsize = b.type.tileSize();
	for (int i = bpos.x; i < bpos.x + bsize.x; i++) {
		for (int j = bpos.y; j < bpos.y + bsize.y; j++) {
			if (i < 0 || i >= Broodwar->mapWidth() || j < 0 || j >= Broodwar->mapHeight())
				continue;
			InformationManager::Instance().setBuildingReserve(i, j, false);
		}
	}
}
