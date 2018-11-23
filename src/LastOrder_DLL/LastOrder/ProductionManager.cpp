
#include "ProductionManager.h"
#include "AttackManager.h"


#define BOADD(N, T, B) for (int i=0; i<N; ++i) { queue.queueAsLowestPriority(MetaType(T), B); }

#define GOAL_ADD(G, M, N) G.push_back(std::pair<MetaType, int>(M, N))

ProductionManager::ProductionManager()
	: 
	reservedMinerals(0)
	, reservedGas(0)
{
	nextSupplyDeadlockDetectTime = 0;
	timeAfterLastProduce = 0;
	reservedSupplys = 0;
	morphFailCount = 0;
}


int	ProductionManager::countUnitTypeInQueue(UnitType u)
{
	int count = 0;
	for (int i = 0; i < int(queue.size()); i++)
	{
		if (queue[i].metaType.unitType == u)
			count++;
	}
	return count;
}



void ProductionManager::setBuildOrder(const std::vector<MetaType> & buildOrder, bool isBlock)
{
	// clear the current build order
	//queue.clearAll();

	// for each item in the results build order, add it
	for (size_t i(0); i < buildOrder.size(); ++i)
	{
		// queue the item
		if (buildOrder[i].isUpgrade() || buildOrder[i].isTech())
			queue.queueAsLowestPriority(buildOrder[i], false);
		else
		{
			queue.queueAsLowestPriority(buildOrder[i], isBlock);
			if (buildOrder[i].unitType == UnitTypes::Zerg_Overlord)
			{
				unitsUnderProduce[buildOrder[i].unitType] += 1;
			}
		}
			
	}
}


void ProductionManager::triggerBuilding(BWAPI::UnitType buildingType, BWAPI::TilePosition buildingLocation, int count, bool isBlocking, bool needBuildWorker)
{
	std::vector<MetaType> buildingOrder;
	if (buildingType == BWAPI::UnitTypes::Zerg_Sunken_Colony || buildingType == BWAPI::UnitTypes::Zerg_Spore_Colony)
	{
		buildingOrder.push_back(MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony, buildingLocation));
		buildingOrder.push_back(MetaType(buildingType, buildingLocation));
	}
	else
	{
		buildingOrder.push_back(MetaType(buildingType, buildingLocation));
	}
	
	MetaType buildingTarget = buildingOrder.front();
	buildingOrder.erase(buildingOrder.begin());

	logInfo("ProductionManager", "trigger building " + to_string(int(buildingTarget.unitType)) + " " + to_string(count));
	for (int i = 0; i < count; i++)
	{
		queue.queueAsHighestPriority(buildingTarget, isBlocking, buildingOrder);
	}
}


void ProductionManager::triggerUnit(BWAPI::UnitType unitType, int unitCount, bool isHighPriority, bool isBlocking)
{
	logInfo("ProductionManager", "trigger unit " + to_string(int(unitType)) + " " + to_string(unitCount));
	for (int i = 0; i < unitCount; i++)
	{
		if (isHighPriority)
			queue.queueAsHighestPriority(MetaType(unitType), isBlocking);
		else
			queue.queueAsLowestPriority(MetaType(unitType), isBlocking);
	}
}


std::vector<BWAPI::UnitType> ProductionManager::getWaitingProudctUnit()
{
	std::vector<BWAPI::UnitType> topWaitingProduct;
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& ourBuildings = InformationManager::Instance().getOurAllBuildingUnit();
	for (int i = queue.size() - 1; i >= 0; i--)
	{
		if (queue[i].metaType.isUnit() && !queue[i].metaType.unitType.isWorker() && queue[i].metaType.unitType.canAttack() && !queue[i].metaType.unitType.isBuilding())
		{
			const std::map<BWAPI::UnitType, int>& unitRequiredBuilding = queue[i].metaType.unitType.requiredUnits();
			bool preBuildingComplete = true;
			for (std::map<BWAPI::UnitType, int>::const_iterator it = unitRequiredBuilding.begin(); it != unitRequiredBuilding.end(); it++)
			{
				if (it->first != BWAPI::UnitTypes::Zerg_Larva && (ourBuildings[it->first].size() == 0 || (ourBuildings[it->first].size() > 0 && !(*ourBuildings[it->first].begin())->isCompleted())))
				{
					preBuildingComplete = false;
					break;
				}
			}
			if (preBuildingComplete)
				topWaitingProduct.push_back(queue[i].metaType.unitType);
		}
	}
	return topWaitingProduct;
}



void ProductionManager::update()
{
	//// detect if we need to morph overlord first.
	//note check too quick may lead to build more supply

	//unit's morph may take some time to change to egg, so set a check interval
	if (detectNeedMorphOverLord())
	{
		int supplyInProgress = unitsUnderProduce[UnitTypes::Zerg_Overlord] * 8 * 2;
		int supplyAvailable = BWAPI::Broodwar->self()->supplyTotal() - BWAPI::Broodwar->self()->supplyUsed();
		logInfo("ProductionManager", "add unit " + to_string(int(UnitTypes::Zerg_Overlord)) + 
			" supplyInProgress: " + to_string(supplyInProgress) +
			" supplyAvailable: " + to_string(supplyAvailable));
		unitsUnderProduce[UnitTypes::Zerg_Overlord] += 1;
		queue.queueAsHighestPriority(MetaType(BWAPI::Broodwar->self()->getRace().getSupplyProvider()), true);
	}

	// check the queue for stuff we can build
	manageBuildOrderQueue();

	timeAfterLastProduce += 1;
	if (timeAfterLastProduce >= 60 * 24 * 1 && Broodwar->self()->supplyUsed() <= 190 * 2 && !queue.isEmpty())
	{
		BuildOrderItem<PRIORITY_TYPE> currentItem = queue.getHighestPriorityItem();
		if (!meetsReservedResources(currentItem.metaType))
		{
			return;
		}

		string itemInfo;
		if (currentItem.metaType.isUnit())
		{
			itemInfo = "is Unit " + to_string(int(currentItem.metaType.unitType));
		}
		else if (currentItem.metaType.isUpgrade())
		{
			itemInfo = "is Upgrade " + to_string(int(currentItem.metaType.upgradeType));
		}
		else if (currentItem.metaType.isTech())
		{
			itemInfo = "is Tech " + to_string(int(currentItem.metaType.techType));
		}

		BWAPI::Unit producer = selectUnitOfType(currentItem.metaType.whatBuilds(), currentItem.metaType.unitType, currentItem.metaType.buildingPosition);
		int producerType = -1;
		if (producer != NULL)
		{
			producerType = producer->getType();
		}
		else
		{
			return;
		}
		bool canMake = canMakeNow(producer, currentItem.metaType);
		int supplyInProgress = unitsUnderProduce[UnitTypes::Zerg_Overlord] * 8 * 2;
		int supplyAvailable = BWAPI::Broodwar->self()->supplyTotal() - BWAPI::Broodwar->self()->supplyUsed();


		logInfo("ProductionManager", "60 seconds has no production top queue item info " + itemInfo +
			" producer type " + to_string(producerType) +
			" can make " + to_string(canMake) +
			" item mineral " + to_string(currentItem.metaType.mineralPrice()) +
			" item gas " + to_string(currentItem.metaType.gasPrice()) +
			" current mineral " + to_string(Broodwar->self()->minerals()) +
			" current gas " + to_string(Broodwar->self()->gas()) + 
			" current reserved mineral " + to_string(reservedMinerals) +
			" current reserved gas " + to_string(reservedGas) + 
			" building reserved mineral " + to_string(BuildingManager::Instance().getReservedMinerals()) + 
			" building reserved gas " + to_string(BuildingManager::Instance().getReservedGas()) + 
			" supplyInProgress " + to_string(supplyInProgress) + 
			" supply total " + to_string(BWAPI::Broodwar->self()->supplyTotal()) + 
			" supply used " + to_string(BWAPI::Broodwar->self()->supplyUsed()), "BIG_ERROR_ProductionManager");
		timeAfterLastProduce = 0;

	}
}


void ProductionManager::buildingCallback(BWAPI::Unit curBuildingUnit, std::vector<MetaType> buildingOrders)
{
	if (buildingOrders.size() == 0)
		return;

	MetaType targetBuilding = buildingOrders[0];
	buildingOrders.erase(buildingOrders.begin());

	const std::function<void(BWAPI::Game*)> buildingAction = [=](BWAPI::Game* g)->void
	{
		ProductionManager::Instance().addItemInQueue(targetBuilding, true, buildingOrders);
	};

	const std::function<bool(BWAPI::Game*)> buildingCondition = [=](BWAPI::Game* g)->bool
	{
		if (curBuildingUnit->exists())
		{
			if (targetBuilding.unitType == UnitTypes::Zerg_Sunken_Colony)
			{
				if (Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Spawning_Pool) == 1 && curBuildingUnit->isCompleted())
					return true;
				else
					return false;
			}
			else
			{
				if (curBuildingUnit->isCompleted())
					return true;
				else
					return false;
			}
		}
		else
			return false;
	};
	BWAPI::Broodwar->registerEvent(buildingAction, buildingCondition, 1, 24);
}


// on unit destroy
void ProductionManager::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit->getType() == BWAPI::UnitTypes::Zerg_Egg && unit->getBuildType() == UnitTypes::Zerg_Overlord)
	{
		unitsUnderProduce[UnitTypes::Zerg_Overlord] -= 1;
	}
}


int ProductionManager::getTopProductionNeed()
{
	BuildOrderItem<PRIORITY_TYPE> currentItem = queue.getHighestPriorityItem();
	if (currentItem.metaType.mineralPrice() > getFreeMinerals())
		return 1;
	else
		return 0;
}

bool ProductionManager::IsUpgradeInQueue(BWAPI::UpgradeType up)
{
	return queue.isUpgradeInQueue(up);
}


void ProductionManager::manageBuildOrderQueue()
{
	// if there is nothing in the queue, oh well
	if (queue.isEmpty())
	{
		return;
	}

	// the current item to be used
	BuildOrderItem<PRIORITY_TYPE> currentItem = queue.getHighestPriorityItem();
	if (!StrategyManager::Instance().isAtOpeningStage() && !StrategyManager::Instance().checkMetaValid(currentItem.metaType))
	{
		logInfo("ProductionManager", "ProductionDeadLock remove item " + std::to_string(int(currentItem.metaType.unitType))
			+ " what build " + to_string(int(currentItem.metaType.whatBuilds()))
			+ " complete count " + to_string(BWAPI::Broodwar->self()->completedUnitCount(currentItem.metaType.whatBuilds()))
			+ " checkProductionDeadLock " + to_string(!StrategyManager::Instance().checkMetaValid(currentItem.metaType)), "ERROR");
		if (currentItem.metaType.unitType.isBuilding())
		{
			StrategyManager::Instance().buildingFinish(currentItem.metaType.unitType);
		}
		else
		{
			StrategyManager::Instance().unitMorphingFinish(currentItem.metaType.unitType);
		}

		BWAPI::Broodwar->printf("checkProductionDeadLock !!!!!!");
		queue.removeHighestPriorityItem();
		timeAfterLastProduce = 0;
		return;
	}
	
	// while there is still something left in the queue, we can skip something and continue build
	while (!queue.isEmpty())
	{
		// this is the unit which can produce the currentItem
		BWAPI::Unit producer = selectUnitOfType(currentItem.metaType.whatBuilds(), currentItem.metaType.unitType, currentItem.metaType.buildingPosition);

		// check to see if we can make it right now, meet the required resource, tech, building
		// do not check if have legal building place
		bool canMake = canMakeNow(producer, currentItem.metaType);
		if (currentItem.metaType.unitType == UnitTypes::Zerg_Hatchery && !canMake && currentItem.hasAssignWorkerToMove == false)
		{
			predictWorkerMovement(Building(currentItem.metaType.unitType, currentItem.metaType.buildingPosition), producer);
		}

		// if we can make the current item
		if (producer && canMake)
		{
			// create it
			createMetaType(producer, currentItem);
			timeAfterLastProduce = 0;

			// and remove it from the queue

			queue.removeCurrentHighestPriorityItem();

			// don't actually loop around in here
			break;
		}
		// otherwise, if we can skip the current item
		else if (queue.canSkipItem())
		{
			// skip it
			queue.skipItem();

			// and get the next one
			currentItem = queue.getNextHighestPriorityItem();
		}
		else
		{
			// so break out
			break;
		}
	}
}


bool ProductionManager::canMakeNow(BWAPI::Unit producer, MetaType t)
{
	bool canMake = meetsReservedResources(t);

	if (canMake)
	{
		if (t.isUnit())
		{
			int requiredSupply = t.unitType.supplyRequired();
			if (t.unitType == UnitTypes::Zerg_Zergling || t.unitType == UnitTypes::Zerg_Scourge)
			{
				requiredSupply = t.unitType.supplyRequired() * 2;
			}
			int remainSupply = Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed();
			bool supplyMeets = remainSupply - reservedSupplys >= requiredSupply;
			if (t.unitType == UnitTypes::Zerg_Overlord || t.unitType.isBuilding())
				supplyMeets = true;
			bool mineralMeets = Broodwar->self()->minerals() - reservedMinerals >= t.unitType.mineralPrice();
			bool gasMeets = Broodwar->self()->gas() - reservedGas >= t.unitType.gasPrice();
			bool otherMeets = BWAPI::Broodwar->canMake(t.unitType, producer);

			return supplyMeets && mineralMeets && gasMeets && otherMeets;
		}
		else if (t.isTech())
		{
			canMake = true;
		}
		//canUpgrade bug for upgrading item in Hive
		else if (t.isUpgrade())
		{
			canMake = true;
		}
		else
		{
			assert(false);
		}
	}

	return canMake;
}


void ProductionManager::onUnitMorph(BWAPI::Unit unit)
{
	if (unit->getType() == UnitTypes::Zerg_Overlord)
	{
		int checkFrame = Broodwar->getFrameCount() + 24 * 2;
		const std::function<void(BWAPI::Game*)> productionAction = [=](BWAPI::Game* g)->void
		{
			unitsUnderProduce[UnitTypes::Zerg_Overlord] -= 1;
			logInfo("ProductionManager", "onUnitMorph " + to_string(unit->getType()) + " waiting count " + to_string(unitsUnderProduce[unit->getType()]));
		};

		const std::function<bool(BWAPI::Game*)> productionCondition = [=](BWAPI::Game* g)->bool
		{
			if (Broodwar->getFrameCount() > checkFrame)
				return true;
			else
				return false;
		};
		BWAPI::Broodwar->registerEvent(productionAction, productionCondition, 1, 10);

	}
}


bool ProductionManager::detectNeedMorphOverLord()
{
	int supplyInProgress = unitsUnderProduce[UnitTypes::Zerg_Overlord] * 8 * 2;
	int supplyAvailable = BWAPI::Broodwar->self()->supplyTotal() - BWAPI::Broodwar->self()->supplyUsed();
	int topUnitNeedSupply = 0;
	if (queue.size() > 0)
	{
		BuildOrderItem<PRIORITY_TYPE> currentItem = queue.getHighestPriorityItem();
		if (currentItem.metaType.isUnit())
		{
			topUnitNeedSupply = currentItem.metaType.unitType.supplyRequired();
		}
	}

	supplyAvailable = supplyAvailable - topUnitNeedSupply;
	
	if (supplyAvailable + supplyInProgress <= 1 * 2 && BWAPI::Broodwar->self()->supplyTotal() <= 10 * 2)
		return true;
	else if (supplyAvailable + supplyInProgress <= 2 * 2 && BWAPI::Broodwar->self()->supplyTotal() < 30 * 2 && BWAPI::Broodwar->self()->supplyTotal() > 10 * 2)
		return true;
	else if (supplyAvailable + supplyInProgress <= 6 * 2 && BWAPI::Broodwar->self()->supplyTotal() < 70 * 2 && BWAPI::Broodwar->self()->supplyTotal() >= 30 * 2)
		return true;
	else if (supplyAvailable + supplyInProgress <= 12 * 2 && BWAPI::Broodwar->self()->supplyTotal() < 200 * 2  && BWAPI::Broodwar->self()->supplyTotal() >= 70 * 2)
		return true;
	else
		return false;
	
}


int ProductionManager::OverlordIsBeingBuilt()
{
	int supplyMorph = 0;
	int minRemainBuildTime = 999999;

	for(auto const& unit : BWAPI::Broodwar->getAllUnits())
	{
		if (unit->getType() == BWAPI::UnitTypes::Zerg_Egg)
		{
			if (unit->getBuildType() == BWAPI::UnitTypes::Zerg_Overlord)
			{
				if (unit->getRemainingBuildTime() < minRemainBuildTime)
					minRemainBuildTime = unit->getRemainingBuildTime();
				supplyMorph += 8;
			}
		}
	}

	// for morphing info delay several frame bug..
	if (minRemainBuildTime < 50)
	{
		nextSupplyDeadlockDetectTime = BWAPI::Broodwar->getFrameCount() + 100;
	}

	return supplyMorph;
}

// When the next item in the queue is a building, this checks to see if we should move to it
// This function is here as it needs to access production manager's reserved resources info
void ProductionManager::predictWorkerMovement(Building b, Unit moveWorker)
{
	if (b.desiredPosition == TilePositions::None)
	{
		return;
	}
	BWAPI::TilePosition predictedTilePosition = b.desiredPosition;

	// where we want the worker to walk to
	BWAPI::Position walkToPosition = BWAPI::Position(predictedTilePosition);
	int mineralsRequired = b.type.mineralPrice() - getFreeMinerals() > 0 ? b.type.mineralPrice() - getFreeMinerals() : 0;
	int gasRequired = b.type.gasPrice() - getFreeGas() > 0 ? b.type.gasPrice() - getFreeGas() : 0;

	if (moveWorker != NULL && WorkerManager::Instance().willHaveResources(mineralsRequired, gasRequired, moveWorker->getDistance(walkToPosition)))
	{
		// tell the worker manager to move this worker
		WorkerManager::Instance().setMoveWorker(mineralsRequired, gasRequired, walkToPosition, moveWorker);
		queue.queue[queue.queue.size() - 1 - queue.numSkippedItems].hasAssignWorkerToMove = true;
		queue.queue[queue.queue.size() - 1 - queue.numSkippedItems].producer = moveWorker;
	}
}


int ProductionManager::getFreeMinerals()
{
	return BWAPI::Broodwar->self()->minerals() - BuildingManager::Instance().getReservedMinerals() - reservedMinerals;
}

int ProductionManager::getFreeGas()
{
	return BWAPI::Broodwar->self()->gas() - BuildingManager::Instance().getReservedGas() - reservedGas;
}

// return whether or not we meet resources, including building reserves
bool ProductionManager::meetsReservedResources(MetaType type)
{
	// return whether or not we meet the resources
	return (type.mineralPrice() <= getFreeMinerals()) && (type.gasPrice() <= getFreeGas());
}


BWAPI::TilePosition	ProductionManager::getNextHatcheryLocation()
{
	std::set<BWAPI::Unit>& selfAllbase = InformationManager::Instance().getOurAllBaseUnit();
	//the third hatchery build near the choke
	if (selfAllbase.size() == 1)
	{
		return InformationManager::Instance().getOurNatrualLocation();
	}
	else
	{
		return BWAPI::Broodwar->self()->getStartLocation();
	}
}


void ProductionManager::productionCheck(Unit producer, MetaType t)
{
	int checkFrame = Broodwar->getFrameCount() + 2 * 24;
	const std::function<void(BWAPI::Game*)> productionAction = [=](BWAPI::Game* g)->void
	{
		if (t.isUnit())
		{
			reservedMinerals -= t.unitType.mineralPrice();
			reservedGas -= t.unitType.gasPrice();
			if (t.unitType == UnitTypes::Zerg_Zergling || t.unitType == UnitTypes::Zerg_Scourge)
			{
				reservedSupplys -= t.unitType.supplyRequired() * 2;
			}
			else
			{
				reservedSupplys -= t.unitType.supplyRequired();
			}
			
			usedProducer.erase(producer);
			if (producer->isMorphing())
			{
				logInfo("ProductionManager", "unit morphing " + to_string(t.unitType) + " is processing "
					+ " producer id " + to_string(producer->getID())
					+ " reservedMinerals" + to_string(reservedMinerals)
					+ " reservedGas " + to_string(reservedGas)
					+ " reservedSupply " + to_string(reservedSupplys));
			}
			else
			{
				logInfo("ProductionManager", "unit morphing " + to_string(t.unitType) + " fail "
					+ " reservedMinerals" + to_string(reservedMinerals)
					+ " reservedGas" + to_string(reservedGas), "BIG_ERROR");
				if (!t.isBuilding())
				{
					//this->queue.queueAsLowestPriority(t, false);
					morphFailCount += 1;
				}
			}
		}
		else if (t.isUpgrade())
		{
			reservedMinerals -= t.upgradeType.mineralPrice();
			reservedGas -= t.upgradeType.gasPrice();
			usedProducer.erase(producer);

			if (Broodwar->self()->isUpgrading(t.upgradeType))
			{
				logInfo("ProductionManager", "unit upgrade " + to_string(t.upgradeType) + " is processing " 
					+ " producer id " + to_string(producer->getID())
					+ " reservedMinerals" + to_string(reservedMinerals)
					+ " reservedGas" + to_string(reservedGas));
			}
			else
			{
				logInfo("ProductionManager", "unit upgrade " + to_string(t.upgradeType) + " fail "
					+ " reservedMinerals" + to_string(reservedMinerals)
					+ " reservedGas" + to_string(reservedGas), "BIG_ERROR");
				//this->queue.queueAsLowestPriority(t, true);

				StrategyManager::Instance().unitMorphingFinish(t.unitType);
			}
		}
		else if (t.isTech())
		{
			reservedMinerals -= t.techType.mineralPrice();
			reservedGas -= t.techType.gasPrice();
			usedProducer.erase(producer);

			if (Broodwar->self()->isResearching(t.techType))
			{
				logInfo("ProductionManager", "unit tech " + to_string(t.techType) + " is processing"
					+ " producer id " + to_string(producer->getID())
					+ " reservedMinerals" + to_string(reservedMinerals)
					+ " reservedGas" + to_string(reservedGas));
			}
			else
			{
				logInfo("ProductionManager", "unit tech " + to_string(t.techType) + " fail "
					+ " reservedMinerals" + to_string(reservedMinerals)
					+ " reservedGas" + to_string(reservedGas), "BIG_ERROR");
				//this->queue.queueAsLowestPriority(t, true);
			}
		}
	};

	const std::function<bool(BWAPI::Game*)> productionCondition = [=](BWAPI::Game* g)->bool
	{
		if (Broodwar->getFrameCount() > checkFrame)
			return true;
		else
			return false;
	};
	BWAPI::Broodwar->registerEvent(productionAction, productionCondition, 1, 10);

}



// this function will check to see if all preconditions are met and then create a unit
void ProductionManager::createMetaType(BWAPI::Unit producer, BuildOrderItem<PRIORITY_TYPE> currentItem)
{
	if (!producer)
	{
		return;
	}

	MetaType t = currentItem.metaType;

	// if we're dealing with a not upgrade building
	if (t.isUnit() && t.unitType.isBuilding()
		&& t.unitType != BWAPI::UnitTypes::Zerg_Lair
		&& t.unitType != BWAPI::UnitTypes::Zerg_Hive
		&& t.unitType != BWAPI::UnitTypes::Zerg_Greater_Spire
		&& t.unitType != BWAPI::UnitTypes::Zerg_Sunken_Colony
		&& t.unitType != BWAPI::UnitTypes::Zerg_Spore_Colony
		&& t.unitType != BWAPI::UnitTypes::Zerg_Greater_Spire)
	{
		Unit actualProducer = producer;
		if (currentItem.producer != NULL)
		{
			actualProducer = currentItem.producer;
		}
		//opening building
		if (t.buildingPosition == BWAPI::TilePositions::None)
		{
			if (t.unitType == BWAPI::UnitTypes::Zerg_Hatchery)
				BuildingManager::Instance().addBuildingTask(t.unitType, getNextHatcheryLocation(), std::vector<MetaType>(), actualProducer);
			//build defend colony near chokepoint
			else
				BuildingManager::Instance().addBuildingTask(t.unitType, BWAPI::Broodwar->self()->getStartLocation(), std::vector<MetaType>(), actualProducer);
		}
		else
		{
			BuildingManager::Instance().addBuildingTask(t.unitType, t.buildingPosition, currentItem.waitingBuildType, actualProducer);
		}
		logInfo("ProductionManager", "issue add building " + std::to_string(int(t.unitType)) );
	}

	// if we're dealing with a non-building unit
	else if (t.isUnit())
	{
		// if the race is zerg, morph the unit
		if (t.unitType.getRace() == BWAPI::Races::Zerg) {

			bool result = producer->morph(t.unitType);
			reservedMinerals += t.unitType.mineralPrice();
			reservedGas += t.unitType.gasPrice();
			if (t.unitType == UnitTypes::Zerg_Zergling || t.unitType == UnitTypes::Zerg_Scourge)
			{
				reservedSupplys += t.unitType.supplyRequired() * 2;
			}
			else
			{
				reservedSupplys += t.unitType.supplyRequired();
			}

			logInfo("ProductionManager", "issue morph command " + std::to_string(int(t.unitType))
				+ " producer id " + to_string(producer->getID())
				+ "reserved mineral " + to_string(reservedMinerals)
				+ "reserved gas " + to_string(reservedGas)
				+ "reserved supply " + to_string(reservedSupplys));
			productionCheck(producer, t);
			usedProducer.insert(producer);

		}
	}

	// if we're dealing with a tech research
	else if (t.isTech())
	{
		BWAPI::Broodwar->printf("produce research  %s", t.techType.getName().c_str());
		producer->research(t.techType);
		reservedMinerals += t.techType.mineralPrice();
		reservedGas += t.techType.gasPrice();

		logInfo("ProductionManager", "issue tech command " + std::to_string(int(t.techType))
			+ "reserved mineral " + to_string(reservedMinerals)
			+ "reserved gas " + to_string(reservedGas));
		productionCheck(producer, t);
		usedProducer.insert(producer);
	}
	else if (t.isUpgrade())
	{
		BWAPI::Broodwar->printf("produce upgrade  %s", t.upgradeType.getName().c_str());
		producer->upgrade(t.upgradeType);
		reservedMinerals += t.upgradeType.mineralPrice();
		reservedGas += t.upgradeType.gasPrice();

		logInfo("ProductionManager", "issue upgrade command " + std::to_string(int(t.upgradeType))
			+ "reserved mineral " + to_string(reservedMinerals)
			+ "reserved gas " + to_string(reservedGas));
		productionCheck(producer, t);
		usedProducer.insert(producer);
	}
	else
	{
		// critical error check
		//		assert(false);

		//Logger::Instance().log("createMetaType error: " + t.getName() + "\n");
	}
}


// selects a unit of a given type
BWAPI::Unit ProductionManager::selectUnitOfType(BWAPI::UnitType type, BWAPI::UnitType targetType, TilePosition closestTo) {

	// we only want our start base to upgrade
	if (type == BWAPI::UnitTypes::Zerg_Hive ||
		type == BWAPI::UnitTypes::Zerg_Lair ||
		type == BWAPI::UnitTypes::Zerg_Hatchery)
	{
		return InformationManager::Instance().GetOurBaseUnit();
	}

	BWAPI::Unit unit = NULL;
	if (targetType.isBuilding() && type == UnitTypes::Zerg_Drone)
	{
		double minDist(1000000);
		for (auto u : BWAPI::Broodwar->self()->getUnits()) {
			if (usedProducer.find(u) != usedProducer.end())
				continue;

			if (u->getType() == type && WorkerManager::Instance().getWorkerCurJob(u) == WorkerData::Minerals) {
				double distance = u->getDistance(Position(closestTo));
				if (!unit || distance < minDist) {
					unit = u;
					minDist = distance;
				}
			}
		}
	}
	else
	{
		for (auto const& u : BWAPI::Broodwar->self()->getUnits())
		{
			if (usedProducer.find(u) != usedProducer.end())
				continue;
			if (u->isLoaded())
				continue;

			if (u->getType() == type && u->isCompleted() && u->getHitPoints() > 0)
			{
				return u;
			}
		}
	}
	
	// return what we've found so far
	return unit;
}


ProductionManager & ProductionManager::Instance() {

	static ProductionManager instance;
	return instance;
}


