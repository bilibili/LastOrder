#include "testMain.h"

bool isInit = false;
vector<int> actions;
int nextBuildTime = 0;


void testMain(string mapFileName)
{
	if (mapFileName == "(0)test_scout.scm" || mapFileName == "(0)test.scx" || mapFileName == "(0)test_scout2.scm" || mapFileName.find("aiide")!=-1)
	{
		TimerManager::Instance().startTimer(TimerManager::All);

		TimerManager::Instance().startTimer(TimerManager::Worker);
		WorkerManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::Worker);

		TimerManager::Instance().startTimer(TimerManager::Information);
		InformationManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::Information);

		TimerManager::Instance().startTimer(TimerManager::Production);
		ProductionManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::Production);

		TimerManager::Instance().startTimer(TimerManager::Building);
		BuildingManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::Building);

		TimerManager::Instance().startTimer(TimerManager::Attack);
		AttackManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::Attack);

		TimerManager::Instance().startTimer(TimerManager::tactic);
		TacticManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::tactic);

		TimerManager::Instance().startTimer(TimerManager::strategy);
		StrategyManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::strategy);

		TimerManager::Instance().stopTimer(TimerManager::All);
	}

	else if (mapFileName.find("test_attack")!=-1) {
		
		//Broodwar->drawBoxMap(Position(47 * 32, 53 * 32), Position(47 * 32 + 32, 53 * 32 + 32), BWAPI::Colors::Red);
		
		for (auto const& startLocation : BWAPI::Broodwar->getStartLocations()) {
			if (startLocation != Broodwar->self()->getStartLocation()) {
				InformationManager::Instance().setLocationEnemyBase(startLocation);
			}
		}
		BWAPI::Position enemyBase = InformationManager::Instance().GetEnemyBasePosition();

		TimerManager::Instance().startTimer(TimerManager::All);
		TimerManager::Instance().startTimer(TimerManager::Information);
		InformationManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::Information);


		TimerManager::Instance().startTimer(TimerManager::Attack);
		AttackManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::Attack);

		TimerManager::Instance().startTimer(TimerManager::tactic);
		TacticManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::tactic);

		TimerManager::Instance().startTimer(TimerManager::strategy);
		StrategyManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::strategy);
		TimerManager::Instance().stopTimer(TimerManager::All);

		if (!TacticManager::Instance().isOneTacticRun(MutaliskHarassTac))
		{
			//AttackManager::Instance().issueAttackCommand(MutaliskHarassTac, Position(47 * 32, 53 * 32));
			AttackManager::Instance().issueAttackCommand(MutaliskHarassTac, InformationManager::Instance().GetEnemyBasePosition());
		}
		if (!TacticManager::Instance().isOneTacticRun(HydraliskPushTactic))
		{
			//AttackManager::Instance().issueAttackCommand(MutaliskHarassTac, Position(47 * 32, 53 * 32));
			AttackManager::Instance().issueAttackCommand(HydraliskPushTactic, InformationManager::Instance().GetEnemyBasePosition());
		}
		

		/*
		Position pos1(38 * 32, 55 * 32);
		Position pos2(45 * 32, 55 * 32);
		Position pos3(45 * 32, 62 * 32);
		Position pos4(38 * 32, 62 * 32);
		Position comp(5,5);
		Broodwar->drawBoxMap(pos1, pos1 + comp, BWAPI::Colors::Red);
		Broodwar->drawBoxMap(pos2, pos2 + comp, BWAPI::Colors::Red);
		Broodwar->drawBoxMap(pos3, pos3 + comp, BWAPI::Colors::Red);
		Broodwar->drawBoxMap(pos4, pos4 + comp, BWAPI::Colors::Red);
		for (auto unit : Broodwar->getAllUnits()) {
			if (unit->getPlayer() == Broodwar->self() && (unit->getType() == BWAPI::UnitTypes::Zerg_Hydralisk || unit->getType() == BWAPI::UnitTypes::Zerg_Mutalisk)) {
				if (Broodwar->getFrameCount() == 1) {
					unit->move(pos1);
					continue;
				}
				Position curPosition = unit->getPosition();
				Broodwar->drawBoxMap(curPosition, curPosition + comp, BWAPI::Colors::Blue);
				Broodwar->printf("dis to p1: %.3f, p2: %.3f, p3: %.3f, p4: %.3f, lf: %d, v=(%.2f,%.2f), |v|=%.3f",curPosition.getDistance(pos1),curPosition.getDistance(pos2), curPosition.getDistance(pos3), curPosition.getDistance(pos4),Broodwar->getRemainingLatencyFrames(),unit->getVelocityX(),unit->getVelocityY(),sqrt(unit->getVelocityX()*unit->getVelocityX() +unit->getVelocityY()*unit->getVelocityY()));
				if (curPosition.getDistance(pos1) < 32) {
					unit->move(pos2);
				}
				else if(curPosition.getDistance(pos2) < 32) {
					unit->move(pos1);
				}
				else if (curPosition.getDistance(pos3) < 32) {
					unit->move(pos4);
				}
				else if (curPosition.getDistance(pos4) < 32) {
					unit->move(pos1);
				}
			}
		}
		*/
		
	}

	else if (mapFileName.find("test_bwem") != -1) {
		for (auto const& startLocation : BWAPI::Broodwar->getStartLocations()) {
			if (startLocation != Broodwar->self()->getStartLocation()) {
				InformationManager::Instance().setLocationEnemyBase(startLocation);
			}
		}
		for (auto const& u : Broodwar->getAllUnits()) {
			if (u->getType() == UnitTypes::Zerg_Mutalisk) {
				//int pLongth = 0;
				//const CPPath& path = BWEMMap.GetPath(u->getPosition(), InformationManager::Instance().GetEnemyBasePosition(), &pLongth);
				//Broodwar->printf("%d", pLongth);

				const Area* enemyArea = BWEMMap.GetArea(u->getTilePosition());
				if (enemyArea == NULL)
				{
					enemyArea = BWEMMap.GetNearestArea(WalkPosition(u->getTilePosition()));
				}
			}
		}
	}

	else if (mapFileName.find("test_retreat") != -1) {
		for (auto const& startLocation : BWAPI::Broodwar->getStartLocations()) {
			if (startLocation != Broodwar->self()->getStartLocation()) {
				InformationManager::Instance().setLocationEnemyBase(startLocation);
			}
		}

		TimerManager::Instance().startTimer(TimerManager::All);
		TimerManager::Instance().startTimer(TimerManager::Information);
		InformationManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::Information);


		TimerManager::Instance().startTimer(TimerManager::Attack);
		AttackManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::Attack);

		TimerManager::Instance().startTimer(TimerManager::tactic);
		TacticManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::tactic);
		TimerManager::Instance().stopTimer(TimerManager::All);

		if (!TacticManager::Instance().isOneTacticRun(HydraliskPushTactic))
		{
			AttackManager::Instance().issueAttackCommand(HydraliskPushTactic, Position(InformationManager::Instance().getOurBaseLocation()));
		}
	}

	else if (mapFileName.find("test_irradiate") != -1) {
		for (auto const& startLocation : BWAPI::Broodwar->getStartLocations()) {
			if (startLocation != Broodwar->self()->getStartLocation()) {
				InformationManager::Instance().setLocationEnemyBase(startLocation);
			}
		}

		TimerManager::Instance().startTimer(TimerManager::All);
		TimerManager::Instance().startTimer(TimerManager::Information);
		InformationManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::Information);


		TimerManager::Instance().startTimer(TimerManager::Attack);
		AttackManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::Attack);

		TimerManager::Instance().startTimer(TimerManager::tactic);
		TacticManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::tactic);

		TimerManager::Instance().startTimer(TimerManager::strategy);
		StrategyManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::strategy);
		TimerManager::Instance().stopTimer(TimerManager::All);

		//Position pos= InformationManager::Instance().GetEnemyBasePosition();
		//Position comp(5, 5);
		//Broodwar->drawBoxMap(pos, pos + comp, BWAPI::Colors::Red, true);

		if (!TacticManager::Instance().isOneTacticRun(HydraliskPushTactic))
		{
			AttackManager::Instance().issueAttackCommand(HydraliskPushTactic, InformationManager::Instance().GetEnemyBasePosition());
		}
	}

	else if (mapFileName.find("test_formation") != -1) {
		for (auto const& startLocation : BWAPI::Broodwar->getStartLocations()) {
			if (startLocation != Broodwar->self()->getStartLocation()) {
				InformationManager::Instance().setLocationEnemyBase(startLocation);
			}
		}

		TimerManager::Instance().startTimer(TimerManager::All);
		TimerManager::Instance().startTimer(TimerManager::Information);
		InformationManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::Information);

		TimerManager::Instance().startTimer(TimerManager::Attack);
		AttackManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::Attack);

		TimerManager::Instance().startTimer(TimerManager::tactic);
		TacticManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::tactic);
		TimerManager::Instance().stopTimer(TimerManager::All);

		if (!TacticManager::Instance().isOneTacticRun(HydraliskPushTactic))
		{
			AttackManager::Instance().issueAttackCommand(HydraliskPushTactic, InformationManager::Instance().GetEnemyBasePosition());
		}
	}

	else if (mapFileName.find("test_patrol") != -1)
	{
		TimerManager::Instance().startTimer(TimerManager::All);

		TimerManager::Instance().startTimer(TimerManager::Information);
		InformationManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::Information);

		TimerManager::Instance().startTimer(TimerManager::Attack);
		AttackManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::Attack);

		TimerManager::Instance().startTimer(TimerManager::tactic);
		TacticManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::tactic);

		TimerManager::Instance().stopTimer(TimerManager::All);
	}

	else if (mapFileName.find("test_airdrop") != -1) {
		TimerManager::Instance().startTimer(TimerManager::All);

		TimerManager::Instance().startTimer(TimerManager::Information);
		InformationManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::Information);

		TimerManager::Instance().startTimer(TimerManager::Attack);
		AttackManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::Attack);

		TimerManager::Instance().startTimer(TimerManager::tactic);
		TacticManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::tactic);

		TimerManager::Instance().stopTimer(TimerManager::All);
		vector<Position> attackPosition;
		for (auto const& u : Broodwar->getAllUnits()) {
			if (u->getType() == UnitTypes::Terran_Command_Center) {
				attackPosition.push_back(u->getPosition());
				Broodwar->drawCircleMap(u->getPosition(), 10, BWAPI::Colors::Red, false);
			}
		}
		if (attackPosition.size() == 0)
			return;
		Broodwar->drawCircleMap(attackPosition[0], 10, BWAPI::Colors::Red, true);
		if (!TacticManager::Instance().isOneTacticRun(AirdropTac))
		{
			AttackManager::Instance().issueAttackCommand(AirdropTac, attackPosition[0]);
		}
	}

	else if (mapFileName.find("test_map") != -1) {
		TimerManager::Instance().startTimer(TimerManager::All);

		TimerManager::Instance().startTimer(TimerManager::Information);
		InformationManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::Information);

		TimerManager::Instance().stopTimer(TimerManager::All);

		Unit unit = nullptr;
		double maxDis = -1;
		for (auto const& u : Broodwar->getAllUnits()) {
			if (u->getPlayer() == Broodwar->self() && u->getType() == UnitTypes::Zerg_Drone) {
				double dis = u->getPosition().getDistance(Position(Broodwar->self()->getStartLocation()));
				if (dis > maxDis) {
					maxDis = dis;
					unit = u;
				}
			}
		}
		Broodwar->drawCircleMap(unit->getPosition(), 20, BWAPI::Colors::Red, false);
		//int pLongth = 0;
		//const CPPath& path = BWEMMap.GetPath(unit->getPosition(), Position(Broodwar->self()->getStartLocation()), &pLongth);
		//Position cur = unit->getPosition();
		//for (auto const& p : path) {
		//	Broodwar->drawLineMap(cur, Position(p->Center()), BWAPI::Colors::Blue);
		//	cur = Position(p->Center());
		//}
		//Broodwar->drawLineMap(cur, Position(Broodwar->self()->getStartLocation()), BWAPI::Colors::Blue);

		//Position nextP = Astar::Instance().getNextMovePosition(unit, Position(Broodwar->self()->getStartLocation()));

		if (Broodwar->getFrameCount() <= 70 * 24) {
			//unit->move(Position(10 * 32, 7 * 32));
			//unit->move(Position(118 * 32, 103 * 32));
			Position nextP = Astar::Instance().getNextMovePosition(unit, Position(126 * 32, 50 * 32));
			Broodwar->drawLineMap(unit->getPosition(), nextP, BWAPI::Colors::Blue);
			if (Broodwar->getFrameCount() % 10 == 0) {
				unit->move(nextP);
			}
		}
		else {
			//Position nextP = Astar::Instance().getNextMovePosition(unit, Position(19 * 32, 73 * 32));
			//Position nextP = Astar::Instance().getNextMovePosition(unit, Position(15 * 32, 98 * 32));
			//Position nextP = Astar::Instance().getNextMovePosition(unit, Position(115 * 32, 12 * 32));
			//Position nextP = Astar::Instance().getNextMovePosition(unit, Position(110 * 32, 39 * 32));
			Position nextP = Astar::Instance().getNextMovePosition(unit, Position(125 * 32, 24 * 32));
			Broodwar->drawLineMap(unit->getPosition(), nextP, BWAPI::Colors::Blue);
			if (Broodwar->getFrameCount() % 10 == 0) {
				unit->move(nextP);
			}
		}
	}

	else if (mapFileName.find("test_muta_hass") != -1) {
		InformationManager::Instance().update();
		AttackManager::Instance().update();
		TacticManager::Instance().update();

		for (auto const& startLocation : BWAPI::Broodwar->getStartLocations()) {
			if (startLocation != Broodwar->self()->getStartLocation()) {
				InformationManager::Instance().setLocationEnemyBase(startLocation);
			}
		}
		BWAPI::Position enemyBasePosition = InformationManager::Instance().GetEnemyBasePosition();

		//for (auto const&u : Broodwar->getAllUnits()) {
		//	if (u->getType() == UnitTypes::Protoss_Photon_Cannon) {
		//		Broodwar->drawCircleMap(u->getPosition(),u->getType().airWeapon().maxRange(),Colors::Red,false);
		//	}else if (u->getType() == UnitTypes::Zerg_Mutalisk) {
		//		Broodwar->drawCircleMap(u->getPosition(), u->getType().groundWeapon().maxRange(), Colors::Blue, false);
		//	}
		//}

		if (Broodwar->getFrameCount() == 10 * 24 && !TacticManager::Instance().isOneTacticRun(MutaliskHarassTac))
		{
			AttackManager::Instance().issueAttackCommand(MutaliskHarassTac, enemyBasePosition);
		}

		//if (Broodwar->getFrameCount() == 10 * 24 && !TacticManager::Instance().isOneTacticRun(HydraliskPushTactic))
		//{
		//	AttackManager::Instance().issueAttackCommand(HydraliskPushTactic, enemyBasePosition);
		//}
	}

	else if (mapFileName.find("test_upgrade") != -1) {
		std::map<std::pair<BWAPI::UpgradeType, int>, std::string>& upgradeUnderProcess = StrategyManager::Instance().getUpgradeUnderProcess();
		std::map<BWAPI::TechType, std::string>& techUnderProcess = StrategyManager::Instance().getTechProcess();
		std::set<UnitType>& researchingBuildingType = StrategyManager::Instance().getResearchingBuildingType();
		InformationManager::Instance().update();
		ProductionManager::Instance().update();
		if (BWAPI::Broodwar->getFrameCount() % 25 == 0) {
			for (std::map<BWAPI::TechType, std::string>::iterator it = techUnderProcess.begin(); it != techUnderProcess.end();)
			{
				if (BWAPI::Broodwar->self()->hasResearched(it->first))
				{
					researchingBuildingType.erase(it->first.whatResearches());
					it = techUnderProcess.erase(it);
				}
				else
				{
					it++;
				}
			}
			for (std::map<std::pair<BWAPI::UpgradeType, int>, std::string>::iterator it = upgradeUnderProcess.begin(); it != upgradeUnderProcess.end();)
			{
				if (BWAPI::Broodwar->self()->getUpgradeLevel(it->first.first) == it->first.second)
				{
					researchingBuildingType.erase(it->first.first.whatUpgrades());
					it = upgradeUnderProcess.erase(it);
				}
				else
				{
					it++;
				}
			}
		}
		UpgradeType ut1 = UpgradeTypes::Zerg_Melee_Attacks;
		UpgradeType ut2 = UpgradeTypes::Zerg_Missile_Attacks;
		UpgradeType ut3 = UpgradeTypes::Zerg_Carapace;
		UpgradeType ut4 = UpgradeTypes::Zerg_Flyer_Attacks;
		UpgradeType ut5 = UpgradeTypes::Zerg_Flyer_Carapace;
		UpgradeType ut6 = UpgradeTypes::Metabolic_Boost;
		UpgradeType ut7 = UpgradeTypes::Adrenal_Glands;
		for (auto upg : upgradeUnderProcess) {
			Broodwar->printf("type:%d,level:%d,name:%s", upg.first.first, upg.first.second, upg.second.c_str());
		}
		for (auto const& u : researchingBuildingType) {
			Broodwar->printf("Type:%d",u);
		}
		Broodwar->printf("melee attack:   %d m:%d g:%d", StrategyManager::Instance().isBuildActionValid("Upgrade_Zerg_Melee_Attacks"), ut1.mineralPrice(BWAPI::Broodwar->self()->getUpgradeLevel(ut1) + 1), ut1.gasPrice(BWAPI::Broodwar->self()->getUpgradeLevel(ut1) + 1));
		Broodwar->printf("missile attack: %d m:%d g:%d", StrategyManager::Instance().isBuildActionValid("Upgrade_Zerg_Missile_Attacks"), ut2.mineralPrice(BWAPI::Broodwar->self()->getUpgradeLevel(ut2) + 1), ut2.gasPrice(BWAPI::Broodwar->self()->getUpgradeLevel(ut2) + 1));
		Broodwar->printf("carapace:       %d m:%d g:%d", StrategyManager::Instance().isBuildActionValid("Upgrade_Zerg_Carapace"), ut3.mineralPrice(BWAPI::Broodwar->self()->getUpgradeLevel(ut3) + 1), ut3.gasPrice(BWAPI::Broodwar->self()->getUpgradeLevel(ut3) + 1));
		Broodwar->printf("flyer attack:   %d m:%d g:%d", StrategyManager::Instance().isBuildActionValid("Upgrade_Zerg_Flyer_Attacks"), ut4.mineralPrice(BWAPI::Broodwar->self()->getUpgradeLevel(ut4) + 1), ut4.gasPrice(BWAPI::Broodwar->self()->getUpgradeLevel(ut4) + 1));
		Broodwar->printf("flyer carapace: %d m:%d g:%d", StrategyManager::Instance().isBuildActionValid("Upgrade_Zerg_Flyer_Carapace"), ut5.mineralPrice(BWAPI::Broodwar->self()->getUpgradeLevel(ut5) + 1), ut5.gasPrice(BWAPI::Broodwar->self()->getUpgradeLevel(ut5) + 1));
		Broodwar->printf("MetabolicBoost: %d m:%d g:%d", StrategyManager::Instance().isBuildActionValid("Upgrade_ZerglingsSpeed"), ut6.mineralPrice(BWAPI::Broodwar->self()->getUpgradeLevel(ut6) + 1), ut6.gasPrice(BWAPI::Broodwar->self()->getUpgradeLevel(ut6) + 1));
		Broodwar->printf("ZergAttacSpeed: %d m:%d g:%d", StrategyManager::Instance().isBuildActionValid("Upgrade_ZerglingsAttackSpeed"), ut7.mineralPrice(BWAPI::Broodwar->self()->getUpgradeLevel(ut7) + 1), ut7.gasPrice(BWAPI::Broodwar->self()->getUpgradeLevel(ut7) + 1));
		Broodwar->printf("  ");
		if (StrategyManager::Instance().isBuildActionValid("Upgrade_Zerg_Melee_Attacks")) {
			StrategyManager::Instance().executeBuildAction("Upgrade_Zerg_Melee_Attacks");
		}
		else if (StrategyManager::Instance().isBuildActionValid("Upgrade_Zerg_Missile_Attacks")) {
			StrategyManager::Instance().executeBuildAction("Upgrade_Zerg_Missile_Attacks");
		}
		else if (StrategyManager::Instance().isBuildActionValid("Upgrade_Zerg_Carapace")) {
			StrategyManager::Instance().executeBuildAction("Upgrade_Zerg_Carapace");
		}
		else if (StrategyManager::Instance().isBuildActionValid("Upgrade_Zerg_Flyer_Attacks")) {
			StrategyManager::Instance().executeBuildAction("Upgrade_Zerg_Flyer_Attacks");
		}
		else if (StrategyManager::Instance().isBuildActionValid("Upgrade_Zerg_Flyer_Carapace")) {
			StrategyManager::Instance().executeBuildAction("Upgrade_Zerg_Flyer_Carapace");
		}
		else if (StrategyManager::Instance().isBuildActionValid("Upgrade_ZerglingsSpeed")) {
			StrategyManager::Instance().executeBuildAction("Upgrade_ZerglingsSpeed");
		}
		else if (StrategyManager::Instance().isBuildActionValid("Upgrade_ZerglingsAttackSpeed")) {
			StrategyManager::Instance().executeBuildAction("Upgrade_ZerglingsAttackSpeed");
		}
		else if (StrategyManager::Instance().isBuildActionValid("Upgrade_HydraliskSpeed")) {
			StrategyManager::Instance().executeBuildAction("Upgrade_HydraliskSpeed");
		}
		else if (StrategyManager::Instance().isBuildActionValid("Upgrade_HydraliskRange")) {
			StrategyManager::Instance().executeBuildAction("Upgrade_HydraliskRange");
		}
		else if (StrategyManager::Instance().isBuildActionValid("Upgrade_OverlordSpeed")) {
			StrategyManager::Instance().executeBuildAction("Upgrade_OverlordSpeed");
		}
		else if (StrategyManager::Instance().isBuildActionValid("Upgrade_OverlordLoad")) {
			StrategyManager::Instance().executeBuildAction("Upgrade_OverlordLoad");
		}
		else if (StrategyManager::Instance().isBuildActionValid("Upgrade_OverlordSight")) {
			StrategyManager::Instance().executeBuildAction("Upgrade_OverlordSight");
		}
		else if (StrategyManager::Instance().isBuildActionValid("Tech_LurkerTech")) {
			StrategyManager::Instance().executeBuildAction("Tech_LurkerTech");
		}
		else if (StrategyManager::Instance().isBuildActionValid("Building_Lair")) {
			StrategyManager::Instance().executeBuildAction("Building_Lair");
		}
		else if (StrategyManager::Instance().isBuildActionValid("Building_Hive")) {
			StrategyManager::Instance().executeBuildAction("Building_Hive");
		}
	}

	else if (mapFileName.find("test_iron") != -1 || mapFileName.find("test_antikite") != -1) {
		if (InformationManager::Instance().GetEnemyBasePosition() == Positions::None) {
			for (auto const& startLocation : BWAPI::Broodwar->getStartLocations()) {
				if (startLocation.x == 7 && startLocation.y == 88) {
					InformationManager::Instance().setLocationEnemyBase(startLocation);
				}
			}
		}

		InformationManager::Instance().update();
		AttackManager::Instance().update();
		TacticManager::Instance().update();

		if (!TacticManager::Instance().isOneTacticRun(HydraliskPushTactic))
		{
			AttackManager::Instance().issueAttackCommand(HydraliskPushTactic, InformationManager::Instance().GetEnemyBasePosition());
		}
		
		//for (int i = 0; i < Broodwar->mapWidth(); i++) {
		//	for (int j = 0; j < Broodwar->mapHeight(); j++) {
		//		TilePosition a(i, j);
		//		if (BWEMMap.GetTile(a).Doodad() > 0) {
		//			Broodwar->drawTextMap(Position(a).x, Position(a).y, "%d", BWEMMap.GetTile(a).Doodad());
		//		}
		//	}
		//}
		//for (int i = 0; i < Broodwar->mapWidth(); i++) {
		//	for (int j = 0; j < Broodwar->mapHeight(); j++) {
		//		TilePosition a(i, j);
		//		if (BWEMMap.GetMiniTile(WalkPosition(a)).Altitude() > 0) {
		//			Broodwar->drawTextMap(Position(a).x, Position(a).y, "%d", BWEMMap.GetMiniTile(WalkPosition(a)).Altitude());
		//		}
		//	}
		//}

		//Broodwar->drawCircleMap(Position(InformationManager::Instance().GetEnemyBasePosition()), 20, Colors::Red, true);
		//Broodwar->drawCircleMap(Position(InformationManager::Instance().GetEnemyNaturalPosition()), 20, Colors::Blue, true);

		//Broodwar->printf("%d", InformationManager::Instance().getEnemyBlockingBase());

		//const Area* enemyBaseArea = BWEMMap.GetArea(TilePosition(InformationManager::Instance().GetEnemyBasePosition()));
		//for (auto const& cp : enemyBaseArea->ChokePoints()) {
		//	Broodwar->drawCircleMap(Position(cp->Center()), 10 * 32, Colors::Red, false);
		//}

		for (int i = 0; i < Broodwar->mapWidth(); i++) {
			for (int j = 0; j < Broodwar->mapHeight(); j++) {
				TilePosition a(i, j);
				if (InformationManager::Instance().getEnemyInfluenceMap(i, j).unavailable) {
					Broodwar->drawCircleMap(Position(a)+Position(16,16),5,Colors::Blue,false);
				}
			}
		}

	}

	else if (mapFileName == "test_path_finding1.scm")
	{
		InformationManager::Instance().update();
		TacticManager::Instance().update();
		AttackManager::Instance().update();

		if (!TacticManager::Instance().isOneTacticRun(MutaliskHarassTac))
		{
			AttackManager::Instance().issueAttackCommand(MutaliskHarassTac, Position(20 * 32, 120 * 32));
		}
	}

	else if (mapFileName == "test_mutalisk_attack.scx")
	{
		InformationManager::Instance().update();
		TacticManager::Instance().update();
		AttackManager::Instance().update();

		if (!TacticManager::Instance().isOneTacticRun(MutaliskHarassTac))
		{
			AttackManager::Instance().issueAttackCommand(MutaliskHarassTac, Position(20 * 32, 120 * 32));
		}
	}

	else if (mapFileName == "test_all_in.scm")
	{
		InformationManager::Instance().update();
		TacticManager::Instance().update();
		AttackManager::Instance().update();
		StrategyManager::Instance().update();

		if (!TacticManager::Instance().isOneTacticRun(HydraliskPushTactic))
		{
			AttackManager::Instance().issueAttackCommand(HydraliskPushTactic, Position(10 * 32, 100 * 32));
		}
		else
		{
			if (StrategyManager::Instance().isActionValid(0, false))
			{
				AttackManager::Instance().addArmyToTactic(HydraliskPushTactic);
			}

		}
	}
	else if (mapFileName == "test_defend.scm")
	{
		InformationManager::Instance().update();
		TacticManager::Instance().update();
		AttackManager::Instance().update();
		//StrategyManager::Instance().update();
		//WorkerManager::Instance().update();
	}
	else if (mapFileName == "test_overlord_protect.scx")
	{
		InformationManager::Instance().update();
		TacticManager::Instance().update();
		AttackManager::Instance().update();
		StrategyManager::Instance().update();
	}
	else if (mapFileName == "test_defend_muta.scm")
	{
		InformationManager::Instance().update();
		TacticManager::Instance().update();
		AttackManager::Instance().update();
		StrategyManager::Instance().update();
	}
	else if (mapFileName == "test_muta_vs_army.scm")
	{
		InformationManager::Instance().update();
		TacticManager::Instance().update();
		AttackManager::Instance().update();
		StrategyManager::Instance().update();

		if (!TacticManager::Instance().isOneTacticRun(MutaliskHarassTac))
		{
			AttackManager::Instance().issueAttackCommand(MutaliskHarassTac, InformationManager::Instance().GetEnemyBasePosition());
		}
		else
		{
			if (StrategyManager::Instance().isActionValid(11, false))
			{
				AttackManager::Instance().addArmyToTactic(MutaliskHarassTac);
			}
		}
	}
	else if (mapFileName == "test_defend_tank.scm")
	{
		InformationManager::Instance().update();
		TacticManager::Instance().update();
		AttackManager::Instance().update();
		WorkerManager::Instance().update();
	}
	else if (mapFileName == "test_muta_vs_hydra.scm")
	{
		InformationManager::Instance().update();
		TacticManager::Instance().update();
		AttackManager::Instance().update();
		if (!TacticManager::Instance().isOneTacticRun(MutaliskHarassTac))
		{
			AttackManager::Instance().issueAttackCommand(MutaliskHarassTac, InformationManager::Instance().GetEnemyBasePosition());
		}
	}
	else if (mapFileName == "test_muta_defend.scm")
	{
		return;
	}
	else if (mapFileName == "test_muta_attack_bunker.scm")
	{
		if (!TacticManager::Instance().isOneTacticRun(MutaliskHarassTac))
		{
			AttackManager::Instance().issueAttackCommand(MutaliskHarassTac, InformationManager::Instance().GetEnemyBasePosition());
		}
	}
	else if (mapFileName == "test_hydra_attack_bunker.scm")
	{
		if (!TacticManager::Instance().isOneTacticRun(HydraliskPushTactic))
		{
			AttackManager::Instance().issueAttackCommand(HydraliskPushTactic, InformationManager::Instance().GetEnemyBasePosition());
		}
	}
	else if (mapFileName == "test_feature_defend.scm")
	{

	}
	else if (mapFileName == "test_muta_vs_scv.scm")
	{
		if (!TacticManager::Instance().isOneTacticRun(MutaliskHarassTac))
		{
			AttackManager::Instance().issueAttackCommand(MutaliskHarassTac, InformationManager::Instance().GetEnemyBasePosition());
		}
	}

	else if (mapFileName == "test_harass_bug.scm")
	{
		if (!TacticManager::Instance().isOneTacticRun(MutaliskHarassTac))
		{
			AttackManager::Instance().issueAttackCommand(MutaliskHarassTac, InformationManager::Instance().GetEnemyBasePosition());
		}
	}

	else if (mapFileName == "test_hydra.scm")
	{
		InformationManager::Instance().update();
		TacticManager::Instance().update();
		AttackManager::Instance().update();

		//if (!TacticManager::Instance().isOneTacticRun(HydraliskPushTactic))
		//{
		//	AttackManager::Instance().issueAttackCommand(HydraliskPushTactic, Position(5 * 32, 70 * 32));
		//}
	}

	else if (mapFileName == "test_tmp.scm")
	{
		WorkerManager::Instance().update();
		InformationManager::Instance().update();
		ProductionManager::Instance().update();
		AttackManager::Instance().update();
		StrategyManager::Instance().update();

		if (ProductionManager::Instance().IsQueueEmpty())
		{
			StrategyManager::Instance().executeBuildAction("Unit_Lurker");
		}

	}

	else if (mapFileName.find("test_kite") != -1) {
		InformationManager::Instance().update();
		TacticManager::Instance().update();
		AttackManager::Instance().update();
		StrategyManager::Instance().update();

		if (!TacticManager::Instance().isOneTacticRun(HydraliskPushTactic))
		{
			AttackManager::Instance().issueAttackCommand(HydraliskPushTactic, Position(46 * 32, 55 * 32));
		}
	}



	else if (mapFileName == "test_production.scx")
	{
		InformationManager::Instance().update();
		StrategyManager::Instance().update();
		WorkerManager::Instance().update();
		ProductionManager::Instance().update();
		BuildingManager::Instance().update();
		AttackManager::Instance().update();
		TacticManager::Instance().update();

		for (auto&u : Broodwar->self()->getUnits())
		{
			if (u->isSelected())
			{
				Position center = u->getPosition();
				Position topLeft = Position(center.x - u->getType().tileWidth() * 32 / 2, center.y - u->getType().tileHeight() * 32 / 2);
				Broodwar->drawTextMap(topLeft, "%d,%d", TilePosition(topLeft).x, TilePosition(topLeft).y);
				Position topRight = Position(center.x + u->getType().tileWidth() * 32 / 2, center.y - u->getType().tileHeight() * 32 / 2);
				Broodwar->drawTextMap(topRight, "%d,%d", TilePosition(topRight).x, TilePosition(topRight).y);
				Position bottomLeft = Position(center.x - u->getType().tileWidth() * 32 / 2, center.y + u->getType().tileHeight() * 32 / 2);
				Broodwar->drawTextMap(bottomLeft, "%d,%d", TilePosition(bottomLeft).x, TilePosition(bottomLeft).y);
				Position bottomRight = Position(center.x + u->getType().tileWidth() * 32 / 2, center.y + u->getType().tileHeight() * 32 / 2);
				Broodwar->drawTextMap(bottomRight, "%d,%d", TilePosition(bottomRight).x, TilePosition(bottomRight).y);
			}
		}

		if (isInit == false)
		{
			actions.clear();
			std::string source = "12,0,10,10,10,10,10,8,10,10,10,10,10,10,8";
			vector<std::string> result = splitStr(source, ',');
			for (auto s : result)
			{
				actions.push_back(std::stoi(s));
			}
			isInit = true;
		}

		if (ProductionManager::Instance().IsQueueEmpty())
		{
			if (StrategyManager::Instance().isActionValid(actions.front(), true)
				&& Broodwar->getFrameCount() > nextBuildTime)
			{
				StrategyManager::Instance().executeAction(actions.front(), true);
				actions.erase(actions.begin());
				nextBuildTime = Broodwar->getFrameCount() + 5 * 24;
			}
		}

	}
}


