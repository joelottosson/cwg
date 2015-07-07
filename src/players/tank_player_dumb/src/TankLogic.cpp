/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#include "../../tank_player_dumb/src/TankLogic.h"

#include "../../tank_player_dumb/src/BfsHelper.h"
#include "../../tank_player_dumb/src/GameMap.h"

const std::wstring TankLogic::PlayerName = L"dumb"; //TODO: change to your team name

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


    moveDirection=Consoden::TankGame::Direction::Neutral;


    //Advanced tower aim stategy
    auto towerDirection=static_cast<Consoden::TankGame::Direction::Enumeration>((1+currentPosition.first+currentPosition.second)%4);

    //Of course we always want to fire
    bool fire=false;

    //Sometimes we also drop a mine
    bool dropMine=(static_cast<int>(gameState->ElapsedTime().GetVal()) % 3)==0;

    //Move our joystick.
    SetJoystick(moveDirection, towerDirection, false, false);
}
