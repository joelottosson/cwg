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



namespace TankTeamPlayer
{
    PlayerEntityHandler::PlayerEntityHandler(
        const std::wstring& playerName) :
        m_PlayerName(playerName),
        m_GameStateHandler(this)
    {
    }

    void PlayerEntityHandler::Init()
    {
        m_Connection.Attach();

        // Register as player and joystick entity handler.
        m_HandlerId = Safir::Dob::Typesystem::HandlerId(L"Consoden::TankTeamPlayer(" + m_PlayerName + L")");

        m_Connection.RegisterEntityHandler(
            Consoden::TankGame::Player::ClassTypeId, 
            m_HandlerId,
            Safir::Dob::InstanceIdPolicy::HandlerDecidesInstanceId,
            this);

        m_Connection.RegisterEntityHandler(
            Consoden::TankGame::Joystick::ClassTypeId, 
            m_HandlerId,
            Safir::Dob::InstanceIdPolicy::HandlerDecidesInstanceId,
            this);

        CreatePlayer();
        InitGameStateHandler();
        InitJoystickEntityHandler();
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

    void PlayerEntityHandler::NewGameStateCB(
        int tankId, 
        const Consoden::TankGame::GameStatePtr gamePtr,
        Safir::Dob::Typesystem::InstanceId gameId)
    {
        CreateJoystick(tankId, gamePtr, gameId);
        InitStrategy(tankId, gamePtr);

        // Evaluate next move and set joystick
        RunStrategy(gamePtr);
    }

    void PlayerEntityHandler::UpdateGameStateCB(const Consoden::TankGame::GameStatePtr gamePtr)
    {
        // Evaluate next move and set joystick
        RunStrategy(gamePtr);
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

        if(!m_Connection.IsCreated(entityId))
        {
            Consoden::TankGame::PlayerPtr player = Consoden::TankGame::Player::Create();
            player->Name().SetVal(m_PlayerName);

            // Store object in the Dob.
            m_Connection.SetAll(player, m_PlayerId, m_HandlerId);
        }

    }

    void PlayerEntityHandler::InitGameStateHandler()
    {
        m_GameStateHandler.Init(m_PlayerId);
    }

    void PlayerEntityHandler::InitJoystickEntityHandler()
    {
        m_JoystickEntityHandler.Init(m_HandlerId);
    }

    void PlayerEntityHandler::InitStrategy(int tankId, const Consoden::TankGame::GameStatePtr gamePtr)
    {
        m_Strategy.Init(tankId, gamePtr, &m_JoystickEntityHandler);
    }

    void PlayerEntityHandler::RunStrategy(const Consoden::TankGame::GameStatePtr gamePtr)
    {
        m_Strategy.Run(gamePtr);
    }

    void PlayerEntityHandler::CreateJoystick(
        int tankId,  
        const Consoden::TankGame::GameStatePtr gamePtr, 
        Safir::Dob::Typesystem::InstanceId gameId)
    {
        m_JoystickEntityHandler.CreateJoystick(tankId, m_PlayerId, gamePtr, gameId); 
    }

    void PlayerEntityHandler::DeleteJoystick()
    {
        m_JoystickEntityHandler.DeleteJoystick();
    }
 };
