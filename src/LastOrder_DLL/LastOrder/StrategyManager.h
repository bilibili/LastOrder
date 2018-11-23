#pragma once


#include "zhelpers.h"
#include "Common.h"
#include "BuildOrderQueue.h"
#include "WorkerManager.h"
#include "ProductionManager.h"
#include "TacticManager.h"

#include <iostream>
#include <fstream>
#include <iomanip>



typedef std::pair<int, int> IntPair;
typedef std::pair<MetaType, UnitCountType> MetaPair;
typedef std::vector<MetaPair> MetaPairVector;


using namespace BWAPI;
using namespace std;


struct sampleParsedInfo
{
	vector<double> modelInputs;
	int actualChosen;
	double targetQValue;
};


class StrategyManager
{
	StrategyManager();
	~StrategyManager() {}
	int							droneProductionSpeed;
	int							nextBuildCheckTime;
	int							nextAttackCheckTime;

	void						onGoalProduction();
	void						onDroneProduction();
	void						onExtractorProduction();
	void						onHatcheryProduction();
	void						buildExtractorTrick();

	enum modelType {BuildModel, updateState};

	map<string, int>			allExecutedActions;
	int							nextLogFeatureTime;

	enum ProductionState { fixBuildingOrder, goalOriented };
	ProductionState				productionState;
	vector<int>					latestActions;
	map<std::string, Position>	enemyBaseNameToPosition;
	map<std::string, const Area*>	ourBaseNameToPosition;

	vector<int>					testActions;
	bool						isExtractTrickFinish;
	int							nextDroneBuildTime;

	BWAPI::Race					selfRace;
	BWAPI::Race					enemyRace;
	std::vector<MetaType> 		actions;
	std::vector<MetaType>		getMetaVector(std::string buildString);
	openingStrategy				currentopeningStrategy;

	std::vector<std::string>	openingStrategyName;

	std::vector<std::string>							stateActions;
	std::map<std::string, std::pair<BWAPI::UnitType, BWAPI::UnitType>>			actionBuildingTarget;
	std::map<std::string, std::map<std::string, std::vector<std::string>>>		featureNames;
	int													previousScore;
	std::string											previousAction;
	std::map<std::string, std::map<std::string, double>>						parameterValue;
	std::map<std::string, std::map<std::string, int>>						featureValue;


	std::map<std::string, MetaType> AttackActions;
	std::map<std::string, MetaType> BuildActions;

	std::vector<std::string>	features;
	map<UnitType, std::string>	enemyBattleBasicFeature;
	map<UnitType, std::string>	ourBattleBasicFeature;

	map<UnitType, std::string>	waitToBuildArmyFeature;
	map<UnitType, std::string>	waitToBuildBuildingFeature;
	string						waitToExpandFeature;

	map<UnitType, std::string>	enemyDefendBuildingFeature;
	map<UnitType, std::string>	ourDefendBuildingFeature;

	std::vector<std::string>	NNInputAction;

	std::vector<std::string>	NNOutputBuildAction;
	std::vector<std::string>	NNOutputAttackAction;
	std::map<std::string, int>	buildActionsOutputIndexMapping;
	std::map<std::string, int>	attackActionsOutputIndexMapping;

	std::map<std::string, int> featureValues;
	std::vector<std::vector<std::string>> trainingData;

	std::pair<int, int>				waitToExpand;
	std::map<BWAPI::UnitType, int>	unitWaitToBuild;
	std::map<BWAPI::UnitType, std::pair<int, int>>		buildingsUnderProcess;
	std::map<BWAPI::UnitType, std::string> buildingsName;
	std::map<BWAPI::TechType, std::string> techsName;
	std::map<BWAPI::UpgradeType, std::string> upgradesName;
	std::map<BWAPI::TechType, std::string> techUnderProcess;
	std::map<std::pair<BWAPI::UpgradeType, int>, std::string> upgradeUnderProcess;
	std::set<UnitType>			researchingBuildingType;
	std::string					previousChosenAction;
	int							replayLength;

	int							curActionTime;

	std::map<BWAPI::UnitType, int> morphUnits;


	std::vector<std::vector<std::string>> testSetData;
	double						testSetAvgQValue;

	std::map<std::string, std::map<std::string, double>>						parameterCumulativeGradient;


	std::vector<std::vector<std::string>>	curEpisodeData;
	double						discountRate;
	void						calCurrentStateFeature();
	std::string					opponentWinrate;

	int							playMatchCount;
	bool 						muteBuilding;
	bool						isInBuildingMutalisk;
	std::string					curBuildingAction;

	int							getCurrentPlayerScore();
	
	string						serialize(vector<string> features, double instantScore, double terminalScore, vector<string> actionValids,
								int end, string attackInfo, string extra, modelType mt);

	zmq::socket_t*				modelResultReceiver;
	zmq::socket_t*				modelTrigger;
	zmq::context_t*				context;
	zmq_pollitem_t				pollItems[1];
	bool						isRequestBuildModel;
	int							startRequestBuildModelFrame;
	bool						hasSendBlockMessage;

	bool						isTerminated;

	int							startBuildRequestFrame;
	int							previousStateAccumulatedReward;

	int							HatcheryProductionCheckTime;
	int							extractorProductionCheckTime;

	double						selfLostScore;
	double						enemyLostScore;

	set<int>					preValidActionIndex;

	void						sendStateUpdate();
	int							nextStateUpdateTime;

public:

	static	StrategyManager &	Instance();

	std::vector<MetaType>		getOpeningBook();
	void						setOpeningStrategy(openingStrategy opening);

	void						expandFinish() { waitToExpand.first -= 1; }
	void						buildingFinish(BWAPI::UnitType u);
	void						unitMorphingFinish(BWAPI::UnitType u);
	
	openingStrategy				getCurrentopeningStrategy() { return currentopeningStrategy; }
	void						update();

	int							getStrategyByName(std::string strategy);
	std::string					getStrategyName(openingStrategy strategy);
	std::vector<std::string>	getStrategyNameArray() { return openingStrategyName; }
	int							getScore(bool isSelf);

	std::string					getCurrentBuildingAction() { return curBuildingAction; }

	void						setOpponentWinrate(std::string rate) { opponentWinrate = rate; }
	int							getPlayeMatchCount() { return playMatchCount; }
	bool						isBuildingMutaliskBuilding() { return isInBuildingMutalisk; }

	void						gameEnd(double win, bool discard);

	bool						isAtOpeningStage() { return productionState == fixBuildingOrder; }

	void						triggerModel(double win, int isGameEnd, modelType mt);


	void						executeBuildAction(string chosedAction);
	void						executeAttackAction(string chosedAction);

	void						executeAction(int chosedAction, bool buildOrAttack);
	bool						isActionValid(int actionName, bool buildOrAttack);

	bool						isBuildActionValid(string actionName);
	bool						isAttackActionValid(string actionName);

	bool						checkMetaValid(MetaType metaType);
	bool						isUnitValid(UnitType targetType);
	bool						isBuildingValid(UnitType targetType);
	bool						isUpgradeValid(UpgradeType upgradeType);
	bool						isTechValid(TechType techType);

	void						addScore(Unit u);
	void						onUnitDestroy(BWAPI::Unit unit);
	std::map<std::pair<BWAPI::UpgradeType, int>, std::string>& getUpgradeUnderProcess() { return upgradeUnderProcess; }
	std::map<BWAPI::TechType, std::string>& getTechProcess() { return techUnderProcess; }
	std::set<UnitType>& getResearchingBuildingType() { return researchingBuildingType; }
};
