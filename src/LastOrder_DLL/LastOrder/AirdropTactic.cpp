#include "AirdropTactic.h"

AirdropTactic::AirdropTactic()
{
	lastUpdateAstar = -1;
	airDropstate = AirdropTactic::begin;
	//Broodwar->printf("start loading");
	char idname[20];
	sprintf_s(idname, "%p_%d", this, Broodwar->getFrameCount());
	objectID = string(idname);
	Broodwar->printf(objectID.c_str());
	outputRetreat = false;
}

void AirdropTactic::airDropAttack() {
	int curFrame = Broodwar->getFrameCount();
	updateArmyInfo(false);
	//clear shared info
	unitAttackPath.clear();
	unitRetreatPath.clear();
	enemyAssign.clear();
	unitRetreatInfo.clear();

	//main update logic
	hasTargetToAttack = false;
	encounteredEnemy.clear(); 
	for (auto it = kiteAssign.begin(); it != kiteAssign.end();) {
		if (!it->first->exists() || !it->second->exists()) {
			it = kiteAssign.erase(it);
		}
		else {
			it++;
		}
	}

	for (auto const& army : tacticArmy)
	{
		if (army.first == UnitTypes::Zerg_Overlord)
			continue;
		for (auto& armyU : army.second->getUnits())
		{
			army.second->microUpdate(armyU, unitAttackPath, unitRetreatPath, unitRetreatInfo,
				enemyAssign, isRetreat, attackPosition, AirdropTac, firstGroundUnit, 0, set<Unit>(), encounteredEnemy, kiteAssign);
			if (armyU.enemyTarget != nullptr)
			{
				hasTargetToAttack = true;
			}
		}
	}

	if (!isRetreat)
	{
		if (!attackPositionHasEnemy()) {
			logInfo("AirdropTactic", "start retreat, no enemy");
			setRetreat();
			return;
		}
	}
	else {
		if (BWEMMap.GetArea(TilePosition(centerPosition)) == BWEMMap.GetArea(TilePosition(attackPosition))
			|| centerPosition.getDistance(attackPosition) < 10 * 32)
		{
			state = END;
			//logInfo("Airdrop", "ID: " + objectID + " finish task, end drop tactic! using " + toString((Broodwar->getFrameCount() - timestamp) / 24) + " seconds", "BIG_ERROR_Airdrop");
			logInfo("AirdropTactic", "retreat back to base, tactic end");
			return;
		}
	}
}

void AirdropTactic::loaderMove(Unit loader, Position destination, bool plotPath) {
	Position nextMove = Astar::Instance().getAirPath(loader, destination, true);
	BattleArmy::smartMove(loader, nextMove);
	/*
	while (overLordStoredPath.size() > 1 && overLordStoredPath.front().getDistance(loader->getTilePosition()) < 3)
	{
		overLordStoredPath.pop_front();
	}
	if (plotPath)
	{
		for (auto p = overLordStoredPath.begin(); p != overLordStoredPath.end(); p++)
		{
			auto nx = std::next(p, 1);
			if (nx == overLordStoredPath.end())
			{
				continue;
			}
			Broodwar->drawLineMap(Position(*p), Position(*nx), Colors::Red);
		}
	}
	//BattleArmy::smartMove(loader, Position(overLordStoredPath.front()));
	loader->move(Position(overLordStoredPath.front()));
	*/
}

void AirdropTactic::update()
{
	//Broodwar->printf("%d %d", overlords.size(), airdropArmy.size());
	//for (auto const& u : overlords) {
	//	Broodwar->drawCircleMap(u->getPosition(), 20, BWAPI::Colors::Red);
	//}
	//for (auto const& u : airdropArmy) {
	//	Broodwar->drawCircleMap(u->getPosition(), 20, BWAPI::Colors::Blue);
	//}
	//for (auto const& army : tacticArmy) {
	//	for (auto const& u : army.second->getUnits()) {
	//		Broodwar->drawCircleMap(u.unit->getPosition(), 15, BWAPI::Colors::Green);
	//	}
	//}
	if (airdropArmy.size() == 0) {
		state = END;
		//logInfo("Airdrop", "ID: " + objectID + " all airdrop army is killed, end drop tactic! using " + toString((Broodwar->getFrameCount() - timestamp) / 24) + " seconds", "BIG_ERROR_Airdrop");
		logInfo("AirDropTactic", "has no army, tactic end");
		return;
	}
	if ((airDropstate == AirdropTactic::loading || airDropstate == AirdropTactic::moving) && overlords.size() == 0) {
		state = END;
		//logInfo("Airdrop", "ID: " + objectID + " all airdrop overlord is killed, end drop tactic! using " + toString((Broodwar->getFrameCount() - timestamp) / 24) + " seconds", "BIG_ERROR_Airdrop");
		logInfo("AirDropTactic", "has no army, tactic end");
		return;
	}
	switch (airDropstate)
	{
	case AirdropTactic::begin: {
		airDropstate = AirdropTactic::loading;
		std::string armyType = "";
		if (airdropArmy[0]->getType() == UnitTypes::Zerg_Zergling)
			armyType = "Zergling";
		else if (airdropArmy[0]->getType() == UnitTypes::Zerg_Lurker)
			armyType = "Lurker";
		else if (airdropArmy[0]->getType() == UnitTypes::Zerg_Hydralisk)
			armyType = "Hydralisk";
		//logInfo("Airdrop", "ID: " + objectID + " start air drop tactic! start loading, using type " + armyType, "BIG_ERROR_Airdrop");
		timestamp = Broodwar->getFrameCount();
	}
		break;
	case AirdropTactic::loading: {
		Unit loader = overlords[0];
		bool allUnitLoaded = true;
		for (auto const& u : airdropArmy) {
			if (!u->isLoaded()) {
				if (u->getPosition().getDistance(loader->getPosition()) > 7 * 32) {
					if (alreadyMoveFlag.find(u) == alreadyMoveFlag.end()) {
						alreadyMoveFlag.insert(u);
						u->move(Position(Broodwar->self()->getStartLocation()));
					}
				}
				else {
					if (alreadyStopFlag.find(u) == alreadyStopFlag.end()) {
						alreadyStopFlag.insert(u);
						u->stop();
					}
				}
				loader->load(u);
				allUnitLoaded = false;
			}
		}
		if (allUnitLoaded) {
			//logInfo("Airdrop", "ID: " + objectID + " finish loading, using " + toString((Broodwar->getFrameCount() - timestamp) / 24) + " seconds", "BIG_ERROR_Airdrop");
			timestamp = Broodwar->getFrameCount();
			airDropstate = AirdropTactic::moving;
			overlordDestination = attackPosition;
			//updateAstar();
			//Broodwar->printf("start moving");
		}
	}
		break;
	case AirdropTactic::moving: {
		Unit loader = overlords[0];
		bool needRetreat = (100 * loader->getHitPoints() / loader->getType().maxHitPoints() < 50);
		bool canUnload = (BWEMMap.GetArea(TilePosition(loader->getPosition())) == BWEMMap.GetArea(TilePosition(attackPosition)));
		if ((needRetreat || loader->isUnderAttack()) && canUnload) {
			// unload as soon as possible
			//logInfo("Airdrop", "ID: " + objectID + " start unloading, using " + toString((Broodwar->getFrameCount() - timestamp) / 24) + " seconds", "BIG_ERROR_Airdrop");
			timestamp = Broodwar->getFrameCount();
			airDropstate = AirdropTactic::unloading;
			//Broodwar->printf("start unloading");
		}
		else if (needRetreat && !canUnload) {
			// retreat
			if (!outputRetreat) {
				//logInfo("Airdrop", "ID: " + objectID + " overlord need retreat", "BIG_ERROR_Airdrop");
				outputRetreat = true;
			}
			attackPosition = Position(Broodwar->self()->getStartLocation());
			Position nextMove = Astar::Instance().getAirPath(loader, Position(Broodwar->self()->getStartLocation()));
			loader->move(nextMove);
			if (BWEMMap.GetArea(TilePosition(overlords[0]->getPosition())) == BWEMMap.GetArea(Broodwar->self()->getStartLocation())) {
				//logInfo("Airdrop", "ID: " + objectID + " retreat to base, start unloading, using " + toString((Broodwar->getFrameCount() - timestamp) / 24) + " seconds", "BIG_ERROR_Airdrop");
				timestamp = Broodwar->getFrameCount();
				airDropstate = AirdropTactic::unloading;
			}
		}
		else {
			// enough hp, drop as close as possible
			if (loader->getPosition().getDistance(attackPosition) > 6 * 32)
				loaderMove(loader, attackPosition);
			else {
				//logInfo("Airdrop", "ID: " + objectID + " start unloading, using " + toString((Broodwar->getFrameCount() - timestamp) / 24) + " seconds", "BIG_ERROR_Airdrop");
				timestamp = Broodwar->getFrameCount();
				airDropstate = AirdropTactic::unloading;
				//Broodwar->printf("start unloading");
			}
		}
	}
		break;
	case AirdropTactic::unloading: {
		if (overlords.size() > 0) {
			Unit loader = overlords[0];
			bool allunloaded = true;
			for (auto const& u : airdropArmy) {
				if (u->isLoaded()) {
					loader->unload(u);
					allunloaded = false;
				}
			}
			if (allunloaded) {
				//logInfo("Airdrop", "ID: " + objectID + " finish unloading, using " + toString((Broodwar->getFrameCount() - timestamp) / 24) + " seconds", "BIG_ERROR_Airdrop");
				timestamp = Broodwar->getFrameCount();
				airDropstate = AirdropTactic::attacking;
				overlordDestination = Position(Broodwar->self()->getStartLocation());
				//updateAstar();
				//Broodwar->printf("start attacking");
			}
		}
		else {
			//logInfo("Airdrop", "ID: " + objectID + " finish unloading(overlord destroyed), using " + toString((Broodwar->getFrameCount() - timestamp) / 24) + " seconds", "BIG_ERROR_Airdrop");
			timestamp = Broodwar->getFrameCount();
			// if overlord is destroyed, transfer to attacking immediately
			airDropstate = AirdropTactic::attacking;
			//Broodwar->printf("start attacking");
		}
	}
		break;
	case AirdropTactic::attacking: {
		if (overlords.size() > 0) {
			Unit loader = overlords[0];
			if (Broodwar->self()->getStartLocation().getDistance(loader->getTilePosition()) > 3) {
				loaderMove(loader, overlordDestination);
			}
		}
		airDropAttack();
	}
		break;
	default:
		break;
	}

}

void AirdropTactic::addArmyUnit(BWAPI::Unit unit) {
	if (unit->getType() == UnitTypes::Zerg_Overlord) {
		overlords.push_back(unit);
	}
	else {
		airdropArmy.push_back(unit);
	}
}

void AirdropTactic::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit == NULL || tacticArmy.find(unit->getType()) == tacticArmy.end())
		return;
	tacticArmy[unit->getType()]->removeUnit(unit);
	for (std::vector<Unit>::iterator it = overlords.begin(); it != overlords.end(); it++)
	{
		if (*it == unit) {
			overlords.erase(it);
			break;
		}
	}
	for (std::vector<Unit>::iterator it = airdropArmy.begin(); it != airdropArmy.end(); it++)
	{
		if (*it == unit) {
			airdropArmy.erase(it);
			break;
		}
	}
}

void AirdropTactic::updateAstar() {
	if (overlords.size() == 0)
		return;
	//Broodwar->printf("update astar!");
	overLordStoredPath.clear();
	overLordStoredPath = Astar::Instance().aStarPathFinding(overlords[0]->getTilePosition(), TilePosition(overlordDestination), true, false, false, true, 6, 3, -1);
}

void AirdropTactic::onUnitShow(BWAPI::Unit unit) {
	/*
	// the same frame do not update twice
	if (Broodwar->getFrameCount() == lastUpdateAstar)
		return;
	// if not enemy
	if (unit->getPlayer() == Broodwar->self())
		return;
	// if has not air weapon
	if (unit->getType().airWeapon() == WeaponTypes::None)
		return;
	// if overlord is destroied
	if (overlords.size() == 0)
		return;
	// if overlord has already arrive to destination
	if (overlords[0]->getPosition().getDistance(overlordDestination) < 5 * 32)
		return;
	// if too far away
	bool canAffectPath = false;
	for (auto const& p : overLordStoredPath) {
		if (p.getDistance(unit->getTilePosition()) < 10) {
			canAffectPath = true;
			break;
		}
	}
	if (!canAffectPath)
		return;
	lastUpdateAstar = Broodwar->getFrameCount();
	updateAstar();
	*/
}

