#pragma once
#include "Common.h"
#include "AttackManager.h"
#include "BattleArmy.h"
#include "BattleTactic.h"
#include "MutaliskHarassTactic.h"
#include "HydraliskTactic.h"
#include "DefendTactic.h"
#include "ScoutTactic.h"
#include "PatrolTactic.h"
#include "AirdropTactic.h"



struct tacKey
{
	tacticType tacName;
	BWAPI::Position attackPosition;

	tacKey(tacticType t, BWAPI::Position a)
	{
		tacName = t;
		attackPosition = a;
	}

	bool operator < (const tacKey& t) const
	{
		if (tacName != t.tacName)
		{
			return tacName < t.tacName;
		}
		else
		{
			return attackPosition.getLength() < t.attackPosition.getLength();
		}
	}
};


class TacticManager
{
	
public:
	
	void				update();
	void				onUnitDestroy(BWAPI::Unit unit);
	void				onLurkerMorph();
	void				addTactic(tacticType tactic, BWAPI::Position attackPosition);
	void				addTacticUnit(tacticType tactic, BWAPI::Position attackPosition, BWAPI::Unit unit);
	void				addTacticArmy(tacticType tactic, BWAPI::Position attackPosition, std::map<BWAPI::UnitType, BattleArmy*>& Army, BWAPI::UnitType unitType, int count);

	bool				isTacticRun(tacticType tactic, BWAPI::Position attackPosition);
	bool				isOneTacticRun(tacticType tactic);
	BWAPI::Position		getTacticPosition(tacticType tactic);
	void				checkTacticEnd();

	static TacticManager&		Instance();
	int					getTacArmyForce(tacticType tactic, BWAPI::Position attackPosition);
	map<BWAPI::UnitType, int>					getTacArmyForceDetail(tacticType tactic, BWAPI::Position attackPosition);
	bool				isAssignScoutZergling();
	bool				isAssignPatrolOverlord();

	void				onUnitShow(BWAPI::Unit unit);
	map<tacKey, BattleTactic*>& getAllTactic() { return myTactic; }
	std::map<BWAPI::UnitType, BattleArmy*>&	getTacArmy(tacticType tactic, BWAPI::Position attackPosition);

private:
	TacticManager() {}
	std::map<tacKey, BattleTactic*> myTactic;

	int					addDefendArmyType(BattleTactic* tac, BWAPI::UnitType addArmyType, int needArmy, BWAPI::Position defendPosition);
};


