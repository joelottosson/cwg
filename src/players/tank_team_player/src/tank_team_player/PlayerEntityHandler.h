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

#ifndef __PLAYER_ENTITY_HANDLER_H
#define __PLAYER_ENTITY_HANDLER_H

#include <Safir/Dob/Connection.h>

#include "PlayerEntityHandlerGameStateHandlerIF.h" 
#include "GameStateHandler.h"
#include "JoystickEntityHandler.h"
#include "Strategy.h"

namespace TankTeamPlayer
{
    /** 
     * Defines a player owner. This class handles the
     * registration as a data owner and processes request
     * on that data.
     */
    class PlayerEntityHandler :
        public Safir::Dob::EntityHandler,
        public PlayerEntityHandlerGameStateHandlerIF
    {
    public:

        PlayerEntityHandler(const std::wstring& playerName);

        /** 
         * Initiates this class. Creates a secondary DOB
         * connection and registeres as handler.
         */
        void Init();

        /** 
         * Methods derived from Safir::Dob::EntityHandlerInjection.
         */
        void OnCreateRequest(
            const Safir::Dob::EntityRequestProxy entityRequestProxy,
            Safir::Dob::ResponseSenderPtr responseSender);

        void OnUpdateRequest(
            const Safir::Dob::EntityRequestProxy entityRequestProxy,
            Safir::Dob::ResponseSenderPtr responseSender);

        void OnDeleteRequest(
            const Safir::Dob::EntityRequestProxy entityRequestProxy,
            Safir::Dob::ResponseSenderPtr responseSender);

        void OnRevokedRegistration(
            const Safir::Dob::Typesystem::TypeId typeId,
            const Safir::Dob::Typesystem::HandlerId& handlerId);

        /***
          * Local configuration 
          */
        enum Limits {
            MAX_GAMES = 1
        };

        // Implementation of the PlayerEntityHandlerGameStateHandlerIF callback functions
        void NewGameStateCB(
            int tankId, 
            const Consoden::TankGame::GameStatePtr gamePtr, 
            Safir::Dob::Typesystem::InstanceId gameId);
        void UpdateGameStateCB(const Consoden::TankGame::GameStatePtr gamePtr);
        void DeleteGameStateCB();

    private:

        void InitGameStateHandler();
        void InitJoystickEntityHandler();

        void InitStrategy(int tankId, const Consoden::TankGame::GameStatePtr gamePtr);
        void RunStrategy(const Consoden::TankGame::GameStatePtr gamePtr);

        void CreatePlayer();
        void CreateJoystick(
            int tankId, 
            const Consoden::TankGame::GameStatePtr gamePtr,
            Safir::Dob::Typesystem::InstanceId gameId);
        void DeleteJoystick();

        // This class uses this secondary connection for DOB calls.
        Safir::Dob::SecondaryConnection m_Connection;

        std::wstring m_PlayerName;
        Safir::Dob::Typesystem::HandlerId m_HandlerId;
        Safir::Dob::Typesystem::InstanceId m_PlayerId;

        GameStateHandler m_GameStateHandler;
        JoystickEntityHandler m_JoystickEntityHandler;
        Strategy m_Strategy;
    };
};
#endif
