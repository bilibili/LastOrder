#pragma once
#include "Common.h"
#include "UnitState.h"
#include "InformationManager.h"
#include "AstarPath.h"

using namespace BWAPI;
using namespace std;
using namespace BWEM;



class BattleArmy
{
protected:
	std::vector<UnitState>		units;
	Position					regroupPosition;
	vector<std::pair<int, int>>	moveDirections;
	std::vector<BWAPI::UnitType>			movePositionOrder;
	Position								centerPosition;
	
	int							onlyGroundAttackPriority(BWAPI::Unit unit);
	int							onlyAirAttackPriority(BWAPI::Unit unit);
	int							groundAirAttackPriority(BWAPI::Unit unit);
	
	std::pair<int, set<Unit>>	getCanAttackTargets(const UnitState& curUnit, const map<UnitType, set<Unit>>& ourUnits,
		const map<UnitType, set<Unit>>& enemyUnits, map<Unit, int>& unitRetreatInfo, bool ourHasDetector = false, 
		bool enemyHasDetector = false, tacticType curTactic = tactictypeEnd);
	Unit						chooseTarget(UnitState& unit, set<Unit>& units, map<Unit, int>& assignTargets, map<Unit,Unit>& kiteAssign = map<Unit,Unit>());

	void						logDetail(UnitState& unit, map<Unit, int>& enemyAssign);	
	bool						needRegroupFlag;
	int							regroupFrontLine;
	bool						hasNearbyEnemy;
	Position					lastPosition;     // the position of the tail of the group
	int							lastLineDistance;

public:
	map<Unit, int>							groundDistanceSet;
	BattleArmy();
	virtual int					getAttackPriority(BWAPI::Unit unit);
	virtual bool				targetFilter(UnitType type);

	void						microUpdate(UnitState& unit, map<Unit, Position>& unitAttackPath, map<Unit, Position>& unitRetreatPath,
		map<Unit, int>& unitRetreatInfo, map<Unit, int>& enemyAssign, bool isRetreat, Position attackPosition,
		tacticType curTactic, Unit groundUnit = NULL, int groundArmyCount = 0, set<BWAPI::Unit>& defendEnemy = set<BWAPI::Unit>(),
		map<UnitType, set<Unit>>& encounteredEnemy = map<UnitType, set<Unit>>(), map<Unit, Unit>& kiteAssign = map<Unit, Unit>());

	void						airUnitMicroUpdate(UnitState& unit, map<Unit, Position>& unitAttackPath, map<Unit, Position>& unitRetreatPath,
		map<Unit, int>& unitRetreatInfo, map<Unit, int>& enemyAssign, bool isRetreat, Position attackPosition,
		map<UnitType, set<Unit>>& encounteredEnemy = map<UnitType, set<Unit>>());

	virtual void				move(UnitState&  attacker, BWAPI::Position targetPositio);
	virtual void				attack(UnitState&  attacker, BWAPI::Unit target);

	std::pair<int, Unit>		assignTarget(UnitState& attacker, map<Unit, int>& assignTargets,
		map<Unit, int>& unitRetreatInfo, int ourArmyRadius = 8, tacticType curTactic = HydraliskPushTactic, 
		Position attackPosition = Positions::None, set<BWAPI::Unit>& defendEnemy = set<BWAPI::Unit>(),
		map<UnitType, set<Unit>>& encounteredEnemy = map<UnitType, set<Unit>>(), map<Unit, Unit>& kiteAssign = map<Unit, Unit>());

	TilePosition				calNextMovePosition(UnitState& u, Position destination, Position armyCenterPosition, int moveMode);
	void						setRegroupPosition(Position newRegroupPosition);
	void						setRegrouping(bool newFlag) { needRegroupFlag = newFlag; }
	void						setCenterPosition(Position newCenterPosition) { centerPosition = newCenterPosition; }
	void						setNearByEnemy(bool newNearbyEnemy) { hasNearbyEnemy = newNearbyEnemy; }
	void						setGroundDistanceSet(map<Unit, int> newgroundDistanceSet) { groundDistanceSet = newgroundDistanceSet; }
	void						setRegroupFrontLine(int newregroupFrontLine) { regroupFrontLine = newregroupFrontLine; }
	void						setLastPosition(Position newlastPosition) { lastPosition = newlastPosition; }
	void						setlastLineDistance(int newlastLineDistance) { lastLineDistance = newlastLineDistance; }
	Position					getLastPosition() { return lastPosition; }
	int							getlastLineDistance() { return lastLineDistance; }

	static const int			keepMoveDurtion = 1 * 24;
	static const int			retreatMoveDurtion = 4 * 24;
	static const int			stopMoveDurtion = 10 * 24;
	static const int			groupingDurtion = 15 * 24;
	static const int			pathShareRadius = 5;
	static const int			retreatPathShareRadius = 3;
	static const int			maxHeadTailDistance = 15 * 32;
	static const int			seigeMoveDurtion = 2 * 24;
	static const int			maximumNoneAttackDurtion = 90 * 24;
	static const int			maximumMutaHarassDurtion = 3 * 60 * 24;

	void						addUnit(BWAPI::Unit u);
	std::vector<UnitState>&		getUnits() { return units; }
	void						removeUnit(BWAPI::Unit u);
	bool						hasUnit(Unit u);

	static void					smartAttackUnit(BWAPI::Unit attacker, BWAPI::Unit target);
	static void					smartAttackMove(BWAPI::Unit attacker, BWAPI::Position targetPosition);
	static void					smartMove(BWAPI::Unit attacker, BWAPI::Position targetPosition);

	static int					needRetreat(map<string, std::pair<int, int>> ourArmyTypeScoreDict, map<string, std::pair<int, int>> enemyArmyTypeScoreDict,
		set<string>& overWhelmEnemyArmy, bool ourHasDetector = false, bool enemyHasDetector = false, tacticType curTactic = tactictypeEnd);

	static void					armyTypeSaveFunc(const Unit& u, const UnitType& curType, int count, map<string, std::pair<int, int>>& armyTypeScoreDict, bool hasDetector);
	static int					calTypeScore(UnitType u);
	static map<string, vector<string>>	armyTypeCounterDict;
	static map<string, vector<string>>	armyTypeOverwhelmDict;
	static map<UnitType, string>			armyToType;
	static int							sightRadius;
	static void					parsedArmyScore(const map<UnitType, set<Unit>>& units, map<string, std::pair<int, int>>& result, bool hasDetector, bool isOur);


	static bool					needLog;
	static Unit					logUnit;
};


 

