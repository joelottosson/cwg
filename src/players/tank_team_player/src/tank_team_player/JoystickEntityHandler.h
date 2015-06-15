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

#ifndef __JOYSTICK_ENTITY_HANDLER_H
#define __JOYSTICK_ENTITY_HANDLER_H

#include <Safir/Dob/Connection.h>
#include <Consoden/TankGame/GameState.h>
#include <Consoden/TankGame/Direction.h>
#include <Consoden/TankGame/Joystick.h>

#include "JoystickEntityHandlerStrategyIF.h"

namespace TankTeamPlayer
{
    /** 
     * Defines a joystick owner. This class handles the
     * registration as a data owner and processes request
     * on that data.
     */
    class JoystickEntityHandler :
        public Safir::Dob::EntityHandler,
        public JoystickEntityHandlerStrategyIF
    {
    public:

        JoystickEntityHandler();

        virtual ~JoystickEntityHandler();

        /** 
         * Initiates this class. Creates a secondary DOB
         * connection and registeres as handler.
         */
        void Init(Safir::Dob::Typesystem::HandlerId handlerId);

        /** 
         * Methods derived from Safir::Dob::EntityHandler.
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


        void CreateJoystick(int tankId, 
                            Safir::Dob::Typesystem::InstanceId playerId, 
                            const Consoden::TankGame::GameStatePtr gamePtr,
                            Safir::Dob::Typesystem::InstanceId gameId);

        void DeleteJoystick();

        /***
          * These methods are used to update the joystick state
          */
        void MoveDirection(Consoden::TankGame::Direction::Enumeration direction);
        void TowerDirection(Consoden::TankGame::Direction::Enumeration direction);
        void Fire(bool fire);

    private:

        Consoden::TankGame::JoystickPtr GetJoystick();

        // This class uses this secondary connection for DOB calls.
        Safir::Dob::SecondaryConnection m_Connection;
        int m_TankId;
        Safir::Dob::Typesystem::HandlerId m_HandlerId;
        Safir::Dob::Typesystem::InstanceId m_InstanceId;
        Safir::Dob::Typesystem::EntityId m_JoystickEntity;
        bool m_JoystickEntityExists;
    };
};
#endif
