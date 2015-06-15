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

#ifndef __STRATEGY_H
#define __STRATEGY_H

#include <Consoden/TankGame/GameState.h>

#include "JoystickEntityHandlerStrategyIF.h"

namespace TankTeamPlayer
{
    /** 
     * Main strategy class. Evaluates the next move for the player,
     * and calls the JoystickEntityHandler with the results.
     */
    class Strategy 
    {
    public:

        Strategy();

        void Init(
            int tankId,
            const Consoden::TankGame::GameStatePtr gamePtr, 
            JoystickEntityHandlerStrategyIF* joyHandler);

        void Run(const Consoden::TankGame::GameStatePtr gamePtr);

    private:
        int m_TankId; // The tank id of the player
        JoystickEntityHandlerStrategyIF* m_JoystickEntityHandlerStrategyIF;
    };
};
#endif
