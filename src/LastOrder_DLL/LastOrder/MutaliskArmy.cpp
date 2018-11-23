#include "MutaliskArmy.h"
#include "InformationManager.h"



int MutaliskArmy::getAttackPriority(BWAPI::Unit unit)
{
	BWAPI::UnitType type = unit->getType();
	if (type == BWAPI::UnitTypes::Zerg_Egg || type == BWAPI::UnitTypes::Zerg_Larva)
	{
		return 0;
	}

	if ((type == UnitTypes::Terran_SCV && unit->isRepairing()) || type==UnitTypes::Terran_Medic) {
		return 18;
	}

	// highest priority is something that can attack us or aid in combat
	if ((type.airWeapon() != BWAPI::WeaponTypes::None && type != UnitTypes::Protoss_Interceptor) ||
		type == BWAPI::UnitTypes::Protoss_Carrier || type == UnitTypes::Terran_Bunker || type == BWAPI::UnitTypes::Protoss_High_Templar)
	{
		return 15;
	}
	else if (type.isWorker())
	{
		return 10;
	}
	else if (type.groundWeapon() != BWAPI::WeaponTypes::None)
	{
		return 10;
	}
	// next is special buildings
	else if (type == BWAPI::UnitTypes::Protoss_Pylon || type == BWAPI::UnitTypes::Zerg_Spire)
	{
		return 9;
	}
	// next is buildings that cost gas
	else if (type.gasPrice() > 0)
	{
		return 8;
	}
	else if (type.mineralPrice() > 0)
	{
		return 7;
	}
	// then everything else
	else
	{
		return 1;
	}
}



void MutaliskArmy::attack(UnitState&  attacker, BWAPI::Unit target)
{
	MutaDanceTarget(attacker);
}


void MutaliskArmy::MutaDanceTarget(UnitState& attacker)
{
	Unit muta = attacker.unit;
	Unit target = attacker.enemyTarget;
	//double distanceToTarget = muta->getDistance(target->getPosition());
	double distanceToTarget = muta->getDistance(target);
	double distanceToAttack = std::max(0.0, distanceToTarget - UnitTypes::Zerg_Mutalisk.airWeapon().maxRange());
	double flyFrame = (distanceToAttack / UnitTypes::Zerg_Mutalisk.topSpeed()) + Broodwar->getLatencyFrames() + 15;
	const int currentCooldown = muta->getGroundWeaponCooldown();

	Position armyCenterPosition(0,0);
	for (auto u : this->units) {
		armyCenterPosition += u.unit->getPosition();
	}
	armyCenterPosition /= this->units.size();

	//BattleArmy::smartAttackUnit(muta, target);
	/*
	if (currentCooldown > 0) {
		if (currentCooldown <= orderFrame) {
			muta->attack(target,true);
			BWAPI::Broodwar->drawLineMap(muta->getPosition().x, muta->getPosition().y,target->getPosition().x, target->getPosition().y, BWAPI::Colors::Red);
		}
		else {
			Position nextMove = Position(calNextMovePosition(attacker, target->getPosition(), muta->getPosition(), 6));
			muta->move(nextMove);
			BWAPI::Broodwar->drawLineMap(muta->getPosition().x, muta->getPosition().y, nextMove.x, nextMove.y, BWAPI::Colors::Blue);
		}
	}
	else {
		distanceToAttack = std::max(0.0, distanceToTarget - UnitTypes::Zerg_Mutalisk.airWeapon().maxRange());
		if (distanceToAttack < muta->getType().topSpeed() / 2 * Broodwar->getLatencyFrames()) {
			Position nextMove = Position(calNextMovePosition(attacker, target->getPosition(), muta->getPosition(), 6));
			muta->move(nextMove);
			BWAPI::Broodwar->drawLineMap(muta->getPosition().x, muta->getPosition().y, nextMove.x, nextMove.y, BWAPI::Colors::Cyan);
		}
		else {
			muta->attack(target, true);
			BWAPI::Broodwar->drawLineMap(muta->getPosition().x, muta->getPosition().y, target->getPosition().x, target->getPosition().y, BWAPI::Colors::Green);
		}
	}
	*/

	if (Broodwar->getLatencyFrames() > 6) {
	//if (Broodwar->mapFileName().find("o_")!=-1) {
		BattleArmy::smartAttackUnit(muta, target);
	}
	else {
		int mutaliskSpeed = int(UnitTypes::Zerg_Mutalisk.topSpeed());
		int mutaliskRange = UnitTypes::Zerg_Mutalisk.airWeapon().maxRange();
		int mutaliskTurnFrame = 2;
		//double enemySpeed = target->getType().isBuilding() ? 0 : target->getType().topSpeed();
		double enemySpeed = target->getType().topSpeed() / 2;
		Position predictedEnePos = target->getPosition() + Position(int(target->getVelocityX()), int(target->getVelocityY()))*currentCooldown / 2;
		int orderFrame = (muta->getDistance(predictedEnePos) - mutaliskRange)/ mutaliskSpeed + 2 * Broodwar->getLatencyFrames() + mutaliskTurnFrame;
		if (attacker.unit->isSelected())
		{
			//Broodwar->drawTextMap(Position(muta->getPosition().x + 2, muta->getPosition().y), "cd:%d, of:%d", currentCooldown,orderFrame);
			Broodwar->drawTextMap(Position(muta->getPosition().x + 2, muta->getPosition().y), "%d", currentCooldown);
		}
		if (InformationManager::Instance().getEnemyInfluenceMap(TilePosition(muta->getPosition()).x, TilePosition(muta->getPosition()).y).psionicStormDamage > 0) {
			Position nextMove = Position(calNextMovePosition(attacker, target->getPosition(), armyCenterPosition, 6));
			Position newNextMove = (nextMove - muta->getPosition()) * 7 + muta->getPosition();
			muta->move(newNextMove);
			return;
		}
		if (attacker.seigeFlag) {
			if (InformationManager::Instance().getEnemyInfluenceMap(TilePosition(muta->getPosition()).x, TilePosition(muta->getPosition()).y).strongAirForce > 0) {
				Position nextMove = Position(calNextMovePosition(attacker, target->getPosition(), armyCenterPosition, 6));
				Position newNextMove = (nextMove - muta->getPosition()) * 7 + muta->getPosition();
				muta->move(newNextMove);
				return;
			}
		}
		if (currentCooldown > 0) {
			if (attacker.performDance == true) {
				attacker.performDance = false;
				Position nextMove = Position(calNextMovePosition(attacker, target->getPosition(), armyCenterPosition, 6));
				Position newNextMove = (nextMove - muta->getPosition()) * 5 + muta->getPosition();
				muta->move(newNextMove);
				//BWAPI::Broodwar->drawLineMap(muta->getPosition().x, muta->getPosition().y, newNextMove.x, newNextMove.y, BWAPI::Colors::Blue);
			}
			else if (currentCooldown < orderFrame) {
				//muta->attack(target);
				muta->move(target->getPosition());
				//BWAPI::Broodwar->drawLineMap(muta->getPosition().x, muta->getPosition().y, target->getPosition().x, target->getPosition().y, BWAPI::Colors::Red);
			}
		}
		else {
			attacker.performDance = true;
			/*
			if (BWAPI::Broodwar->getFrameCount() - muta->getLastCommandFrame() >= 7) {
				muta->attack(target); 
				BWAPI::Broodwar->drawLineMap(muta->getPosition().x, muta->getPosition().y, target->getPosition().x, target->getPosition().y, BWAPI::Colors::Green);
			}
			*/

			/*
			if (attacker.attackReady==true) {
				muta->attack(target);
				attacker.attackReady = false;
				BWAPI::Broodwar->drawLineMap(muta->getPosition().x, muta->getPosition().y, target->getPosition().x, target->getPosition().y, BWAPI::Colors::Green);
			}
			*/
			
			
			/*
			//double rangeWithLatency = mutaliskRange + (mutaliskSpeed / 2 - 1)*(Broodwar->getLatencyFrames() - 1);
			double rangeWithLatency = mutaliskRange + std::max(0.0, (mutaliskSpeed - enemySpeed) / 2) * (Broodwar->getLatencyFrames() - 1);
			//Broodwar->printf("distance: %.3f rangeWithLatency: %.3f", distanceToTarget, rangeWithLatency);
			if (distanceToTarget > rangeWithLatency) {
				muta->attack(target);
				//BWAPI::Broodwar->drawLineMap(muta->getPosition().x, muta->getPosition().y, target->getPosition().x, target->getPosition().y, BWAPI::Colors::Green);
			}
			else{
				muta->attack(target);
				Position nextMove = Position(calNextMovePosition(attacker, target->getPosition(), muta->getPosition(), 6));
				muta->move(nextMove);
				//BWAPI::Broodwar->drawLineMap(muta->getPosition().x, muta->getPosition().y, nextMove.x, nextMove.y, BWAPI::Colors::White);
			}
			*/

			Position predictedPosWithLatency = muta->getPosition() + Position(int(muta->getVelocityX()), int(muta->getVelocityY()))*Broodwar->getLatencyFrames();
			Position predictedEnePosWithLatency = target->getPosition() + Position(int(target->getVelocityX()), int(target->getVelocityY()))*Broodwar->getLatencyFrames();
			double distanceAfterLatency = predictedPosWithLatency.getDistance(predictedEnePosWithLatency);
			if (distanceAfterLatency > mutaliskRange) {
				muta->attack(target);
			}
			//else {
			//	muta->attack(target);
			//	Position nextMove = Position(calNextMovePosition(attacker, target->getPosition(), armyCenterPosition, 6));
			//	Position newNextMove = (nextMove - muta->getPosition()) * 5 + muta->getPosition();
			//	muta->move(newNextMove);
			//}

		}
	}

	/*
	if (target->getType().airWeapon() == WeaponTypes::None && target->getType() != UnitTypes::Terran_Bunker)
	{
		if (currentCooldown < 10)
			BattleArmy::smartAttackUnit(muta, target);
		else
		{
			if (InformationManager::Instance().getEnemyInfluenceMap(muta->getTilePosition().x, muta->getTilePosition().y).airForce != 0)
			{
				TilePosition nextMove = calNextMovePosition(attacker, target->getPosition(), muta->getPosition(), 4);
				BattleArmy::smartMove(muta, Position(nextMove));
			}
		}
	}
	else
	{
		if (flyFrame > currentCooldown)
			BattleArmy::smartAttackUnit(muta, target);
		else
		{
			TilePosition nextMove = calNextMovePosition(attacker, target->getPosition(), muta->getPosition(), 3);
			BattleArmy::smartMove(muta, Position(nextMove));
		}
	}
	*/

	/*
	//if too far away, do one step move
	if (distanceToTarget > 6 * 32 && !isAirDefendBuilding(target->getType()))
	{
		TilePosition nextMove = calNextMovePosition(attacker, target->getPosition(), muta->getPosition(), 2);
		BattleArmy::smartMove(muta, Position(nextMove));
	}
	else
	{
		BattleArmy::smartAttackUnit(muta, target);
	}
	*/
}


