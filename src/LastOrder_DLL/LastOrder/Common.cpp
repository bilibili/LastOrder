#include "Common.h"


BWAPI::AIModule * __NewAIModule();

developMode curMode = Develop;

double armyForceMultiply = 1.5;

bool isTestMode;

std::string initReadResourceFolder = "./bwapi-data/AI/";
std::string readResourceFolder = "./bwapi-data/write/";
std::string writeResourceFolder = "./bwapi-data/write/";


void logInit()
{
	fstream historyFile;
	std::string filePath = "./bwapi-data/write/log_detail_file";
	historyFile.open(filePath.c_str(), ios::out);
	historyFile << endl;
	historyFile.close();
}


void logInfo(std::string module, std::string s, std::string level)
{
	/*
	if (module.find("Tactic") != std::string::npos ||
		module.find("Army") != std::string::npos)
	{
		return;
	}
	
	fstream historyFile;
	std::string filePath = "./bwapi-data/write/log_detail_file";
	historyFile.open(filePath.c_str(), ios::app);
	int curTime = int(Broodwar->getFrameCount() * 67 / 1000);
	historyFile << curTime/60 << ":" << curTime % 60 << "  "
		<< "level:" << level << "  "
		<< "module:" << module << "  "
		<< s << endl;
	historyFile.close();
	*/
}


bool isUnitAttacking(Unit u)
{
	return u->isAttacking() || u->getGroundWeaponCooldown() > 0 || u->getAirWeaponCooldown() > 0;
}


vector<std::string> splitStr(std::string source, char split)
{
	std::stringstream ss(source);
	std::vector<string> itemList;
	string item;
	while (getline(ss, item, split))
	{
		if (item != "")
			itemList.push_back(item);
	}
	return itemList;
}

bool isPositionValid(BWAPI::Position p)
{
	if (p.x > BWAPI::Broodwar->mapWidth() * 32 - 1 || p.x < 0
		|| p.y > BWAPI::Broodwar->mapHeight() * 32 - 1 || p.y < 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}


bool isTilePositionValid(BWAPI::TilePosition p)
{
	if (p.x > BWAPI::Broodwar->mapWidth() - 1 || p.x < 0
		|| p.y > BWAPI::Broodwar->mapHeight() - 1 || p.y < 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool checkDetectorFunc(const UnitType& ut)
{
	if (ut == BWAPI::UnitTypes::Zerg_Overlord
		|| ut == BWAPI::UnitTypes::Terran_Science_Vessel
		|| ut == BWAPI::UnitTypes::Protoss_Observer
		|| ut == BWAPI::UnitTypes::Protoss_Photon_Cannon
		|| ut == BWAPI::UnitTypes::Terran_Missile_Turret
		|| ut == BWAPI::UnitTypes::Zerg_Spore_Colony)
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool isAirDefendBuilding(const UnitType& ut)
{
	if (ut == BWAPI::UnitTypes::Protoss_Photon_Cannon
		|| ut == BWAPI::UnitTypes::Zerg_Spore_Colony
		|| ut == BWAPI::UnitTypes::Terran_Missile_Turret
		|| ut == BWAPI::UnitTypes::Terran_Bunker)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool isDefendBuilding(const UnitType& ut)
{
	if (ut == BWAPI::UnitTypes::Protoss_Photon_Cannon
		|| ut == BWAPI::UnitTypes::Zerg_Spore_Colony
		|| ut == BWAPI::UnitTypes::Zerg_Sunken_Colony
		|| ut == BWAPI::UnitTypes::Terran_Missile_Turret
		|| ut == BWAPI::UnitTypes::Terran_Bunker)
	{
		return true;
	}
	else
	{
		return false;
	}
}

int getPathLongth(Unit unit, vector<Position>& path)
{
	Position destination = path[0];
	int curLonth = unit->getDistance(destination);
	for (auto i = 0; i < int(path.size()); i++)
	{
		if (i + 1 < int(path.size()))
		{
			curLonth += int(path[i].getDistance(path[i + 1]));
		}
	}
	return curLonth;
}



void unitGridMap::Add(Unit unit)
{
	auto & cell = GetCell(TilePosition(unit->getPosition()));
	if (unit->getType().isBuilding())
	{
		cell.buildings.insert(buildingInfo(unit));
	}
	cell.units.insert(unit);
}


void unitGridMap::RemoveBuilding(Unit unit, TilePosition initP)
{
	auto & cell = GetCell(initP);
	if (unit->getType().isBuilding())
	{
		cell.buildings.erase(buildingInfo(unit));
	}
}


std::pair<map<UnitType, set<Unit>>, map<UnitType, set<Unit>>> unitGridMap::GetUnits(set<TilePosition>& unitPositions) const
{
	std::pair<map<UnitType, set<Unit>>, map<UnitType, set<Unit>>> Res;
	for (auto& p : unitPositions)
	{
		for (auto& unit : GetCell(p).units)
		{
			if (unit->getPlayer() == Broodwar->enemy())
			{
				Res.second[unit->getType()].insert(unit);
			}
			if (unit->getPlayer() == Broodwar->self())
			{
				Res.first[unit->getType()].insert(unit);
			}
		}
	}
	return Res;
}


map<UnitType, set<Unit>> unitGridMap::GetUnits(TilePosition origin, int radius, Player player, bool commonLog) const
{
	int y_start = origin.y - radius > 0 ? origin.y - radius : 0;
	int y_end = origin.y + radius > BWAPI::Broodwar->mapHeight() - 1 ? BWAPI::Broodwar->mapHeight() - 1 : origin.y + radius;
	int x_start = origin.x - radius > 0 ? origin.x - radius : 0;
	int x_end = origin.x + radius > BWAPI::Broodwar->mapWidth() - 1 ? BWAPI::Broodwar->mapWidth() - 1 : origin.x + radius;

	std::pair<int, int>	topleft = GetCellCoords(TilePosition(x_start, y_start));
	std::pair<int, int>	bottomright = GetCellCoords(TilePosition(x_end, y_end));

	map<UnitType, set<Unit>> Res;
	for (int i = topleft.first; i <= bottomright.first; i++)
	{
		for (int j = topleft.second; j <= bottomright.second; j++)
		{
			for (auto& unit : GetCell(i, j).units)
			{
				if (unit->getPlayer() == player)
				{
					Res[unit->getType()].insert(unit);
				}
			}
		}
	}

	if (commonLog)
	{
		logInfo("Common", "GetUnits, origin x: " + to_string(x_start) + " " + to_string(x_end)
			+ "y: " + to_string(y_start) + " " + to_string(y_end)
			+ "gridmap x: " + to_string(topleft.first) + " " + to_string(bottomright.first)
			+ "y: " + to_string(topleft.second) + " " + to_string(bottomright.second));
	}

	return Res;
}



set<Unit> unitGridMap::GetUnits(TilePosition origin, int radius, Player player) const
{
	int y_start = origin.y - radius > 0 ? origin.y - radius : 0;
	int y_end = origin.y + radius > BWAPI::Broodwar->mapHeight() - 1 ? BWAPI::Broodwar->mapHeight() - 1 : origin.y + radius;
	int x_start = origin.x - radius > 0 ? origin.x - radius : 0;
	int x_end = origin.x + radius > BWAPI::Broodwar->mapWidth() - 1 ? BWAPI::Broodwar->mapWidth() - 1 : origin.x + radius;

	std::pair<int, int>	topleft = GetCellCoords(TilePosition(x_start, y_start));
	std::pair<int, int>	bottomright = GetCellCoords(TilePosition(x_end, y_end));

	set<Unit> Res;
	for (int i = topleft.first; i <= bottomright.first; i++)
	{
		for (int j = topleft.second; j <= bottomright.second; j++)
		{
			for (auto& unit : GetCell(i, j).units)
			{
				if (unit->getPlayer() == player)
				{
					Res.insert(unit);
				}
			}
		}
	}
	return Res;
}

bool unitGridMap::hasUnits(TilePosition origin) const {
	return GetCell(origin).units.size() > 0;
}

void unitGridMap::getAllTilePosition(TilePosition origin, int radius, set<TilePosition>& result)
{
	int y_start = origin.y - radius > 0 ? origin.y - radius : 0;
	int y_end = origin.y + radius > BWAPI::Broodwar->mapHeight() - 1 ? BWAPI::Broodwar->mapHeight() - 1 : origin.y + radius;
	int x_start = origin.x - radius > 0 ? origin.x - radius : 0;
	int x_end = origin.x + radius > BWAPI::Broodwar->mapWidth() - 1 ? BWAPI::Broodwar->mapWidth() - 1 : origin.x + radius;

	for (int i = x_start; i <= x_end; i++)
	{
		for (int j = y_start; j <= y_end; j++)
		{
			result.insert(TilePosition(i, j));
		}
	}
}

int cloestDirection(double degree, bool reversed) {
	// degree range: 0 <= x < 360
	if (degree >= 22.5 && degree < 67.5) {
		double2 enemyDirection(1, 1);
		int id = 1;
		return reversed ? 5 : 1;
	}
	else if (degree >= 67.5 && degree < 112.5) {
		double2 enemyDirection(0, 1);
		int id = 2;
		return reversed ? 6 : 2;
	}
	else if (degree >= 112.5 && degree < 157.5) {
		double2 enemyDirection(-1, 1);
		int id = 3;
		return reversed ? 7 : 3;
	}
	else if (degree >= 157.5 && degree < 202.5) {
		double2 enemyDirection(-1, 0);
		int id = 4;
		return reversed ? 0 : 4;
	}
	else if (degree >= 202.5 && degree < 247.5) {
		double2 enemyDirection(-1, -1);
		int id = 5;
		return reversed ? 1 : 5;
	}
	else if (degree >= 247.5 && degree < 292.5) {
		double2 enemyDirection(0, -1);
		int id = 6;
		return reversed ? 2 : 6;
	}
	else if (degree >= 292.5 && degree < 337.5) {
		double2 enemyDirection(1, -1);
		int id = 7;
		return reversed ? 3 : 7;
	}
	else {
		double2 enemyDirection(1, 0);
		int id = 0;
		return reversed ? 4 : 0;
	}
}

int cloestDirection(BWAPI::Position myPosition, BWAPI::Position enemyPosition, bool reversed)
{
	/*ID
	0 : (1, 0)(right)
	1 : (1, 1)
	2 : (0, 1)(down)
	3 : (-1, 1)
	4 : (-1, 0)
	5 : (-1, -1)
	6 : (0, -1)
	7 : (1, -1)*/
	double eps = 1e-8;
	double PI = acos(-1.0);
	double2 v = enemyPosition - myPosition;
	if (abs(v.x) < eps && abs(v.y) < eps)
	{
		// same position, move right
		double2 enemyDirection(0, 0);
		int id = 0;
		return reversed ? 4 : 0;
	}
	if (v.x > 0 && abs(v.y) < eps) {
		double2 enemyDirection(1, 0);
		int id = 0;
		return reversed ? 4 : 0;
	}
	if (v.x < 0 && abs(v.y) < eps) {
		double2 enemyDirection(-1, 0);
		int id = 4;
		return reversed ? 0 : 4;
	}
	if (abs(v.x) < eps && v.y > 0) {
		double2 enemyDirection(0, 1);
		int id = 2;
		return reversed ? 6 : 2;
	}
	if (abs(v.x) < eps && v.y < 0) {
		double2 enemyDirection(0, -1);
		int id = 6;
		return reversed ? 2 : 6;
	}
	double degree = atan2(v.y, v.x) * 180.0 / PI; 
	if (degree < 0) {
		degree += 360.0;
	}
	return cloestDirection(degree, reversed);
}
