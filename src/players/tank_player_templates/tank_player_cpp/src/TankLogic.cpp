/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#include "TankLogic.h"
#include "GameMap.h"
#include "BfsHelper.h"

const std::wstring TankLogic::PlayerName = L"tank_player_cpp"; //TODO: change to your team name

void TankLogic::MakeMove(Consoden::TankGame::GameStatePtr gameState)
{
    //-------------------------------------------------------
    //Example of a stupid tank logic:
    //Remove it and write your own brilliant version!
    //-------------------------------------------------------
    GameMap gm(m_ownTankId, gameState);
    auto currentPosition=gm.OwnPosition();
    auto enemyPosition=gm.EnemyPosition();
    BfsHelper bfs(gameState, currentPosition);
    Consoden::TankGame::Direction::Enumeration moveDirection;

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

    //Advanced tower aim stategy
    auto towerDirection=static_cast<Consoden::TankGame::Direction::Enumeration>((1+currentPosition.first+currentPosition.second)%4);

    //Of course we always want to fire
    bool fire=true;

    //Sometimes we also drop a mine
    bool dropMine=(static_cast<int>(gameState->ElapsedTime().GetVal()) % 3)==0;

    //lets fire the laser if we can
    bool fire_laser = false;

    if(gm.LaserAmmoCount() > 0){
    	fire_laser = true;
    }

    bool fire_redeemer = false;
    int redemer_timer = 3;
    if(gm.HasRedeemer()){
    	fire_redeemer = true;
    }

    bool deploy_smoke = false;
    if(gm.HasSmoke()){
    	deploy_smoke = true;
    }
    //Move our joystick.
    SetJoystick(moveDirection, towerDirection, fire, dropMine, fire_laser,deploy_smoke,fire_redeemer, redemer_timer);
}
