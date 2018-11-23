#pragma once

#include "Common.h"
#include "MetaType.h"



class Building {

public:
	enum BuildingState { initBuilderAndLocation, exploreMove, issueBuildOrder, refinerySpecial, buildingOrderCheck, end, specialEnd, waitLatency };

	BuildingState buildingState;

	BWAPI::TilePosition desiredPosition;
	BWAPI::TilePosition finalPosition;
	BWAPI::Position position;
	BWAPI::UnitType type;
	BWAPI::Unit buildingUnit;
	BWAPI::Unit builderUnit;
	int lastOrderFrame;
	bool buildCommandGiven;
	bool underConstruction;
	bool buildFail;
	int tryBuildTimes;
	int checkBuilderStatusTime;
	int startBuildingFrame;
	std::vector<MetaType> waitingBuildType;
	int nextTryBuildTime;
	int assignPositionTime;

	Building()
		: desiredPosition(0, 0), finalPosition(BWAPI::TilePositions::None), position(0, 0),
		type(BWAPI::UnitTypes::Unknown), buildingUnit(nullptr),
		builderUnit(nullptr), lastOrderFrame(0), buildCommandGiven(false), underConstruction(false),
		buildFail(false)
	{
		buildingState = initBuilderAndLocation;
		tryBuildTimes = 0;
		waitingBuildType = std::vector<MetaType>();
		startBuildingFrame = Broodwar->getFrameCount();
		nextTryBuildTime = 0;
		assignPositionTime = -1;
	}

	// constructor we use most often
	Building(BWAPI::UnitType t, BWAPI::TilePosition desired)
		: desiredPosition(desired), finalPosition(0, 0), position(0, 0),
		type(t), buildingUnit(nullptr), builderUnit(nullptr),
		lastOrderFrame(0), buildCommandGiven(false), underConstruction(false),
		buildFail(false)
	{
		buildingState = initBuilderAndLocation;
		tryBuildTimes = 0;
		startBuildingFrame = Broodwar->getFrameCount();
		nextTryBuildTime = 0;
		assignPositionTime = -1;
	}

	Building(BWAPI::UnitType t, BWAPI::TilePosition desired, std::vector<MetaType> w)
		: desiredPosition(desired), finalPosition(0, 0), position(0, 0),
		type(t), buildingUnit(nullptr), builderUnit(nullptr),
		lastOrderFrame(0), buildCommandGiven(false), underConstruction(false),
		buildFail(false)
	{
		buildingState = initBuilderAndLocation;
		tryBuildTimes = 0;
		waitingBuildType = w;
		startBuildingFrame = Broodwar->getFrameCount();
		nextTryBuildTime = 0;
		assignPositionTime = -1;
	}

	// equals operator
	bool operator==(const Building & b) {
		// buildings are equal if their worker unit or building unit are equal
		return (b.buildingUnit == buildingUnit) || (b.builderUnit == builderUnit);
	}
};
