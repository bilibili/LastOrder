
#include "WorkerManager.h"

WorkerManager::WorkerManager()
	: workersPerRefinery(3)
{
	previousClosestWorker = NULL;
	assignScout = false;
	depotTriggerTime = 0;
	latestEnemyPosition = Positions::None;
}

void WorkerManager::update()
{
	updateWorkerStatus();

	//handleWorkerMoving();

	handleWorkerReact();

	// set the gas workers
	handleGasWorkers();

	// handle idle workers
	handleIdleWorkers();

	// handle move workers
	handleMoveWorkers();

	// handle combat workers
	handleCombatWorkers();

	drawResourceDebugInfo();
	//drawWorkerInformation(450,20);

	workerData.drawDepotDebugInfo();
}


//for worker moving to another base to mine
void WorkerManager::handleWorkerMoving()
{
	for (auto& worker : workerData.getWorkers())
	{
		if (!worker->isCompleted())
		{
			continue;
		}
		if ((workerData.getWorkerJob(worker) == WorkerData::Minerals) ||
			(workerData.getWorkerJob(worker) == WorkerData::Gas))
		{
			Unit depot = workerData.getWorkerDepot(worker);
			if (depot == NULL )
			{
				continue;
			}
			if (BWEMMap.GetArea(depot->getTilePosition()) != BWEMMap.GetArea(worker->getTilePosition()))
			{
				Position nextP = Astar::Instance().getNextMovePosition(worker, Position(depot->getTilePosition()));
				worker->move(nextP);
			}
		}
	}
}


void WorkerManager::handleWorkerReact()
{
	int droneAttackRange = BWAPI::UnitTypes::Zerg_Drone.groundWeapon().maxRange();
	set<Unit> enemyNearby;
	int battleWorkerCount = 0;
	for(BWAPI::Unit worker : workerData.getWorkers())
	{
		if (workerData.getWorkerJob(worker) == WorkerData::Gas || workerData.getWorkerJob(worker) == WorkerData::Minerals)
		{
			map<UnitType, set<Unit>> unitsNearby =
				InformationManager::Instance().getUnitGridMap().GetUnits(worker->getTilePosition(), 4, Broodwar->enemy(), false);
			int closeDistance = 99999;
			BWAPI::Unit close = NULL;
			for(auto const& eArmy : unitsNearby)
			{
				for (auto& enemy : eArmy.second)
				{
					enemyNearby.insert(enemy);
					if (!enemy->getType().isFlyer()
						&& enemy->getType().canAttack() && worker->getDistance(enemy->getPosition()) < closeDistance
						&& isUnitAttacking(enemy))
					{
						closeDistance = worker->getDistance(enemy->getPosition());
						close = enemy;
					}
				}
			}

			if (close != NULL && battleWorkerCount < 2 * int(enemyNearby.size()))
			{
				BattleArmy::smartAttackUnit(worker, close);
				battleWorkerCount += 1;
			}
			else
			{
				if (!worker->isGatheringMinerals() && !worker->isGatheringGas())
				{
					if (workerData.getWorkerJob(worker) == WorkerData::Minerals)
						worker->rightClick(workerData.getMineralWorkerMine(worker));
					else
						worker->rightClick(workerData.getGasWorkerRefinery(worker));
				}
			}
		}
	}
}


Unit WorkerManager::addScountWorker()
{
	for(auto const& worker : workerData.getWorkers())
	{
		if (workerData.getWorkerJob(worker) != WorkerData::Scout)
		{
			setScoutWorker(worker);
			return worker;
		}
	}

	return NULL;
}


void WorkerManager::balanceWorkerOnDepotComplete(BWAPI::Unit depot)
{
	workerData.balanceWorker(depot);
}


void WorkerManager::updateWorkerStatus()
{
	// for each of our Workers
	for(auto& worker : workerData.getWorkers())
	{
		if (!worker->isCompleted())
		{
			continue;
		}

		// if it's idle, and worker type is Mine or Gas
		if (worker->isIdle() && workerData.getWorkerJob(worker) == WorkerData::Minerals)
		{
			//printf("Worker %d set to idle", worker->getID());
			// set its job to idle
			workerData.setWorkerJob(worker, WorkerData::Idle, NULL);
		}

		// if its job is gas
		if (workerData.getWorkerJob(worker) == WorkerData::Gas)
		{
			BWAPI::Unit refinery = workerData.getWorkerResource(worker);

			// if the refinery doesn't exist anymore
			if (!refinery || !refinery->exists() || refinery->getHitPoints() <= 0)
			{
				setMineralWorker(worker);
			}
		}
	}
}


void WorkerManager::handleGasWorkers()
{
	//if in the middle game stage, we have worker less than 10(means most worker have been killed), mine first
	//if (workerData.getNumWorkers() <= 5)
	//{
	//	for(auto const& unit : workerData.getWorkers())
	//	{
	//		if (workerData.getWorkerJob(unit) != WorkerData::Minerals && workerData.getWorkerJob(unit) != WorkerData::Scout)
	//		{
	//			setMineralWorker(unit);
	//		}
	//	}
	//	return;
	//}

	// for each unit we have
	for (auto const& unit : BWAPI::Broodwar->self()->getUnits())
	{
		// if that unit is a refinery
		if (unit->getType().isRefinery() && unit->isCompleted())
		{
			std::map<const Area*, TilePosition>& ourBaseAreas = InformationManager::Instance().getBaseOccupiedRegions(Broodwar->self());
			if (ourBaseAreas.find(BWEMMap.GetArea(unit->getTilePosition())) == ourBaseAreas.end())
			{
				continue;
			}

			// get the number of workers currently assigned to it
			int numAssigned = workerData.getNumAssignedWorkers(unit);

			// if it's less than we want it to be, fill 'er up
			for (int i = 0; i < (workersPerRefinery - numAssigned); ++i)
			{
				BWAPI::Unit gasWorker = getGasWorker(unit);
				if (gasWorker)
				{
					workerData.setWorkerJob(gasWorker, WorkerData::Gas, unit);
				}
			}
		}
	}
}


void WorkerManager::handleIdleWorkers()
{
	// for each of our workers
	for (auto const& worker : workerData.getWorkers())
	{
		// if it is idle
		if (workerData.getWorkerJob(worker) == WorkerData::Idle)
		{
			// send it to the nearest mineral patch
			setMineralWorker(worker);
		}
	}
}

// just for attack enemy scouter
void WorkerManager::handleCombatWorkers()
{
	for (auto const& worker : workerData.getWorkers())
	{
		if (workerData.getWorkerJob(worker) == WorkerData::Combat)
		{
			//BWAPI::Broodwar->drawCircleMap(worker->getPosition().x, worker->getPosition().y, 4, BWAPI::Colors::Yellow, true);
			BWAPI::Unit target = getClosestEnemyUnit(worker);

			if (target)
			{
				//BattleArmy::smartAttackUnit(worker, target);
				worker->attack(target);
				latestEnemyPosition = target->getPosition();
			}
			//else
			//{
			//	Broodwar->printf("no enemy");
			//	if (latestEnemyPosition != Positions::None)
			//	{
			//		worker->move(latestEnemyPosition);
			//	}
			//}
		}
	}
}


void WorkerManager::setCombatWorkerArmy(int combatWorkerNum)
{
	for (auto const& worker : workerData.getWorkers())
	{
		if (workerData.getWorkerJob(worker) == WorkerData::Minerals || (workerData.getWorkerJob(worker) == WorkerData::Gas))
		{
			workerData.setWorkerJob(worker, WorkerData::Combat, NULL);
			combatWorkerNum--;
			if (combatWorkerNum == 0)
				break;
		}
	}
	
}

BWAPI::Unit WorkerManager::getClosestEnemyUnit(BWAPI::Unit worker)
{
	BWAPI::Unit closestUnit = NULL;
	double closestDist = 1000;

	for (auto const& unit : BWAPI::Broodwar->enemy()->getUnits())
	{
		double dist = unit->getDistance(worker);
		if (dist < closestDist && !unit->getType().isBuilding())
		{
			closestUnit = unit;
			closestDist = dist;
		}
	}

	return closestUnit;
}

void WorkerManager::finishedWithCombatWorkers()
{
	for (auto const& worker : workerData.getWorkers())
	{
		if (workerData.getWorkerJob(worker) == WorkerData::Combat)
		{
			//Broodwar->printf("finish set worker to idle");
			//setMineralWorker(worker);
			workerData.setWorkerJob(worker, WorkerData::Idle, NULL);
		}
	}
	previousClosestWorker = NULL;
	latestEnemyPosition = Positions::None;
}


BWAPI::Unit WorkerManager::getClosestMineralWorkerTo(BWAPI::Unit enemyUnit)
{
	BWAPI::Unit closestMineralWorker = NULL;
	double closestDist = 100000;
	if (previousClosestWorker)
	{
		if (previousClosestWorker->getHitPoints() > 25)
		{
			return previousClosestWorker;
		}
		else
		{
			workerData.setWorkerJob(previousClosestWorker, WorkerData::Idle, NULL);
		}
	}

	// for each of our workers
	for (auto const& worker : workerData.getWorkers())
	{
		// if it is a move worker
		if (workerData.getWorkerJob(worker) == WorkerData::Minerals)
		{
			double dist = worker->getDistance(enemyUnit);

			if (!closestMineralWorker || dist < closestDist)
			{
				closestMineralWorker = worker;
				dist = closestDist;
			}
		}
	}

	previousClosestWorker = closestMineralWorker;
	return closestMineralWorker;
}

void WorkerManager::handleMoveWorkers()
{
	// for each of our workers
	for (auto const& worker : workerData.getWorkers())
	{
		// if it is a move worker
		if (workerData.getWorkerJob(worker) == WorkerData::Move)
		{
			WorkerMoveData data = workerData.getWorkerMoveData(worker);

			//BattleArmy::smartMove(worker, data.position);
			Position nextP = Astar::Instance().getNextMovePosition(worker, Position(data.position));
			worker->move(nextP);
		}
	}
}

// set a worker to mine minerals
void WorkerManager::setMineralWorker(BWAPI::Unit unit)
{
	if (unit == NULL)
	{
		assert(false);
	}

	// check if there is a mineral available to send the worker to
	BWAPI::Unit depot = getClosestDepot(unit);

	// if there is a valid mineral
	if (depot)
	{
		if (BWEMMap.GetArea(depot->getTilePosition()) != BWEMMap.GetArea(unit->getTilePosition()))
		{
			Position nextP = Astar::Instance().getNextMovePosition(unit, Position(depot->getTilePosition()));
			unit->move(nextP);
		}
		else
		{
			if (workerData.getWorkerJob(unit) == WorkerData::Minerals)
				return;
			else
				// update workerData with the new job
				workerData.setWorkerJob(unit, WorkerData::Minerals, depot);
		}
	}
}

BWAPI::Unit WorkerManager::getClosestDepot(BWAPI::Unit worker)
{
	if (worker == NULL)
	{
		assert(false);
	}

	BWAPI::Unit closestDepot = NULL;
	double closestDistance = 9999999;

	for (auto const& myBase : InformationManager::Instance().getOurAllBaseUnit())
	{
		if (((myBase->isCompleted() || myBase->getType() != BWAPI::UnitTypes::Zerg_Hatchery) && !workerData.depotIsFull(myBase)))
		{
			double distance = worker->getDistance(myBase);
			if (!closestDepot || distance < closestDistance)
			{
				closestDepot = myBase;
				closestDistance = distance;
			}
		}
	}
		

	return closestDepot;
}


bool WorkerManager::isGasRequireMeet(int price)
{
	if (price > 0
		&& getNumGasWorkers() == 0
		&& BWAPI::Broodwar->self()->gas() < price)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool WorkerManager::isMineRequireMeet(int price)
{
	return true;

	if (price > 0
		&& getNumMineralWorkers() == 0
		&& BWAPI::Broodwar->self()->minerals() < price)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool WorkerManager::isAllDepotFull()
{
	bool full = true;
	for (auto const& d : workerData.depots)
	{
		if (!workerData.depotIsFull(d))
		{
			full = false;
			break;
		}
	}
	return full;
}



// other managers that need workers call this when they're done with a unit
void WorkerManager::finishedWithWorker(BWAPI::Unit unit)
{
	if (!unit->exists())
	{
		BWAPI::Broodwar->printf("finishedWithWorker called with NULL unit");
		return;
	}

	workerData.setWorkerJob(unit, WorkerData::Idle, NULL);
}

BWAPI::Unit WorkerManager::getGasWorker(BWAPI::Unit refinery)
{
	if (refinery == NULL)
	{
		assert(false);
	}

	BWAPI::Unit closestWorker = NULL;
	double closestDistance = 0;

	for (auto const& unit : workerData.getWorkers())
	{
		if (workerData.getWorkerJob(unit) == WorkerData::Minerals)
		{
			double distance = unit->getDistance(refinery);
			if (!closestWorker || distance < closestDistance)
			{
				closestWorker = unit;
				closestDistance = distance;
			}
		}
	}

	return closestWorker;
}


WorkerData::WorkerJob WorkerManager::getWorkerCurJob(Unit worker)
{
	return workerData.getWorkerJob(worker);
}


// gets a builder for BuildingManager to use
// if setJobAsBuilder is true (default), it will be flagged as a builder unit
// set 'setJobAsBuilder' to false if we just want to see which worker will build a building
BWAPI::Unit WorkerManager::getBuilder(Building & b, bool setJobAsBuilder)
{
	BWAPI::Unit closestMovingWorker = nullptr;
	BWAPI::Unit closestMiningWorker = nullptr;
	double closestMovingWorkerDistance = 0;
	double closestMiningWorkerDistance = 0;

	// look through each worker that had moved there first
	for (auto const& unit : workerData.getWorkers())
	{
		if (unit->isCompleted() && (workerData.getWorkerJob(unit) == WorkerData::Minerals
			|| workerData.getWorkerJob(unit) == WorkerData::Gas
			|| workerData.getWorkerJob(unit) == WorkerData::Idle
			|| workerData.getWorkerJob(unit) == WorkerData::Move))
		{
			// if it is a new closest distance, set the pointer
			double distance = unit->getDistance(BWAPI::Position(b.finalPosition));
			if (!closestMiningWorker || distance < closestMiningWorkerDistance)
			{
				closestMiningWorker = unit;
				closestMiningWorkerDistance = distance;
			}
		}
	}

	BWAPI::Unit chosenWorker = closestMiningWorker;
	if (chosenWorker && setJobAsBuilder)
	{
		//workerData.setWorkerJob(chosenWorker, WorkerData::Build, b.type);
		workerData.setWorkerJob(chosenWorker, WorkerData::Build, NULL);
		chosenWorker->stop();
	}

	// return the worker
	return chosenWorker;
}

// sets a worker as a scout
void WorkerManager::setScoutWorker(BWAPI::Unit worker)
{
	if (worker == NULL)
	{
		assert(false);
	}

	workerData.setWorkerJob(worker, WorkerData::Scout, NULL);
}

// sets a worker to move to a given location
void WorkerManager::setMoveWorker(int mineralsNeeded, int gasNeeded, BWAPI::Position p, Unit u)
{
	workerData.setWorkerJob(u, WorkerData::Move, WorkerMoveData(mineralsNeeded, gasNeeded, p));
}

// will we have the required resources by the time a worker can travel a certain distance
bool WorkerManager::willHaveResources(int mineralsRequired, int gasRequired, double distance)
{
	// if we don't require anything, we will have it
	if (mineralsRequired <= 0 && gasRequired <= 0)
	{
		return true;
	}

	// the speed of the worker unit
	double speed = BWAPI::Broodwar->self()->getRace().getWorker().topSpeed();

	// how many frames it will take us to move to the building location
	// add a second to account for worker getting stuck. better early than late
	double framesToMove = (distance / speed) + 100;

	// magic numbers to predict income rates
	double mineralRate = getNumMineralWorkers() * 0.045;
	double gasRate = getNumGasWorkers() * 0.07;

	// calculate if we will have enough by the time the worker gets there
	if (mineralRate * framesToMove >= mineralsRequired &&
		gasRate * framesToMove >= gasRequired)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void WorkerManager::setCombatWorker(BWAPI::Unit worker)
{
	if (worker == NULL)
	{
		assert(false);
	}

	workerData.setWorkerJob(worker, WorkerData::Combat, NULL);
}

void WorkerManager::onUnitMorph(BWAPI::Unit unit)
{
	if (unit == NULL)
	{
		assert(false);
	}

	// if something morphs into a worker, add it
	if (unit->getType().isWorker() && unit->getPlayer() == BWAPI::Broodwar->self() && unit->getHitPoints() >= 0)
	{
		workerData.addWorker(unit);
	}

	if (unit->getType().isResourceDepot() && unit->getPlayer() == BWAPI::Broodwar->self())
	{
		for (auto& area : BWEMMap.Areas())
		{
			for (auto& base : area.Bases())
			{
				if (base.Location().getDistance(unit->getTilePosition()) < 2)
				{
					workerData.addDepot(unit);
					break;
				}
			}
		}
	}

	// if something morphs into a building, it was a worker?
	if (unit->getType().isBuilding() && unit->getPlayer() == BWAPI::Broodwar->self() && unit->getPlayer()->getRace() == BWAPI::Races::Zerg)
	{
		//BWAPI::Broodwar->printf("A Drone started building");
		workerData.workerDestroyed(unit);
	}
}

void WorkerManager::onUnitShow(BWAPI::Unit unit)
{
	if (unit == NULL)
	{
		assert(false);
	}

	// add the depot if it exists
	if (unit->getType().isResourceDepot() && unit->getPlayer() == BWAPI::Broodwar->self())
	{
		workerData.addDepot(unit);
	}

	// if something morphs into a worker, add it
	if (unit->getType().isWorker() && unit->getPlayer() == BWAPI::Broodwar->self() && unit->getHitPoints() >= 0)
	{
		//BWAPI::Broodwar->printf("A worker was shown %d", unit->getID());
		workerData.addWorker(unit);
	}
}


void WorkerManager::rebalanceWorkers()
{
	// for each worker
	for (auto const& worker : workerData.getWorkers())
	{
		// we only care to rebalance mineral workers
		if (!workerData.getWorkerJob(worker) == WorkerData::Minerals)
		{
			continue;
		}

		// get the depot this worker works for
		BWAPI::Unit depot = workerData.getWorkerDepot(worker);

		// if there is a depot and it's full
		if (depot && workerData.depotIsFull(depot))
		{
			// set the worker to idle
			workerData.setWorkerJob(worker, WorkerData::Idle, NULL);
		}
		// if there's no depot
		else if (!depot)
		{
			// set the worker to idle
			workerData.setWorkerJob(worker, WorkerData::Idle, NULL);
		}
	}
}

void WorkerManager::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit == NULL)
	{
		assert(false);
	}

	// remove the depot if it exists
	if (unit->getType().isResourceDepot() && unit->getPlayer() == BWAPI::Broodwar->self())
	{
		workerData.removeDepot(unit);
	}

	// if the unit that was destroyed is a worker
	if (unit->getType().isWorker() && unit->getPlayer() == BWAPI::Broodwar->self())
	{
		if (workerData.getWorkerJob(unit) == WorkerData::WorkerJob::Combat)
		{
			AttackManager::Instance().resetHasWorkerScouter();
		}
		// tell the worker data it was destroyed
		workerData.workerDestroyed(unit);
	}

	if (unit->getType() == BWAPI::UnitTypes::Resource_Mineral_Field)
	{
		rebalanceWorkers();
	}
}


void WorkerManager::drawResourceDebugInfo() {

	for (auto const& worker : workerData.getWorkers()) {

		char job = workerData.getJobCode(worker);

		BWAPI::Position pos = worker->getTargetPosition();

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawTextMap(worker->getPosition().x, worker->getPosition().y - 5, "\x07%c", job);

		if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawLineMap(worker->getPosition().x, worker->getPosition().y, pos.x, pos.y, BWAPI::Colors::Cyan);

		BWAPI::Unit depot = workerData.getWorkerDepot(worker);
		if (depot)
		{
			if (Options::Debug::DRAW_UALBERTABOT_DEBUG) BWAPI::Broodwar->drawLineMap(worker->getPosition().x, worker->getPosition().y, depot->getPosition().x, depot->getPosition().y, BWAPI::Colors::Orange);
		}
	}
}


bool WorkerManager::isFree(BWAPI::Unit worker)
{
	return workerData.getWorkerJob(worker) == WorkerData::Minerals || workerData.getWorkerJob(worker) == WorkerData::Idle;
}

bool WorkerManager::isWorkerScout(BWAPI::Unit worker)
{
	return (workerData.getWorkerJob(worker) == WorkerData::Scout);
}

bool WorkerManager::isBuilder(BWAPI::Unit worker)
{
	return (workerData.getWorkerJob(worker) == WorkerData::Build);
}

int WorkerManager::getNumMineralWorkers()
{
	return workerData.getNumMineralWorkers();
}

int WorkerManager::getNumIdleWorkers()
{
	return workerData.getNumIdleWorkers();
}

int WorkerManager::getNumGasWorkers()
{
	return workerData.getNumGasWorkers();
}


WorkerManager & WorkerManager::Instance() {

	static WorkerManager instance;
	return instance;
}