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

#ifndef __PLAYER_ENTITY_HANDLER_H
#define __PLAYER_ENTITY_HANDLER_H

#include <Safir/Dob/Connection.h>
#include "PlayerEntityHandlerGameStateHandlerIF.h" 
#include "GameStateHandler.h"
#include "JoystickEntityHandler.h"

namespace TankJoyPlayer
{
    /** 
     * Defines a vehicle owner. This class handles the
     * registration as a data owner and processes request
     * on that data.
     */
    class PlayerEntityHandler :
        // Allows this class to register as an entity owner.
        public Safir::Dob::EntityHandlerInjection,
        public PlayerEntityHandlerGameStateHandlerIF
    {
    public:

        PlayerEntityHandler(boost::asio::io_service& io, 
            const std::wstring& playerName);

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

        void OnInjectedNewEntity(
            const Safir::Dob::InjectedEntityProxy injectedEntityProxy);

        void OnInjectedDeletedEntity(
            const Safir::Dob::InjectedEntityProxy injectedEntityProxy);

        void OnInitialInjectionsDone(
            const Safir::Dob::Typesystem::TypeId typeId,
            const Safir::Dob::Typesystem::HandlerId& handlerId);

        enum Limits {
            MAX_GAMES = 1
        };


        // Callback functions for GameStateListner
        void NewGameStateCB(int tankId, Safir::Dob::Typesystem::InstanceId gameId);
        void UpdateGameStateCB();
        void DeleteGameStateCB();

    private:

        void InitGameStateHandler();


        void CreatePlayer();
        void CreateJoystick(int tankId, Safir::Dob::Typesystem::InstanceId gameId);
        void MoveJoystick();
        void DeleteJoystick();

        // This class uses this secondary connection for DOB calls.
        Safir::Dob::SecondaryConnection m_connection;
        std::wstring mPlayerName;
        Safir::Dob::Typesystem::HandlerId m_HandlerId;
        Safir::Dob::Typesystem::InstanceId m_PlayerId;
        GameStateHandler m_GameStateHandler;
        //KeyboardReader m_KeyboardReader;
        boost::shared_ptr<JoystickEntityHandler> m_JoystickEntityHandlerPtr;

        boost::asio::io_service& io_service_ptr;
    };
};
#endif
