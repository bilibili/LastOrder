#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>
#include <cstdlib>
#include <cassert>

#include <stdexcept>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <deque>
#include <list>
#include <set>
#include <map>

#include <BWAPI.h>
#include "BWEM/src/bwem.h"

#include "Options.h"

using namespace BWAPI;
using namespace std;
using namespace BWEM;






// type definitions for storing data
typedef		unsigned char		IDType;
typedef		unsigned char		UnitCountType;
typedef		unsigned char		ChildCountType;
typedef 	int					PositionType;
typedef 	int					TimeType;
typedef		short				HealthType;
typedef		int					ScoreType;
typedef		unsigned int		HashType;
typedef     int                 UCTValue;



typedef std::vector<BWAPI::Unit> UnitVector;

BWAPI::AIModule * __NewAIModule();


enum tacticType { HydraliskPushTactic, MutaliskHarassTac, DefendTactic, ScoutTac, PatrolTac, AirdropTac, GroupArmy, tactictypeEnd };
enum openingStrategy { TwelveHatchMuta, NinePoolling, TenHatchMuta };


enum developMode { Develop, Release };
extern developMode	curMode;

extern double armyForceMultiply;

extern std::string initReadResourceFolder;
extern std::string readResourceFolder;
extern std::string writeResourceFolder;

extern bool isTestMode;



struct double2
{
	double x, y;

	double2() : x(0), y(0) {}
	double2(const double2& p) : x(p.x), y(p.y) {}
	double2(double x, double y) : x(x), y(y) {}
	double2(const BWAPI::Position & p) : x(p.x), y(p.y) {}

	operator BWAPI::Position()				const { return BWAPI::Position(static_cast<int>(x), static_cast<int>(y)); }

	double2 operator + (const double2 & v)	const { return double2(x + v.x, y + v.y); }
	double2 operator - (const double2 & v)	const { return double2(x - v.x, y - v.y); }
	double2 operator * (double s)			const { return double2(x*s, y*s); }
	double2 operator / (double s)			const { return double2(x / s, y / s); }
	bool operator < (const double2 & v)		const 
	{
		if (x != v.x)
		{
			return x < v.x;
		}
		else
		{
			return y < v.y;
		}
	}

	double dot(const double2 & v)			const { return x*v.x + y*v.y; }
	double lenSq()							const { return x*x + y*y; }
	double len()							const { return sqrt(lenSq()); }
	double2 normal()						const 
	{
		if (len() == 0)
			return *this;
		else
			return *this / len(); 
	}

	void normalise() { double s(len()); x /= s; y /= s; }
	void rotate(double angle)
	{
		angle = angle*M_PI / 180.0;
		*this = double2(x * cos(angle) - y * sin(angle), y * cos(angle) + x * sin(angle));
	}

	double2 rotateReturn(double angle)
	{
		angle = angle*M_PI / 180.0;
		return double2(x * cos(angle) - y * sin(angle), y * cos(angle) + x * sin(angle));
	}

	double getVectorDegree(double2& other)
	{
		if (lenSq() == 0 || other.lenSq() == 0)
		{
			return 0;
		}
		double cosValue = (x * other.x + y * other.y) / (len() * other.len());
		double divergeDegree = std::acos(cosValue) * 180.0 / 3.14159265;
		return divergeDegree;
	}
};


vector<std::string> splitStr(std::string source, char split);

bool isPositionValid(BWAPI::Position p);
bool isTilePositionValid(BWAPI::TilePosition p);

bool checkDetectorFunc(const UnitType& ut);
bool isAirDefendBuilding(const UnitType& ut);
bool isDefendBuilding(const UnitType& ut);

int getPathLongth(Unit unit, vector<Position>& path);

void logInfo(std::string module, std::string s, std::string level = "INFO");
void logInit();

bool isUnitAttacking(Unit u);


//for enemy influence map
struct gridInfo
{
	double airForce;
	double groundForce;

	double enemyUnitAirForce;
	double enemyUnitGroundForce;
	double strongAirForce;

	int psionicStormDamage;

	int walkableArea;  // If not currently walkable, it is -1, else it is areaID
	bool unavailable;  // walkable but unreachable by ground unit;
	Unit buildingOnTile;
	bool isInEnemyHome;

	map<UnitType, int> buildingDetails;
	map<string, std::pair<int, int>> buildingScore;

	gridInfo()
	{
		airForce = 0;
		groundForce = 0;

		enemyUnitAirForce = 0;
		enemyUnitGroundForce = 0;

		psionicStormDamage = 0;
		walkableArea = 0;
		strongAirForce = 0;
		unavailable = false;
		buildingOnTile = nullptr;
		isInEnemyHome = false;
	}
};


struct unitInfo
{
	BWAPI::Unit	unit;
	BWAPI::TilePosition latestPosition;
	int latestUpdateFrame;

	unitInfo(Unit unit)
	{
		unit = unit;
		latestPosition = TilePosition(unit->getPosition());
		latestUpdateFrame = Broodwar->getFrameCount();
	}
	unitInfo() = default;

	bool operator < (const unitInfo& other) const
	{
		return unit < other.unit;
	}
};


struct buildingInfo
{
	BWAPI::Unit	unit;
	BWAPI::UnitType unitType;
	BWAPI::TilePosition initPosition;
	Player player;

	buildingInfo(BWAPI::Unit u)
	{
		if (u == nullptr)
		{
			return;
		}
		unit = u;
		initPosition = TilePosition(u->getPosition());
		unitType = u->getType();
		player = u->getPlayer();
	}
	buildingInfo(UnitType type, Player p)
	{
		unitType = type;
		player = p;
	}

	buildingInfo() = default;
	
	bool operator < (const buildingInfo& other) const
	{
		return unit < other.unit;
	}
};


struct gridMapCell
{
	set<buildingInfo> buildings;
	set<Unit> units;
	gridInfo im;
	void clearUnits()
	{
		units.clear();
	}
};


class unitGridMap : public utils::GridMap<gridMapCell, 4>
{
public:
	unitGridMap(const Map * pMap) : GridMap(pMap) {}

	void				Add(Unit unit);
	void				RemoveBuilding(Unit unit, TilePosition initP);
	map<UnitType, set<Unit>>						GetUnits(TilePosition origin, int radius, Player player, bool commonLog) const;
	std::pair<map<UnitType, set<Unit>>, map<UnitType, set<Unit>>>		GetUnits(set<TilePosition>& unitPositions) const;
	set<Unit>															GetUnits(TilePosition origin, int radius, Player player) const;
	bool																hasUnits(TilePosition origin) const;

	void				getAllTilePosition(TilePosition origin, int radius, set<TilePosition>& result);

};


class IMGridMap : public utils::GridMap<gridMapCell, 1>
{
public:
	IMGridMap(const Map * pMap) : GridMap(pMap) {}
};

int cloestDirection(double degree, bool reversed = false);
int cloestDirection(BWAPI::Position myPosition, BWAPI::Position enemyPosition, bool reversed = false);
