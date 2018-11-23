#pragma once
#include "Common.h"
#include "BattleTactic.h"
#include "BattleArmy.h"



class ArmyDefendTactic : public BattleTactic
{
	
	set<const Area*> adjacentAreas;

public:
	ArmyDefendTactic();
	virtual void			update() override;

};



