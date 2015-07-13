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
        //Move our joystick.
    SetJoystick(Consoden::TankGame::Direction::Neutral, Consoden::TankGame::Direction::Neutral, false, false);
}
