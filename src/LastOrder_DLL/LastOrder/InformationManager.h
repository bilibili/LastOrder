#pragma once

#include "Common.h"
#include "BWEM/src/bwem.h"
#include "BattleArmy.h"


using namespace BWAPI;
using namespace std;
using namespace BWEM;



class InformationManager {

	InformationManager();

	//self info;
	BWAPI::Unit							selfBaseUnit;
	BWAPI::Position						selfNaturalChokePoint;
	BWAPI::Position						selfNaturalToEnemyChoke;

	BWAPI::TilePosition					selfStartBaseLocation;
	BWAPI::TilePosition					selfNaturalBaseLocation;

	std::set<BWAPI::Unit>				selfAllBase;
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>		selfAllBuilding;
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>		selfAllBattleUnit;

	//enemy info
	BWAPI::TilePosition					enemyStartBaseLocation;
	BWAPI::TilePosition					enemyNaturalBaseLocation;
	std::set<BWAPI::Unit>				enemyAllBase;
	map<BWAPI::UnitType, map<Unit, buildingInfo>>			enemyAllBuilding;
	map<BWAPI::UnitType, map<Unit, unitInfo>>				enemyAllBattleUnit;
	//enemy location info
	map<const Area*, map<BWAPI::UnitType, map<Unit, unitInfo>>> enemyNearEnemyBaseInfo;
	map<const Area*, map<BWAPI::UnitType, map<Unit, unitInfo>>> enemyNearOurBaseInfo;

	//occupied region which include base
	std::map<const Area*, TilePosition>			occupiedRegions[2];
	//all building info
	std::map<const Area*, std::map<BWAPI::Unit, buildingInfo>> selfOccupiedDetail;
	std::map<const Area*, std::map<BWAPI::Unit, buildingInfo>> enemyOccupiedDetail;

	void							updateUnit(BWAPI::Unit unit);
	void							addUnitInfluenceMap(BWAPI::Unit unit, bool addOrdestroy);
	void							updateAllUnit();
	void							setInfluenceMap(BWAPI::Position initPosition, int attackRange, int exactAttackRange, int groundDamage, int airDamage, bool addOrdestroy, UnitType type);

	std::map<const Area*, int>	baseGroundDistance;
	std::map<const Area*, int>	baseAirDistance;

	unitGridMap						unitGM;
	IMGridMap						IMGM;
	bool							buildingReserve[256][256];

	std::map<const Area*, std::set<BWAPI::Unit>> enemyUnitsInRegion;
	std::map<const Area*, std::set<BWAPI::Unit>> enemyUnitsTargetRegion;
	void							updateDefendInfo();
	void							updateEnemyLocationInfo();
	const Area*						getNearstRegion(TilePosition p, bool isFlyer, bool checkEnemy);

	TilePosition					baseSunkenBuildingPosition;
	TilePosition					natrualSunkenBuildingPosition;

	map<BWAPI::UnitType, int>		killedEnemyCount;
	map<BWAPI::UnitType, int>		killedOurCount;
	void							updateEnemyBlockInfo();
	bool							enemyBlockingBase;
	bool							enemyBlockingNatural;
	TilePosition					enemyBaseChoke;
	TilePosition					enemyNaturalChoke;
	bool							enemyEverBlockingBase;
	bool							enemyEverBlockingNatural;
	set<const Area*>				enemyNaturaltoBaseAreas;

public:

	map<BWAPI::UnitType, int>&		getKilledEnemyInfo() { return killedEnemyCount; }
	map<BWAPI::UnitType, int>&		getKilledOurInfo() { return killedOurCount; }

	void							setLocationEnemyBase(BWAPI::TilePosition Here);

	void							updateOccupiedRegions(TilePosition basePosition, BWAPI::Player player);
	void							addOccupiedRegionsDetail(const Area * region, BWAPI::Player player, BWAPI::Unit building);
	void							destroyOccupiedRegionsDetail(const Area * region, BWAPI::Player player, BWAPI::Unit building);
	void							checkOccupiedDetail();

	BWAPI::Unit						GetOurBaseUnit();

	std::map<const Area*, int>&		getBaseGroudDistance();
	std::map<const Area*, int>&		getBaseAirDistance();
	BWAPI::TilePosition				GetNextExpandLocation();
	
	BWAPI::TilePosition				getOurBaseLocation() { return BWAPI::Broodwar->self()->getStartLocation(); }
	BWAPI::TilePosition				getOurNatrualLocation();
	BWAPI::Position					GetEnemyBasePosition();
	BWAPI::Position					GetEnemyNaturalPosition();
	bool							isHasNatrualBase();

	Position						getOurBaseToNatrualChokePosition();


	std::set<BWAPI::Unit>&			getOurAllBaseUnit() { return selfAllBase; }
	std::set<BWAPI::Unit>&			getEnemyAllBaseUnit() { return enemyAllBase; }
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& getOurAllBattleUnit() { return selfAllBattleUnit; }
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& getOurAllBuildingUnit() { return selfAllBuilding; }
	int								getOurTotalBattleForce();
	int								getEnemyTotalAntiGroundBattleForce();

	std::map<BWAPI::UnitType, map<Unit, unitInfo>>& getEnemyAllBattleUnit() { return enemyAllBattleUnit; }
	std::map<BWAPI::UnitType, map<Unit, buildingInfo>>& getEnemyAllBuildingUnit() { return enemyAllBuilding; }

	map<const Area*, map<BWAPI::UnitType, map<Unit, unitInfo>>>& getEnemyNearEnemyLocationInfo() { return enemyNearEnemyBaseInfo; }
	map<const Area*, map<BWAPI::UnitType, map<Unit, unitInfo>>>& getEnemyNearOurLocationInfo() { return enemyNearOurBaseInfo; }

	gridInfo&						getEnemyInfluenceMap(int x, int y) { return IMGM.GetCell(TilePosition(x, y)).im; }
	unitGridMap&					getUnitGridMap() { return unitGM; }
	map<string, std::pair<int, int>>				getMaxInfluenceMapValue(TilePosition p, int range);
	map<string, std::pair<int, int>>				getMaxInfluenceMapValue(set<TilePosition>& unitPositions);

	std::map<const Area*, TilePosition> &		getBaseOccupiedRegions(BWAPI::Player player);
	std::map<const Area*, std::map<BWAPI::Unit, buildingInfo>>& getEnemyOccupiedDetail() { return enemyOccupiedDetail; }
	std::map<const Area*, std::map<BWAPI::Unit, buildingInfo>>& getSelfOccupiedDetail() { return selfOccupiedDetail; }

	void							onUnitShow(BWAPI::Unit unit);
	void							onUnitMorph(BWAPI::Unit unit);
	void							onUnitDestroy(BWAPI::Unit unit);
	void							onLurkerMorph(Unit unit);

	bool							isEnemyHasInvisibleUnit();
	bool							isEnemyHasFlyerAttacker();

	static InformationManager&		Instance();
	void							update();

	std::string						getEnemyRace();
	Position						getRetreatDestination();
	std::map<const Area*, std::set<BWAPI::Unit>>& getDefendEnemyInfo() { return enemyUnitsTargetRegion; }
	BWAPI::TilePosition				getSunkenBuildingPosition(std::string sunkenArea);

	bool							hasAttackArmy();
	void							resetBuildingReserve() { memset(buildingReserve, 0, sizeof(buildingReserve)); }
	bool							getBuildingReserve(int x, int y) { return buildingReserve[x][y]; }
	void							setBuildingReserve(int x, int y, bool reserve) { buildingReserve[x][y] = reserve; }
	
	bool getEnemyBlockingBase()		{ return enemyBlockingBase; }
	bool getEnemyBlockingNatural()	{ return enemyBlockingNatural; }
	void setUnavailableTile(TilePosition pos) { IMGM.GetCell(pos).im.unavailable = 1; }
	TilePosition getEnemyBaseChoke() { return enemyBaseChoke; }
	TilePosition getEnemyNaturalChoke() { return enemyNaturalChoke; }
	bool getEnemyEverBlockingHome() { return (enemyEverBlockingBase || enemyEverBlockingNatural); }
	Position getAreaEntrance(const ChokePoint* cp, const Area* target);
};


