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

#ifndef __JOYSTICK_ENTITY_HANDLER_H
#define __JOYSTICK_ENTITY_HANDLER_H

#include <Safir/Dob/Connection.h>
#include "GameStateHandler.h"
#include <Consoden/TankGame/Direction.h>

#include "KeyboardReader.h"

namespace TankJoyPlayer
{
    /** 
     * Defines a vehicle owner. This class handles the
     * registration as a data owner and processes request
     * on that data.
     */
    class JoystickEntityHandler :
        // Allows this class to register as an entity owner.
        public Safir::Dob::EntityHandlerInjection
    {
    public:

        JoystickEntityHandler(boost::asio::io_service& io, 
            int tankId,
            Safir::Dob::Typesystem::HandlerId handler_id);

        virtual ~JoystickEntityHandler();

        /** 
         * Initiates this class. Creates a secondary DOB
         * connection and registeres as handler.
         */
        void Init(Safir::Dob::Typesystem::InstanceId playerId, Safir::Dob::Typesystem::InstanceId gameId);

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


        void MoveNeutral();
        void MoveDirection(Consoden::TankGame::Direction::Enumeration move);
        void TowerDirection(Consoden::TankGame::Direction::Enumeration aim);
        void Fire(bool fire);

    private:


        void CreateJoystick(int tankId, Safir::Dob::Typesystem::InstanceId playerId, Safir::Dob::Typesystem::InstanceId gameId);
        void DeleteJoystick();

        // This class uses this secondary connection for DOB calls.
        Safir::Dob::SecondaryConnection m_connection;
        int m_TankId;
        Safir::Dob::Typesystem::HandlerId m_HandlerId;
        Safir::Dob::Typesystem::InstanceId m_InstanceId;
        Safir::Dob::Typesystem::EntityId m_JoystickEntity;
        //GameStateHandler m_GameStateHandler;
        KeyboardReader m_KeyboardReader;
    };
};
#endif
