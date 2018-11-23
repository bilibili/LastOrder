
#include "StrategyManager.h"

// constructor
StrategyManager::StrategyManager()
	: selfRace(BWAPI::Broodwar->self()->getRace())
	, enemyRace(BWAPI::Broodwar->enemy()->getRace())
{
	if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg)
	{
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Drone)); //0
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Overlord)); //1
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Hatchery)); //2
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Spawning_Pool)); //3
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Zergling)); //4
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Extractor)); //5
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Lair)); //6
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk_Den)); //7
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Spire)); //8
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk)); //9
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Mutalisk)); //10
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony)); //11
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Sunken_Colony)); //12
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Evolution_Chamber)); //13
		actions.push_back(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk_Den)); //14
		

		actions.push_back(MetaType(BWAPI::UpgradeTypes::Metabolic_Boost)); // 15
		actions.push_back(MetaType(BWAPI::UpgradeTypes::Zerg_Carapace)); // 16

		actions.push_back(MetaType(BWAPI::UpgradeTypes::Grooved_Spines)); // 17
		actions.push_back(MetaType(BWAPI::UpgradeTypes::Muscular_Augments)); // 18

		actions.push_back(MetaType(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace)); // 19
		actions.push_back(MetaType(BWAPI::UpgradeTypes::Zerg_Flyer_Attacks)); // 20

		actions.push_back(MetaType(BWAPI::UpgradeTypes::Zerg_Missile_Attacks)); // 21
		actions.push_back(MetaType(BWAPI::UpgradeTypes::Zerg_Melee_Attacks)); // 22
		actions.push_back(MetaType(BWAPI::UpgradeTypes::Zerg_Carapace)); // 23
		actions.push_back(MetaType(BWAPI::UpgradeTypes::Pneumatized_Carapace)); // 24
		actions.push_back(MetaType(BWAPI::UpgradeTypes::Ventral_Sacs)); // 25
		actions.push_back(MetaType(BWAPI::UpgradeTypes::Antennae)); // 26


		actions.push_back(MetaType(BWAPI::UpgradeTypes::Zerg_Melee_Attacks)); // 27
		actions.push_back(MetaType(BWAPI::UpgradeTypes::Zerg_Missile_Attacks)); // 28
		actions.push_back(MetaType(BWAPI::UpgradeTypes::Zerg_Flyer_Attacks)); // 29
		actions.push_back(MetaType(BWAPI::UpgradeTypes::Zerg_Carapace)); // 30
		actions.push_back(MetaType(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace)); // 31
	}

	openingStrategyName.push_back("TwelveHatchMuta");
	openingStrategyName.push_back("NinePoolling");
	openingStrategyName.push_back("TenHatchMuta");

	BuildActions = decltype(BuildActions)
	{
		//BattleUnit
		{ "Unit_Zergling", MetaType(BWAPI::UnitTypes::Zerg_Zergling)},
		{ "Unit_Hydralisk", MetaType(BWAPI::UnitTypes::Zerg_Hydralisk) },
		{ "Unit_Mutalisk", MetaType(BWAPI::UnitTypes::Zerg_Mutalisk) },
		{ "Unit_Lurker", MetaType(BWAPI::UnitTypes::Zerg_Lurker) },
		{ "Unit_Scourage", MetaType(BWAPI::UnitTypes::Zerg_Scourge) },
		{ "Unit_Ultralisk", MetaType(BWAPI::UnitTypes::Zerg_Ultralisk) },
		{ "Unit_Drone", MetaType(BWAPI::UnitTypes::Zerg_Drone) },

		//Defend
		{ "Defense_Sunken_start", MetaType(BWAPI::UnitTypes::Zerg_Sunken_Colony) },
		{ "Defense_Sunken_natural", MetaType(BWAPI::UnitTypes::Zerg_Sunken_Colony) },
		{ "Defense_Spore_start", MetaType(BWAPI::UnitTypes::Zerg_Spore_Colony) },
		{ "Defense_Spore_natural", MetaType(BWAPI::UnitTypes::Zerg_Spore_Colony) },

		//TechBuildings
		{ "Building_Chamber", MetaType(BWAPI::UnitTypes::Zerg_Evolution_Chamber) },
		{ "Building_Lair", MetaType(BWAPI::UnitTypes::Zerg_Lair) },
		{ "Building_Hive", MetaType(BWAPI::UnitTypes::Zerg_Hive) },
		{ "Building_HydraliskDen", MetaType(BWAPI::UnitTypes::Zerg_Hydralisk_Den) },
		{ "Building_QueenNest", MetaType(BWAPI::UnitTypes::Zerg_Queens_Nest) },
		{ "Building_SpawningPool", MetaType(BWAPI::UnitTypes::Zerg_Spawning_Pool) },
		{ "Building_Spire", MetaType(BWAPI::UnitTypes::Zerg_Spire) },
		{ "Building_UltraliskCavern", MetaType(BWAPI::UnitTypes::Zerg_Ultralisk_Cavern) },
		{ "Building_Hatchery", MetaType(BWAPI::UnitTypes::Zerg_Hatchery) },
		{ "Building_Extractor", MetaType(BWAPI::UnitTypes::Zerg_Extractor) },

		//TechResearch
		{ "Tech_LurkerTech", MetaType(BWAPI::TechTypes::Lurker_Aspect) },

		//Upgrade
		{ "Upgrade_ZerglingsSpeed", MetaType(BWAPI::UpgradeTypes::Metabolic_Boost) },
		{ "Upgrade_ZerglingsAttackSpeed", MetaType(BWAPI::UpgradeTypes::Adrenal_Glands) },
		{ "Upgrade_OverlordSpeed", MetaType(BWAPI::UpgradeTypes::Pneumatized_Carapace) },
		{ "Upgrade_OverlordLoad", MetaType(BWAPI::UpgradeTypes::Ventral_Sacs) },
		{ "Upgrade_OverlordSight", MetaType(BWAPI::UpgradeTypes::Antennae) },
		{ "Upgrade_HydraliskSpeed", MetaType(BWAPI::UpgradeTypes::Muscular_Augments) },
		{ "Upgrade_HydraliskRange", MetaType(BWAPI::UpgradeTypes::Grooved_Spines) },
		{ "Upgrade_UltraliskArmor", MetaType(BWAPI::UpgradeTypes::Chitinous_Plating) },
		{ "Upgrade_UltraliskSpeed", MetaType(BWAPI::UpgradeTypes::Anabolic_Synthesis) },
		{ "Upgrade_Zerg_Melee_Attacks", MetaType(BWAPI::UpgradeTypes::Zerg_Melee_Attacks) },
		{ "Upgrade_Zerg_Missile_Attacks", MetaType(BWAPI::UpgradeTypes::Zerg_Missile_Attacks) },
		{ "Upgrade_Zerg_Flyer_Attacks", MetaType(BWAPI::UpgradeTypes::Zerg_Flyer_Attacks) },
		{ "Upgrade_Zerg_Carapace", MetaType(BWAPI::UpgradeTypes::Zerg_Carapace) },
		{ "Upgrade_Zerg_Flyer_Carapace", MetaType(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace) },

		//Expand
		{ "Expand_BaseExpand", MetaType(BWAPI::UnitTypes::Zerg_Hatchery) },

		//AttackCommand
		{ "Attack_AllInAttack_start", MetaType() },
		{ "Attack_AllInAttack_natural", MetaType() },
		{ "Attack_AllInAttack_other1", MetaType() },
		{ "Attack_AllInAttack_other2", MetaType() },
		{ "Attack_AllInAttack_other3", MetaType() },

		{ "AllIn_addArmy", MetaType() },

		{ "Attack_MutaliskHarass_start", MetaType() },
		{ "Attack_MutaliskHarass_natural", MetaType() },
		{ "Attack_MutaliskHarass_other1", MetaType() },
		{ "Attack_MutaliskHarass_other2", MetaType() },
		{ "Attack_MutaliskHarass_other3", MetaType() },

		{ "Attack_Airdrop_start", MetaType() },
		{ "Attack_Airdrop_natural", MetaType() },
		{ "Attack_Airdrop_other1", MetaType() },
		{ "Attack_Airdrop_other2", MetaType() },
		{ "Attack_Airdrop_other3", MetaType() },

		{ "MutaliskHarass_addArmy", MetaType() },

		//Wait
		{ "Wait_doNothing", MetaType() }
	};

	for (auto& entry : BuildActions)
	{
		NNOutputBuildAction.push_back(entry.first);
	}
	string tmpConcat = "";
	for (size_t i = 0; i < NNOutputBuildAction.size(); i++)
	{
		buildActionsOutputIndexMapping[NNOutputBuildAction[i]] = i;
		tmpConcat += NNOutputBuildAction[i] + ":" + std::to_string(i) + "&";
	}
	//write action config file
	fstream historyFile;
	string filePath = "./bwapi-data/AI/build_action_config";
	historyFile.open(filePath.c_str(), ios::out);
	historyFile << tmpConcat << endl;
	historyFile.close();

	//state feature
	features =
	{
		//state feature
		"time",
		"enemyRace_z",
		"enemyRace_t",
		"enemyRace_p",
		"airDistance",
		"groundDistance",
		"mapWidth",
		"mapHeight",
		"mapBaseCount",
		"mapStartBaseCount",
		"ourLarvaCount",
		"ourSupplyUsed",
		"enemyEverBlockingBase",

		//tech
		"ourKeyUpgrade_LurkerResearch",
		"ourKeyUpgrade_ZerglingsAttackSpeed",
		"ourKeyUpgrade_Zerg_Melee_Attacks_level",
		"ourKeyUpgrade_Zerg_Missile_Attacks_level",
		"ourKeyUpgrade_Zerg_Flyer_Attacks_level",
		"ourKeyUpgrade_Zerg_Carapace_level",
		"ourKeyUpgrade_Zerg_Flyer_Carapace_level",

		//building
		"ourKeyBuilding_Spire",
		"ourKeyBuilding_GreaterSpire",
		"ourKeyBuilding_HydraliskDen",
		"ourKeyBuilding_QueenNest",
		"ourKeyBuilding_SpawningPool",
		"ourKeyBuilding_UltraliskCavern",
		"ourKeyBuilding_Lair",
		"ourKeyBuilding_Hive",
		"ourHatchery",
		"ourExtractor",

		"enemyKeyBuilding_hasZ_Spire",
		"enemyKeyBuilding_hasZ_hive",
		"enemyKeyBuilding_hasZ_Lair",
		"enemyKeyBuilding_hasZ_HydraliskDen",
		"enemyKeyBuilding_hasZ_QueenNest",
		"enemyKeyBuilding_hasZ_SpawningPool",

		"enemyKeyBuilding_hasT_starPort",
		"enemyKeyBuilding_hasT_engineerBay",
		"enemyKeyBuilding_hasT_Academy",
		"enemyKeyBuilding_hasT_Armory",
		"enemyKeyBuilding_hasT_scienceFacility",

		"enemyKeyBuilding_hasP_robotics_facility",
		"enemyKeyBuilding_hasP_temple",
		"enemyKeyBuilding_hasP_fleet_beacon",
		"enemyKeyBuilding_hasP_Citadel",
		"enemyKeyBuilding_hasP_cybernetics",
		"enemyKeyBuilding_hasP_forge",
		"enemyKeyBuilding_hasP_observatory",

		"enemyZ_Hatchery",
		"enemyP_Gateway",
		"enemyP_Stargate",
		"enemyT_Barracks",
		"enemyT_Factory",

		//economy
		"ourMineral",
		"ourGas",
		"ourExpandBase",
		"ourWorkers",
		"ourMineralWorkers",
		"ourMineralLeft",
		"enemyExpandBase",
		"enemyWorkers"
	};

	//waiting to build feature
	waitToBuildArmyFeature = decltype(waitToBuildArmyFeature)
	{
		//{ UnitTypes::Zerg_Drone, "waitingBuild_Drone" },
		{ UnitTypes::Zerg_Zergling, "waitingBuild_Zergling" },
		{ UnitTypes::Zerg_Mutalisk, "waitingBuild_Mutalisk" },
		{ UnitTypes::Zerg_Hydralisk, "waitingBuild_Hydra" },
		{ UnitTypes::Zerg_Lurker, "waitingBuild_Lurker" },
		{ UnitTypes::Zerg_Scourge, "waitingBuild_Scourage" },
		{ UnitTypes::Zerg_Ultralisk, "waitingBuild_Ultralisk" }
	};
	for (auto& f : waitToBuildArmyFeature)
	{
		features.push_back(f.second);
	}
	waitToBuildBuildingFeature = decltype(waitToBuildBuildingFeature)
	{
		{ UnitTypes::Zerg_Sunken_Colony, "waitingBuild_Sunken" },
		{ UnitTypes::Zerg_Spore_Colony, "waitingBuild_Spore" },
		{ UnitTypes::Zerg_Hatchery, "waitingBuild_Hatchery"},
		{ UnitTypes::Zerg_Lair, "waitingBuild_Lair"},
		{ UnitTypes::Zerg_Hive, "waitingBuild_Hive"},
		{ UnitTypes::Zerg_Hydralisk_Den, "waitingBuild_Hydralisk_Den"},
		{ UnitTypes::Zerg_Spire, "waitingBuild_Spire"}
	};
	for (auto& f : waitToBuildBuildingFeature)
	{
		features.push_back(f.second);
	}
	waitToExpandFeature = "waitingBuild_Expand";
	features.push_back(waitToExpandFeature);

	//self battle Unit
	ourBattleBasicFeature = decltype(ourBattleBasicFeature)
	{
		{ UnitTypes::Zerg_Zergling, "ourZergling" },
		{ UnitTypes::Zerg_Mutalisk, "ourMutalisk" },
		{ UnitTypes::Zerg_Hydralisk, "ourHydra" },
		{ UnitTypes::Zerg_Lurker, "ourLurker" },
		{ UnitTypes::Zerg_Scourge, "ourScourage" },
		{ UnitTypes::Zerg_Ultralisk, "ourUltralisk" },
		{ UnitTypes::Zerg_Overlord, "ourOverLord" },
		{ UnitTypes::Zerg_Drone, "ourDrone" }
	};
	//our total army
	for (auto& f : ourBattleBasicFeature)
	{
		features.push_back(f.second);
	}
	ourDefendBuildingFeature = decltype(ourDefendBuildingFeature)
	{
		{ UnitTypes::Zerg_Sunken_Colony, "ourSunken" },
		{ UnitTypes::Zerg_Spore_Colony, "ourSpore" }
	};

	//our has been killed info
	for (auto& f : ourBattleBasicFeature)
	{
		features.push_back("killed_" + f.second);
	}
	for (auto& f : ourDefendBuildingFeature)
	{
		features.push_back("killed_" + f.second);
	}

	//our base defend info
	for (std::string prefix : {"ourStart", "ourNatural", "ourOther1", "ourOther2", "ourOther3"})
	{
		for (auto& f : ourDefendBuildingFeature)
		{
			features.push_back(prefix + "_" + f.second);
		}
		features.push_back(prefix + "_" + "PositionX");
		features.push_back(prefix + "_" + "PositionY");
	}

	//our idle army
	for (auto& f : ourBattleBasicFeature)
	{
		features.push_back("idle_" + f.second);
	}

	//enemy battle unit
	enemyBattleBasicFeature = decltype(enemyBattleBasicFeature)
	{
		{ UnitTypes::Protoss_Probe, "enemyP_Probe" },
		{ UnitTypes::Terran_SCV, "enemyT_scv" },
		{ UnitTypes::Zerg_Drone, "enemyZ_drone" },

		{ UnitTypes::Zerg_Zergling, "enemyZ_Zergling" },
		{ UnitTypes::Zerg_Mutalisk, "enemyZ_Mutalisk" },
		{ UnitTypes::Zerg_Hydralisk, "enemyZ_Hydra" },
		{ UnitTypes::Zerg_Lurker, "enemyZ_Lurker" },
		{ UnitTypes::Zerg_Scourge, "enemyZ_Scourage" },
		{ UnitTypes::Zerg_Ultralisk, "enemyZ_Ultralisk" },
		{ UnitTypes::Zerg_Defiler, "enemyZ_Defiler" },
		{ UnitTypes::Zerg_Overlord, "enemyZ_Overlord" },

		{ UnitTypes::Protoss_Zealot, "enemyP_Zealot" },
		{ UnitTypes::Protoss_Dragoon, "enemyP_Dragon" },
		{ UnitTypes::Protoss_High_Templar, "enemyP_High_templar" },
		{ UnitTypes::Protoss_Carrier, "enemyP_Carrier" },
		{ UnitTypes::Protoss_Corsair, "enemyP_Corsair" },
		{ UnitTypes::Protoss_Shuttle, "enemyP_Shuttle" },
		{ UnitTypes::Protoss_Archon, "enemyP_Archon" },
		{ UnitTypes::Protoss_Dark_Templar, "enemyP_Dark_templar" },
		{ UnitTypes::Protoss_Scout, "enemyP_scout" },

		{ UnitTypes::Terran_Goliath, "enemyT_Goliath" },
		{ UnitTypes::Terran_Marine, "enemyT_Marine" },
		{ UnitTypes::Terran_Siege_Tank_Tank_Mode, "enemyT_Tank" },
		{ UnitTypes::Terran_Vulture, "enemyT_Vulture" },
		{ UnitTypes::Terran_Dropship, "enemyT_Dropship" },
		{ UnitTypes::Terran_Valkyrie, "enemyT_Valkyrie" },
		{ UnitTypes::Terran_Science_Vessel, "enemyT_Science" },
		{ UnitTypes::Terran_Firebat, "enemyT_Firebat" },
		{ UnitTypes::Terran_Medic, "enemyT_Terran_Medic" },
		{ UnitTypes::Terran_Wraith, "enemyT_Terran_wriath" }
	};
	for (auto& f : enemyBattleBasicFeature)
	{
		features.push_back(f.second);
	}

	enemyDefendBuildingFeature = decltype(enemyDefendBuildingFeature)
	{
		{ UnitTypes::Protoss_Photon_Cannon, "enemyP_cannon" },
		{ UnitTypes::Terran_Missile_Turret, "enemyT_missile" },
		{ UnitTypes::Terran_Bunker, "enemyT_Bunker" },
		{ UnitTypes::Zerg_Sunken_Colony, "enemyZ_Sunken" },
		{ UnitTypes::Zerg_Spore_Colony, "enemyZ_Spore" }
	};

	//our kill enemy count
	for (auto& f : enemyBattleBasicFeature)
	{
		features.push_back("kill_" + f.second);
	}
	for (auto& f : enemyDefendBuildingFeature)
	{
		features.push_back("kill_" + f.second);
	}

	//our attack info
	for (std::string prefix : {"MutaliskHarassAttac", "AllInAttack", "Airdrop"})
	{
		for (auto& f : ourBattleBasicFeature)
		{
			features.push_back(prefix + "_" + f.second);
		}
		//our attack tactic's destination
		for (std::string b : {"enemyStart", "enemyNatural", "enemyOther1", "enemyOther2", "enemyOther3"})
		{
			features.push_back(prefix + "_" + b);
		}
		for (auto& f : enemyBattleBasicFeature)
		{
			features.push_back(prefix + "_" + f.second);
		}
		for (auto& f : enemyDefendBuildingFeature)
		{
			features.push_back(prefix + "_" + f.second);
		}
		features.push_back(prefix + "_" + "ArmyPositionX");
		features.push_back(prefix + "_" + "ArmyPositionY");
	}

	//our defend info
	for (std::string prefix : {"Defend01", "Defend02"})
	{
		for (auto& f : ourBattleBasicFeature)
		{
			features.push_back(prefix + "_" + f.second);
		}
		//our defend tactic's destination
		for (std::string b : {"ourStart", "ourNatural", "ourOther1", "ourOther2", "ourOther3"})
		{
			features.push_back(prefix + "_" + b);
		}
		for (auto& f : enemyBattleBasicFeature)
		{
			features.push_back(prefix + "_" + f.second);
		}
		for (auto& f : enemyDefendBuildingFeature)
		{
			features.push_back(prefix + "_" + f.second);
		}
		features.push_back(prefix + "_" + "ArmyPositionX");
		features.push_back(prefix + "_" + "ArmyPositionY");
	}

	//enemy base defend info
	for (std::string prefix : {"enemyStart", "enemyNatural", "enemyOther1", "enemyOther2", "enemyOther3"})
	{
		for (auto& f : enemyDefendBuildingFeature)
		{
			features.push_back(prefix + "_" + f.second);
		}
		features.push_back(prefix + "_" + "PositionX");
		features.push_back(prefix + "_" + "PositionY");
	}

	//current visiable enemy army info
	string prefix = "visible";
	for (auto& f : enemyBattleBasicFeature)
	{
		features.push_back(prefix + "_" + f.second);
	}
	features.push_back(prefix + "_" + "PositionX");
	features.push_back(prefix + "_" + "PositionY");

	tmpConcat = "";
	for (size_t i = 0; i < features.size(); i++)
	{
		tmpConcat += features[i] + ":" + std::to_string(i) + "&";
	}
	//write action config file
	filePath = "./bwapi-data/AI/feature_config";
	historyFile.open(filePath.c_str(), ios::out);
	historyFile << tmpConcat << endl;
	historyFile.close();

	NNInputAction = features;
	
	context = new zmq::context_t(1);
	modelTrigger = new zmq::socket_t(*context, ZMQ_PUSH);
	modelTrigger->connect("tcp://127.0.0.1:5555");
	modelResultReceiver = new zmq::socket_t(*context, ZMQ_PULL);
	modelResultReceiver->connect("tcp://127.0.0.1:5556");

	pollItems[0].socket = *modelResultReceiver;
	pollItems[0].events = ZMQ_POLLIN;
	pollItems[0].fd = 0;
	pollItems[0].revents = 0;
	
	isRequestBuildModel = false;
	isTerminated = false;

	startRequestBuildModelFrame = 0;
	hasSendBlockMessage = false;

	previousStateAccumulatedReward = 0;
	productionState = fixBuildingOrder;
	HatcheryProductionCheckTime = 0;
	extractorProductionCheckTime = 0;
	droneProductionSpeed = 24 * 5;
	nextBuildCheckTime = 0;
	nextAttackCheckTime = 0;

	isExtractTrickFinish = false;
	nextDroneBuildTime = 0;

	std::string source = "22,22,22,22";
	vector<std::string> result = splitStr(source, ',');
	for (auto s : result)
	{
		testActions.push_back(std::stoi(s));
	}

	selfLostScore = 0;
	enemyLostScore = 0;
	waitToExpand.first = 1;
	nextLogFeatureTime = 0;
	nextStateUpdateTime = 0;
}


void StrategyManager::setOpeningStrategy(openingStrategy opening)
{
	currentopeningStrategy = opening;
	ProductionManager::Instance().setBuildOrder(getOpeningBook(), true);
}

int StrategyManager::getStrategyByName(std::string strategy)
{
	for (int i = 0; i <int(openingStrategyName.size()); i++)
	{
		if (openingStrategyName[i] == strategy)
		{
			return i;
		}
	}
	return -1;
}

std::string	StrategyManager::getStrategyName(openingStrategy strategy)
{
	return openingStrategyName[int(strategy)];
}


void StrategyManager::calCurrentStateFeature()
{
	//state general
	int curTimeMinuts = BWAPI::Broodwar->getFrameCount() / (24 * 60);
	featureValues["time"] = curTimeMinuts;
	BWAPI::UnitType enemyWork = BWAPI::UnitTypes::None;
	int featureCount = 0;

	std::string enemyRaceName;
	if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Zerg)
	{
		enemyRaceName = "enemyRace_z";
		featureValues["enemyRace_z"] = 1;
		enemyWork = BWAPI::UnitTypes::Zerg_Drone;
	}
	else if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Terran)
	{
		enemyRaceName = "enemyRace_t";
		featureValues["enemyRace_t"] = 1;
		enemyWork = BWAPI::UnitTypes::Terran_SCV;
	}
	else if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Protoss)
	{
		enemyRaceName = "enemyRace_p";
		featureValues["enemyRace_p"] = 1;
		enemyWork = BWAPI::UnitTypes::Protoss_Probe;
	}
	
	int supplyUsed = BWAPI::Broodwar->self()->supplyUsed() / 2;
	featureValues["ourSupplyUsed"] = supplyUsed;

	std::map<const Area*, int>& baseGroundDistanceInfo = InformationManager::Instance().getBaseGroudDistance();
	std::map<const Area*, int>& baseAirDistanceInfo = InformationManager::Instance().getBaseAirDistance();
	if (InformationManager::Instance().GetEnemyBasePosition() != BWAPI::Positions::None)
	{
		int airDistance = baseAirDistanceInfo[BWEMMap.GetArea(TilePosition(InformationManager::Instance().GetEnemyBasePosition()))];
		featureValues["airDistance"] = airDistance;
		int groundDistance = baseGroundDistanceInfo[BWEMMap.GetArea(TilePosition(InformationManager::Instance().GetEnemyBasePosition()))];
		featureValues["groundDistance"] = groundDistance;
	}
	featureValues["mapWidth"] = Broodwar->mapWidth();
	featureValues["mapHeight"] = Broodwar->mapHeight();
	featureValues["enemyEverBlockingBase"] = int(InformationManager::Instance().getEnemyEverBlockingHome());
	
	int totalBases = 0;
	for (auto& area : BWEMMap.Areas())
	{
		totalBases += area.Bases().size();
	}
	featureValues["mapBaseCount"] = totalBases;
	featureValues["mapStartBaseCount"] = Broodwar->getStartLocations().size();
	int ourLarvaCount = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Larva);
	featureValues["ourLarvaCount"] = ourLarvaCount;

	//state tech
	if (BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Lurker_Aspect))
		featureValues["ourKeyUpgrade_LurkerResearch"] = 1;
	if (BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Adrenal_Glands) > 0)
		featureValues["ourKeyUpgrade_ZerglingsAttackSpeed"] = 1;
	featureValues["ourKeyUpgrade_Zerg_Melee_Attacks_level"] = BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Melee_Attacks);
	featureValues["ourKeyUpgrade_Zerg_Missile_Attacks_level"] = BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Missile_Attacks);
	featureValues["ourKeyUpgrade_Zerg_Flyer_Attacks_level"] = BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Flyer_Attacks);
	featureValues["ourKeyUpgrade_Zerg_Carapace_level"] = BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Carapace);
	featureValues["ourKeyUpgrade_Zerg_Flyer_Carapace_level"] = BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace);

	//state buildings
	//self buildings
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& ourBuildings = InformationManager::Instance().getOurAllBuildingUnit();
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Spire) > 0)
		featureValues["ourKeyBuilding_Spire"] = 1;
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Greater_Spire) > 0)
	{
		featureValues["ourKeyBuilding_GreaterSpire"] = 1;
		featureValues["ourKeyBuilding_Spire"] = 1;
	}
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Hydralisk_Den) > 0)
		featureValues["ourKeyBuilding_HydraliskDen"] = 1;
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Queens_Nest) > 0)
		featureValues["ourKeyBuilding_QueenNest"] = 1;
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Spawning_Pool) > 0)
		featureValues["ourKeyBuilding_SpawningPool"] = 1;
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Ultralisk_Cavern) > 0)
		featureValues["ourKeyBuilding_UltraliskCavern"] = 1;
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Lair) > 0)
		featureValues["ourKeyBuilding_Lair"] = 1;
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Hive) > 0)
	{
		featureValues["ourKeyBuilding_Hive"] = 1;
		featureValues["ourKeyBuilding_Lair"] = 1;
	}
	int hatcheryCount = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Hatchery) +
		BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Lair) +
		BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Hive);
	featureValues["ourHatchery"] = hatcheryCount;
	featureValues["ourExtractor"] = BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Extractor);
	
	//enemy buildings
	std::map<BWAPI::UnitType, map<Unit, buildingInfo>>& enemyBuildings = InformationManager::Instance().getEnemyAllBuildingUnit();
	if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Zerg)
	{
		int enemyHatcheryCount = 0;
		if (enemyBuildings.find(BWAPI::UnitTypes::Zerg_Spire) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Zerg_Spire].size() > 0)
			featureValues["enemyKeyBuilding_hasZ_Spire"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Zerg_Hive) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Zerg_Hive].size() > 0)
		{
			featureValues["enemyKeyBuilding_hasZ_hive"] = 1;
			featureValues["enemyKeyBuilding_hasZ_Lair"] = 1;
			enemyHatcheryCount += enemyBuildings[BWAPI::UnitTypes::Zerg_Hive].size();
		}
		if (enemyBuildings.find(BWAPI::UnitTypes::Zerg_Lair) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Zerg_Lair].size() > 0)
		{
			featureValues["enemyKeyBuilding_hasZ_Lair"] = 1;
			enemyHatcheryCount += enemyBuildings[BWAPI::UnitTypes::Zerg_Lair].size();
		}
		if (enemyBuildings.find(BWAPI::UnitTypes::Zerg_Hydralisk_Den) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Zerg_Hydralisk_Den].size() > 0)
			featureValues["enemyKeyBuilding_hasZ_HydraliskDen"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Zerg_Queens_Nest) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Zerg_Queens_Nest].size() > 0)
			featureValues["enemyKeyBuilding_hasZ_QueenNest"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Zerg_Spawning_Pool) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Zerg_Spawning_Pool].size() > 0)
			featureValues["enemyKeyBuilding_hasZ_SpawningPool"] = 1;

		if (enemyBuildings.find(BWAPI::UnitTypes::Zerg_Hatchery) != enemyBuildings.end())
			enemyHatcheryCount += enemyBuildings[BWAPI::UnitTypes::Zerg_Hatchery].size();
		featureValues["enemyZ_Hatchery"] = enemyHatcheryCount;

	}
	else if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Terran)
	{
		if (enemyBuildings.find(BWAPI::UnitTypes::Terran_Starport) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Terran_Starport].size() > 0)
			featureValues["enemyKeyBuilding_hasT_starPort"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Terran_Engineering_Bay) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Terran_Engineering_Bay].size() > 0)
			featureValues["enemyKeyBuilding_hasT_engineerBay"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Terran_Academy) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Terran_Academy].size() > 0)
			featureValues["enemyKeyBuilding_hasT_Academy"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Terran_Armory) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Terran_Armory].size() > 0)
			featureValues["enemyKeyBuilding_hasT_Armory"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Terran_Science_Facility) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Terran_Science_Facility].size() > 0)
			featureValues["enemyKeyBuilding_hasT_scienceFacility"] = 1;

		featureCount = enemyBuildings.find(BWAPI::UnitTypes::Terran_Barracks) != enemyBuildings.end() ? enemyBuildings[BWAPI::UnitTypes::Terran_Barracks].size() : 0;
		featureValues["enemyT_Barracks"] = featureCount;

		featureCount = enemyBuildings.find(BWAPI::UnitTypes::Terran_Factory) != enemyBuildings.end() ? enemyBuildings[BWAPI::UnitTypes::Terran_Factory].size() : 0;
		featureValues["enemyT_Factory"] = featureCount;
	}
	else if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Protoss)
	{
		if (enemyBuildings.find(BWAPI::UnitTypes::Protoss_Robotics_Facility) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Protoss_Robotics_Facility].size() > 0)
			featureValues["enemyKeyBuilding_hasP_robotics_facility"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Protoss_Templar_Archives) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Protoss_Templar_Archives].size() > 0)
			featureValues["enemyKeyBuilding_hasP_temple"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Protoss_Fleet_Beacon) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Protoss_Fleet_Beacon].size() > 0)
			featureValues["enemyKeyBuilding_hasP_fleet_beacon"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Protoss_Citadel_of_Adun) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Protoss_Citadel_of_Adun].size() > 0)
			featureValues["enemyKeyBuilding_hasP_Citadel"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Protoss_Cybernetics_Core) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Protoss_Cybernetics_Core].size() > 0)
			featureValues["enemyKeyBuilding_hasP_cybernetics"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Protoss_Forge) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Protoss_Forge].size() > 0)
			featureValues["enemyKeyBuilding_hasP_forge"] = 1;
		if (enemyBuildings.find(BWAPI::UnitTypes::Protoss_Observatory) != enemyBuildings.end() && enemyBuildings[BWAPI::UnitTypes::Protoss_Observatory].size() > 0)
			featureValues["enemyKeyBuilding_hasP_observatory"] = 1;

		featureCount = enemyBuildings.find(BWAPI::UnitTypes::Protoss_Stargate) != enemyBuildings.end() ? enemyBuildings[BWAPI::UnitTypes::Protoss_Stargate].size() : 0;
		featureValues["enemyP_Stargate"] = featureCount;
		featureCount = enemyBuildings.find(BWAPI::UnitTypes::Protoss_Gateway) != enemyBuildings.end() ? enemyBuildings[BWAPI::UnitTypes::Protoss_Gateway].size() : 0;
		featureValues["enemyP_Gateway"] = featureCount;
	}

	//state economy
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& ourUnits = InformationManager::Instance().getOurAllBattleUnit();
	std::map<BWAPI::UnitType, map<Unit, unitInfo>>& enemyUnits = InformationManager::Instance().getEnemyAllBattleUnit();

	featureValues["ourMineral"] = BWAPI::Broodwar->self()->minerals();
	featureValues["ourGas"] = BWAPI::Broodwar->self()->gas();
	featureValues["ourWorkers"] = ourUnits[BWAPI::UnitTypes::Zerg_Drone].size();
	int enemyWorkerCount = enemyUnits.find(enemyWork) != enemyUnits.end() ? enemyUnits[enemyWork].size() : 0;
	featureValues["enemyWorkers"] = enemyWorkerCount;

	std::map<const Area*, TilePosition> & ourBases = InformationManager::Instance().getBaseOccupiedRegions(Broodwar->self());
	std::map<const Area*, TilePosition> & enemyBases = InformationManager::Instance().getBaseOccupiedRegions(Broodwar->enemy());

	featureValues["ourExpandBase"] = ourBases.size() - 1;
	featureValues["enemyExpandBase"] = enemyBases.size() - 1;
	int mineralWorkerCount = WorkerManager::Instance().getNumMineralWorkers();
	featureValues["ourMineralWorkers"] = mineralWorkerCount;
	int mineralLeft = 0;
	for (auto const& r : ourBases)
	{
		mineralLeft += r.first->Minerals().size();
	}
	featureValues["ourMineralLeft"] = mineralLeft;

	//state battle
	const Area* baseArea = BWEMMap.GetArea(InformationManager::Instance().getOurBaseLocation());
	const Area* natrualArea = BWEMMap.GetArea(InformationManager::Instance().getOurNatrualLocation());
	const Area* enemyBaseArea = NULL;
	const Area* enemyNatrualArea = NULL;
	if (InformationManager::Instance().GetEnemyBasePosition() != Positions::None)
	{
		enemyBaseArea = BWEMMap.GetArea(TilePosition(InformationManager::Instance().GetEnemyBasePosition()));
		enemyNatrualArea = BWEMMap.GetArea(TilePosition(InformationManager::Instance().GetEnemyNaturalPosition()));
	}

	vector<std::pair<const Area*, int>> ourBaseDistance;
	map<const Area*, Position> ourBaseToPosition;
	for (auto& b : ourBases)
	{
		logInfo("StrategyManager", "our has " + to_string(b.second.x) + " " + to_string(b.second.y));

		ourBaseToPosition[b.first] = Position(b.second);
		int pathLong = 0;
		BWEMMap.GetPath(Position(b.second), Position(Broodwar->self()->getStartLocation()), &pathLong);
		ourBaseDistance.push_back(std::pair<const Area*, int>(b.first, pathLong));
	}
	std::sort(ourBaseDistance.begin(), ourBaseDistance.end(),
		[](const std::pair<const Area*, int>& a, const std::pair<const Area*, int>& b)
	{
		return a.second < b.second;
	});

	ourBaseNameToPosition.clear();
	map<const Area*, std::string> ourBaseName;
	int count = 1;
	for (auto i = 0; i < int(ourBaseDistance.size()); i++)
	{
		if (ourBaseDistance[i].first == baseArea)
		{
			ourBaseName[baseArea] = "ourStart";
			ourBaseNameToPosition["start"] = ourBaseDistance[i].first;
		}
		else if (ourBaseDistance[i].first == natrualArea)
		{
			ourBaseName[natrualArea] = "ourNatural";
			ourBaseNameToPosition["natural"] = ourBaseDistance[i].first;
		}
		else
		{
			if (count >= 4)
				continue;
			ourBaseNameToPosition["other" + std::to_string(int(count))] = ourBaseDistance[i].first;
			ourBaseName[ourBaseDistance[i].first] = "ourOther" + std::to_string(int(count));
			count += 1;
		}
	}

	vector<std::pair<const Area*, int>> enemyBaseDistance;
	map<const Area*, Position> baseToPosition;
	//enemy base area
	for (auto& b : enemyBases)
	{
		logInfo("StrategyManager", "enemyBase has " + to_string(b.second.x) + " " + to_string(b.second.y));

		int pathLong = 0;
		int airPathLong = 0;
		Position attackPosition = Position(enemyBases[b.first]);
		BWEMMap.GetPath(attackPosition, Position(Broodwar->self()->getStartLocation()), &pathLong);
		baseToPosition[b.first] = attackPosition;
		enemyBaseDistance.push_back(std::pair<const Area*, int>(b.first, pathLong));
	}
	//other area
	std::map<const Area*, std::map<BWAPI::Unit, buildingInfo>>& enemyAllArea = InformationManager::Instance().getEnemyOccupiedDetail();
	for (auto& b : enemyAllArea)
	{
		if (enemyBases.find(b.first) == enemyBases.end())
		{
			int pathLong = 0;
			int airPathLong = 0;
			Position attackPosition = Position((*b.second.begin()).second.initPosition);
			BWEMMap.GetPath(attackPosition, Position(Broodwar->self()->getStartLocation()), &pathLong);
			//Area do not has base has lower priority
			pathLong = pathLong * 10;
			baseToPosition[b.first] = attackPosition;
			enemyBaseDistance.push_back(std::pair<const Area*, int>(b.first, pathLong));
		}
	}
	std::sort(enemyBaseDistance.begin(), enemyBaseDistance.end(),
		[](const std::pair<const Area*, int>& a, const std::pair<const Area*, int>& b)
	{
		return a.second < b.second;
	});
	map<const Area*, std::string> enemyBaseName;
	enemyBaseNameToPosition.clear();
	if (InformationManager::Instance().GetEnemyBasePosition() != Positions::None)
	{
		int count = 1;
		for (auto i = 0; i < int(enemyBaseDistance.size()); i++)
		{
			if (enemyBaseDistance[i].first == enemyBaseArea)
			{
				enemyBaseName[enemyBaseArea] = "enemyStart";
				enemyBaseNameToPosition["start"] = InformationManager::Instance().GetEnemyBasePosition();
			}
			else if (enemyBaseDistance[i].first == enemyNatrualArea)
			{
				enemyBaseName[enemyNatrualArea] = "enemyNatural";
				enemyBaseNameToPosition["natural"] = InformationManager::Instance().GetEnemyNaturalPosition();
			}
			else
			{
				if (count >= 4)
					continue;
				enemyBaseName[enemyBaseDistance[i].first] = "enemyOther" + std::to_string(count);
				enemyBaseNameToPosition["other" + std::to_string(count)] = baseToPosition[enemyBaseDistance[i].first];
				count++;
			}
		}
	}

	//our total army
	for (auto& army : ourUnits)
	{
		if (ourBattleBasicFeature.find(army.first) != ourBattleBasicFeature.end())
		{
			featureValues[ourBattleBasicFeature[army.first]] = army.second.size();
		}
	}
	//our idle army
	std::map<BWAPI::UnitType, BattleArmy*>& ourIdleArmy = AttackManager::Instance().getIdelArmy();
	for (auto& army : ourIdleArmy)
	{
		std::string prefix = "idle_";
		if (ourBattleBasicFeature.find(army.first) != ourBattleBasicFeature.end())
		{
			featureValues[prefix + ourBattleBasicFeature[army.first]] = army.second->getUnits().size();
		}
	}

	//waiting to build feature
	for (auto& u : unitWaitToBuild)
	{
		if (waitToBuildArmyFeature.find(u.first) != waitToBuildArmyFeature.end())
		{
			featureValues[waitToBuildArmyFeature[u.first]] = u.second;
		}
	}
	for (auto& u : buildingsUnderProcess)
	{
		if (waitToBuildBuildingFeature.find(u.first) != waitToBuildBuildingFeature.end())
		{
			featureValues[waitToBuildBuildingFeature[u.first]] = u.second.first;
		}
	}
	featureValues[waitToExpandFeature] = waitToExpand.first;

	//kill enemy info
	map<BWAPI::UnitType, int>& killedEnemy = InformationManager::Instance().getKilledEnemyInfo();
	for (auto& kE : killedEnemy)
	{
		if (enemyBattleBasicFeature.find(kE.first) != enemyBattleBasicFeature.end())
		{
			featureValues["kill_" + enemyBattleBasicFeature[kE.first]] = kE.second;
		}
		else if (enemyDefendBuildingFeature.find(kE.first) != enemyDefendBuildingFeature.end())
		{
			featureValues["kill_" + enemyDefendBuildingFeature[kE.first]] = kE.second;
		}
	}

	//our killed info
	map<BWAPI::UnitType, int>& ourKilled = InformationManager::Instance().getKilledOurInfo();
	for (auto& o : ourKilled)
	{
		if (ourBattleBasicFeature.find(o.first) != ourBattleBasicFeature.end())
		{
			featureValues["killed_" + ourBattleBasicFeature[o.first]] = o.second;
		}
		else if (ourDefendBuildingFeature.find(o.first) != ourDefendBuildingFeature.end())
		{
			featureValues["killed_" + ourDefendBuildingFeature[o.first]] = o.second;
		}
	}

	//our base defend info
	std::map<const Area*, std::map<BWAPI::Unit, buildingInfo>>& ourBuildingDetail = InformationManager::Instance().getSelfOccupiedDetail();
	for (auto& b : ourBaseName)
	{
		string prefix = b.second;
		if (ourBuildingDetail.find(b.first) != ourBuildingDetail.end())
		{
			for (auto& u : ourBuildingDetail[b.first])
			{
				if (ourDefendBuildingFeature.find(u.first->getType()) != ourDefendBuildingFeature.end())
				{
					featureValues[prefix + "_" + ourDefendBuildingFeature[u.first->getType()]] += 1;
				}
			}
		}
		featureValues[prefix + "_PositionX"] = TilePosition(ourBaseToPosition[b.first]).x;
		featureValues[prefix + "_PositionY"] = TilePosition(ourBaseToPosition[b.first]).y;
	}

	//our attack info
	map<tacKey, BattleTactic*>& allTactics = TacticManager::Instance().getAllTactic();
	int defendCount = 0;
	for (const auto& t : allTactics)
	{
		if (t.first.tacName != HydraliskPushTactic 
			&& t.first.tacName != MutaliskHarassTac
			&& t.first.tacName != AirdropTac
			&& t.first.tacName != DefendTactic)
			continue;

		const Area*	attackArea = t.second->getAttackRegion();
		map<UnitType, set<Unit>>& enemyArmy = t.second->getEncounteredEnemy();
		TilePosition avgPosition = t.second->getArmyAvgPosition();

		std::string prefix = "";
		if (t.first.tacName == HydraliskPushTactic)
		{
			prefix = "AllInAttack";
			if (enemyBaseName.find(attackArea) != enemyBaseName.end())
				featureValues[prefix + "_" + enemyBaseName[attackArea]] = 1;
		}
		else if (t.first.tacName == MutaliskHarassTac)
		{
			prefix = "MutaliskHarassAttac";
			if (enemyBaseName.find(attackArea) != enemyBaseName.end())
				featureValues[prefix + "_" + enemyBaseName[attackArea]] = 1;
		}
		else if (t.first.tacName == AirdropTac)
		{
			prefix = "Airdrop";
			if (enemyBaseName.find(attackArea) != enemyBaseName.end())
				featureValues[prefix + "_" + enemyBaseName[attackArea]] = 1;
		}
		else if (t.first.tacName == DefendTactic)
		{
			defendCount += 1;
			if (defendCount > 2)
				continue;
			prefix = "Defend0" + std::to_string(defendCount);
			if (ourBaseName.find(attackArea) != ourBaseName.end())
			{
				featureValues[prefix + "_" + ourBaseName[attackArea]] = 1;
			}
		}
		featureValues[prefix + "_ArmyPositionX"] = avgPosition.x;
		featureValues[prefix + "_ArmyPositionY"] = avgPosition.y;
		
		for (const auto& army : t.second->getArmy())
		{
			if (!army.second->getUnits().empty()
				&& ourBattleBasicFeature.find(army.first) != ourBattleBasicFeature.end())
			{
				featureValues[prefix + "_" + ourBattleBasicFeature[army.first]] = army.second->getUnits().size();
			}
		}
		for (auto& army : enemyArmy)
		{
			if (army.second.size() == 0)
				continue;
			if (enemyBattleBasicFeature.find(army.first) != enemyBattleBasicFeature.end())
			{
				featureValues[prefix + "_" + enemyBattleBasicFeature[army.first]] = army.second.size();
			}
			else if (enemyDefendBuildingFeature.find(army.first) != enemyDefendBuildingFeature.end())
			{
				featureValues[prefix + "_" + enemyDefendBuildingFeature[army.first]] = army.second.size();
			}
		}
	}

	//enemy battle unit
	for (auto& army : enemyUnits)
	{
		if (enemyBattleBasicFeature.find(army.first) != enemyBattleBasicFeature.end())
		{
			featureValues[enemyBattleBasicFeature[army.first]] = army.second.size();
		}
	}

	//enemy base defend info
	std::map<const Area*, std::map<BWAPI::Unit, buildingInfo>>& enemyAreaDetail = InformationManager::Instance().getEnemyOccupiedDetail();
	for (const auto& b : enemyBaseName)
	{
		std::string prefix = b.second;
		if (enemyAreaDetail.find(b.first) != enemyAreaDetail.end())
		{
			for (const auto& u : enemyAreaDetail[b.first])
			{
				if (enemyDefendBuildingFeature.find(u.second.unitType) != enemyDefendBuildingFeature.end())
				{
					featureValues[prefix + "_" + enemyDefendBuildingFeature[u.second.unitType]] += 1;
				}
			}
		}
		featureValues[prefix + "_PositionX"] = TilePosition(baseToPosition[b.first]).x;
		featureValues[prefix + "_PositionY"] = TilePosition(baseToPosition[b.first]).y;
	}

	//current visible enemy army
	string prefix = "visible_";
	int curFrame = Broodwar->getFrameCount();
	int unitCount = 0;
	int avgX = 0;
	int avgY = 0;
	for (auto& army : enemyUnits)
	{
		for (auto& unit : army.second)
		{
			//unit's position is valid while last update frame less than 10 seconds
			if (curFrame - unit.second.latestUpdateFrame < 24 * 10)
			{
				if (enemyBattleBasicFeature.find(army.first) != enemyBattleBasicFeature.end())
				{
					if (army.first.isWorker())
					{
						continue;
					}
					featureValues[prefix + enemyBattleBasicFeature[army.first]] += 1;
					unitCount += 1;
					avgX += unit.second.latestPosition.x;
					avgY += unit.second.latestPosition.y;
				}
			}
		}
	}
	if (unitCount > 0)
	{
		featureValues[prefix + "PositionX"] = avgX / unitCount;
		featureValues[prefix + "PositionY"] = avgY / unitCount;
	}
}


void StrategyManager::buildingFinish(BWAPI::UnitType u)
{
	if (buildingsUnderProcess.find(u) != buildingsUnderProcess.end())
	{
		buildingsUnderProcess[u].first = buildingsUnderProcess[u].first - 1;
		logInfo("StrategyManager", to_string(u) + " buildingFinish. remain count " + to_string(buildingsUnderProcess[u].first));
		if (buildingsUnderProcess[u].first == 0)
		{
			buildingsUnderProcess.erase(u);
		}
	}
}


void StrategyManager::unitMorphingFinish(BWAPI::UnitType u)
{
	if (unitWaitToBuild.find(u) != unitWaitToBuild.end())
	{
		int checkFrame = Broodwar->getFrameCount() + 24 * 2;
		const std::function<void(BWAPI::Game*)> productionAction = [=](BWAPI::Game* g)->void
		{
			unitWaitToBuild[u] -= 1;
			logInfo("StrategyManager", to_string(u) + " morphingFinish. remain count " + to_string(unitWaitToBuild[u]));
		};

		const std::function<bool(BWAPI::Game*)> productionCondition = [=](BWAPI::Game* g)->bool
		{
			if (Broodwar->getFrameCount() > checkFrame)
				return true;
			else
				return false;
		};
		BWAPI::Broodwar->registerEvent(productionAction, productionCondition, 1, 10);

	}
}


bool StrategyManager::isUnitValid(UnitType targetType)
{
	if (!WorkerManager::Instance().isGasRequireMeet(targetType.gasPrice()) ||
		!WorkerManager::Instance().isMineRequireMeet(targetType.mineralPrice()))
	{
		return false;
	}

	int supplyRequired = targetType.supplyRequired();
	if (targetType == BWAPI::UnitTypes::Zerg_Zergling
		|| targetType == BWAPI::UnitTypes::Zerg_Scourge)
	{
		supplyRequired = supplyRequired * 2;
	}
	if (BWAPI::Broodwar->self()->supplyUsed() + supplyRequired > 200 * 2)
	{
		return false;
	}

	if (targetType == UnitTypes::Zerg_Drone)
	{
		bool isWorkerNotFull = WorkerManager::Instance().isAllDepotFull();
		//keep the drone production interval the same as one base's larva production rate
		if (!isWorkerNotFull && BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Drone) <= 70)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	std::map<BWAPI::UnitType, int> requireUnits = targetType.requiredUnits();
	for (auto u : requireUnits)
	{
		if (u.first != BWAPI::UnitTypes::Zerg_Larva && BWAPI::Broodwar->self()->completedUnitCount(u.first) == 0)
		{
			if (u.first == BWAPI::UnitTypes::Zerg_Spire && BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Greater_Spire) > 0)
			{
				continue;
			}
			if (u.first == BWAPI::UnitTypes::Zerg_Lair && BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Hive) > 0)
			{
				continue;
			}
			return false;
		}
	}

	if (targetType == UnitTypes::Zerg_Lurker && !BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Lurker_Aspect))
	{
		return false;
	}
	if (targetType == UnitTypes::Zerg_Lurker && Broodwar->self()->allUnitCount(UnitTypes::Zerg_Hydralisk) == 0)
	{
		return false;
	}

	if (targetType == UnitTypes::Zerg_Scourge) {
		int curScourgeCount = 0;
		curScourgeCount += Broodwar->self()->allUnitCount(UnitTypes::Zerg_Scourge);  // current scourge
		curScourgeCount += unitWaitToBuild[UnitTypes::Zerg_Scourge] * 2;  // morphing scourge
		if (curScourgeCount > 60) {
			return false;
		}
	}
	return true;
}


bool StrategyManager::isUpgradeValid(UpgradeType upgradeType)
{
	int next_level = BWAPI::Broodwar->self()->getUpgradeLevel(upgradeType) + 1;
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& ourBuildings = InformationManager::Instance().getOurAllBuildingUnit();
	if (!WorkerManager::Instance().isGasRequireMeet(upgradeType.gasPrice(next_level)) ||
		!WorkerManager::Instance().isMineRequireMeet(upgradeType.mineralPrice(next_level)))
	{
		return false;
	}

	BWAPI::UnitType researchingUnit = upgradeType.whatUpgrades();
	BWAPI::UnitType requireUnit = upgradeType.whatsRequired(next_level);

	if (BWAPI::Broodwar->self()->getUpgradeLevel(upgradeType) < upgradeType.maxRepeats()
		&& BWAPI::Broodwar->self()->isUpgrading(upgradeType) == false)
	{
		if (upgradeType == BWAPI::UpgradeTypes::Pneumatized_Carapace
			|| upgradeType == BWAPI::UpgradeTypes::Ventral_Sacs
			|| upgradeType == BWAPI::UpgradeTypes::Antennae)
		{
			BWAPI::Unit baseUnit = InformationManager::Instance().GetOurBaseUnit();
			if (baseUnit == NULL)
			{
				return false;
			}

			//unit maybe researching other upgrades
			if (baseUnit->isCompleted() && (baseUnit->getType() == BWAPI::UnitTypes::Zerg_Lair || baseUnit->getType() == BWAPI::UnitTypes::Zerg_Hive)
				&& !baseUnit->isUpgrading() && !baseUnit->isResearching())
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			if (BWAPI::Broodwar->self()->completedUnitCount(researchingUnit) > 0)
			{
				BWAPI::Unit builder = *ourBuildings[researchingUnit].begin();
				if (!builder->isResearching() && !builder->isUpgrading()
					&& (requireUnit == BWAPI::UnitTypes::None || BWAPI::Broodwar->self()->completedUnitCount(requireUnit) != 0 || (requireUnit == BWAPI::UnitTypes::Zerg_Lair && BWAPI::Broodwar->self()->incompleteUnitCount(UnitTypes::Zerg_Hive)+ BWAPI::Broodwar->self()->completedUnitCount(UnitTypes::Zerg_Hive) != 0)))
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}
}


bool StrategyManager::isTechValid(TechType techType)
{
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& ourBuildings = InformationManager::Instance().getOurAllBuildingUnit();
	if (!WorkerManager::Instance().isGasRequireMeet(techType.gasPrice()) ||
		!WorkerManager::Instance().isMineRequireMeet(techType.mineralPrice()))
	{
		return false;
	}
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Hydralisk_Den) > 0)
	{
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Lair) == 0 && 
			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Hive) == 0)
		{
			return false;
		}

		Unit hydraliskDen = *ourBuildings[BWAPI::UnitTypes::Zerg_Hydralisk_Den].begin();
		if (BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Lurker_Aspect) ||
			BWAPI::Broodwar->self()->isResearching(BWAPI::TechTypes::Lurker_Aspect) ||
			hydraliskDen->isResearching() ||
			hydraliskDen->isUpgrading())
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		return false;
	}
}

bool StrategyManager::isBuildingValid(UnitType targetType)
{
	if (!WorkerManager::Instance().isGasRequireMeet(targetType.gasPrice()) ||
		!WorkerManager::Instance().isMineRequireMeet(targetType.mineralPrice()))
	{
		return false;
	}
	if (targetType == BWAPI::UnitTypes::Zerg_Lair)
	{
		if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Hive) > 0 ||
			BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Lair) > 0)
		{
			return false;
		}
	}
	if (targetType == BWAPI::UnitTypes::Zerg_Hive)
	{
		if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Hive) > 0)
		{
			return false;
		}

		BWAPI::Unit baseUnit = InformationManager::Instance().GetOurBaseUnit();
		if (baseUnit != NULL && (baseUnit->isResearching() || baseUnit->isUpgrading() || researchingBuildingType.find(baseUnit->getType())!=researchingBuildingType.end()))
		{
			return false;
		}
	}
	if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Greater_Spire) > 0
		&& targetType == BWAPI::UnitTypes::Zerg_Spire)
	{
		return false;
	}

	std::map<BWAPI::UnitType, int> requireUnits = targetType.requiredUnits();
	bool isValid = true;
	for (auto u : requireUnits)
	{
		if (u.first.isBuilding() && BWAPI::Broodwar->self()->completedUnitCount(u.first) == 0)
		{
			isValid = false;
			return false;
		}
	}

	return true;
}


bool StrategyManager::checkMetaValid(MetaType metaType)
{
	if (metaType.isBuilding())
	{
		return isBuildingValid(metaType.unitType);
	}
	else if (metaType.isUnit())
	{
		return isUnitValid(metaType.unitType);
	}
	else if (metaType.isUpgrade())
	{
		return isUpgradeValid(metaType.upgradeType);
	}
	else if (metaType.isTech())
	{
		return isTechValid(metaType.techType);
	}
	else
		return false;
}


bool StrategyManager::isAttackActionValid(std::string actionName)
{
	return FALSE;
}


bool StrategyManager::isBuildActionValid(std::string actionName)
{
	//hatchery and extractor's special valid check logic
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& ourBuildings = InformationManager::Instance().getOurAllBuildingUnit();
	if (actionName.find("Building_") != std::string::npos)
	{
		UnitType targetType = BuildActions[actionName].unitType;
		if (targetType == UnitTypes::Zerg_Hatchery)
		{
			int underBuildCount = 0;
			if (buildingsUnderProcess.find(targetType) != buildingsUnderProcess.end())
			{
				underBuildCount = buildingsUnderProcess[targetType].first;
			}
			if (Broodwar->self()->allUnitCount(targetType) + underBuildCount <= 10)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		if (targetType == UnitTypes::Zerg_Extractor)
		{
			bool hasGasPosition = false;
			std::map<const Area*, TilePosition> & myRegions = InformationManager::Instance().getBaseOccupiedRegions(BWAPI::Broodwar->self());
			for (auto const& geyser : BWAPI::Broodwar->getGeysers())
			{
				if (!geyser->exists())
				{
					continue;
				}
				//not my region
				if (myRegions.find(BWEMMap.GetArea(geyser->getTilePosition())) == myRegions.end())
				{
					continue;
				}
				hasGasPosition = true;
				break;
			}
			if (hasGasPosition &&
				buildingsUnderProcess.find(targetType) == buildingsUnderProcess.end())
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		//tech buildings just need one instance
		if (BWAPI::Broodwar->self()->allUnitCount(targetType) > 0
			|| buildingsUnderProcess.find(targetType) != buildingsUnderProcess.end())
		{
			return false;
		}

		bool isValid = checkMetaValid(BuildActions[actionName]);
		return isValid;
	}

	else if (actionName.find("Defense_") != std::string::npos)
	{
		vector<std::string> splitResult = splitStr(actionName, '_');
		std::string defendType = splitResult[1];
		std::string defendPosition = splitResult[2];
		if (!WorkerManager::Instance().isGasRequireMeet(BuildActions[actionName].unitType.gasPrice()) ||
			!WorkerManager::Instance().isMineRequireMeet(BuildActions[actionName].unitType.mineralPrice()))
		{
			return false;
		}
		
		//accept incompleted spawning pool
		if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Spawning_Pool) == 0 && defendType == "Sunken")
			return false;
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Evolution_Chamber) == 0 && defendType == "Spore")
			return false;
		if (ourBaseNameToPosition.find(defendPosition) == ourBaseNameToPosition.end())
			return false;

		std::map<const Area*, std::map<BWAPI::Unit, buildingInfo>>& ourBuildingDetail = InformationManager::Instance().getSelfOccupiedDetail();
		int areaHasCount = 0;
		bool hasCompleteBase = false;
		if (ourBuildingDetail.find(ourBaseNameToPosition[defendPosition]) != ourBuildingDetail.end())
		{
			for (auto& b : ourBuildingDetail[ourBaseNameToPosition[defendPosition]])
			{
				if (b.first->getType() == BuildActions[actionName].unitType)
				{
					areaHasCount++;
				}
				if (b.first->getType() == UnitTypes::Zerg_Hatchery && !b.first->isMorphing())
				{
					hasCompleteBase = true;
				}
				if (b.first->getType() == UnitTypes::Zerg_Lair || b.first->getType() == UnitTypes::Zerg_Hive)
				{
					hasCompleteBase = true;
				}
			}
		}
		if (hasCompleteBase == false)
		{
			return false;
		}

		int underBuildCount = 0;
		if (buildingsUnderProcess.find(BuildActions[actionName].unitType) != buildingsUnderProcess.end())
		{
			underBuildCount = buildingsUnderProcess[BuildActions[actionName].unitType].first;
		}

		if (defendType == "Sunken")
		{
			if (areaHasCount + underBuildCount >= 10 || Broodwar->self()->allUnitCount(BuildActions[actionName].unitType) > 20)
			{
				return false;
			}
			else
			{
				return true;
			}
		}
		else
		{
			if (areaHasCount + underBuildCount >= 5 || Broodwar->self()->allUnitCount(BuildActions[actionName].unitType) > 10)
			{
				return false;
			}
			else
			{
				return true;
			}
		}

		
	}

	else if (actionName.find("Unit_") != std::string::npos)
	{
		bool isvalid = checkMetaValid(BuildActions[actionName]);
		return isvalid;
	}

	//only the lurker tech
	else if (actionName.find("Tech_") != std::string::npos)
	{
		if (techUnderProcess.find(BuildActions[actionName].techType) != techUnderProcess.end())
		{
			return false;
		}
		if (researchingBuildingType.find(BuildActions[actionName].techType.whatResearches()) != researchingBuildingType.end()) {
			return false;
		}
		bool isvalid = checkMetaValid(BuildActions[actionName]);
		return isTechValid(BuildActions[actionName].techType);
	}

	else if (actionName.find("Upgrade_") != std::string::npos)
	{
		int next_level = BWAPI::Broodwar->self()->getUpgradeLevel(BuildActions[actionName].upgradeType) + 1;
		if (upgradeUnderProcess.find(std::make_pair(BuildActions[actionName].upgradeType,next_level)) != upgradeUnderProcess.end())
		{
			return false;
		}
		if (researchingBuildingType.find(BuildActions[actionName].upgradeType.whatUpgrades()) != researchingBuildingType.end()) {
			return false;
		}
		bool isvalid = checkMetaValid(BuildActions[actionName]);
		return isvalid;
	}

	else if (actionName.find("Expand_") != std::string::npos)
	{
		BWAPI::TilePosition nextBase = InformationManager::Instance().GetNextExpandLocation();
		if (nextBase == BWAPI::TilePositions::None)
		{
			return false;
		}

		if (waitToExpand.first == 1)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	else if (actionName.find("Attack_") != std::string::npos)
	{
		std::map<const Area*, TilePosition> & enemyBases = InformationManager::Instance().getBaseOccupiedRegions(Broodwar->enemy());
		vector<std::string> splitResult = splitStr(actionName, '_');
		std::string attackTarget = splitResult[2];
		std::map<BWAPI::UnitType, int> remain = AttackManager::Instance().reaminArmy();
		if (actionName.find("MutaliskHarass") != std::string::npos
			&& enemyBaseNameToPosition.find(attackTarget) != enemyBaseNameToPosition.end()
			&& remain[BWAPI::UnitTypes::Zerg_Mutalisk] > 0
			&& !TacticManager::Instance().isOneTacticRun(MutaliskHarassTac))
		{
			return true;
		}
		else if (actionName.find("AllInAttack") != std::string::npos
			&& AttackManager::Instance().hasGroundUnit()
			&& enemyBaseNameToPosition.find(attackTarget) != enemyBaseNameToPosition.end()
			&& !TacticManager::Instance().isOneTacticRun(HydraliskPushTactic))
		{
			return true;
		}
		else if (actionName.find("Airdrop") != std::string::npos
			&& enemyBaseNameToPosition.find(attackTarget) != enemyBaseNameToPosition.end()
			&& AttackManager::Instance().availableAirDropArmy().first != UnitTypes::None
			&& enemyBases.find(BWEMMap.GetArea(TilePosition(enemyBaseNameToPosition[attackTarget]))) != enemyBases.end())
		{
			return true;
		}

		return false;
	}

	else if (actionName.find("AllIn_addArmy") != std::string::npos)
	{
		if (AttackManager::Instance().hasAttackArmy()
			&& TacticManager::Instance().isOneTacticRun(HydraliskPushTactic))
		{
			return true;
		}
		return false;
	}

	else if (actionName.find("MutaliskHarass_addArmy") != std::string::npos)
	{
		std::map<BWAPI::UnitType, int> remain = AttackManager::Instance().reaminArmy();
		if ((remain[BWAPI::UnitTypes::Zerg_Mutalisk] > 0 || remain[BWAPI::UnitTypes::Zerg_Scourge] > 0)
			&& TacticManager::Instance().isOneTacticRun(MutaliskHarassTac))
		{
			return true;
		}
		return false;
	}

	else if (actionName.find("Wait_") != std::string::npos)
	{
		if (Broodwar->getFrameCount() > 24 * 60 * 20)
		{
			return false;
		}

		return true;
	}
	else
	{
		return false;
	}

}


bool StrategyManager::isActionValid(int actionName, bool buildOrAttack)
{
	if (buildOrAttack)
	{
		return isBuildActionValid(NNOutputBuildAction[actionName]);
	}
	else
	{
		return isAttackActionValid(NNOutputAttackAction[actionName]);
	}
}

void StrategyManager::executeAction(int chosedAction, bool buildOrAttack)
{
	if (buildOrAttack)
	{
		return executeBuildAction(NNOutputBuildAction[chosedAction]);
	}
	else
	{
		return executeAttackAction(NNOutputAttackAction[chosedAction]);
	}
}


void StrategyManager::triggerModel(double win, int isGameEnd, modelType mt)
{
	featureValues.clear();
	//get current state features
	calCurrentStateFeature();

	vector<string> nnInput(NNInputAction.size());
	vector<string> notNullFeature(NNInputAction.size());
	for (size_t i = 0; i < NNInputAction.size(); i++)
	{
		if (featureValues.find(NNInputAction[i]) != featureValues.end())
		{
			nnInput[i] = NNInputAction[i] + ":" + std::to_string(featureValues[NNInputAction[i]]);
			if (featureValues[NNInputAction[i]] != 0)
				notNullFeature[i] = NNInputAction[i] + ":" + std::to_string(featureValues[NNInputAction[i]]);
			featureValues.erase(NNInputAction[i]);
		}
		else
		{
			nnInput[i] = NNInputAction[i] + ":0";
		}
	}

	if (featureValues.size() != 0)
	{
		for (auto& f : featureValues)
		{
			logInfo("StrategyManager", "feature error " + f.first, "BIG_ERROR_StrategyManager");
		}
	}

	std::vector<std::string> actionValids;
	vector<int> actionValidIndex;
	preValidActionIndex.clear();
	if (isGameEnd != 1)
	{
		for (size_t i = 0; i < NNOutputBuildAction.size(); i++)
		{
			bool isValid = isBuildActionValid(NNOutputBuildAction[i]);
			logInfo("StrategyManager", "isActionValid " + NNOutputBuildAction[i] + " valid " + to_string(isValid));
			if (isValid)
			{
				actionValidIndex.push_back(i);
				preValidActionIndex.insert(int(i));
			}
			std::string actionInfo = NNOutputBuildAction[i] + ":" + std::to_string(int(isValid));
			actionValids.push_back(actionInfo);
		}

		if (mt == modelType::BuildModel)
		{
			if (actionValidIndex.size() == 0)
			{
				logInfo("StrategyManager", "triggerModel " + to_string(mt) + " no valid action");
				return;
			}
			else if (actionValidIndex.size() == 1)
			{
				executeBuildAction(NNOutputBuildAction[actionValidIndex.front()]);
				BWAPI::Broodwar->printf("build only has one valid action: %s", NNOutputBuildAction[actionValidIndex.front()].c_str());
				logInfo("StrategyManager", "triggerModel " + to_string(mt) + " only one action valid, execute it");
				return;
			}
		}
	}

	//double curGameScore = enemyLostScore;
	double curGameScore = getCurrentPlayerScore() / 100000.0;
	//double instanceScore = (curGameScore - previousStateAccumulatedReward) / 10000.0;
	//previousStateAccumulatedReward = curGameScore;

	int mutaliskRunning = int(TacticManager::Instance().isOneTacticRun(MutaliskHarassTac));
	int allInRunning = int(TacticManager::Instance().isOneTacticRun(HydraliskPushTactic));
	stringstream attackInfoString;
	attackInfoString << mutaliskRunning << "&" << allInRunning;

	std::string enemyRace = InformationManager::Instance().getEnemyRace();
	stringstream serializeString;
	serializeString << Broodwar->getFrameCount() << "&" << chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count() << "&"
		<< BWAPI::Broodwar->enemy()->getName() << "&" << enemyRace << "&" << Broodwar->mapFileName();

	std::string serializedString = serialize(nnInput, curGameScore, win, actionValids, isGameEnd, 
		attackInfoString.str(), serializeString.str(), mt);

	std::string serializedStringWrite = serialize(nnInput, 0, win, vector<string>(), isGameEnd, attackInfoString.str(), serializeString.str(), mt);
	//fstream historyFile;
	//std::string filePath = "./bwapi-data/write/serializedString_" + BWAPI::Broodwar->enemy()->getName();
	//historyFile.open(filePath.c_str(), ios::app);
	//historyFile << serializedString << endl;
	//historyFile.close();
	if (Broodwar->getFrameCount() > nextLogFeatureTime)
	{
		logInfo("StrategyManager", "model feature: " + serializedStringWrite);
		nextLogFeatureTime = Broodwar->getFrameCount() + 24 * 30;
	}

	//output format: model_type, feature, reward, action_valid_info, is_game_end, extra_info
	s_send(*modelTrigger, serializedString);

	if (mt == modelType::BuildModel)
	{
		logInfo("StrategyManager", "triggerModel " + to_string(mt) + " start trigger model");
		isRequestBuildModel = true;
		startRequestBuildModelFrame = Broodwar->getFrameCount();
	}
}


void StrategyManager::sendStateUpdate()
{
	triggerModel(0, 0, modelType::updateState);
}


void StrategyManager::gameEnd(double win, bool discard)
{
	int timeoutFrameCount = TimerManager::Instance().getMaxFrameCount();
	stringstream serializeString;
	serializeString << "log_timeout|||" << timeoutFrameCount;
	s_send(*modelTrigger, serializeString.str());
	int failCount = ProductionManager::Instance().getMorphFailCount();
	if (failCount >= 15)
	{
		stringstream serializeString;
		serializeString << "end_game|||" << 1;
		s_send(*modelTrigger, serializeString.str());
	}
	else
	{
		if (discard == false)
		{
			triggerModel(win, 1, modelType::BuildModel);
		}
		else
		{
			stringstream serializeString;
			serializeString << "end_game|||" << 1;
			s_send(*modelTrigger, serializeString.str());
		}

	}

	isTerminated = true;
	Sleep(1000);

	modelResultReceiver->close();
	modelTrigger->close();
	context->close();
}


void StrategyManager::buildExtractorTrick()
{
	//extractor trick
	for(auto const& u : BWAPI::Broodwar->self()->getUnits())
	{
		if (u->getType() == BWAPI::UnitTypes::Zerg_Extractor && u->getRemainingBuildTime() / double(BWAPI::UnitTypes::Zerg_Extractor.buildTime()) < 0.7
			&& BWAPI::Broodwar->self()->supplyUsed() == 9 * 2)
		{
			u->cancelMorph();
			isExtractTrickFinish = true;
		}

		//for enemy early harass
		if (u->getType() == BWAPI::UnitTypes::Zerg_Extractor && u->getRemainingBuildTime() / double(BWAPI::UnitTypes::Zerg_Extractor.buildTime()) < 0.2)
		{
			u->cancelMorph();
			isExtractTrickFinish = true;
		}
	}
}


void StrategyManager::onGoalProduction()
{
	/*
	if (isTestMode)
		return;

	if (ProductionManager::Instance().IsQueueEmpty())
	{
		featureValues.clear();
		//get current state features
		calCurrentStateFeature();

		if (!testActions.empty())
		{
			if (isActionValid(testActions.front()))
			{
				StrategyManager::Instance().executeAction(testActions.front());
				testActions.erase(testActions.begin());
			}
		}
	}
	*/

	if (isTerminated == false && Broodwar->getFrameCount() > nextStateUpdateTime)
	{
		sendStateUpdate();
		nextStateUpdateTime = Broodwar->getFrameCount() + 2 * 24;
	}
	
	if (ProductionManager::Instance().IsQueueEmpty())
	{
		//trigger interval is at least one second
		if (BWAPI::Broodwar->getFrameCount() > nextBuildCheckTime && !isRequestBuildModel)
		{
			nextBuildCheckTime = Broodwar->getFrameCount() + 6;
			triggerModel(0, 0, modelType::BuildModel);
		}
	}
}


void StrategyManager::onDroneProduction()
{
	int waitDroneCount = ProductionManager::Instance().countUnitTypeInQueue(UnitTypes::Zerg_Drone);
	int maxWaitingBuildCount = 1;
	bool isWorkerNotFull = WorkerManager::Instance().isAllDepotFull();

	int expandCount = InformationManager::Instance().getBaseOccupiedRegions(BWAPI::Broodwar->self()).size() - 1;
	if (expandCount < 0)
		return;

	//if (ProductionManager::Instance().IsQueueEmpty() == false)
		//return;

	//keep the drone production interval the same as one base's larva production rate
	if (!isWorkerNotFull && waitDroneCount == 0 && BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Drone) <= 70
		&& BWAPI::Broodwar->getFrameCount() > nextDroneBuildTime)
	{
		ProductionManager::Instance().triggerUnit(UnitTypes::Zerg_Drone, 1, false);
		nextDroneBuildTime = Broodwar->getFrameCount() + (150 / (expandCount + 1));
		logInfo("StrategyManager", "trigger drone build");
	}
}


void StrategyManager::onExtractorProduction()
{
	bool hasGasPosition = false;
	std::map<const Area*, TilePosition> & myRegions = InformationManager::Instance().getBaseOccupiedRegions(BWAPI::Broodwar->self());
	for(auto const& geyser : BWAPI::Broodwar->getGeysers())
	{
		if (!geyser->exists())
		{
			continue;
		}
		//not my region
		if (myRegions.find(BWEMMap.GetArea(geyser->getTilePosition())) == myRegions.end())
		{
			continue;
		}
		hasGasPosition = true;
		break;
	}

	//if has extractor building location && we do not have much gas && (have enough worker || mineral is too many), build one
	if (BWAPI::Broodwar->getFrameCount() > extractorProductionCheckTime && hasGasPosition && ProductionManager::Instance().getFreeGas() <= 500)
	{
		double factor = 1;
		if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Drone) >= int((BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Extractor) + factor) * 12)
			|| ProductionManager::Instance().getFreeMinerals() >= 1000)
		{
			//extractor location is determined in building manager 
			logInfo("StrategyManager", "trigger Extractor build");
			ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Extractor, BWAPI::Broodwar->self()->getStartLocation(), 1);
			extractorProductionCheckTime = BWAPI::Broodwar->getFrameCount() + BWAPI::UnitTypes::Zerg_Extractor.buildTime() + 30 * 25;
		}
	}
}


void StrategyManager::onHatcheryProduction()
{
	int waitHatcheryCount = ProductionManager::Instance().countUnitTypeInQueue(UnitTypes::Zerg_Hatchery);
	if (waitHatcheryCount > 0)
	{
		return;
	}

	int larvaCount = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Larva);

	//check if base is full of worker
	std::map<const Area*, TilePosition> & ourRegion = InformationManager::Instance().getBaseOccupiedRegions(BWAPI::Broodwar->self());

	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& selfAllBuilding = InformationManager::Instance().getOurAllBuildingUnit();
	//if we do not have much idle larva and has enough mineral, build a hatchery 
	if (BWAPI::Broodwar->getFrameCount() > HatcheryProductionCheckTime &&
		((larvaCount <= 3 && ProductionManager::Instance().getFreeMinerals() >= 500 && selfAllBuilding[BWAPI::UnitTypes::Zerg_Hatchery].size() <= 15))) //|| !hasIdleDepot))
	{
		HatcheryProductionCheckTime = BWAPI::Broodwar->getFrameCount() + 25 * 40;
		logInfo("StrategyManager", "trigger Hatchery build");
		int workingDepots = ourRegion.size();
		ProductionManager::Instance().triggerBuilding(UnitTypes::Zerg_Hatchery, Broodwar->self()->getStartLocation(), 1);
	}
}


void StrategyManager::update()
{
	openingStrategy opening = StrategyManager::Instance().getCurrentopeningStrategy();
	std::map<BWAPI::UnitType, std::set<BWAPI::Unit>>& ourBuilding = InformationManager::Instance().getOurAllBuildingUnit();

	// production FSM
	switch (productionState)
	{
	case StrategyManager::fixBuildingOrder:
	{
		switch (opening)
		{
		case TwelveHatchMuta:
		{
			
		}
		break;
		case NinePoolling:
		{
			buildExtractorTrick();
			if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Zerg_Spawning_Pool) > 0 && isExtractTrickFinish
				&& ProductionManager::Instance().IsQueueEmpty())
			{
				BWAPI::Broodwar->drawTextScreen(150, 10, "fix building order finish");
				productionState = goalOriented;

				//if (Broodwar->self()->minerals() >= 300)
				//{
				//	BWAPI::TilePosition nextBase = InformationManager::Instance().GetNextExpandLocation();
				//	ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Hatchery, nextBase, 1);
				//
				//	//ProductionManager::Instance().triggerBuilding(UnitTypes::Zerg_Sunken_Colony, InformationManager::Instance().getSunkenBuildingPosition("start"), 5);
				//	//productionState = goalOriented;
				//}

				//for (size_t i = 0; i < 9; i++)
				//{
				//	if (ProductionManager::Instance().IsQueueEmpty())
				//	{
				//		ProductionManager::Instance().triggerBuilding(UnitTypes::Zerg_Hatchery, Broodwar->self()->getStartLocation(), 1);
				//		ProductionManager::Instance().triggerUnit(UnitTypes::Zerg_Drone, 1);
				//	}
				//}
				
			}
		}
		break;
		case TenHatchMuta:
		{
			
		}
		break;
		default:
			break;
		}
	}
	break;
	case StrategyManager::goalOriented:
	{
		onGoalProduction();
		//onDroneProduction();
		//onHatcheryProduction();
		//onExtractorProduction();
	}
	break;
	default:
		break;
	}

	if (BWAPI::Broodwar->getFrameCount() % 25  == 0)
	{
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

		for (std::map<std::pair<BWAPI::UpgradeType,int>, std::string>::iterator it = upgradeUnderProcess.begin(); it != upgradeUnderProcess.end();)
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

		for (std::map<BWAPI::UnitType, std::pair<int, int>>::iterator it = buildingsUnderProcess.begin(); it != buildingsUnderProcess.end();)
		{
			it->second.second += 25;
			//do not erase defend building
			if (it->second.second > 25 * 90 
				&& BWAPI::Broodwar->self()->allUnitCount(it->first) == 0)
			{
				logInfo("StrategyManager", to_string(int(it->first)) + " building not build", "ERROR");
				it = buildingsUnderProcess.erase(it);
				BWAPI::Broodwar->printf("buildingsUnderProcess building not build!!!!!!!");
			}
			else
			{
				it++;
			}
		}

		if (waitToExpand.first == 1)
		{
			waitToExpand.second += 25;
			if (waitToExpand.second > 24 * 180)
			{
				logInfo("StrategyManager", "expand fail", "ERROR");
				waitToExpand.first = 0;
				waitToExpand.second = 0;
			}
		}
	}

	if (isTerminated == false && (isRequestBuildModel))
	{
		if (!hasSendBlockMessage)
		{
			if ((isRequestBuildModel && Broodwar->getFrameCount() - startRequestBuildModelFrame > 20 * 24))
			{
				//isRequestBuildModel = false;
				stringstream serializeString;
				serializeString << "log_latency|||" << 100;
				s_send(*modelTrigger, serializeString.str());
				//hasSendBlockMessage = true;
				logInfo("StrategyManager", " poll no return", "BIG_ERROR_StrategyManager");
			}
		}

		//wait for another 10ms, in order to not miss too many frames
		int rec = zmq_poll(pollItems, 1, 5);
		if (rec > 0)
		{
			string message = s_recv(*modelResultReceiver);
			std::stringstream ss(message);
			std::vector<string> itemList;
			string item;
			while (getline(ss, item, '|'))
			{
				itemList.push_back(item);
			}
			string modelType = itemList[0];
			int chooseAction = std::stoi(itemList[1]);

			std::stringstream ss2(itemList[2]);
			std::vector<string> startInfoList;
			while (getline(ss2, item, '&'))
			{
				startInfoList.push_back(item);
			}
			int startFrame = std::stoi(startInfoList[0]);
			long long startTime = std::stoll(startInfoList[1]);

			if (chooseAction != -1)
			{
				if (preValidActionIndex.find(chooseAction) == preValidActionIndex.end())
				{
					logInfo("StrategyManager", "not valid action!!! " + to_string(chooseAction), "BIG_ERROR_StrategyManager");
				}

				latestActions.push_back(chooseAction);
				long long curTime = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count();
				long long elapseTime = curTime - startTime;
				int elapseFrame = Broodwar->getFrameCount() - startFrame;
				stringstream serializeString;
				serializeString << "log_latency|||" << elapseTime / 1000;
				if (elapseFrame > 24 * 10)
				{
					logInfo("StrategyManager", " poll too large latency", "BIG_ERROR_StrategyManager");
				}
				s_send(*modelTrigger, serializeString.str());

				if (modelType == "build")
				{
					Broodwar->printf("build chooseAction: %s, elapse frame:%d, time:%d", NNOutputBuildAction[chooseAction].c_str(), elapseFrame, elapseTime);
					logInfo("StrategyManager", " poll " + modelType + " get model action:" + NNOutputBuildAction[chooseAction] + 
						" elapse frame:" + to_string(elapseFrame) + " elapse time:" + to_string(elapseTime));

					string actionName = NNOutputBuildAction[chooseAction];
					allExecutedActions[actionName] += 1;
					if ((actionName.find("Building_") != std::string::npos && allExecutedActions[actionName] > 15) ||
						(actionName.find("Attack_") != std::string::npos && allExecutedActions[actionName] > 15) || 
						(actionName.find("Defense_") != std::string::npos && allExecutedActions[actionName] > 15) ||
						(actionName.find("Expand_") != std::string::npos && allExecutedActions[actionName] > 10))
					{
						logInfo("StrategyManager", "execute too many action " + actionName + " " +
							to_string(allExecutedActions[actionName]), "BIG_ERROR_StrategyManager");
					}

					executeBuildAction(NNOutputBuildAction[chooseAction]);
				}
			}
			else
			{
				logInfo("StrategyManager", " poll, no valid action");
			}
			if (modelType == "build")
				isRequestBuildModel = false;
		}
	}
}


void StrategyManager::addScore(Unit u)
{
	auto calFunc = [=](Unit unit) -> double
	{
		double tmpScore = 0;
		if (u->getType().isBuilding())
		{
			if (u->getType().canAttack() || u->getType() == UnitTypes::Terran_Bunker)
			{
				tmpScore += double(8.0) / 500;
			}
		}
		else
		{
			tmpScore += double(u->getType().supplyRequired()) / 500;
		}
		return tmpScore;
	};

	double lostScore = calFunc(u);
	if (u->getPlayer() == Broodwar->self())
	{
		selfLostScore += lostScore;
	}
	else
	{
		enemyLostScore += lostScore;
	}
}


void StrategyManager::executeAttackAction(string chosedAction)
{
	
}


void StrategyManager::executeBuildAction(string chosedAction)
{
	//chosedAction = "Expand_BaseExpand";
	//Broodwar->printf("exec %s", chosedAction.c_str());

	if (chosedAction.find("Expand_") != std::string::npos)
	{
		BWAPI::TilePosition nextBase = InformationManager::Instance().GetNextExpandLocation();
		if (nextBase == BWAPI::TilePositions::None)
			nextBase = BWAPI::Broodwar->self()->getStartLocation();

		logInfo("StrategyManager", "add expand base " + std::to_string(int(UnitTypes::Zerg_Hatchery)));
		//buildingsUnderProcess[BuildActions[chosedAction].unitType].first = 1;
		waitToExpand.first = 1;
		ProductionManager::Instance().triggerBuilding(BWAPI::UnitTypes::Zerg_Hatchery, nextBase, 1);
	}
	else if (chosedAction.find("Building_") != std::string::npos)
	{
		logInfo("StrategyManager", "add building " + std::to_string(int(BuildActions[chosedAction].unitType)));
		buildingsUnderProcess[BuildActions[chosedAction].unitType].first += 1;
		ProductionManager::Instance().triggerBuilding(BuildActions[chosedAction].unitType, Broodwar->self()->getStartLocation(), 1);
	}
	else if (chosedAction.find("Defense_") != std::string::npos)
	{
		if (buildingsUnderProcess.find(BuildActions[chosedAction].unitType) == buildingsUnderProcess.end())
			buildingsUnderProcess[BuildActions[chosedAction].unitType].first = 0;
		buildingsUnderProcess[BuildActions[chosedAction].unitType].first += 1;
		vector<std::string> splitResult = splitStr(chosedAction, '_');
		TilePosition buildingPosition;
		if (splitResult[2] == "start")
		{
			if (splitResult[1] == "Spore")
				buildingPosition = Broodwar->self()->getStartLocation();
			else
				buildingPosition = InformationManager::Instance().getSunkenBuildingPosition("start");
		}
		else
		{
			if (splitResult[1] == "Spore")
				buildingPosition = InformationManager::Instance().getOurNatrualLocation();
			else
				buildingPosition = InformationManager::Instance().getSunkenBuildingPosition("natural");
		}

		logInfo("StrategyManager", "add defend building " + std::to_string(int(BuildActions[chosedAction].unitType)));
		ProductionManager::Instance().triggerBuilding(BuildActions[chosedAction].unitType, buildingPosition, 1);
	}
	else if (chosedAction.find("Tech_") != std::string::npos)
	{
		logInfo("StrategyManager", "add tech " + std::to_string(int(BuildActions[chosedAction].techType)));
		ProductionManager::Instance().triggerTech(BuildActions[chosedAction].techType);
		techUnderProcess[BuildActions[chosedAction].techType] = chosedAction;
		researchingBuildingType.insert(BuildActions[chosedAction].techType.whatResearches());
	}
	else if (chosedAction.find("Upgrade_") != std::string::npos)
	{
		logInfo("StrategyManager", "add upgrade " + std::to_string(int(BuildActions[chosedAction].upgradeType)));
		ProductionManager::Instance().triggerUpgrade(BuildActions[chosedAction].upgradeType);
		int next_level = BWAPI::Broodwar->self()->getUpgradeLevel(BuildActions[chosedAction].upgradeType) + 1;
		upgradeUnderProcess[std::make_pair(BuildActions[chosedAction].upgradeType,next_level)] = chosedAction;
		researchingBuildingType.insert(BuildActions[chosedAction].upgradeType.whatUpgrades());
	}
	else if (chosedAction.find("Wait_") != std::string::npos)
	{
		nextBuildCheckTime = Broodwar->getFrameCount() + 24 * 10;
		return;
	}
	else if (chosedAction.find("Unit_") != std::string::npos)
	{
		unitWaitToBuild[BuildActions[chosedAction].unitType] += 1;
		logInfo("StrategyManager", "add unit " + std::to_string(int(BuildActions[chosedAction].unitType)) + 
		" remain count " + to_string(unitWaitToBuild[BuildActions[chosedAction].unitType]));
		ProductionManager::Instance().triggerUnit(BuildActions[chosedAction].unitType, 1, false, true);
	}
	else if (chosedAction.find("Attack_") != std::string::npos)
	{
		vector<std::string> splitResult = splitStr(chosedAction, '_');
		std::string attackMode = splitResult[1];
		std::string attackTarget = splitResult[2];
		logInfo("StrategyManager", "issue Attack, attack mode: " + attackMode + " target: " + to_string(enemyBaseNameToPosition[attackTarget].x) + 
		" " + to_string(enemyBaseNameToPosition[attackTarget].y));

		if (attackMode == "AllInAttack")
		{
			if (enemyBaseNameToPosition.find(attackTarget) == enemyBaseNameToPosition.end())
			{
				logInfo("StrategyManager", "attack target not find " + attackTarget, "BIG_ERROR_StrategyManager");
				return;
			}
			Position attackPosition = enemyBaseNameToPosition[attackTarget];
			AttackManager::Instance().issueAttackCommand(HydraliskPushTactic, attackPosition);
		}
		else if (attackMode == "MutaliskHarass")
		{
			if (enemyBaseNameToPosition.find(attackTarget) == enemyBaseNameToPosition.end())
			{
				logInfo("StrategyManager", "attack target not find " + attackTarget, "BIG_ERROR_StrategyManager");
				return;
			}
			Position attackPosition = enemyBaseNameToPosition[attackTarget];
			logInfo("StrategyManager", "trigger MutaliskHarass " + std::to_string(attackPosition.x) + " " + to_string(attackPosition.y));
			AttackManager::Instance().issueAttackCommand(MutaliskHarassTac, attackPosition);
		}
		else if (attackMode == "Airdrop")
		{
			if (enemyBaseNameToPosition.find(attackTarget) == enemyBaseNameToPosition.end())
			{
				logInfo("StrategyManager", "attack target not find " + attackTarget, "BIG_ERROR_StrategyManager");
				return;
			}
			Position attackPosition = enemyBaseNameToPosition[attackTarget];
			logInfo("StrategyManager", "trigger Airdrop tactic " + std::to_string(attackPosition.x) + " " + to_string(attackPosition.y));
			AttackManager::Instance().issueAttackCommand(AirdropTac, attackPosition);
		}
	}
	else if (chosedAction.find("AllIn_addArmy") != std::string::npos)
	{
		AttackManager::Instance().addArmyToTactic(HydraliskPushTactic);
		logInfo("StrategyManager", "execute AllIn_addArmy ");
	}
	else if (chosedAction.find("MutaliskHarass_addArmy") != std::string::npos)
	{
		AttackManager::Instance().addArmyToTactic(MutaliskHarassTac);
		logInfo("StrategyManager", "execute MutaliskHarass_addArmy ");
	}
	else
	{
		return;
	}
}


// get an instance of this
StrategyManager & StrategyManager::Instance()
{
	static StrategyManager instance;
	return instance;
}


std::vector<MetaType> StrategyManager::getOpeningBook()
{
	// according to the game progress, return start/mid/end strategy
	if (selfRace == BWAPI::Races::Zerg)
	{	//12hatch opening 
		//return getMetaVector("0 0 0 0 0 1 0 0 0 2 3 5 0 0 0 0 0 0 4 6 1 11 11 0 0 0 0 12 12 8 0 5 1 1 0 0 10 10 10 10 10 10 10 1 10 10 10 10");

		//overPool opening
		//return getMetaVector("0 0 0 0 0 1 3 0 0 0 2 4 4 4 0 5 0 0 0 6 0 0 0 0 0 0 8 5 0 0 0 1 1 0 0 0 10 10 10 10 10 10 10 1 10 10 10 10 10 19 20 19 20 19 20");
		
		if (currentopeningStrategy == TwelveHatchMuta)
		{
			//12 hatch mutalisk
			return getMetaVector("0 0 0 0 0 5 0 2 3 0 1 4 4 4 5 0 0 0 0 6 0 1 0 0 0 0 8 5 0 0 0 1 1 0 0 0 10 10 10 10 10 10 10 1 10 10 10");
		}
		else if (currentopeningStrategy == NinePoolling)
		{
			//9 pool zergling
			return getMetaVector("0 0 0 0 0 3 0 5 0 1");
		}
		else if (currentopeningStrategy == TenHatchMuta)
		{
			// 10 hatch counter 2 gate zealot
			return getMetaVector("0 0 0 0 0 5 0 2 3 0 1 4 4 4 0 0");
		}
		else
		{
			return std::vector<MetaType>();
		}
	}
	else
	{
		return std::vector<MetaType>();
	}
}


int StrategyManager::getScore(bool isSelf)
{
	if (isSelf)
	{
		int unitScore = BWAPI::Broodwar->self()->getUnitScore();
		int buildingScore = BWAPI::Broodwar->self()->getBuildingScore();
		int resourceScore = BWAPI::Broodwar->self()->gatheredMinerals() + BWAPI::Broodwar->self()->gatheredGas();
		return unitScore + buildingScore + resourceScore;
	}
	else
	{
		int unitScore = BWAPI::Broodwar->enemy()->getUnitScore();
		int buildingScore = BWAPI::Broodwar->enemy()->getBuildingScore();
		int resourceScore = BWAPI::Broodwar->enemy()->gatheredMinerals() + BWAPI::Broodwar->enemy()->gatheredGas();
		return unitScore + buildingScore + resourceScore;
	}
}


// need add the upgrade and tech unit type
std::vector<MetaType> StrategyManager::getMetaVector(std::string buildString)
{
	std::stringstream ss;
	ss << buildString;
	std::vector<MetaType> meta;

	int action(0);
	while (ss >> action)
	{
		meta.push_back(actions[action]);
	}
	return meta;
}


int	StrategyManager::getCurrentPlayerScore()
{
	return Broodwar->self()->getRazingScore() +
		Broodwar->self()->getKillScore();

	//return Broodwar->self()->getBuildingScore() + Broodwar->self()->getRazingScore() +
		//Broodwar->self()->getKillScore() + Broodwar->self()->getUnitScore() +
		//Broodwar->self()->gatheredMinerals() + Broodwar->self()->gatheredGas();
}


string StrategyManager::serialize(vector<string> features, double instantScore, double terminalScore, vector<string> actionValids, 
	int end, string attackInfo, string extra, modelType mt)
{
	stringstream serializeString;
	serializeString.precision(15);
	serializeString << std::fixed;
	if (mt == modelType::BuildModel)
	{
		serializeString << "build_model_request|||Macro_selection" << "|";
	}
	else if (mt == modelType::updateState)
	{
		serializeString << "update_state|||Macro_selection" << "|";
	}
	
	for (auto item : features)
	{
		serializeString << item << "\n";
	}
	serializeString << "|" << instantScore << "|" << terminalScore << "|";
	for (auto item : actionValids)
	{
		serializeString << item << "\n";
	}
	serializeString << "|" << end << "|" << attackInfo << "|" << extra;
	return serializeString.str();
}


void StrategyManager::onUnitDestroy(BWAPI::Unit unit) {
	if (unit->getType().isBuilding()) {
		if (unit->getUpgrade() != UpgradeTypes::None) {
			for (std::map<std::pair<BWAPI::UpgradeType, int>, std::string>::iterator it = upgradeUnderProcess.begin(); it != upgradeUnderProcess.end();)
			{
				if (it->first.first == unit->getUpgrade())
				{
					researchingBuildingType.erase(unit->getType());
					it = upgradeUnderProcess.erase(it);
				}
				else
				{
					it++;
				}
			}
		}
		if (unit->getTech() != TechTypes::None) {
			for (std::map<BWAPI::TechType, std::string>::iterator it = techUnderProcess.begin(); it != techUnderProcess.end();)
			{
				if (it->first == unit->getTech())
				{
					researchingBuildingType.erase(unit->getType());
					it = techUnderProcess.erase(it);
				}
				else
				{
					it++;
				}
			}
		}
	}
}