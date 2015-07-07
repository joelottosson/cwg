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
#include "JoystickEntityHandler.h"

#include <Consoden/TankGame/Joystick.h>

#include <Safir/Logging/Log.h>
#include <Safir/Dob/SuccessResponse.h>
#include <Safir/Dob/ErrorResponse.h>
#include <Safir/Dob/EntityIdResponse.h>
#include <Safir/Dob/NotFoundException.h>
#include <Safir/Dob/ResponseGeneralErrorCodes.h>

// For randomization
#include <time.h>
#include <stdlib.h>

namespace TankRandomPlayer
{
    JoystickEntityHandler::JoystickEntityHandler(boost::asio::io_service& io, 
        int tankId,
        Safir::Dob::Typesystem::HandlerId handler_id) :
        m_TankId(tankId),
        m_HandlerId(handler_id)
    {
    }

    JoystickEntityHandler::~JoystickEntityHandler()
    {
        DeleteJoystick();
    }
    
    void JoystickEntityHandler::Init(Safir::Dob::Typesystem::InstanceId playerId, Safir::Dob::Typesystem::InstanceId gameId)
    {
        m_connection.Attach();

        // Register as game entity handler.
        //std::wstring tid;
        //tid += m_TankId;
        //m_HandlerId = Safir::Dob::Typesystem::HandlerId(L"Consoden::TankRandomPlayer(" + tid + L")"); //::GenerateRandom();


        CreateJoystick(m_TankId, playerId, gameId);

        // Init random seed
        srandom(clock());
    }

    void JoystickEntityHandler::OnRevokedRegistration(
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

    void JoystickEntityHandler::OnInjectedNewEntity(const Safir::Dob::InjectedEntityProxy injectedEntityProxy)
    {
        Safir::Logging::SendSystemLog(Safir::Logging::Critical,
                                      L"Unexpected InjectNew, inject not supported");
        return;
    }

    void JoystickEntityHandler::OnInjectedDeletedEntity(const Safir::Dob::InjectedEntityProxy injectedEntityProxy)
    {
        Safir::Logging::SendSystemLog(Safir::Logging::Critical,
                                      L"Unexpected InjectDelete, inject not supported");
        return;
    }

    void JoystickEntityHandler::OnInitialInjectionsDone(
        const Safir::Dob::Typesystem::TypeId typeId,
        const Safir::Dob::Typesystem::HandlerId& handlerId)
    {
        // This is just notification - no actions need to be taken.
        (void)typeId;    // fix 'unused' warning
        (void)handlerId; // fix 'unused' warning
    }

    void JoystickEntityHandler::OnCreateRequest(
        const Safir::Dob::EntityRequestProxy entityRequestProxy,
        Safir::Dob::ResponseSenderPtr responseSender)
    {
        Safir::Logging::SendSystemLog(Safir::Logging::Critical,
                                      L"OnCreateRequest: Create request, not supported");

        Safir::Dob::ErrorResponsePtr errorResponse = Safir::Dob::ErrorResponse::Create();
        errorResponse -> Code().SetVal(Safir::Dob::ResponseGeneralErrorCodes::SafirReqErr());
        responseSender -> Send(errorResponse);
    }

    void JoystickEntityHandler::OnUpdateRequest(
        const Safir::Dob::EntityRequestProxy entityRequestProxy,
        Safir::Dob::ResponseSenderPtr responseSender)
    {
        Safir::Logging::SendSystemLog(Safir::Logging::Critical,
                                      L"Unexpected UpdateRequest, not supported");



        Safir::Dob::ErrorResponsePtr errorResponse = Safir::Dob::ErrorResponse::Create();
        errorResponse -> Code().SetVal(Safir::Dob::ResponseGeneralErrorCodes::SafirReqErr());
        responseSender -> Send(errorResponse);
    }

    void JoystickEntityHandler::OnDeleteRequest(
        const Safir::Dob::EntityRequestProxy entityRequestProxy,
        Safir::Dob::ResponseSenderPtr responseSender)
    {
        Safir::Logging::SendSystemLog(Safir::Logging::Critical,
                                      L"Unexpected DeleteRequest, not supported");

        Safir::Dob::ErrorResponsePtr errorResponse = Safir::Dob::ErrorResponse::Create();
        errorResponse -> Code().SetVal(Safir::Dob::ResponseGeneralErrorCodes::SafirReqErr());
        responseSender -> Send(errorResponse);
    }

    void JoystickEntityHandler::CreateJoystick(int tankId, Safir::Dob::Typesystem::InstanceId playerId, Safir::Dob::Typesystem::InstanceId gameId)
    {
        m_InstanceId = Safir::Dob::Typesystem::InstanceId::GenerateRandom();

        // Check if entity with given value already exist.
        Safir::Dob::Typesystem::EntityId entityId = Safir::Dob::Typesystem::EntityId(
            Consoden::TankGame::Joystick::ClassTypeId,
            m_InstanceId);

        if(!m_connection.IsCreated(entityId))
        {
            Consoden::TankGame::JoystickPtr joystick = Consoden::TankGame::Joystick::Create();
            // New state counter
            joystick->Counter().SetVal(1);
            joystick->TankId().SetVal(tankId);
            joystick->PlayerId().SetVal(playerId);
            joystick->GameId().SetVal(gameId);
            joystick->Fire().SetVal(false);

            float r = random();
            r = r / RAND_MAX;
            if (r < 0.25) {
                m_LastDirection = Consoden::TankGame::Direction::Left;
            } else if (r < 0.50) {
                m_LastDirection = Consoden::TankGame::Direction::Right;
            } else if (r < 0.75) {
                m_LastDirection = Consoden::TankGame::Direction::Up;
            } else {
                m_LastDirection = Consoden::TankGame::Direction::Down;
            }

            //player->Name().SetVal(mPlayerName);
            //player->PlayerId().SetVal(instanceId.GetRawValue());
            // Store object in the Dob.
            m_connection.SetAll(joystick, m_InstanceId, m_HandlerId);
            m_JoystickEntity = entityId;
        }

    }

    Consoden::TankGame::Direction::Enumeration JoystickEntityHandler::InvertDirection(Consoden::TankGame::Direction::Enumeration d) {
        switch (d) {
            case Consoden::TankGame::Direction::Left: return Consoden::TankGame::Direction::Right;
            case Consoden::TankGame::Direction::Right: return Consoden::TankGame::Direction::Left;
            case Consoden::TankGame::Direction::Up: return Consoden::TankGame::Direction::Down;
            case Consoden::TankGame::Direction::Down: return Consoden::TankGame::Direction::Up;
            default: return d;
        }
    }

    Consoden::TankGame::Direction::Enumeration JoystickEntityHandler::TurnLeft(Consoden::TankGame::Direction::Enumeration d) {
        switch (d) {
            case Consoden::TankGame::Direction::Left: return Consoden::TankGame::Direction::Down;
            case Consoden::TankGame::Direction::Right: return Consoden::TankGame::Direction::Up;
            case Consoden::TankGame::Direction::Up: return Consoden::TankGame::Direction::Left;
            case Consoden::TankGame::Direction::Down: return Consoden::TankGame::Direction::Right;
            default: return d;
        }
    }

    Consoden::TankGame::Direction::Enumeration JoystickEntityHandler::TurnRight(Consoden::TankGame::Direction::Enumeration d) {
        switch (d) {
            case Consoden::TankGame::Direction::Left: return Consoden::TankGame::Direction::Up;
            case Consoden::TankGame::Direction::Right: return Consoden::TankGame::Direction::Down;
            case Consoden::TankGame::Direction::Up: return Consoden::TankGame::Direction::Right;
            case Consoden::TankGame::Direction::Down: return Consoden::TankGame::Direction::Left;
            default: return d;
        }
    }

    void JoystickEntityHandler::MoveJoystick() 
    {
        // Safir::Logging::SendSystemLog(Safir::Logging::Critical,
        //                              L"JoystickEntityHandler::MoveJoystick");
     
             Safir::Dob::EntityProxy entityProxy = m_connection.Read(m_JoystickEntity);
        Consoden::TankGame::JoystickPtr joystick = 
            boost::static_pointer_cast<Consoden::TankGame::Joystick>(entityProxy.GetEntity());

        Consoden::TankGame::Direction::Enumeration newDirection;

        // Make a random movement
        float r = random();
        r = r / RAND_MAX;

        // Since moving backwards will kill the tank, make that unprobable (1%)
        if (r < 0.01) {
            // Backwards (1%)
            newDirection = InvertDirection(m_LastDirection);
            MoveDirection(newDirection, joystick);
            m_LastDirection = newDirection;
        } else if (r < 0.41) {
            // Forward (40%)
            newDirection = m_LastDirection;
            MoveDirection(newDirection, joystick);
            m_LastDirection = newDirection;
        } else if (r < 0.61) {
            // Turn right (20%)
            newDirection = TurnRight(m_LastDirection);
            MoveDirection(newDirection, joystick);
            m_LastDirection = newDirection;
        } else if (r < 0.81) {
            // Turn left (20%)
            newDirection = TurnLeft(m_LastDirection);
            MoveDirection(newDirection, joystick);
            m_LastDirection = newDirection;
        } else {
            // Stand still (19%) 
            MoveDirection(Consoden::TankGame::Direction::Neutral, joystick);
        }

        // Set a random fire direction
        r = random();
        r = r / RAND_MAX;
        if (r < 0.25) {
            TowerDirection(Consoden::TankGame::Direction::Left, joystick);
        } else if (r < 0.50) {
            TowerDirection(Consoden::TankGame::Direction::Right, joystick);
        } else if (r < 0.75) {
            TowerDirection(Consoden::TankGame::Direction::Up, joystick);
        } else {
            TowerDirection(Consoden::TankGame::Direction::Down, joystick);
        }

        // Fire?
        r = random();
        r = r / RAND_MAX;
        if (r < 0.2) {
            // Yes! 20%
            Fire(true, joystick);
        } else {
            // No! 80%
            Fire(false, joystick);
        }

    }

    void JoystickEntityHandler::MoveNeutral(Consoden::TankGame::JoystickPtr& joystick)
    {
        // New state counter
        joystick->Counter().SetVal(joystick->Counter().GetVal() + 1);

        joystick->MoveDirection().SetVal(Consoden::TankGame::Direction::Neutral);
        m_connection.SetChanges(joystick, m_InstanceId, m_HandlerId);        
    }

    void JoystickEntityHandler::MoveDirection(Consoden::TankGame::Direction::Enumeration move_d, Consoden::TankGame::JoystickPtr& joystick)
    {
        // New state counter
        joystick->Counter().SetVal(joystick->Counter().GetVal() + 1);
        joystick->MoveDirection().SetVal(move_d);
        m_connection.SetChanges(joystick, m_InstanceId, m_HandlerId);
    }

    void JoystickEntityHandler::TowerDirection(Consoden::TankGame::Direction::Enumeration aim, Consoden::TankGame::JoystickPtr& joystick)
    {
        // New state counter
        joystick->Counter().SetVal(joystick->Counter().GetVal() + 1);

        joystick->TowerDirection().SetVal(aim);
        m_connection.SetChanges(joystick, m_InstanceId, m_HandlerId);
    }

    void JoystickEntityHandler::Fire(bool fire, Consoden::TankGame::JoystickPtr& joystick)
    {
        // New state counter
        joystick->Counter().SetVal(joystick->Counter().GetVal() + 1);

        joystick->Fire().SetVal(fire);
        m_connection.SetChanges(joystick, m_InstanceId, m_HandlerId);
    }

    void JoystickEntityHandler::DeleteJoystick()
    {
        m_connection.Delete(m_JoystickEntity, m_HandlerId);
    }
 };
