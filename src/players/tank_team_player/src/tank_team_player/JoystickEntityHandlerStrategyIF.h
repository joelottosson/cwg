/******************************************************************************
*
* Copyright Consoden AB, 2013-2014 (http://www.consoden.se)
*
* Created by: Björn Weström / bjws
*
*******************************************************************************
*
* This file is part of Consodens Programming Challenge 2014.
* This is the player template used by the competing teams.
* It is based on the Safir SDK Core framework. (http://safir.sourceforge.net)
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

#ifndef __JOYSTICK_ENTITY_HANDLER_STRATEGY_IF_H
#define __JOYSTICK_ENTITY_HANDLER_STRATEGY_IF_H

#include <Consoden/TankGame/GameState.h>

namespace TankTeamPlayer
{
    /** 
     * Defines the interface between the JoystickEntityHandler and the Strategy. 
     */
    class JoystickEntityHandlerStrategyIF 
    {
    public:

        /***
          * These methods are used to update the joystick state from the Strategy
          */
        virtual void MoveDirection(Consoden::TankGame::Direction::Enumeration direction) = 0;
        virtual void TowerDirection(Consoden::TankGame::Direction::Enumeration direction) = 0;
        virtual void Fire(bool fire) = 0;

    };
};
#endif
