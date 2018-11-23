#pragma once
#include "BattleTactic.h"
#include "AttackManager.h"

class AirdropTactic : public BattleTactic
{
	enum AirdropState { begin = 0, loading = 1, moving = 2, unloading = 3, attacking = 4 };
	AirdropState airDropstate;
	vector<Unit>				overlords;
	vector<Unit>				airdropArmy;
	set<Unit>					alreadyMoveFlag;
	set<Unit>					alreadyStopFlag;
	Position					overlordDestination;
	std::list<BWAPI::TilePosition>		overLordStoredPath;
	void						airDropAttack();
	void						loaderMove(Unit loader, Position destination, bool plotPath = false);
	int							lastUpdateAstar;
	void						updateAstar();

	// for debug
	int							timestamp;
	std::string toString(int n) { stringstream ss; ss << n; return ss.str(); }
	std::string					objectID;
	bool						outputRetreat;

public:
	AirdropTactic();
	virtual void				update() override;
	virtual void				onUnitDestroy(BWAPI::Unit unit) override;
	virtual void				onUnitShow(BWAPI::Unit unit) override;
	virtual void				addArmyUnit(BWAPI::Unit unit) override;
};

