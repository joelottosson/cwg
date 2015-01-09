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
    //TODO: implement your own tank logic and call SetJoystick

    //-------------------------------------------------------
    //Example of a stupid tank logic:
    //Remove it and write your own brilliant version!
    //-------------------------------------------------------
    GameMap gm(m_ownTankId, gameState);
    auto currentPosition=gm.OwnPosition();
    auto enemyPosition=gm.EnemyPosition();
    BfsHelper bfs(gameState, currentPosition);
    Consoden::TankGame::Direction::Enumeration moveDirection=m_move;

    bool dropMine=m_dropMine;
    m_dropMine=false;
    if (m_move==Consoden::TankGame::Direction::Left)
    {
        m_move=Consoden::TankGame::Direction::Down;
        if (m_ownTankId==1)
            m_dropMine=true;

    }

    //Advanced tower aim stategy
    auto towerDirection=static_cast<Consoden::TankGame::Direction::Enumeration>((1+currentPosition.first+currentPosition.second)%4);

    //Of course we always want to fire
    bool fire=true;



    //Move our joystick.
    SetJoystick(moveDirection, towerDirection, fire, dropMine);
}
