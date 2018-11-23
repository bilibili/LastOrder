#pragma once

#include "Common.h"
#include "BuildOrderQueue.h"
#include "BuildingManager.h"
#include "InformationManager.h"
#include "StrategyManager.h"



class ProductionManager
{
	ProductionManager();
	int							reservedMinerals, reservedGas;
	int							reservedSupplys;
	int							morphFailCount;

	BWAPI::Unit					selectUnitOfType(BWAPI::UnitType type, BWAPI::UnitType targetType, TilePosition closestTo);
	BuildOrderQueue				queue;
	bool						meetsReservedResources(MetaType type);
	void						createMetaType(BWAPI::Unit producer, BuildOrderItem<PRIORITY_TYPE> currentItem);
	void						manageBuildOrderQueue();
	bool						canMakeNow(BWAPI::Unit producer, MetaType t);
	void						predictWorkerMovement(Building b, Unit moveWorker);
	bool						detectNeedMorphOverLord();

	int							OverlordIsBeingBuilt();
	int							nextSupplyDeadlockDetectTime;

	BWAPI::TilePosition			getNextHatcheryLocation();

	int							timeAfterLastProduce;
	void						productionCheck(Unit producer, MetaType t);

	map<UnitType, int>			unitsUnderProduce;
	set<Unit>					usedProducer;

public:

	static ProductionManager &	Instance();
	void						update();

	int							getMorphFailCount() { return morphFailCount; }
	void						onUnitMorph(BWAPI::Unit unit);
	void						onUnitDestroy(BWAPI::Unit unit);

	int							getTopProductionNeed();
	std::vector<BWAPI::UnitType>	getWaitingProudctUnit();

	void						triggerBuilding(BWAPI::UnitType buildingType, BWAPI::TilePosition buildingLocation, int count, bool isBlocking = true, bool needBuildWorker = true);
	void						triggerUnit(BWAPI::UnitType unitType, int unitCount, bool isHighPriority = true, bool isBlocking = false);
	void						triggerUpgrade(BWAPI::UpgradeType upgrade, bool isBlocking = false) { 
		queue.queueAsHighestPriority(MetaType(upgrade), isBlocking);
		logInfo("ProductionManager", "add upgrade " + to_string(int(upgrade)));
	}
	void						triggerTech(BWAPI::TechType tech, bool isBlocking = true) { 
		queue.queueAsHighestPriority(MetaType(tech), isBlocking); 
		logInfo("ProductionManager", "add tech " + to_string(int(tech)));
	}
	void						addItemInQueue(MetaType m, bool blocking, std::vector<MetaType> waitingBuildType = vector<MetaType>()) {
		queue.queueAsHighestPriority(m, blocking, waitingBuildType); 
		logInfo("ProductionManager", "add unit " + to_string(int(m.unitType)));
	}

	bool						IsUpgradeInQueue(BWAPI::UpgradeType up);
	void						setBuildOrder(const std::vector<MetaType> & buildOrder, bool isBlock);
	bool						IsQueueEmpty() { return queue.isEmpty(); }
	int							countUnitTypeInQueue(UnitType u);
	int							getFreeMinerals();
	int							getFreeGas();
	void						buildingCallback(BWAPI::Unit curBuildingUnit, std::vector<MetaType> buildingOrders);

};
