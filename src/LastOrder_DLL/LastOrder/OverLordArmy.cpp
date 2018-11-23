#include "OverLordArmy.h"

OverLordArmy::OverLordArmy()
{
	splashDamgeRange = 3;
}


TilePosition OverLordArmy::calOverlordNextMovePosition(UnitState& u, Position destination)
{
	map<UnitType, set<Unit>> nearyByOurUnits =
		InformationManager::Instance().getUnitGridMap().GetUnits(u.unit->getTilePosition(), 4, Broodwar->self(), false);

	TilePosition unitPosition = TilePosition(u.unit->getPosition());
	double minScore = 99999;
	TilePosition minPosition = unitPosition;
	for (auto const& direction : moveDirections)
	{
		TilePosition tmp(unitPosition.x + direction.first, unitPosition.y + direction.second);
		if (isTilePositionValid(tmp))
		{
			double inSplashAttackRangeUnit = 0;
			if (nearyByOurUnits.find(UnitTypes::Zerg_Overlord) != nearyByOurUnits.end())
			{
				for (auto& o : nearyByOurUnits[UnitTypes::Zerg_Overlord])
				{
					if (o == u.unit)
						continue;
					if (TilePosition(o->getPosition()).getDistance(tmp) < splashDamgeRange)
					{
						inSplashAttackRangeUnit++;
					}
				}
			}
			double destDistance = Position(tmp).getDistance(destination) / 32;
			double score = destDistance + inSplashAttackRangeUnit * 5;
			if (score < minScore)
			{
				minScore = score;
				minPosition = tmp;
			}

			if (u.unit->isSelected())
			{
				Broodwar->drawTextMap(Position(tmp), "%.1f :%d", score, int(inSplashAttackRangeUnit));
			}
		}
	}
	if (u.unit->isSelected())
	{
		Broodwar->drawCircleMap(Position(minPosition), 16, Colors::Green, false);
	}
	return minPosition;
}


void OverLordArmy::overlordUpdate(UnitState& unit, map<Unit, int>& sporeAssign, int unitIndex)
{
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& ourBuilds = InformationManager::Instance().getOurAllBuildingUnit();
	Position targetDefenderPosition = Positions::None;
	if (ourBuilds.find(UnitTypes::Zerg_Spore_Colony) != ourBuilds.end())
	{
		Unit spore = NULL;
		int minDistance = 99999;
		for (auto& u : ourBuilds[UnitTypes::Zerg_Spore_Colony])
		{
			if (sporeAssign.find(u) == sporeAssign.end())
			{
				sporeAssign[u] = 0;
			}
			if (unit.unit->getDistance(u) < minDistance
				&& sporeAssign[u] < 1)
			{
				minDistance = unit.unit->getDistance(u);
				targetDefenderPosition = u->getPosition();
				spore = u;
			}
		}
		if (spore != NULL)
		{
			sporeAssign[spore] += 1;
		}
	}

	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& ourBattleUnits = InformationManager::Instance().getOurAllBattleUnit();
	Position targetHydrliskPosition = Positions::None;
	if (ourBattleUnits.find(UnitTypes::Zerg_Hydralisk) != ourBattleUnits.end())
	{
		int minDistance = 99999;
		for (auto& u : ourBuilds[UnitTypes::Zerg_Hydralisk])
		{
			if (unit.unit->getDistance(u) < minDistance && unit.unit->getDistance(u) < 20 * 32)
			{
				minDistance = unit.unit->getDistance(u);
				targetHydrliskPosition = u->getPosition();
			}
		}
	}

	Position retreatPosition = InformationManager::Instance().getRetreatDestination();
	//Broodwar->drawCircleMap(retreatPosition, 32, Colors::Blue, true);
	if (unit.unit->isUnderAttack())
	{
		TilePosition next;
		if (targetDefenderPosition != Positions::None)
		{
			next = calOverlordNextMovePosition(unit, targetDefenderPosition);
		}
		else if (targetHydrliskPosition != Positions::None)
		{
			next = calOverlordNextMovePosition(unit, targetHydrliskPosition);
		}
		else
		{
			if (unitIndex % 2 == 0 && InformationManager::Instance().isHasNatrualBase())
			{
				next = calOverlordNextMovePosition(unit, Position(InformationManager::Instance().getOurNatrualLocation()));
			}
			else
			{
				next = calOverlordNextMovePosition(unit, Position(Broodwar->self()->getStartLocation()));
			}
		}
		BattleArmy::smartMove(unit.unit, Position(next));
	}
	else
	{
		TilePosition next;
		if (targetDefenderPosition != Positions::None)
		{
			next = calOverlordNextMovePosition(unit, targetDefenderPosition);
		}
		else
		{
			if (unitIndex % 2 == 0 && InformationManager::Instance().isHasNatrualBase())
			{
				next = calOverlordNextMovePosition(unit, Position(InformationManager::Instance().getOurNatrualLocation()));
			}
			else
			{
				next = calOverlordNextMovePosition(unit, Position(Broodwar->self()->getStartLocation()));
			}
		}
		BattleArmy::smartMove(unit.unit, Position(next));
	}

}



