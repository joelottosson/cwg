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

#ifndef __ENTITY_HANDLER_H
#define __ENTITY_HANDLER_H

#include <Safir/Dob/Connection.h> 
#include "Engine.h"

namespace TankEngine
{
    /** 
     * Defines a vehicle owner. This class handles the
     * registration as a data owner and processes request
     * on that data.
     */
    class EntityHandler :
        // Allows this class to register as an entity owner.
        public Safir::Dob::EntityHandlerInjection
    {
    public:

        EntityHandler(boost::asio::io_service& io);

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

    private:
        // This class uses this secondary connection for DOB calls.
        Safir::Dob::SecondaryConnection m_connection;
        Safir::Dob::Typesystem::HandlerId m_HandlerId;
        int m_ActiveGames;
        int m_GameCounter;
        Engine m_Engine;
    };
};
#endif
