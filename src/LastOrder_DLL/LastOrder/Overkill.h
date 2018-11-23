#pragma once


#include <BWAPI.h>
#include "BWEM/src/bwem.h"

#include <string>
#include <sstream>
#include <ctime>
#include <list>
#include <vector>
#include <map>


#include <iostream>
#include <fstream>
#include <algorithm>
#include <math.h>

#include "WorkerManager.h"
#include "ProductionManager.h"
#include "BuildingManager.h"
#include "InformationManager.h"
#include "AttackManager.h"
#include "StrategyManager.h"
#include "TacticManager.h"

#include "Common.h"
#include "TimeManager.cpp"
#include "testMain.h"





class Overkill : public BWAPI::AIModule
{
	void			readHistoryFile();
	void			writeCurrentPlay(bool isWin);
	
	openingStrategy						chooseOpeningStrategy;
	std::vector<std::vector<string>>	historyInfo;
	std::string winRate;
	bool								hasExit;
	int									pausedFrame;

	int									ourScore;
	int									enemyScore;
	
public:
	

	void	onStart();
	void	onFrame();
	void	onEnd(bool isWinner);
	void	onUnitDestroy(BWAPI::Unit unit);
	void	onUnitMorph(BWAPI::Unit unit);
	void	onSendText(std::string text);
	void	onUnitCreate(BWAPI::Unit unit);
	void	onUnitShow(BWAPI::Unit unit);
	void	onUnitHide(BWAPI::Unit unit);
	void    onNukeDetect(BWAPI::Position target);
	void    onUnitComplete(BWAPI::Unit unit);
	void    onUnitDiscover(BWAPI::Unit unit);
	void	onUnitEvade(BWAPI::Unit unit);

	void	drawStats(); //not part of BWAPI::AIModule

};



