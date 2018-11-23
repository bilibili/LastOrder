#pragma once
#include "TimeManager.cpp"
#include "Common.h"
#include <unordered_map>
#include <queue>
#include <bitset>


struct fValueGridPoint
{
	int x;
	int y;
	double fValue;

	fValueGridPoint(int px, int py, double f)
	{
		x = px;
		y = py;
		fValue = f;
	}

	bool operator < (const fValueGridPoint & g) const
	{
		if (fValue > g.fValue)
			return true;
		else
			return false;
	}
};

class Astar
{
	Astar();
	std::vector<double2>							directions;
	double											openListIndex[256][256];
	bool											closeListIndex[256][256];
	bool											groundBlockTile[256][256];  //judge whether this position is unreachable/cover by mineral/building
	int												backtraceList[256][256];
	bool											bfsCanWalk[256][256];
	double diag_distance(double2 pos);
	int												minimumDistance;
	void											makeGroundBlockTiles(BWAPI::TilePosition startPosition, bool searchArea, int searchRange);

public:
	static Astar& Instance();
	std::list<BWAPI::TilePosition> aStarPathFinding(BWAPI::TilePosition startPosition, BWAPI::TilePosition endPosition,
		bool nearEndPosition = false, bool isGroundPathFinding = false, bool searchArea = true, bool usingIM = true, int expandRate = 4, int minimumDistance = 3, int searchRange = -1);
	Position getNextMovePosition(Unit unit, Position des, bool plotPath = false);

	Position getGroundPath(Unit unit, Position des, bool plotPath = false);
	Position getAirPath(Unit unit, Position des, bool plotPath = false);
	Position getGroundRetreatPath(Unit unit, int searchRange, bool plotPath = false);
	bool bfsTargetArea(TilePosition startPosition, TilePosition targetPosition);
};

