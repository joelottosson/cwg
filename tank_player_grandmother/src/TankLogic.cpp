/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#include "TankLogic.h"

#include "BfsHelper.h"
#include "GameMap.h"

const std::wstring TankLogic::PlayerName = L"grandmother"; //TODO: change to your team name
bool fired_last_round = false;

void TankLogic::MakeMove(Consoden::TankGame::GameStatePtr gameState)
{
    //-------------------------------------------------------
    //Example of a stupid tank logic:
    //Remove it and write your own brilliant version!
    //-------------------------------------------------------
    gm =  GameMap::GameMap(m_ownTankId, gameState);

    auto currentPosition=gm.OwnPosition();
    auto enemyPosition=gm.EnemyPosition();
    BfsHelper bfs(gameState, currentPosition);
    Consoden::TankGame::Direction::Enumeration moveDirection = Consoden::TankGame::Direction::Neutral;
    Consoden::TankGame::Direction::Enumeration towerDirection;

    //check if enemy is at same line as we are.
    bool fire = false;
    towerDirection = isInSight();
    if(towerDirection != Consoden::TankGame::Direction::Neutral && !fired_last_round){
		fire = true;
		fired_last_round = true;

    }else{
    	fired_last_round = false;
    }


    if(!fire){
		if (bfs.CanReachSquare(enemyPosition)) {
			// It is possible to move all the way to the enemy, do it
			moveDirection=bfs.FindDirection(currentPosition, bfs.BacktrackFromSquare(enemyPosition));

		} else {
			//Find an empty sqaure we can move to, otherwise move downwards
			moveDirection=Consoden::TankGame::Direction::Neutral;
			if (!gm.IsWall(gm.Move(currentPosition, Consoden::TankGame::Direction::Left)) &&
				!gm.IsMine(gm.Move(currentPosition, Consoden::TankGame::Direction::Left)))
			{
				moveDirection=Consoden::TankGame::Direction::Left;
			}
			else if (!gm.IsWall(gm.Move(currentPosition, Consoden::TankGame::Direction::Right)) &&
					 !gm.IsMine(gm.Move(currentPosition, Consoden::TankGame::Direction::Right)))
			{
				moveDirection=Consoden::TankGame::Direction::Right;
			}
			else if (!gm.IsWall(gm.Move(currentPosition, Consoden::TankGame::Direction::Up)) &&
					 !gm.IsMine(gm.Move(currentPosition, Consoden::TankGame::Direction::Up)))
			{
				moveDirection=Consoden::TankGame::Direction::Up;
			}
			else if (!gm.IsWall(gm.Move(currentPosition, Consoden::TankGame::Direction::Down)) &&
					 !gm.IsMine(gm.Move(currentPosition, Consoden::TankGame::Direction::Down)))
			{
				moveDirection=Consoden::TankGame::Direction::Down;
			}
		}
    }

    //Move our joystick.
    SetJoystick(moveDirection, towerDirection, fire, false);
}


Consoden::TankGame::Direction::Enumeration TankLogic::isInSight(){

	auto own = gm.OwnPosition();
	auto enemy = gm.EnemyPosition();
	int xdiff = own.first - enemy.first;
	int ydiff = own.second - enemy.second;

	if(xdiff == 0){
		if(ydiff > 0){
			return Consoden::TankGame::Direction::Up;
		}else{
			return Consoden::TankGame::Direction::Down;
		}

	}else if(ydiff == 0){
		if(xdiff > 0){
			return Consoden::TankGame::Direction::Left;
		}else{
			return Consoden::TankGame::Direction::Right;
		}
	}else{
		return Consoden::TankGame::Direction::Neutral;
	}

}











