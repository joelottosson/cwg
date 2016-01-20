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


#include <Safir/Logging/Log.h>
#include <Safir/Dob/SuccessResponse.h>
#include <Safir/Dob/ErrorResponse.h>
#include <Safir/Dob/EntityIdResponse.h>
#include <Safir/Dob/NotFoundException.h>
#include <Safir/Dob/ResponseGeneralErrorCodes.h>

#include <Consoden/TankGame/Joystick.h>


#include "JoystickEntityHandler.h"

namespace TankTeamPlayer
{
    JoystickEntityHandler::JoystickEntityHandler() :
        m_JoystickEntity(),
        m_JoystickEntityExists(false)
    {
    }

    JoystickEntityHandler::~JoystickEntityHandler()
    {
        if (m_JoystickEntityExists) {
            DeleteJoystick();
        }
    }
    
    void JoystickEntityHandler::Init(Safir::Dob::Typesystem::HandlerId handlerId)        
    {
        m_HandlerId = handlerId;
        m_Connection.Attach();
        //CreateJoystick(m_TankId, playerId, gamePtr, gameId);
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

    void JoystickEntityHandler::CreateJoystick(
        int tankId, 
        Safir::Dob::Typesystem::InstanceId playerId, 
        const Consoden::TankGame::GameStatePtr gamePtr,
        Safir::Dob::Typesystem::InstanceId gameId)
    {
        if (m_JoystickEntityExists) {
            DeleteJoystick();
        }

        m_InstanceId = Safir::Dob::Typesystem::InstanceId::GenerateRandom();

        // Check if entity with given value already exist.
        Safir::Dob::Typesystem::EntityId entityId = Safir::Dob::Typesystem::EntityId(
            Consoden::TankGame::Joystick::ClassTypeId,
            m_InstanceId);

        if(!m_Connection.IsCreated(entityId))
        {
            // Initialize joystick
            Consoden::TankGame::JoystickPtr joystick = Consoden::TankGame::Joystick::Create();
            joystick->Counter().SetVal(1);
            joystick->TankId().SetVal(tankId);
            joystick->PlayerId().SetVal(playerId);
            joystick->GameId().SetVal(gameId);
            joystick->Fire().SetVal(false);
            joystick->MoveDirection().SetVal(Consoden::TankGame::Direction::Left);
            joystick->TowerDirection().SetVal(Consoden::TankGame::Direction::Left);

            // Store object in the Dob.
            m_Connection.SetAll(joystick, m_InstanceId, m_HandlerId);
            m_JoystickEntity = entityId;
            m_JoystickEntityExists = true;
        }
    }

    Consoden::TankGame::JoystickPtr JoystickEntityHandler::GetJoystick()
    {
        // Fetch joystick entity for updating
        Safir::Dob::EntityProxy entityProxy = m_Connection.Read(m_JoystickEntity);
        Consoden::TankGame::JoystickPtr joystick = 
            boost::dynamic_pointer_cast<Consoden::TankGame::Joystick>(entityProxy.GetEntity());        
        return joystick;
    }

    /***
      * Use this method to change the direction of movement for the tank
      */
    void JoystickEntityHandler::MoveDirection(Consoden::TankGame::Direction::Enumeration direction)
    {
        Consoden::TankGame::JoystickPtr joystick = GetJoystick();

        // New state counter
        joystick->Counter().SetVal(joystick->Counter().GetVal() + 1);

        joystick->MoveDirection().SetVal(direction);
        m_Connection.SetChanges(joystick, m_InstanceId, m_HandlerId);
    }

    /***
      * Use this method to change the direction of missile fire
      */
    void JoystickEntityHandler::TowerDirection(Consoden::TankGame::Direction::Enumeration direction)
    {
        Consoden::TankGame::JoystickPtr joystick = GetJoystick();

        // New state counter
        joystick->Counter().SetVal(joystick->Counter().GetVal() + 1);

        joystick->TowerDirection().SetVal(direction);
        m_Connection.SetChanges(joystick, m_InstanceId, m_HandlerId);
    }

    /***
      * Use this method to change wheter missiles should be fired
      */
    void JoystickEntityHandler::Fire(bool fire)
    {
        Consoden::TankGame::JoystickPtr joystick = GetJoystick();

        // New state counter
        joystick->Counter().SetVal(joystick->Counter().GetVal() + 1);

        joystick->Fire().SetVal(fire);
        m_Connection.SetChanges(joystick, m_InstanceId, m_HandlerId);
    }

    void JoystickEntityHandler::DeleteJoystick()
    {
        if (m_JoystickEntityExists) {
            m_Connection.Delete(m_JoystickEntity, m_HandlerId);
            m_JoystickEntityExists = false;
        }
    }
 };
