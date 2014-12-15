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

#ifndef __PLAYER_ENTITY_HANDLER_GAME_STATE_HANDLER_IF_H
#define __PLAYER_ENTITY_HANDLER_GAME_STATE_HANDLER_IF_H

#include <Consoden/TankGame/GameState.h>

namespace TankTeamPlayer
{
    /** 
     * Defines the interface between the GameStateHandler and the PlayerEntity. 
     */
    class PlayerEntityHandlerGameStateHandlerIF 
    {
    public:

        // Callback functions for GameStateHandler
        virtual void NewGameStateCB(int tankId, const Consoden::TankGame::GameStatePtr gamePtr, Safir::Dob::Typesystem::InstanceId gameId) = 0;
        virtual void UpdateGameStateCB(const Consoden::TankGame::GameStatePtr gamePtr) = 0;
        virtual void DeleteGameStateCB() = 0;

    };
};
#endif
