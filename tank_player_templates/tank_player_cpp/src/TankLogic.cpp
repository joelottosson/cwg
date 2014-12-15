/******************************************************************************
*
* Copyright Consoden AB, 2014
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

    //Find an empty sqaure we can move to, otherwize move downwards
    Consoden::TankGame::Direction::Enumeration moveDirection=Consoden::TankGame::Direction::Down;
    if (gm.IsEmpty(gm.MoveLeft(currentPosition)))
    {
        moveDirection=Consoden::TankGame::Direction::Left;
    }
    else if (gm.IsEmpty(gm.MoveRight(currentPosition)))
    {
        moveDirection=Consoden::TankGame::Direction::Right;
    }
    else if (gm.IsEmpty(gm.MoveUp(currentPosition)))
    {
        moveDirection=Consoden::TankGame::Direction::Up;
    }

    //Advanced tower aim stategy
    Consoden::TankGame::Direction::Enumeration towerDirection=
            static_cast<Consoden::TankGame::Direction::Enumeration>((currentPosition.first+currentPosition.second)%4);

    //Of course we always want to fire
    bool fire = true;

    //Move our joystick.
    SetJoystick(moveDirection, towerDirection, fire);
}
