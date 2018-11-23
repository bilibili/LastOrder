#pragma once

#include "Common.h"
#include "WorkerData.h"
#include "InformationManager.h"
#include "BattleArmy.h"
#include "Building.h"
#include "AstarPath.h"
#include "AttackManager.h"

using namespace BWAPI;
using namespace std;
using namespace BWEM;


class WorkerManager {

	WorkerData					workerData;
	BWAPI::Unit               previousClosestWorker;

	int							workersPerRefinery;
	bool						assignScout;

	void						setMineralWorker(BWAPI::Unit unit);
	int							depotTriggerTime;
	Position					latestEnemyPosition;

	WorkerManager();
	void						handleWorkerReact();
	void						handleWorkerMoving();

public:

	void						update();
	void						balanceWorkerOnDepotComplete(BWAPI::Unit depot);
	void						onUnitDestroy(BWAPI::Unit unit);
	void						onUnitMorph(BWAPI::Unit unit);
	void						onUnitShow(BWAPI::Unit unit);
	void						addWoker(BWAPI::Unit unit) { workerData.addWorker(unit); }
	void						finishedWithWorker(BWAPI::Unit unit);

	void						handleIdleWorkers();
	void						handleGasWorkers();
	void						handleMoveWorkers();
	void						handleCombatWorkers();
	void						finishedWithCombatWorkers();

	void						drawResourceDebugInfo();
	void						updateWorkerStatus();

	int							getNumMineralWorkers();
	int							getNumGasWorkers();
	int							getNumIdleWorkers();
	void						setScoutWorker(BWAPI::Unit worker);
	Unit						addScountWorker();


	bool						isWorkerScout(BWAPI::Unit worker);
	bool						isFree(BWAPI::Unit worker);
	bool						isBuilder(BWAPI::Unit worker);

	BWAPI::Unit				getBuilder(Building & b, bool setJobAsBuilder = true);
	BWAPI::Unit				getClosestDepot(BWAPI::Unit worker);
	BWAPI::Unit				getGasWorker(BWAPI::Unit refinery);
	BWAPI::Unit				getClosestEnemyUnit(BWAPI::Unit worker);
	BWAPI::Unit               getClosestMineralWorkerTo(BWAPI::Unit enemyUnit);
	int							getDepotMineralWorkerNum(BWAPI::Unit depot) { return workerData.getDepotWorker(depot); }

	void						setMoveWorker(int m, int g, BWAPI::Position p, Unit u);
	void						setCombatWorker(BWAPI::Unit worker);
	void						setCombatWorkerArmy(int combatWorkerNum);
	void						setBuildWorker(Unit worker) { workerData.setWorkerJob(worker, WorkerData::Build, NULL); }

	bool						willHaveResources(int mineralsRequired, int gasRequired, double distance);
	void						rebalanceWorkers();

	static WorkerManager &		Instance();


	bool						isGasRequireMeet(int price);
	bool						isMineRequireMeet(int price);
	bool						isAllDepotFull();
	WorkerData::WorkerJob		getWorkerCurJob(Unit worker);
};
