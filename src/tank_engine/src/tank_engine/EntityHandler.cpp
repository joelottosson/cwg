/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Björn Weström / bjws
*
*******************************************************************************/
#include "App.h"
#include "EntityHandler.h"

#include <Consoden/TankGame/GameState.h>

#include <Safir/Logging/Log.h>
#include <Safir/Dob/SuccessResponse.h>
#include <Safir/Dob/ErrorResponse.h>
#include <Safir/Dob/EntityIdResponse.h>
#include <Safir/Dob/NotFoundException.h>
#include <Safir/Dob/ResponseGeneralErrorCodes.h>

#include <iostream>

namespace TankEngine
{
    EntityHandler::EntityHandler(boost::asio::io_service& io) :
        m_ActiveGames(0),
        m_GameCounter(0),
        m_Engine(io)
    {
    }

    void EntityHandler::Init()
    {
        m_connection.Attach();

        // Register as game entity handler.
        m_HandlerId = Safir::Dob::Typesystem::HandlerId();

        m_connection.RegisterEntityHandlerInjection(
            Consoden::TankGame::GameState::ClassTypeId, 
            m_HandlerId,
            Safir::Dob::InstanceIdPolicy::HandlerDecidesInstanceId,
            this);
    }

    void EntityHandler::OnRevokedRegistration(
        const Safir::Dob::Typesystem::TypeId typeId,
        const Safir::Dob::Typesystem::HandlerId& handlerId)
    {
        // No longer registered for given type.
        Safir::Logging::SendSystemLog(Safir::Logging::Critical,
                                      L"Unexpected revoked registration" +
                                      handlerId.ToString() +  L" is no longer registered for type " +
                                      Safir::Dob::Typesystem::Operations::GetName(typeId));
    }

    void EntityHandler::OnInjectedNewEntity(const Safir::Dob::InjectedEntityProxy injectedEntityProxy)
    {
        Safir::Logging::SendSystemLog(Safir::Logging::Critical,
                                      L"Unexpected InjectNew, inject not supported");
        return;
    }

    void EntityHandler::OnInjectedDeletedEntity(const Safir::Dob::InjectedEntityProxy injectedEntityProxy)
    {
        Safir::Logging::SendSystemLog(Safir::Logging::Critical,
                                      L"Unexpected InjectDelete, inject not supported");
        return;
    }

    void EntityHandler::OnInitialInjectionsDone(
        const Safir::Dob::Typesystem::TypeId typeId,
        const Safir::Dob::Typesystem::HandlerId& handlerId)
    {
        // This is just notification - no actions need to be taken.
        (void)typeId;    // fix 'unused' warning
        (void)handlerId; // fix 'unused' warning
    }

    void EntityHandler::OnCreateRequest(
        const Safir::Dob::EntityRequestProxy entityRequestProxy,
        Safir::Dob::ResponseSenderPtr responseSender)
    {
        std::cout << std::endl << "New game created." << std::endl;

        Safir::Logging::SendSystemLog(Safir::Logging::Informational,
                                      L"OnCreateRequest: Create new game");

        //StartRemoveInExercise
        bool bOk = false;
        Safir::Dob::Typesystem::InstanceId instanceId;
        Safir::Dob::Typesystem::EntityId entityId;
        //StopRemoveInExercise

        // Cast to known type, the game entity. Since we expect one class type only,
        // we can perform a static cast (cheaper than a dynamic cast).

        //StartRemoveInSolution
        //const Consoden::TankGame::GameStatePtr game =
        //    boost::static_pointer_cast<Consoden::TankGame::GameState>(entityRequestProxy.GetRequest());
        //StopRemoveInSolution

        //StartRemoveInExercise
        const Consoden::TankGame::GameStatePtr game =
            boost::static_pointer_cast<Consoden::TankGame::GameState>(entityRequestProxy.GetRequest());

        // Identification is a mandatory member.
        //if (!game -> Identification().IsNull())
        if (m_ActiveGames < EntityHandler::MAX_GAMES)
        {
            // Generate instance number from unique value.
            //instanceId = Safir::Dob::Typesystem::InstanceId(game -> Identification().GetVal());
            instanceId = Safir::Dob::Typesystem::InstanceId::GenerateRandom();

            // Check if entity with given value already exist.
            entityId = Safir::Dob::Typesystem::EntityId(
                Consoden::TankGame::GameState::ClassTypeId,
                instanceId);

            if(!m_connection.IsCreated(entityId))
            {
                //if (game->GameId().IsNull()) {
                //    game->GameId().SetVal(m_GameCounter);
                //}   

                if (game->Counter().IsNull()) {
                    game->Counter().SetVal(1);
                }

                game->Winner().SetVal(Consoden::TankGame::Winner::Unknown);   
                game->Survivor().SetVal(Consoden::TankGame::Winner::Unknown);
                game->NextMove().SetVal(0);   

                // Store object in the Dob.
                m_connection.SetAll(game, instanceId, m_HandlerId);
                ++m_GameCounter;
                ++m_ActiveGames;
                bOk = true;
            }
        }

        if (bOk)
        {
            // Inform requestor about the instance.
            Safir::Dob::EntityIdResponsePtr entIdResponse = Safir::Dob::EntityIdResponse::Create();
            entIdResponse ->Assigned().SetVal(entityId);
            responseSender -> Send(entIdResponse);

            // Set up engine
            int pace=1000; //default is 1 sec
            if (!game->GamePace().IsNull())
            {
                pace=static_cast<int>(game->GamePace().GetVal()*1000.0f);
            }
            int maxGameTime=60000; // default 1 minute
            if (!game->GameTime().IsNull())
            {
                maxGameTime=static_cast<int>(game->GameTime().GetVal());
            }
            m_Engine.Init(entityId, m_HandlerId, pace, maxGameTime);
        }
        else
        {
            Safir::Dob::ErrorResponsePtr errorResponse = Safir::Dob::ErrorResponse::Create();
            errorResponse -> Code().SetVal(Safir::Dob::ResponseGeneralErrorCodes::SafirReqErr());
            responseSender -> Send(errorResponse);
        }
        //StopRemoveInExercise
    }

    void EntityHandler::OnUpdateRequest(
        const Safir::Dob::EntityRequestProxy entityRequestProxy,
        Safir::Dob::ResponseSenderPtr responseSender)
    {
        Safir::Logging::SendSystemLog(Safir::Logging::Critical,
                                      L"Unexpected UpdateRequest, game update not supported");


        //StartRemoveInExercise
        bool bOk = false;
        if (bOk)
        {
            responseSender -> Send(Safir::Dob::SuccessResponse::Create());
        }
        else
        {
            Safir::Dob::ErrorResponsePtr errorResponse = Safir::Dob::ErrorResponse::Create();
            errorResponse -> Code().SetVal(Safir::Dob::ResponseGeneralErrorCodes::SafirReqErr());
            responseSender -> Send(errorResponse);
        }

        //StopRemoveInExercise
        
    }

    void EntityHandler::OnDeleteRequest(
        const Safir::Dob::EntityRequestProxy entityRequestProxy,
        Safir::Dob::ResponseSenderPtr responseSender)
    {
        //StartRemoveInSolution
        (void)entityRequestProxy; // fix 'unused' warning
        //StopRemoveInSolution

        //StartRemoveInExercise
        if (m_connection.IsCreated(entityRequestProxy.GetEntityId()))
        {
            m_Engine.StopAll();
            //m_pEngine.reset();

            m_connection.Delete(entityRequestProxy.GetEntityId(),
                m_HandlerId);
            --m_ActiveGames;


        }
        responseSender -> Send(Safir::Dob::SuccessResponse::Create());
        //StopRemoveInExercise
    }
 };
