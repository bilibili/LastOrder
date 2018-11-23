#pragma once


#include "Common.h"
#include "BattleArmy.h"
#include "InformationManager.h"
#include "WorkerManager.h"

#include "ZerglingArmy.h"
#include "MutaliskArmy.h"
#include "HydraliskArmy.h"
#include "OverLordArmy.h"
#include "LurkerArmy.h"
#include "ScourgeArmy.h"
#include "UltraliskArmy.h"
#include "DevourerArmy.h"
#include "GuardianArmy.h"

#include "TacticManager.h"
#include "UnitState.h"



class AttackManager{

	std::map<BWAPI::UnitType, BattleArmy*>	myArmy;

	AttackManager();
	bool					triggerZerglingBuilding;

	bool					isNeedDefend;
	bool					hasWorkerScouter;
	bool					assignWorkerToScouter;
	bool					assignOverlord;

	bool					zerglingHarassFlag;
	std::vector<BWAPI::Unit> unRallyArmy;
	BWAPI::Position			rallyPosition;

	void					DefendUpdate();

	bool					tacticTrigCondition(int tac, BWAPI::Position attackPosition);
	void					triggerTactic(tacticType tacType, BWAPI::Position attackPosition);

	void					addTacArmy(int needArmySupply, tacticType tacType, BWAPI::Position attackPosition, bool allAirEnemy, bool allGroundEnemy);
	void					ScoutUpdate();
	void					PatrolUpdate();
	int						nextScoutTime;
	int						nextOverlordPatrolTime;

	int						enemyDisappearedTime;

	map<Unit, int>				enemyAssign;
	map<Unit, Position>			unitAttackPath;
	map<Unit, Position>			unitRetreatPath;
	map<Unit, int>				unitRetreatInfo;

public:
	
	void					update();
	void					issueAttackCommand(tacticType type, BWAPI::Position attackPosition);
	void					addArmyToTactic(tacticType type);

	void					onUnitMorph(BWAPI::Unit unit);
	void					onUnitDestroy(BWAPI::Unit unit);
	void					onLurkerMorph();

	void					addTacticRemainArmy(std::map<BWAPI::UnitType, BattleArmy*>& tacticArmy);
	std::map<BWAPI::UnitType, int> reaminArmy();
	bool					hasAttackArmy();

	std::map<BWAPI::UnitType, BattleArmy*>& getIdelArmy() { return myArmy; }
	bool					hasGroundUnit();
	
	static AttackManager&	Instance();
	void					groupArmy();
	void					overlordProtect();
	std::pair<UnitType, int>		availableAirDropArmy();

	void					resetHasWorkerScouter() { hasWorkerScouter = false; }

};

