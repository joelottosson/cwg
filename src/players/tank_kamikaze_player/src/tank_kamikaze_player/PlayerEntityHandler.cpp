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

#include "App.h"
#include "PlayerEntityHandler.h"

#include <Consoden/TankGame/Player.h>
#include <Consoden/TankGame/Joystick.h>

#include <Safir/Logging/Log.h>
#include <Safir/Dob/SuccessResponse.h>
#include <Safir/Dob/ErrorResponse.h>
#include <Safir/Dob/EntityIdResponse.h>
#include <Safir/Dob/NotFoundException.h>
#include <Safir/Dob/ResponseGeneralErrorCodes.h>



namespace TankKamikazePlayer
{
    PlayerEntityHandler::PlayerEntityHandler(boost::asio::io_service& io, 
        const std::wstring& playerName) :
        mPlayerName(playerName),
        m_GameStateHandler(this),
        io_service_ptr(io)
    {
    }

    void PlayerEntityHandler::Init()
    {
        m_connection.Attach();

        // Register as game entity handler.
        m_HandlerId = Safir::Dob::Typesystem::HandlerId(L"Consoden::TankKamikazePlayer(" + mPlayerName + L")"); //::GenerateRandom();

        m_connection.RegisterEntityHandlerInjection(
            Consoden::TankGame::Player::ClassTypeId, 
            m_HandlerId,
            Safir::Dob::InstanceIdPolicy::HandlerDecidesInstanceId,
            this);

        m_connection.RegisterEntityHandlerInjection(
            Consoden::TankGame::Joystick::ClassTypeId, 
            m_HandlerId,
            Safir::Dob::InstanceIdPolicy::HandlerDecidesInstanceId,
            this);

        CreatePlayer();
        InitGameStateHandler();
    }

    void PlayerEntityHandler::OnRevokedRegistration(
        const Safir::Dob::Typesystem::TypeId typeId,
        const Safir::Dob::Typesystem::HandlerId& handlerId)
    {
        // No longer registered for given type.
        Safir::Logging::SendSystemLog(Safir::Logging::Critical,
                                      L"Unexpected revoked registration" +
                                      handlerId.ToString() +  L" is no longer registered for type " +
                                      Safir::Dob::Typesystem::Operations::GetName(typeId));
        // Kill application - bjws hur??
        //App::OnStopOrder();
    }

    void PlayerEntityHandler::OnInjectedNewEntity(const Safir::Dob::InjectedEntityProxy injectedEntityProxy)
    {
        Safir::Logging::SendSystemLog(Safir::Logging::Critical,
                                      L"Unexpected InjectNew, inject not supported");
        return;
    }

    void PlayerEntityHandler::OnInjectedDeletedEntity(const Safir::Dob::InjectedEntityProxy injectedEntityProxy)
    {
        Safir::Logging::SendSystemLog(Safir::Logging::Critical,
                                      L"Unexpected InjectDelete, inject not supported");
        return;
    }

    void PlayerEntityHandler::OnInitialInjectionsDone(
        const Safir::Dob::Typesystem::TypeId typeId,
        const Safir::Dob::Typesystem::HandlerId& handlerId)
    {
        // This is just notification - no actions need to be taken.
        (void)typeId;    // fix 'unused' warning
        (void)handlerId; // fix 'unused' warning
    }

    void PlayerEntityHandler::OnCreateRequest(
        const Safir::Dob::EntityRequestProxy entityRequestProxy,
        Safir::Dob::ResponseSenderPtr responseSender)
    {
        Safir::Logging::SendSystemLog(Safir::Logging::Critical,
                                      L"OnCreateRequest: Create request, not supported");

        Safir::Dob::ErrorResponsePtr errorResponse = Safir::Dob::ErrorResponse::Create();
        errorResponse -> Code().SetVal(Safir::Dob::ResponseGeneralErrorCodes::SafirReqErr());
        responseSender -> Send(errorResponse);
    }

    void PlayerEntityHandler::OnUpdateRequest(
        const Safir::Dob::EntityRequestProxy entityRequestProxy,
        Safir::Dob::ResponseSenderPtr responseSender)
    {
        Safir::Logging::SendSystemLog(Safir::Logging::Critical,
                                      L"Unexpected UpdateRequest, not supported");



        Safir::Dob::ErrorResponsePtr errorResponse = Safir::Dob::ErrorResponse::Create();
        errorResponse -> Code().SetVal(Safir::Dob::ResponseGeneralErrorCodes::SafirReqErr());
        responseSender -> Send(errorResponse);
    }

    void PlayerEntityHandler::OnDeleteRequest(
        const Safir::Dob::EntityRequestProxy entityRequestProxy,
        Safir::Dob::ResponseSenderPtr responseSender)
    {
        Safir::Logging::SendSystemLog(Safir::Logging::Critical,
                                      L"Unexpected DeleteRequest, not supported");

        Safir::Dob::ErrorResponsePtr errorResponse = Safir::Dob::ErrorResponse::Create();
        errorResponse -> Code().SetVal(Safir::Dob::ResponseGeneralErrorCodes::SafirReqErr());
        responseSender -> Send(errorResponse);
    }

    void PlayerEntityHandler::NewGameStateCB(int tankId, const Consoden::TankGame::GameStatePtr game_ptr, Safir::Dob::Typesystem::InstanceId gameId)
    {
        CreateJoystick(tankId, game_ptr, gameId);
    }

    void PlayerEntityHandler::UpdateGameStateCB(const Consoden::TankGame::GameStatePtr game_ptr)
    {
        MoveJoystick(game_ptr);
    }

    void PlayerEntityHandler::DeleteGameStateCB()
    {
        DeleteJoystick();
    }

    void PlayerEntityHandler::CreatePlayer()
    {
        m_PlayerId = Safir::Dob::Typesystem::InstanceId::GenerateRandom();

        // Check if entity with given value already exist.
        Safir::Dob::Typesystem::EntityId entityId = Safir::Dob::Typesystem::EntityId(
            Consoden::TankGame::Player::ClassTypeId,
            m_PlayerId);

        if(!m_connection.IsCreated(entityId))
        {
            Consoden::TankGame::PlayerPtr player = Consoden::TankGame::Player::Create();
            player->Name().SetVal(mPlayerName);
            //player->PlayerId().SetVal(m_PlayerId.GetRawValue());
            // Store object in the Dob.
            m_connection.SetAll(player, m_PlayerId, m_HandlerId);

            // Set the player name

            //bOk = true;
        }

    }

    void PlayerEntityHandler::InitGameStateHandler()
    {
        m_GameStateHandler.Init(m_PlayerId);
    }

    void PlayerEntityHandler::CreateJoystick(int tankId, const Consoden::TankGame::GameStatePtr game_ptr, Safir::Dob::Typesystem::InstanceId gameId)
    {
        m_JoystickEntityHandlerPtr = boost::shared_ptr<JoystickEntityHandler>(new JoystickEntityHandler(io_service_ptr, tankId, m_HandlerId));
        m_JoystickEntityHandlerPtr->Init(m_PlayerId, game_ptr, gameId); 
    }

    void PlayerEntityHandler::MoveJoystick(const Consoden::TankGame::GameStatePtr game_ptr)
    {
        m_JoystickEntityHandlerPtr->MoveJoystick(game_ptr);
    }

    void PlayerEntityHandler::DeleteJoystick()
    {
        m_JoystickEntityHandlerPtr.reset();
    }
 };
