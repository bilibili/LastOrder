#pragma once

#include "Common.h"
#include "MetaType.h"
#include "WorkerManager.h"
#include "InformationManager.h"
#include "Building.h"
#include "ProductionManager.h"
#include "StrategyManager.h"



class BuildingManager {

	BuildingManager();
	
	void						buildingFail(Building & b);
	std::vector<Building>		buildingData;
	bool						debugMode;
	int							totalBuildTasks;

	int							reservedMinerals;				// minerals reserved for planned buildings
	int							reservedGas;					// gas reserved for planned buildings
	int							buildingSpace;					// how much space we want between buildings

	int							latestIssueBuildingFrame;

	// functions
	bool						isBuildingPositionExplored(const Building & b) const;

	// the update() functions
	void						assignWorkersToUnassignedBuildings(Building& b);	// STEP 2
	void						exploreUnseenPosition(Building& b);
	void						constructAssignedBuildings(Building& b);			// STEP 3
	void						buildingOrderCheck(Building& b);

	//building placement info
	BWAPI::TilePosition			getRefineryPosition();
	bool						canBuildHereWithSpace(BWAPI::TilePosition position, const Building & b, int buildDist, bool horizontalOnly) const;
	bool						buildable(int x, int y, const Building & b) const;
	BWAPI::TilePosition			getBuildLocationNear(Building & b, int buildDist, bool inRegion = false, bool horizontalOnly = false) const;

	bool						checkBuilder(Building& b);
	void						reserveBuilding(const Building & b);
	void						cancelBuildingReserve(const Building & b);
public:

	void						update();
	void						onUnitMorph(BWAPI::Unit unit);
	void						onUnitDestroy(BWAPI::Unit unit);
	void						addBuildingTask(BWAPI::UnitType type, BWAPI::TilePosition desiredLocation, 
		std::vector<MetaType> waitingBuildType = std::vector<MetaType>(), Unit producer = NULL);

	//for predict move 
	BWAPI::TilePosition			getBuildingLocation( Building & b);

	int							getReservedMinerals();
	int							getReservedGas();

	static BuildingManager &	Instance();

};


