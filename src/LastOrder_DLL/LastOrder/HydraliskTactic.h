#pragma once
#include "Common.h"
#include "BattleTactic.h"
#include "AstarPath.h"
#include "ZerglingArmy.h"



class HydraliskTactic : public BattleTactic
{
	
public:
	HydraliskTactic();
	virtual void			update() override;
};



