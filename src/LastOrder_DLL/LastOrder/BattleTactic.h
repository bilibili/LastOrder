#pragma once
#include "Common.h"
#include "BattleArmy.h"
#include "UnitState.h"
#include "ZerglingArmy.h"
#include "MutaliskArmy.h"
#include "HydraliskArmy.h"
#include "OverLordArmy.h"
#include "LurkerArmy.h"
#include "ScourgeArmy.h"
#include "UltraliskArmy.h"
#include "DevourerArmy.h"
#include "GuardianArmy.h"


 
class BattleTactic
{
protected:
	Position								centerPosition;
	bool									hasTargetToAttack;

	bool									isRetreat;
	bool									hasNearbyEnemy;
	int										accumulatRetreatFrame;
	int										noEnemyAccumulateFrame;
	std::map<BWAPI::UnitType, BattleArmy*>	tacticArmy;

	map<UnitType, set<Unit>>				encounteredEnemy;

	enum tacticState { GROUPARMY, LOCATIONASSIGN, MOVE, MOVEATTACK, ATTACK, RETREAT, BACKLOCATIONASSIGN, BACKMOVE, WAIT, 
		BASEGROUP, BASEATTACK, WIN, END};
	tacticState							state;
	BWAPI::Position							attackPosition;
	BWAPI::Position							originAttackPosition;

	map<Unit, const Area*>			   unitAreaDict;

	int								   retreatFlag;
	set<string>						   overWhelmEnemyArmy;

	Unit							   firstGroundUnit;
	int								   ourArmyCount;
	int								   groundArmyCount;
	void							   updateArmyInfo(bool usingRegroup = true);

	//army global shared info
	map<Unit, int>						enemyAssign;
	map<Unit, Position>			unitAttackPath;
	map<Unit, Position>			unitRetreatPath;
	map<Unit, int>						unitRetreatInfo;
	map<Unit, Unit>					kiteAssign;


public:
	BattleTactic();
	~BattleTactic();

	virtual void		update() = 0;
	std::map<BWAPI::UnitType, BattleArmy*>& getArmy() { return tacticArmy; }
	virtual bool		isTacticEnd();
	virtual void		onUnitShow(BWAPI::Unit unit) {}
	virtual void		setAttackPosition(BWAPI::Position targetPosition, tacticType tactic);
	virtual bool		attackPositionHasEnemy();

	virtual void		addArmyUnit(BWAPI::Unit unit);
	virtual void		onUnitDestroy(BWAPI::Unit unit);
	void				onLurkerMorph();

	void				setRetreat();
	const Area*			getAttackRegion() { return BWEMMap.GetArea(TilePosition(originAttackPosition)); }
	
	map<UnitType, set<Unit>>& getEncounteredEnemy() { return encounteredEnemy; }
	TilePosition		getArmyAvgPosition() { return TilePosition(centerPosition); }

};



