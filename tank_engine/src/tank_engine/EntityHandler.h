/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Björn Weström / bjws
*
*******************************************************************************/

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
