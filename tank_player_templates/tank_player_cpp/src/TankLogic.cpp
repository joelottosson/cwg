/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#include "TankLogic.h"
#include "GameMap.h"

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

    //Find an empty sqaure we can move to, if no emtpy square can be found we just dont move at all
    auto moveDirection=Consoden::TankGame::Direction::Neutral;
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

    //Advanced tower aim stategy
    auto towerDirection=static_cast<Consoden::TankGame::Direction::Enumeration>((1+currentPosition.first+currentPosition.second)%4);

    //Of course we always want to fire
    bool fire=true;

    //Sometimes we also drop a mine
    bool dropMine=(static_cast<int>(gameState->ElapsedTime().GetVal()) % 3)==0;

    //Move our joystick.
    SetJoystick(moveDirection, towerDirection, fire, dropMine);
}
