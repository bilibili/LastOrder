#include "AttackManager.h"



AttackManager::AttackManager()
{
	// do init first
	myArmy[BWAPI::UnitTypes::Zerg_Zergling] = new ZerglingArmy();
	myArmy[BWAPI::UnitTypes::Zerg_Mutalisk] = new MutaliskArmy();
	myArmy[BWAPI::UnitTypes::Zerg_Hydralisk] = new HydraliskArmy();
	myArmy[BWAPI::UnitTypes::Zerg_Overlord] = new OverLordArmy();
	myArmy[BWAPI::UnitTypes::Zerg_Lurker] = new LurkerArmy();
	myArmy[BWAPI::UnitTypes::Zerg_Scourge] = new ScourgeArmy();
	myArmy[BWAPI::UnitTypes::Zerg_Ultralisk] = new UltraliskArmy();
	myArmy[BWAPI::UnitTypes::Zerg_Devourer] = new DevourerArmy();
	myArmy[BWAPI::UnitTypes::Zerg_Guardian] = new GuardianArmy();

	isNeedDefend = false;
	unRallyArmy.reserve(1000);

	hasWorkerScouter = false;
	assignWorkerToScouter = false;
	nextScoutTime = 24 * 60 * 5;
	nextOverlordPatrolTime = 0;
	assignOverlord = false;
	enemyDisappearedTime = 0;
}


bool AttackManager::hasGroundUnit()
{
	for (auto& army : myArmy)
	{
		if (!army.first.isFlyer() && !army.second->getUnits().empty())
		{
			return true;
		}
	}
	return false;
}


bool AttackManager::hasAttackArmy()
{
	for (auto army : myArmy)
	{
		if (army.first != BWAPI::UnitTypes::Zerg_Overlord && army.second->getUnits().size() > 0)
		{
			return true;
		}
	}
	return false;
}


std::map<BWAPI::UnitType, int> AttackManager::reaminArmy()
{
	std::map<BWAPI::UnitType, int> result;
	for (auto army : myArmy)
	{
		if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
		{
			result[army.first] = army.second->getUnits().size();
		}
	}
	return result;
}


void AttackManager::groupArmy()
{
	std::map<const Area*, TilePosition>	& myRegions = InformationManager::Instance().getBaseOccupiedRegions(BWAPI::Broodwar->self());
	if (myRegions.find(BWEMMap.GetArea(InformationManager::Instance().getOurNatrualLocation())) != myRegions.end())
		rallyPosition = BWAPI::Position(InformationManager::Instance().getOurNatrualLocation());
	else
		rallyPosition = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());

	//battle unit rally to the natural choke center
	for (auto it = unRallyArmy.begin(); it != unRallyArmy.end();)
	{
		if ((*it)->canIssueCommand(BWAPI::UnitCommand(*it, BWAPI::UnitCommandTypes::Move, nullptr, rallyPosition.x, rallyPosition.y, 0)))
		{
			(*it)->move(rallyPosition);
			//may add lurker egg
			if (myArmy.find((*it)->getType()) != myArmy.end())
			{
				myArmy[(*it)->getType()]->addUnit(*it);
			}
			it = unRallyArmy.erase(it);
		}
		else
		{
			it++;
		}
	}

	int count = 0;
	bool isArmyUnderAttack = false;
	BWAPI::Position underAttackPosition;
	int natrualRallyGroundMax = 18;

	unitAttackPath.clear();
	unitRetreatPath.clear();
	enemyAssign.clear();
	unitRetreatInfo.clear();
	//group outside unit
	for (auto armyUnits : myArmy)
	{
		if (armyUnits.first == BWAPI::UnitTypes::Zerg_Overlord)
			continue;

		if (armyUnits.first == BWAPI::UnitTypes::Zerg_Zergling && hasWorkerScouter)
			continue;

		for (auto u : armyUnits.second->getUnits())
		{
			Position trueRallyPosition;
			if (myRegions.find(BWEMMap.GetArea(InformationManager::Instance().getOurNatrualLocation())) != myRegions.end())
			{
				if ((armyUnits.first.isFlyer() == false && count < natrualRallyGroundMax) || armyUnits.first.isFlyer())
				{
					trueRallyPosition = rallyPosition;
					count++;
				}
				else
				{
					trueRallyPosition = Position(BWAPI::Broodwar->self()->getStartLocation());
				}
			}
			else
			{
				trueRallyPosition = Position(BWAPI::Broodwar->self()->getStartLocation());
			}

			//armyUnits.second->microUpdate(u, unitAttackPath, unitRetreatPath, unitRetreatInfo, 
			//	enemyAssign, false, trueRallyPosition, GroupArmy);


			if (BWEMMap.GetArea(u.unit->getTilePosition()) == BWEMMap.GetArea(TilePosition(trueRallyPosition)))
			{
				BattleArmy::smartAttackMove(u.unit, trueRallyPosition);
			}
			else
			{
				Position nextP = Astar::Instance().getNextMovePosition(u.unit, trueRallyPosition);
				BattleArmy::smartAttackMove(u.unit, nextP);
			}
			
		}
	}

}


void AttackManager::overlordProtect()
{
	OverLordArmy* overlords = dynamic_cast<OverLordArmy*>(myArmy[BWAPI::UnitTypes::Zerg_Overlord]);
	map<Unit, int> sporeAssign;
	for (size_t i = 0; i < overlords->getUnits().size(); i++)
	{
		overlords->overlordUpdate(overlords->getUnits()[i], sporeAssign, i);
	}
}


// attack manger trigger for two situation:defend and attack
void AttackManager::update()
{
	groupArmy();
	DefendUpdate();
	ScoutUpdate();
	PatrolUpdate();
	overlordProtect();
}


void AttackManager::addArmyToTactic(tacticType type)
{
	if (TacticManager::Instance().isOneTacticRun(type))
	{
		Position attackPosition = TacticManager::Instance().getTacticPosition(type);
		switch (type)
		{
		case MutaliskHarassTac:
		{
			TacticManager::Instance().addTacticArmy(MutaliskHarassTac, attackPosition, myArmy, BWAPI::UnitTypes::Zerg_Mutalisk, myArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size());
			TacticManager::Instance().addTacticArmy(MutaliskHarassTac, attackPosition, myArmy, BWAPI::UnitTypes::Zerg_Scourge, myArmy[BWAPI::UnitTypes::Zerg_Scourge]->getUnits().size());
		}
		break;

		case HydraliskPushTactic:
		{
			for (auto army : myArmy)
			{
				if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
				{
					if (army.first == BWAPI::UnitTypes::Zerg_Zergling && hasWorkerScouter && army.second->getUnits().size() > 2)
						TacticManager::Instance().addTacticArmy(HydraliskPushTactic, attackPosition, myArmy, army.first, army.second->getUnits().size() - 2);
					else
						TacticManager::Instance().addTacticArmy(HydraliskPushTactic, attackPosition, myArmy, army.first, army.second->getUnits().size());
				}
			}

			//TacticManager::Instance().addTacticArmy(HydraliskPushTactic, attackPosition, myArmy, BWAPI::UnitTypes::Zerg_Overlord, 1);

			map<BWAPI::UnitType, int> tacArmy = TacticManager::Instance().getTacArmyForceDetail(type, attackPosition);
			if (InformationManager::Instance().isEnemyHasInvisibleUnit() && BWAPI::Broodwar->getFrameCount() > 5 * 24 * 60
				&& tacArmy[UnitTypes::Zerg_Overlord] < 2)
			{
				TacticManager::Instance().addTacticArmy(HydraliskPushTactic, attackPosition, myArmy, BWAPI::UnitTypes::Zerg_Overlord, 1);
			}
		}
		break;

		default:
		break;

		}
	}
	//else
	//{
	//	logInfo("AttackManager", "tactic is not running " + to_string(type), "BIG_ERROR_AttackManager");
	//}
}


void AttackManager::issueAttackCommand(tacticType type, BWAPI::Position attackPosition)
{	
	if (attackPosition == BWAPI::Positions::None)
	{
		logInfo("AttackManager", "attackPosition is invalid " + to_string(type), "BIG_ERROR_AttackManager");
		return;
	}
		
	triggerTactic(type, attackPosition);
}

/*
void AttackManager::ScoutUpdate()
{
	int curOverlordCount = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Overlord);
	std::vector<UnitState>& army = myArmy[BWAPI::UnitTypes::Zerg_Overlord]->getUnits();

	bool isTacticRunning = TacticManager::Instance().isOneTacticRun(ScoutTac);
	if (!isTacticRunning)
	{
		TacticManager::Instance().addTactic(ScoutTac, BWAPI::Positions::None);
		std::vector<UnitState>& army = myArmy[BWAPI::UnitTypes::Zerg_Overlord]->getUnits();
		for (auto it = army.begin(); it != army.end();)
		{
			TacticManager::Instance().addTacticUnit(ScoutTac, BWAPI::Positions::None, it->unit);
			it = army.erase(it);
		}
	}
	else
	{
		if (Broodwar->self()->allUnitCount(UnitTypes::Zerg_Drone) == 9 && InformationManager::Instance().GetEnemyBasePosition() == BWAPI::Positions::None &&
			!assignWorkerToScouter)
		{
			Unit scounter = WorkerManager::Instance().addScountWorker();
			TacticManager::Instance().addTacticUnit(ScoutTac, BWAPI::Positions::None, scounter);
			assignWorkerToScouter = true;
		}
		if (curOverlordCount - army.size() < 4)
		{
			for (auto it = army.begin(); it != army.end();)
			{
				//can generate safe attack path 
				TacticManager::Instance().addTacticUnit(ScoutTac, BWAPI::Positions::None, it->unit);
				it = army.erase(it);
			}
		}

		if (TacticManager::Instance().isAssignScoutZergling())
			return;

		int checkInterval = 0;
		if (BWAPI::Broodwar->getFrameCount() >= 4 * 24 * 60 && BWAPI::Broodwar->getFrameCount() < 7 * 24 * 60)
			checkInterval = 24 * 30;
		else if (BWAPI::Broodwar->getFrameCount() >= 7 * 24 * 60 && BWAPI::Broodwar->getFrameCount() < 11 * 24 * 60)
			checkInterval = 24 * 30;
		else
			checkInterval = 24 * 30;

		if (InformationManager::Instance().GetEnemyBasePosition() != Positions::None
			&& BWAPI::Broodwar->getFrameCount() > nextScoutTime)
		{
			nextScoutTime = BWAPI::Broodwar->getFrameCount() + checkInterval;

			int armyZerglingCount = myArmy[BWAPI::UnitTypes::Zerg_Zergling]->getUnits().size();
			if (armyZerglingCount == 0)
			{
				if (Broodwar->self()->allUnitCount(UnitTypes::Zerg_Drone) <= 15)
				{
					return;
				}
				//assign worker to scout
				Unit scounter = WorkerManager::Instance().addScountWorker();
				if (scounter != NULL)
				{
					TacticManager::Instance().addTacticUnit(ScoutTac, BWAPI::Positions::None, scounter);
				}
			}
			else
			{
				std::vector<UnitState>& army = myArmy[BWAPI::UnitTypes::Zerg_Zergling]->getUnits();
				for (auto it = army.begin(); it != army.end();)
				{
					TacticManager::Instance().addTacticUnit(ScoutTac, BWAPI::Positions::None, it->unit);
					it = army.erase(it);
					break;
				}
			}
		}
	}
}
*/


void AttackManager::ScoutUpdate()
{
	int curOverlordCount = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Overlord);
	std::vector<UnitState>& army = myArmy[BWAPI::UnitTypes::Zerg_Overlord]->getUnits();

	bool isTacticRunning = TacticManager::Instance().isOneTacticRun(ScoutTac);
	if (!isTacticRunning)
	{
		//assign scout task to the 9th drone
		if (Broodwar->self()->allUnitCount(UnitTypes::Zerg_Drone) >= 5) {
			TacticManager::Instance().addTactic(ScoutTac, BWAPI::Positions::None);

			Unit scounter = WorkerManager::Instance().addScountWorker();
			if (scounter != NULL) {
				TacticManager::Instance().addTacticUnit(ScoutTac, BWAPI::Positions::None, scounter);
				assignWorkerToScouter = true;
			}
		}
	}
	else
	{
		/*
		if (Broodwar->self()->allUnitCount(UnitTypes::Zerg_Drone) == 9 && InformationManager::Instance().GetEnemyBasePosition() == BWAPI::Positions::None &&
			!assignWorkerToScouter)
		{
			Unit scounter = WorkerManager::Instance().addScountWorker();
			TacticManager::Instance().addTacticUnit(ScoutTac, BWAPI::Positions::None, scounter);
			assignWorkerToScouter = true;
		}
		if (curOverlordCount - army.size() < 4)
		{
			for (auto it = army.begin(); it != army.end();)
			{
				//can generate safe attack path 
				TacticManager::Instance().addTacticUnit(ScoutTac, BWAPI::Positions::None, it->unit);
				it = army.erase(it);
			}
		}
		*/

		if (assignOverlord == false && 
			InformationManager::Instance().GetEnemyBasePosition() != Positions::None &&
			Broodwar->enemy()->getRace() != Races::Terran &&
			army.size() > 0)
		{
			Position enemyBase = InformationManager::Instance().GetEnemyBasePosition();
			double totalDistance = Position(Broodwar->self()->getStartLocation()).getDistance(enemyBase);
			double flyFrame = totalDistance / UnitTypes::Zerg_Overlord.topSpeed();
			int desiredReachTime = 24 * 60 * 8;
			int startScoutTime = desiredReachTime - int(flyFrame);
			if (Broodwar->getFrameCount() > startScoutTime)
			{
				logInfo("AttackManager", "send overlord to enemy base ");
				TacticManager::Instance().addTacticUnit(ScoutTac, BWAPI::Positions::None, army.back().unit);
				army.pop_back();
				assignOverlord = true;
			}
		}

		int checkInterval = 0;
		if (BWAPI::Broodwar->getFrameCount() >= 4 * 24 * 60 && BWAPI::Broodwar->getFrameCount() < 7 * 24 * 60)
			checkInterval = 24 * 30;
		else if (BWAPI::Broodwar->getFrameCount() >= 7 * 24 * 60 && BWAPI::Broodwar->getFrameCount() < 11 * 24 * 60)
			checkInterval = 24 * 30;
		else
			checkInterval = 24 * 30;
		if (TacticManager::Instance().isAssignScoutZergling())
		{
			nextScoutTime = BWAPI::Broodwar->getFrameCount() + checkInterval;
			return;
		}

		if (InformationManager::Instance().GetEnemyBasePosition() != Positions::None
			&& BWAPI::Broodwar->getFrameCount() > nextScoutTime)
		{
			nextScoutTime = BWAPI::Broodwar->getFrameCount() + checkInterval;

			int armyZerglingCount = myArmy[BWAPI::UnitTypes::Zerg_Zergling]->getUnits().size();
			if (armyZerglingCount == 0)
			{
				if (Broodwar->self()->allUnitCount(UnitTypes::Zerg_Drone) <= 15)
				{
					return;
				}
				//assign worker to scout
				Unit scounter = WorkerManager::Instance().addScountWorker();
				if (scounter != NULL)
				{
					TacticManager::Instance().addTacticUnit(ScoutTac, BWAPI::Positions::None, scounter);
				}
			}
			else
			{
				std::vector<UnitState>& army = myArmy[BWAPI::UnitTypes::Zerg_Zergling]->getUnits();
				for (auto it = army.begin(); it != army.end();)
				{
					TacticManager::Instance().addTacticUnit(ScoutTac, BWAPI::Positions::None, it->unit);
					it = army.erase(it);
					break;
				}
			}
		}
	}
}


void AttackManager::PatrolUpdate() {
	std::vector<UnitState>& army = myArmy[BWAPI::UnitTypes::Zerg_Overlord]->getUnits();
	
	bool isTacticRunning = TacticManager::Instance().isOneTacticRun(PatrolTac);
	if (!isTacticRunning) {
		TacticManager::Instance().addTactic(PatrolTac, BWAPI::Positions::None);
		std::vector<UnitState>& army = myArmy[BWAPI::UnitTypes::Zerg_Overlord]->getUnits();
		for (auto it = army.begin(); it != army.end();)
		{
			TacticManager::Instance().addTacticUnit(PatrolTac, BWAPI::Positions::None, it->unit);
			it = army.erase(it);
			break;
		}
	}
	else {
		if (TacticManager::Instance().isAssignPatrolOverlord()) {
			int checkInterval = 24 * 60 * 1;
			nextOverlordPatrolTime = Broodwar->getFrameCount() + checkInterval;
			return;
		}
		if (Broodwar->getFrameCount() > nextOverlordPatrolTime) {
			// not enough overlord(less than the total number of overlord required plus 3)
			if (int(army.size()) <= (Broodwar->self()->supplyUsed() + 15) / 16 + 2)
				return;
			int assignOverlord = 1;

			//// if we have too many overlord, assign two overlord to patrol
			//if (army.size() >= (Broodwar->self()->supplyUsed() + 15) / 16 + 7)
			//	assignOverlord = 2;
			//// if we have too too many overlord, assign two overlord to patrol
			//if (army.size() >= (Broodwar->self()->supplyUsed() + 15) / 16 + 10)
			//	assignOverlord = 3;
			std::vector<UnitState>& army = myArmy[BWAPI::UnitTypes::Zerg_Overlord]->getUnits();
			for (auto it = army.begin(); it != army.end();)
			{
				TacticManager::Instance().addTacticUnit(PatrolTac, BWAPI::Positions::None, it->unit);
				it = army.erase(it);
				assignOverlord--;
				if (assignOverlord == 0)
					break;
			}
		}
	}
}


void AttackManager::triggerTactic(tacticType tacType, BWAPI::Position attackPosition)
{
	bool isTacticRunning;

	switch (tacType)
	{
		
	case MutaliskHarassTac:
	{
		isTacticRunning = TacticManager::Instance().isOneTacticRun(MutaliskHarassTac);
		if (!isTacticRunning)
		{
			if (tacticTrigCondition(MutaliskHarassTac, attackPosition))
			{
				TacticManager::Instance().addTactic(MutaliskHarassTac, attackPosition);

				TacticManager::Instance().addTacticArmy(MutaliskHarassTac, attackPosition, myArmy, BWAPI::UnitTypes::Zerg_Mutalisk, myArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size());
				TacticManager::Instance().addTacticArmy(MutaliskHarassTac, attackPosition, myArmy, BWAPI::UnitTypes::Zerg_Scourge, myArmy[BWAPI::UnitTypes::Zerg_Scourge]->getUnits().size());
			}
		}
	}
		break;
		
	case HydraliskPushTactic:
	{
		// trigger hydrisk attack
		isTacticRunning = TacticManager::Instance().isOneTacticRun(HydraliskPushTactic);
		if (!isTacticRunning)
		{
			if (tacticTrigCondition(HydraliskPushTactic, attackPosition))
			{
				TacticManager::Instance().addTactic(HydraliskPushTactic, attackPosition);
				for (auto army : myArmy)
				{
					if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
					{
						if (army.first == BWAPI::UnitTypes::Zerg_Zergling && hasWorkerScouter && army.second->getUnits().size() > 2)
							TacticManager::Instance().addTacticArmy(HydraliskPushTactic, attackPosition, myArmy, army.first, army.second->getUnits().size() - 2);
						else
							TacticManager::Instance().addTacticArmy(HydraliskPushTactic, attackPosition, myArmy, army.first, army.second->getUnits().size());
					}
				}

				if (InformationManager::Instance().isEnemyHasInvisibleUnit() && BWAPI::Broodwar->getFrameCount() > 5 * 24 * 60)
				{
					TacticManager::Instance().addTacticArmy(HydraliskPushTactic, attackPosition, myArmy, BWAPI::UnitTypes::Zerg_Overlord, 2);
				}
			}
		}
	}
		break;

	case AirdropTac: {
		if (tacticTrigCondition(AirdropTac, attackPosition))
		{
			TacticManager::Instance().addTactic(AirdropTac, attackPosition);
			std::pair<UnitType, int> armyInfo = availableAirDropArmy();
			//TacticManager::Instance().addTacticArmy(AirdropTac, attackPosition, myArmy, UnitTypes::Zerg_Overlord, 1);
			std::vector<UnitState>& army = myArmy[BWAPI::UnitTypes::Zerg_Overlord]->getUnits();
			bool foundFullHPOverlord = false;
			int maxHP = -1;
			auto targetIT = army.begin();
			for (auto it = army.begin(); it != army.end();)
			{
				if ((*it).unit->getHitPoints() > maxHP) {
					maxHP = (*it).unit->getHitPoints();
					targetIT = it;
				}
				if ((*it).unit->getHitPoints() == (*it).unit->getType().maxHitPoints()) {
					foundFullHPOverlord = true;
					TacticManager::Instance().addTacticUnit(AirdropTac, attackPosition, it->unit);
					it = army.erase(it);
					break;
				}
				else {
					it++;
				}
			}
			if (!foundFullHPOverlord) {
				TacticManager::Instance().addTacticUnit(AirdropTac, attackPosition, targetIT->unit);
				army.erase(targetIT);
			}
			TacticManager::Instance().addTacticArmy(AirdropTac, attackPosition, myArmy, armyInfo.first, armyInfo.second);
			//std::vector<UnitState>& army_overlord = TacticManager::Instance().getTacArmy(AirdropTac, attackPosition)[UnitTypes::Zerg_Overlord]->getUnits();
			//for (auto it = army_overlord.begin(); it != army_overlord.end(); it++)
			//	TacticManager::Instance().addTacticUnit(AirdropTac, attackPosition, it->unit);
			std::vector<UnitState>& army_airdrop = TacticManager::Instance().getTacArmy(AirdropTac, attackPosition)[armyInfo.first]->getUnits();
			for (auto it = army_airdrop.begin(); it != army_airdrop.end(); it++)
				TacticManager::Instance().addTacticUnit(AirdropTac, attackPosition, it->unit);
		}
	}
		break;

	default:
	{
		logInfo("AttackManager", "tactype is invalid " + to_string(tacType), "BIG_ERROR_AttackManager");
	}
		break;
	}
}


bool AttackManager::tacticTrigCondition(int tac, BWAPI::Position attackPosition)
{
	if (tac != int(AirdropTac) && TacticManager::Instance().isTacticRun(tacticType(tac), attackPosition))
		return false;

	if (tac == int(MutaliskHarassTac))
	{
		return myArmy[BWAPI::UnitTypes::Zerg_Mutalisk]->getUnits().size() > 0;
	}
	else if (tac == int(HydraliskPushTactic))
	{
		return hasGroundUnit();
	}
	else if (tac == int(AirdropTac)) {
		return availableAirDropArmy().first != UnitTypes::None;
	}
	return false;
}


void AttackManager::addTacArmy(int needArmySupply, tacticType tacType, BWAPI::Position attackPosition, bool allAirEnemy, bool allGroundEnemy)
{
	vector<std::pair<UnitState, double>> unitDistance;
	unitDistance.reserve(1000);
	for (auto& army : myArmy)
	{
		if (army.first != BWAPI::UnitTypes::Zerg_Overlord)
		{
			if (allAirEnemy == true && army.first.airWeapon() == WeaponTypes::None)
			{
				continue;
			}
			if (allGroundEnemy == true && army.first.groundWeapon() == WeaponTypes::None)
			{
				continue;
			}
			for (auto u : army.second->getUnits())
			{
				unitDistance.push_back(std::pair<UnitState, double>(u, u.unit->getDistance(attackPosition)));
			}
			army.second->getUnits().clear();
		}
	}
	std::sort(unitDistance.begin(), unitDistance.end(), 
		[](const std::pair<UnitState, double>& a, const std::pair<UnitState, double>& b)
	{
		return a.second < b.second;
	});

	for (auto army : unitDistance)
	{
		if (needArmySupply > 0)
		{
			TacticManager::Instance().addTacticUnit(DefendTactic, attackPosition, army.first.unit);
			needArmySupply -= army.first.unit->getType().supplyRequired();
		}
		else
		{
			if (myArmy.find(army.first.unit->getType()) != myArmy.end())
				myArmy[army.first.unit->getType()]->addUnit(army.first.unit);
			else
				logInfo("AttackManager", "addTacArmy " + to_string(army.first.unit->getType()) + " not exist!", "ERROR");
		}
	}
}



void AttackManager::DefendUpdate()
{
	//each base corresponding to defend enemy
	std::map<const Area*, std::set<BWAPI::Unit>>& enemyUnitsTargetRegion = \
		InformationManager::Instance().getDefendEnemyInfo();
	std::map<const Area*, TilePosition>& myRegion = InformationManager::Instance().getBaseOccupiedRegions(BWAPI::Broodwar->self());

	std::map<const Area*, int> enemyUnitsInRegionSupply;
	std::map<const Area*, int> enemyUnitsInRegionFlySupply;
	//get grouped enemy supply info
	for (auto baseEnemy : enemyUnitsTargetRegion)
	{
		for (auto enemyUnit : baseEnemy.second)
		{
			if (!enemyUnit->getType().isBuilding())
			{
				if (enemyUnit->getType().isFlyer())
					enemyUnitsInRegionFlySupply[baseEnemy.first] += enemyUnit->getType().supplyRequired();
				enemyUnitsInRegionSupply[baseEnemy.first] += enemyUnit->getType().supplyRequired();
			}
			else
			{
				if (enemyUnit->getType().canAttack())
				{
					enemyUnitsInRegionSupply[baseEnemy.first] += 8;
				}
				else if (enemyUnit->getType() == BWAPI::UnitTypes::Terran_Bunker)
				{
					enemyUnitsInRegionSupply[baseEnemy.first] += 12;
				}
				else
					enemyUnitsInRegionSupply[baseEnemy.first] += 1;
			}
		}
	}

	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& myBuildings = InformationManager::Instance().getOurAllBuildingUnit();
	bool hasSunken = false;
	for(auto const& sunker : myBuildings[BWAPI::UnitTypes::Zerg_Sunken_Colony])
	{
		if (sunker->isCompleted())
		{
			hasSunken = true;
			break;
		}
	}

	if (enemyUnitsTargetRegion.empty())
	{
		enemyDisappearedTime += 1;
		if (enemyDisappearedTime > 10 * 24)
		{
			isNeedDefend = false;
			hasWorkerScouter = false;
			WorkerManager::Instance().finishedWithCombatWorkers();
			enemyDisappearedTime = 0;
		}
		return;
	}
	enemyDisappearedTime = 0;
	
	//special case for scouter
	if (enemyUnitsTargetRegion.size() == 1 && enemyUnitsTargetRegion.begin()->second.size() == 1 &&
		(*enemyUnitsTargetRegion.begin()->second.begin())->getType().isWorker() && myArmy[BWAPI::UnitTypes::Zerg_Zergling]->getUnits().empty())
	{
		hasWorkerScouter = true;
		BWAPI::Unit enemyWorker = *enemyUnitsTargetRegion.begin()->second.begin();
		BWAPI::Unit workerDefender = WorkerManager::Instance().getClosestMineralWorkerTo(enemyWorker);
		WorkerManager::Instance().setCombatWorker(workerDefender);
	}
	//use zergling to kill scout worker
	else if (enemyUnitsTargetRegion.size() == 1 && enemyUnitsTargetRegion.begin()->second.size() == 1 &&
		(*enemyUnitsTargetRegion.begin()->second.begin())->getType().isWorker() && myArmy[BWAPI::UnitTypes::Zerg_Zergling]->getUnits().size() > 0)
	{
		hasWorkerScouter = true;
		WorkerManager::Instance().finishedWithCombatWorkers();
		ZerglingArmy* zerglings = dynamic_cast<ZerglingArmy*>(myArmy[BWAPI::UnitTypes::Zerg_Zergling]);
		zerglings->attackScoutWorker(*enemyUnitsTargetRegion.begin()->second.begin());
	}
	else
	{
		//trig defend tactic
		vector<std::pair<const Area*, double>> enemyAttackRegionsPriority;
		BWAPI::Position myBase = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
		for (auto const& r : enemyUnitsTargetRegion)
		{
			Position enemyTarget = Position(myRegion[r.first]);
			enemyAttackRegionsPriority.push_back(std::pair<const Area*, double>(r.first, myBase.getDistance(enemyTarget)));
		}
		std::sort(enemyAttackRegionsPriority.begin(), enemyAttackRegionsPriority.end(),
			[](const std::pair<const Area*, double>& a, const std::pair<const Area*, double>& b)
		{
			return a.second < b.second;
		});

		// defend priority is defined by attack region's distance to start base, we traverse to assign defend army from high priority to low 
		for (auto const& targetBase : enemyAttackRegionsPriority)
		{
			int myRemainSupply = 0;
			map<UnitType, set<Unit>> ourUnits, enemyUnits;
			for(auto const& p : myArmy)
			{
				myRemainSupply += ((p.first).supplyRequired() * p.second->getUnits().size());
				for (auto& u : p.second->getUnits())
				{
					ourUnits[p.first].insert(u.unit);
				}
			}

			if (myRemainSupply == 0)
			{
				return;
			}

			bool enemyHasDetector = false;
			for (auto const& unit : enemyUnitsTargetRegion[targetBase.first])
			{
				enemyUnits[unit->getType()].insert(unit);
				if (checkDetectorFunc(unit->getType()))
				{
					enemyHasDetector = true;
				}
			}

			int enemySupply = enemyUnitsInRegionSupply[targetBase.first];
			std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& myBuildings = InformationManager::Instance().getOurAllBuildingUnit();

			bool allAirEnemy = false;
			bool allGroundEnemy = false;
			if (enemyUnitsInRegionFlySupply[targetBase.first] > 0)
			{
				int needAirDefend = enemyUnitsInRegionFlySupply[targetBase.first];
				if (enemySupply > 0 && (needAirDefend * 10 / enemySupply >= 9))
				{
					allAirEnemy = true;
				}
			}
			else
			{
				allGroundEnemy = true;
			}

			map<string, std::pair<int, int>> ourArmyScoreDict, enemyArmyScoreDict;
			BattleArmy::parsedArmyScore(ourUnits, ourArmyScoreDict, enemyHasDetector, true);
			BattleArmy::parsedArmyScore(enemyUnits, enemyArmyScoreDict, true, false);
			set<string> overWhelmEnemyArmy;
			int retreatFlag = BattleArmy::needRetreat(ourArmyScoreDict, enemyArmyScoreDict, overWhelmEnemyArmy, true, enemyHasDetector);
			bool isDefendStartBase = false;
			if (targetBase.first == BWEMMap.GetArea(Broodwar->self()->getStartLocation())
				|| targetBase.first == BWEMMap.GetArea(InformationManager::Instance().getOurNatrualLocation()))
			{
				isDefendStartBase = true;
			}

			int needDefendSupply = int(enemySupply * 2);
			Position defendBasePosition = Position(myRegion[targetBase.first]);

			if (TacticManager::Instance().isTacticRun(DefendTactic, defendBasePosition))
			{
				int defendTacArmySupply = TacticManager::Instance().getTacArmyForce(DefendTactic, defendBasePosition);
				if (defendTacArmySupply < needDefendSupply)
				{
					int needSupply = needDefendSupply - defendTacArmySupply;
					addTacArmy(needSupply, DefendTactic, defendBasePosition, allAirEnemy, allGroundEnemy);
				}
			}
			else
			{
				if (retreatFlag == 1 && overWhelmEnemyArmy.empty() && !isDefendStartBase)
				{
					return;
				}
				else
				{
					TacticManager::Instance().addTactic(DefendTactic, defendBasePosition);
					addTacArmy(enemySupply, DefendTactic, defendBasePosition, allAirEnemy, allGroundEnemy);
					//Broodwar->printf("trigger tactic");
				}
			}
		}
	}
}


void AttackManager::onUnitMorph(BWAPI::Unit unit)
{
	if (unit == nullptr)
	{
		return;
	}

	if (myArmy.find(unit->getType()) == myArmy.end())
	{
		BWAPI::UnitType u = unit->getType();
		return;
	}

	unRallyArmy.push_back(unit);
}

void AttackManager::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit == nullptr || myArmy.find(unit->getType()) == myArmy.end())
		return;

	std::vector<UnitState>& army = myArmy[unit->getType()]->getUnits();

	for (auto it = army.begin(); it != army.end(); it++)
	{
		if (it->unit == unit)
		{
			army.erase(it);
			break;
		}
	}

	for (std::vector<BWAPI::Unit>::iterator it = unRallyArmy.begin(); it != unRallyArmy.end(); it++)
	{
		if (*it == unit)
		{
			unRallyArmy.erase(it);
			break;
		}
	}
}

void AttackManager::onLurkerMorph()
{
	std::vector<UnitState>& army = myArmy[BWAPI::UnitTypes::Zerg_Hydralisk]->getUnits();

	for (std::vector<UnitState>::iterator it = army.begin(); it != army.end(); it++)
	{
		if (it->unit->isMorphing())
		{
			army.erase(it);
			break;
		}
	}
}

AttackManager& AttackManager::Instance()
{
	static AttackManager a;
	return a;
}


void AttackManager::addTacticRemainArmy(std::map<BWAPI::UnitType, BattleArmy*>& tacticArmy)
{
	for(auto const& p : tacticArmy)
	{
		if (myArmy.find(p.first) != myArmy.end())
		{
			std::vector<UnitState>& t = myArmy[p.first]->getUnits();
			t.insert(t.end(), p.second->getUnits().begin(), p.second->getUnits().end());
		}
	}
}


std::pair<UnitType,int> AttackManager::availableAirDropArmy() {
	// if two upgrade level are not satisfied 
	if (Broodwar->self()->getUpgradeLevel(UpgradeTypes::Pneumatized_Carapace) == 0 || Broodwar->self()->getUpgradeLevel(UpgradeTypes::Ventral_Sacs) == 0)
		return std::make_pair(UnitTypes::None, -1);
	// if do not have enough overlord
	if (myArmy[UnitTypes::Zerg_Overlord]->getUnits().size() < 5)
		return std::make_pair(UnitTypes::None, -1);
	// if upgraded Adrenal_Glands, use zergling first
	if (Broodwar->self()->getUpgradeLevel(UpgradeTypes::Adrenal_Glands) == 0) {
		// if we have more than 2 free Lurkers
		if (myArmy.find(UnitTypes::Zerg_Lurker) != myArmy.end() && myArmy[UnitTypes::Zerg_Lurker]->getUnits().size() >= 2)
			return std::make_pair(UnitTypes::Zerg_Lurker, 2);
		// if we have enough zerglings
		if (myArmy.find(UnitTypes::Zerg_Zergling) != myArmy.end() && myArmy[UnitTypes::Zerg_Zergling]->getUnits().size() > 0) {
			int minZerglingNum = (hasWorkerScouter ? 10 : 8);
			if (int(myArmy[UnitTypes::Zerg_Zergling]->getUnits().size()) >= minZerglingNum)
				return std::make_pair(UnitTypes::Zerg_Zergling, 8);
		}
	}
	else {
		// if we have enough zerglings
		if (myArmy.find(UnitTypes::Zerg_Zergling) != myArmy.end() && myArmy[UnitTypes::Zerg_Zergling]->getUnits().size() > 0) {
			int minZerglingNum = (hasWorkerScouter ? 10 : 8);
			if (int(myArmy[UnitTypes::Zerg_Zergling]->getUnits().size()) >= minZerglingNum)
				return std::make_pair(UnitTypes::Zerg_Zergling, 8);
		}
		// if we have more than 2 free Lurkers
		if (myArmy.find(UnitTypes::Zerg_Lurker) != myArmy.end() && myArmy[UnitTypes::Zerg_Lurker]->getUnits().size() >= 2)
			return std::make_pair(UnitTypes::Zerg_Lurker, 2);
	}
	// if we have enough hydralisks
	if (myArmy.find(UnitTypes::Zerg_Hydralisk) != myArmy.end() && myArmy[UnitTypes::Zerg_Hydralisk]->getUnits().size() >= 4)
		return std::make_pair(UnitTypes::Zerg_Hydralisk, 4);
	return std::make_pair(UnitTypes::None, -1);
}
