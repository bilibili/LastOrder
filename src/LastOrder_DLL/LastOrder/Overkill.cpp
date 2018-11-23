#include "Overkill.h"
#include "exceptionHandler.h"


void Overkill::readHistoryFile()
{
	historyInfo.clear();
	fstream historyFile;
	string enemyName = BWAPI::Broodwar->enemy()->getName();

	string filePath;
	filePath = readResourceFolder;
	filePath += enemyName;
	
	//for each enemy, create a file
	historyFile.open(filePath.c_str(), ios::in);
	
	//file do not exist, first match to the enemy
	if (!historyFile.is_open())
	{
		BWAPI::Broodwar->printf("first match to:   %s", enemyName.c_str());
		historyFile.close();

		//default opening strategy
		chooseOpeningStrategy = NinePoolling; //TwelveHatchMuta;//TenHatchMuta;//NinePoolling;
		StrategyManager::Instance().setOpeningStrategy(chooseOpeningStrategy);
		StrategyManager::Instance().setOpponentWinrate("-1");
	}
	else
	{
		BWAPI::Broodwar->printf("read match records with :   %s", enemyName.c_str());
		string playerResult;
		// file format:
		// enemy_name | choose strategy | result
		while (getline(historyFile, playerResult))
		{
			std::stringstream ss(playerResult);
			std::vector<string> itemList;
			string item;
			while (getline(ss, item, '|'))
			{
				if (item != "")
					itemList.push_back(item);
			}
			historyInfo.push_back(itemList);
		}

		//do UCB1 calculate, and set the opening strategy
		std::map<std::string, std::pair<int, int>> strategyResults;
		int winCount = 0;
		for (auto record : historyInfo)
		{
			//win
			if (record[2] == "1")
			{
				strategyResults[record[1]].first += 1;
				winCount += 1;
			}
			else
			{
				strategyResults[record[1]].second += 1;
			}
		}
		std::string rate = std::to_string(int(10 * float(winCount) / historyInfo.size()));
		StrategyManager::Instance().setOpponentWinrate(rate);

		double experimentCount = historyInfo.size();
		std::map<std::string, double> strategyUCB;
		for(auto opening : StrategyManager::Instance().getStrategyNameArray())
		{
			strategyUCB[opening] = 99999;
		}

		//for (std::map<std::string, std::pair<int, int>>::iterator it = strategyResults.begin(); it != strategyResults.end(); it++)
		for (auto it : strategyResults)
		{
			double strategyExpectation = double(it.second.first) / (it.second.first + it.second.second);
			double uncertainty = 0.7 * std::sqrt(std::log(experimentCount) / (it.second.first + it.second.second));
			strategyUCB[it.first] = strategyExpectation + uncertainty;
		}
		
		std::string maxUCBStrategy;
		double maxUCB = 0;
		//for (std::map<std::string, double>::iterator it = strategyUCB.begin(); it != strategyUCB.end(); it++)
		for (auto it : strategyUCB)
		{
			if (it.second > maxUCB)
			{
				maxUCBStrategy = it.first;
				maxUCB = it.second;
			}
			BWAPI::Broodwar->printf("%s , UCB: %.4f", it.first.c_str(), it.second);
		}

		BWAPI::Broodwar->printf("choose %s opening", maxUCBStrategy.c_str());
		int openingId = StrategyManager::Instance().getStrategyByName(maxUCBStrategy);
		if (openingId != -1)
		{
			chooseOpeningStrategy = openingStrategy(openingId);
		}
		 
		//for test
		chooseOpeningStrategy = NinePoolling;

		StrategyManager::Instance().setOpeningStrategy(chooseOpeningStrategy);
		historyFile.close();
	}
}


void Overkill::writeCurrentPlay(bool isWin)
{
	fstream historyFile;
	std::string enemyName = BWAPI::Broodwar->enemy()->getName();
	std::string filePath = writeResourceFolder;
	filePath += enemyName;

	//for each enemy, create a file
	historyFile.open(filePath.c_str(), ios::out);

	std::vector<string> currentPlay;
	currentPlay.push_back(enemyName);
	currentPlay.push_back(StrategyManager::Instance().getStrategyName(chooseOpeningStrategy));
	if (isWin)
		currentPlay.push_back("1");
	else
		currentPlay.push_back("0");
	historyInfo.push_back(currentPlay);

	bool enemyEarlyRushFlag = false;
	//defeated by early Rush
	if (enemyEarlyRushFlag && !isWin)
	{
		//12 hatch is slower than 10 hatch, so it may definitely loss in this match
		//add this simulated record to improved ucb learning speed.
		if (chooseOpeningStrategy == TenHatchMuta || chooseOpeningStrategy == TwelveHatchMuta)
		{
			std::vector<string> simulatedPlay;
			simulatedPlay.push_back(enemyName);
			openingStrategy simulateOpening = chooseOpeningStrategy == TenHatchMuta ? TwelveHatchMuta : TenHatchMuta;
			simulatedPlay.push_back(StrategyManager::Instance().getStrategyName(simulateOpening));
			simulatedPlay.push_back("0");
			historyInfo.push_back(simulatedPlay);
		}
	}

	for (auto record : historyInfo)
	{
		for (auto field : record)
		{
			historyFile << field << "|";
		}
		historyFile << endl;
	}
	
	historyFile.close();
}




void Overkill::onStart()
{
	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);

	logInit();

	Broodwar->setLocalSpeed(0);
	//Broodwar->setFrameSkip(1);
	Broodwar->setLatCom(false);

	logInfo("main", "latency type " + to_string(Broodwar->getLatency()));
	logInfo("main", "latency frame " + to_string(Broodwar->getLatencyFrames()));
	logInfo("main", "latency compensation " + to_string(Broodwar->isLatComEnabled()));

	BWAPI::Broodwar->sendText("gl hf :)");

	// Enable some cheat flags
	Broodwar->enableFlag(Flag::UserInput);
	// Uncomment to enable complete map information

	Broodwar << "Map initialization..." << std::endl;
	BWEMMap.Initialize();
	BWEMMap.EnableAutomaticPathAnalysis();
	bool startingLocationsOK = BWEMMap.FindBasesForStartingLocations();
	//assert(startingLocationsOK);
	//const std::vector<const Area *>& neighbor = BWEMMap.GetArea(Broodwar->self()->getStartLocation())->AccessibleNeighbours();
	//const std::vector<Base> & b = neighbor.front()->Bases();

	//BWEM::utils::drawMap(BWEMMap);      // will print the map into the file <StarCraftFolder>bwapi-data/map.bmp
	//BWEM::utils::MapPrinter::Initialize(&BWEMMap);
	//BWEM::utils::pathExample(BWEMMap);   // add to the printed map a path between two starting locations

	for (auto & u : BWAPI::Broodwar->self()->getUnits())
	{
		if (u->getType().isResourceDepot())
			InformationManager::Instance().addOccupiedRegionsDetail(
			BWEMMap.GetArea(Broodwar->self()->getStartLocation()), BWAPI::Broodwar->self(), u);
	}

	readHistoryFile();
	hasExit = false;
	pausedFrame = 0;

	ourScore = 0;
	enemyScore = 0;
}


void Overkill::onEnd(bool isWinner)
{
	int curMinutes = Broodwar->getFrameCount() / (60 * 24);
	double reward = isWinner == true ? 1 : -1;
	logInfo("Main", "game exit " + to_string(isWinner) + 
		" score:" + to_string(reward), "Main");
	if (InformationManager::Instance().GetEnemyBasePosition() == Positions::None) {
		logInfo("Scout","Did not find enemy base in this game!","BIG_ERROR_Scout");
	}

	string mapName = Broodwar->mapFileName();
	int currentSupply = BWAPI::Broodwar->self()->supplyUsed();

	bool discard = false;
	int maxFrameCount = TimerManager::Instance().getMaxFrameCount();
	int gamePlayLength = Broodwar->getFrameCount() / (24 * 60);
	std::string enemyRace = InformationManager::Instance().getEnemyRace();

	std::string maxTimeItem = TimerManager::Instance().getMaxItem();
	double maxValue = TimerManager::Instance().getMaxFrameValue();
	//save match result

	// Under test mode, do not wait reward
	if (!isTestMode)
	{
		StrategyManager::Instance().gameEnd(reward, discard);
	}
}


void Overkill::onFrame()
{
	if (Broodwar->isReplay())
		return;

	//getScore();

	string mapName = Broodwar->mapFileName();
	//test case special command
	if (mapName.find("test") != -1)
	{
		isTestMode = true;
		BWEM::utils::drawMap(BWEMMap);
		drawStats();
		
		TimerManager::Instance().startTimer(TimerManager::All);
		testMain(mapName);
		TimerManager::Instance().stopTimer(TimerManager::All);
		TimerManager::Instance().displayTimers(490, 180);
	}
	else
	{
		if (Broodwar->getFrameCount() >= 10 * 60 * 24
			&& Broodwar->self()->minerals() <= 100
			&& WorkerManager::Instance().getNumMineralWorkers() == 0
			&& InformationManager::Instance().hasAttackArmy() == false
			&& hasExit == false)
		{
			logInfo("Main", "early exit 0", "Main");
			BWAPI::Broodwar->leaveGame();
			hasExit = true;
			return;
		}

		BWEM::utils::drawMap(BWEMMap);
		drawStats();

		TimerManager::Instance().startTimer(TimerManager::All);

		TimerManager::Instance().startTimer(TimerManager::Worker);
		WorkerManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::Worker);

		//MapGrid::Instance().update();


		//self start location only available if the map has base locations
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

		//last update
		TimerManager::Instance().startTimer(TimerManager::strategy);
		StrategyManager::Instance().update();
		TimerManager::Instance().stopTimer(TimerManager::strategy);

		TimerManager::Instance().stopTimer(TimerManager::All);
		TimerManager::Instance().displayTimers(490, 180);
	}

}


void Overkill::onNukeDetect(BWAPI::Position target)
{
	if (target != Positions::Unknown)
		Broodwar->printf("Nuclear Launch Detected at (%d,%d)", target.x, target.y);
	else
		Broodwar->printf("Nuclear Launch Detected");
}

void Overkill::onUnitDiscover(BWAPI::Unit unit)
{

}

void Overkill::onUnitEvade(BWAPI::Unit unit)
{

}

void Overkill::onUnitShow(BWAPI::Unit unit)
{
	if (unit->getType() == BWAPI::UnitTypes::Zerg_Egg || unit->getType() == BWAPI::UnitTypes::Zerg_Larva
		|| unit->getType() == BWAPI::UnitTypes::Zerg_Cocoon || unit->getType() == BWAPI::UnitTypes::Zerg_Lurker_Egg)
		return;

	TacticManager::Instance().onUnitShow(unit);
	InformationManager::Instance().onUnitShow(unit);
	//for initial 
	//ScoutManager::Instance().onUnitShow(unit);
	WorkerManager::Instance().onUnitShow(unit);
	//for first overlord
	if (unit->getPlayer() == BWAPI::Broodwar->self() && 
		(!unit->getType().isBuilding() && !unit->getType().isWorker()))
	{
		AttackManager::Instance().onUnitMorph(unit);
	}
}

void Overkill::onUnitHide(BWAPI::Unit unit)
{
	
}

void Overkill::onUnitCreate(BWAPI::Unit unit)
{

}


void Overkill::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit->getPlayer() == BWAPI::Broodwar->self())
	{
		ProductionManager::Instance().onUnitDestroy(unit);

		if (unit->getType() == BWAPI::UnitTypes::Zerg_Egg)
		{
			StrategyManager::Instance().unitMorphingFinish(unit->getBuildType());
		}
		if (unit->getType() == BWAPI::UnitTypes::Zerg_Lurker_Egg)
		{
			StrategyManager::Instance().unitMorphingFinish(UnitTypes::Zerg_Lurker);
		}
	}
	
	if (unit->getType() == BWAPI::UnitTypes::Zerg_Egg || unit->getType() == BWAPI::UnitTypes::Zerg_Larva
		|| unit->getType() == BWAPI::UnitTypes::Zerg_Cocoon || unit->getType() == BWAPI::UnitTypes::Zerg_Lurker_Egg)
		return;

	try
	{
		if (unit->getType().isMineralField())    
			BWEMMap.OnMineralDestroyed(unit);
		else if (unit->getType().isSpecialBuilding()) 
			BWEMMap.OnStaticBuildingDestroyed(unit);
	}
	catch (const std::exception & e)
	{
		Broodwar << "EXCEPTION: " << e.what() << std::endl;
	}

	InformationManager::Instance().onUnitDestroy(unit);
	WorkerManager::Instance().onUnitDestroy(unit);
	if (unit->getPlayer() == BWAPI::Broodwar->self())
	{
		ProductionManager::Instance().onUnitDestroy(unit);
		if (!unit->getType().isBuilding())
		{
			AttackManager::Instance().onUnitDestroy(unit);
			TacticManager::Instance().onUnitDestroy(unit);
		}
		else {
			StrategyManager::Instance().onUnitDestroy(unit);
		}

		//trigger morph
		if (!unit->isUnderAttack() && unit->getType().isWorker())
		{
			return;
		}
	}

	StrategyManager::Instance().addScore(unit);
}


//trigger on changing unit type
void Overkill::onUnitMorph(BWAPI::Unit unit)
{
	if (unit->getType() == BWAPI::UnitTypes::Zerg_Lurker_Egg && unit->getBuildType() == BWAPI::UnitTypes::Zerg_Lurker)
	{
		AttackManager::Instance().onLurkerMorph();
		TacticManager::Instance().onLurkerMorph();
		InformationManager::Instance().onLurkerMorph(unit);
	}

	if (unit->getPlayer() == BWAPI::Broodwar->self())
	{
		ProductionManager::Instance().onUnitMorph(unit);
		StrategyManager::Instance().unitMorphingFinish(unit->getType());
	}

	if (unit->getType() == BWAPI::UnitTypes::Zerg_Egg || unit->getType() == BWAPI::UnitTypes::Zerg_Larva
		|| unit->getType() == BWAPI::UnitTypes::Zerg_Cocoon || unit->getType() == BWAPI::UnitTypes::Zerg_Lurker_Egg)
		return;

	InformationManager::Instance().onUnitMorph(unit);
	if (unit->getPlayer() == BWAPI::Broodwar->self())
	{
		
		WorkerManager::Instance().onUnitMorph(unit);
		//ScoutManager::Instance().onUnitMorph(unit);
		if (!unit->getType().isBuilding() && !unit->getType().isWorker())
		{
			AttackManager::Instance().onUnitMorph(unit);
		}

	}
}


void Overkill::onUnitComplete(BWAPI::Unit unit)
{
	if (unit->getPlayer() == BWAPI::Broodwar->self())
	{
		if (unit->getType().isBuilding())
		{
			if (unit->getType() == BWAPI::UnitTypes::Zerg_Hatchery)
			{
				bool isExpand = false;
				for (auto const& a : BWEMMap.Areas())
				{
					for (auto const& b : a.Bases())
					{
						if (b.Location().getDistance(unit->getTilePosition()) < 3)
						{
							StrategyManager::Instance().expandFinish();
							isExpand = true;

							//if (Broodwar->self()->completedUnitCount(UnitTypes::Zerg_Hatchery) > 1)
								//WorkerManager::Instance().balanceWorkerOnDepotComplete(unit);
							break;
						}
					}
				}
				if (isExpand == false)
				{
					StrategyManager::Instance().buildingFinish(UnitTypes::Zerg_Hatchery);
				}
			}
			else
			{
				StrategyManager::Instance().buildingFinish(unit->getType());
			}
		}
	}
}


void Overkill::drawStats()
{
	BWAPI::Unitset myUnits = BWAPI::Broodwar->self()->getUnits();
	Broodwar->drawTextScreen(5, 0, "frame: %d minutes:%d", Broodwar->getFrameCount(), Broodwar->getFrameCount() / (24 * 60));
	std::map<UnitType, int> unitTypeCounts;
	//for (std::set<Unit*>::iterator i = myUnits.begin(); i != myUnits.end(); i++)
	for (auto unit : myUnits)
	{
		if (!unit->getType().isBuilding() && unit->getType().canAttack())
		{
			int unitWidth = unit->getType().tileWidth() * 32;
			int unitHeight = unit->getType().tileHeight() * 32 / 8;
			BWAPI::Position unitPosition = unit->getPosition();
			BWAPI::Broodwar->drawBoxMap(unitPosition.x - unitWidth / 2, unitPosition.y + unitHeight / 2, unitPosition.x + unitWidth / 2, unitPosition.y - unitHeight / 2, BWAPI::Colors::Red, true);
			double healthPercent = double(unit->getHitPoints()) / unit->getType().maxHitPoints();
			BWAPI::Broodwar->drawBoxMap(unitPosition.x - unitWidth / 2, unitPosition.y + unitHeight / 2, unitPosition.x - unitWidth / 2 + int(unitWidth * healthPercent), unitPosition.y - unitHeight / 2, BWAPI::Colors::Green, true);
			/*
			if (unit->isSelected())
			{
				BWAPI::Broodwar->printf("Previous Command Frame=%d Pos=(%d, %d) type=%d", unit->getLastCommandFrame(), unit->getLastCommand().getTargetPosition().x / 32, unit->getLastCommand().getTargetPosition().y / 32, unit->getLastCommand().getType());
				BWAPI::Broodwar->drawLineMap(unit->getPosition().x, unit->getPosition().y, unit->getLastCommand().getTargetPosition().x, unit->getLastCommand().getTargetPosition().y, BWAPI::Colors::Green);
				BWAPI::Broodwar->drawCircleMap(unit->getLastCommand().getTargetPosition().x, unit->getLastCommand().getTargetPosition().y, 8, BWAPI::Colors::Green, true);
			}*/

		}

		if (unitTypeCounts.find(unit->getType()) == unitTypeCounts.end())
		{
			unitTypeCounts.insert(std::make_pair(unit->getType(), 0));
		}
		unitTypeCounts.find(unit->getType())->second++;
	}
	int line = 1;
	for (std::map<UnitType, int>::iterator i = unitTypeCounts.begin(); i != unitTypeCounts.end(); i++)
	{
		Broodwar->drawTextScreen(5, 16 * line, "- %d %ss", (*i).second, (*i).first.getName().c_str());
		line++;
	}
	// draw the map tile position at map
	Broodwar->drawTextScreen(BWAPI::Broodwar->getMousePosition().x + 20, BWAPI::Broodwar->getMousePosition().y + 20, "%d %d", (BWAPI::Broodwar->getScreenPosition().x + BWAPI::Broodwar->getMousePosition().x) / TILE_SIZE, (BWAPI::Broodwar->getScreenPosition().y + BWAPI::Broodwar->getMousePosition().y) / TILE_SIZE);
}


void Overkill::onSendText(std::string text)
{
	utils::MapDrawer::ProcessCommand(text);
}
