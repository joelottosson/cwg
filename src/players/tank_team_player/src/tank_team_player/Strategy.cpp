/******************************************************************************
*
* Copyright Saab AB, 2008-2013 (http://safir.sourceforge.net)
*
* Created by: Petter Lönnstedt / stpeln
*
*******************************************************************************
*
* This file is part of Safir SDK Core.
*
* Safir SDK Core is free software: you can redistribute it and/or modify
* it under the terms of version 3 of the GNU General Public License as
* published by the Free Software Foundation.
*
* Safir SDK Core is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Safir SDK Core.  If not, see <http://www.gnu.org/licenses/>.
*
******************************************************************************/

// For debugging
#include <iostream>

// For randomization
#include <time.h>
#include <stdlib.h>

#include "Strategy.h"
#include "GameMap.h"

namespace TankTeamPlayer
{
    Strategy::Strategy() :
        m_JoystickEntityHandlerStrategyIF(NULL)
    {
        // Init random seed
        srandom(clock());
    }

    /***
      * This is called when a new game is starting.
      * You can use this to init game information, but also to clear 
      * any caches from previous games
      */
    void Strategy::Init(
        int tankId,
        const Consoden::TankGame::GameStatePtr gamePtr, 
        JoystickEntityHandlerStrategyIF* joyHandler)
    {
        m_TankId = tankId;
        m_JoystickEntityHandlerStrategyIF = joyHandler;
        
        // Anything more to init?    
    }

    /***
      * This is called when a new round in the game is starting.
      * Calculate your next move and set the joystick accordingly.
      */
    void Strategy::Run(const Consoden::TankGame::GameStatePtr gamePtr)
    {
        Consoden::TankGame::Direction::Enumeration moveDirection;
        Consoden::TankGame::Direction::Enumeration fireDirection;
        bool fire;

        // Setup helper
        GameMap gm(m_TankId, gamePtr);

        // from the helper class you can retrieve:
        // Game board dimensions
            // gm.XSize();
            // gm.YSize();
        // the players position
            // gm.PlayerPosition();
        // the enemys position
            // gm.EnemyPosition();
        // If a square contains no wall or mine
            // gm.EmptySquare(pos);
        // Moving in a direction, including wrapping at end of game board
            // gm.Left(pos); - returns one step left of pos


        // Calculate our next move

        // -*- DUMMY CODE!, replace with you own
        gm.PrintState();
        
        // Make a random movement
        float r = random();
        r = r / RAND_MAX;

        if (r < 0.25) {
            moveDirection = Consoden::TankGame::Direction::Left;
        } else if (r < 0.5) {
            moveDirection = Consoden::TankGame::Direction::Right;
        } else if (r < 0.75) {
            moveDirection = Consoden::TankGame::Direction::Up;
        } else {
            moveDirection = Consoden::TankGame::Direction::Down;
        } 

        // Set a random fire direction
        r = random();
        r = r / RAND_MAX;
        if (r < 0.25) {
            fireDirection = Consoden::TankGame::Direction::Left;
        } else if (r < 0.5) {
            fireDirection = Consoden::TankGame::Direction::Right;
        } else if (r < 0.75) {
            fireDirection = Consoden::TankGame::Direction::Up;
        } else {
            fireDirection = Consoden::TankGame::Direction::Down;
        } 

        // Fire?
        r = random();
        r = r / RAND_MAX;
        if (r < 0.2) {
            fire = true;
        } else {
            fire = false;
        }

        // -*- DUMMY CODE END! 

        // Set the joystick
        m_JoystickEntityHandlerStrategyIF->MoveDirection(moveDirection);
        m_JoystickEntityHandlerStrategyIF->TowerDirection(fireDirection);
        m_JoystickEntityHandlerStrategyIF->Fire(fire);
    }
 };

