#pragma once

#include "Common.h"


struct MetaType {

	enum type_enum { Unit, Tech, Upgrade, Command, Default };
	type_enum type;

	BWAPI::UnitCommandType commandType;
	BWAPI::UnitType unitType;
	BWAPI::TechType techType;
	BWAPI::UpgradeType upgradeType;
	
	BWAPI::TilePosition buildingPosition;
	//for identify the building and unit triggered by strategyChange function
	std::string unitSourceBuildingAction;

	MetaType() : type(MetaType::Default) { buildingPosition = BWAPI::TilePositions::None; }
	MetaType(BWAPI::UnitType t, std::string s = "") : unitType(t), type(MetaType::Unit), unitSourceBuildingAction(s) { buildingPosition = BWAPI::TilePositions::None; }
	MetaType(BWAPI::UnitType t, BWAPI::TilePosition l, std::string s = "") : unitType(t), type(MetaType::Unit), unitSourceBuildingAction(s) { buildingPosition = l; }

	MetaType(BWAPI::TechType t) : techType(t), type(MetaType::Tech) {}
	MetaType(BWAPI::UpgradeType t) : upgradeType(t), type(MetaType::Upgrade) {}
	MetaType(BWAPI::UnitCommandType t) : commandType(t), type(MetaType::Command) {}

	bool isUnit() const	{ return type == Unit; }
	bool isTech() const		{ return type == Tech; }
	bool isUpgrade() const	{ return type == Upgrade; }
	bool isCommand() const	{ return type == Command; }
	bool isBuilding() const	{ return type == Unit && unitType.isBuilding(); }
	bool isRefinery() const	{ return isBuilding() && unitType.isRefinery(); }

	int supplyRequired()
	{
		if (isUnit())
		{
			return unitType.supplyRequired();
		}
		else
		{
			return 0;
		}
	}

	int mineralPrice()
	{
		return isUnit() ? unitType.mineralPrice() : (isTech() ? techType.mineralPrice() : upgradeType.mineralPrice());
	}

	int gasPrice()
	{
		return isUnit() ? unitType.gasPrice() : (isTech() ? techType.gasPrice() : upgradeType.gasPrice());
	}

	BWAPI::UnitType whatBuilds()
	{
		return isUnit() ? unitType.whatBuilds().first : (isTech() ? techType.whatResearches() : upgradeType.whatUpgrades());
	}

	std::string getName()
	{
		if (isUnit())
		{
			return unitType.getName();
		}
		else if (isTech())
		{
			return techType.getName();
		}
		else if (isUpgrade())
		{
			return upgradeType.getName();
		}
		else if (isCommand())
		{
			return commandType.getName();
		}
		else
		{
			assert(false);
			return "LOL";
		}
	}
};