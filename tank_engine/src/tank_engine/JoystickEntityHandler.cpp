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

#include "JoystickEntityHandler.h"
#include <Consoden/TankGame/Joystick.h>
#include <Safir/Logging/Log.h>
#include "JoystickEngineIF.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning (disable: 4127)
#pragma warning (disable: 4251)
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace TankEngine
{

    JoystickEntityHandler::JoystickEntityHandler(JoystickEngineIF* engineIf) :
        m_engine(engineIf)
    {

    }


    JoystickEntityHandler::~JoystickEntityHandler()
    {
        // Dob will automatically be disconnected.
    }


    void JoystickEntityHandler::Init(Safir::Dob::Typesystem::InstanceId gameId)
    {
        // Attach to the secondary Dob connection.
        m_secDobConnection.Attach();

        // Subscribe for vehicle.
        m_secDobConnection.SubscribeEntity(Consoden::TankGame::Joystick::ClassTypeId, this);
        m_gameId = gameId;
    }

    void JoystickEntityHandler::OnNewEntity(const Safir::Dob::EntityProxy entityProxy)
    {
        OnNewEntityInternal(entityProxy);
    }
    
    void JoystickEntityHandler::OnUpdatedEntity(const Safir::Dob::EntityProxy entityProxy)
    {
        OnUpdatedEntityInternal(entityProxy);
    }

    void JoystickEntityHandler::OnDeletedEntity(const Safir::Dob::EntityProxy entityProxy, const bool /*deprecated*/)
    {
        OnDeletedEntityInternal(entityProxy);
    }

    void JoystickEntityHandler::OnNewEntityInternal(const Safir::Dob::EntityProxy &entityProxy)
    {
        Safir::Logging::SendSystemLog(Safir::Logging::Informational,
                                      L"New Joystick entity: " + entityProxy.GetEntityId().ToString());

        const Consoden::TankGame::JoystickPtr joystick_ptr = 
            boost::static_pointer_cast<Consoden::TankGame::Joystick>(entityProxy.GetEntity());

        if (joystick_ptr->GameId() == m_gameId) {
            // Joystick for our game
            m_engine->NewJoystickCB(joystick_ptr->TankId().GetVal(), entityProxy.GetEntityId());
        }
    }

    void JoystickEntityHandler::OnUpdatedEntityInternal(const Safir::Dob::EntityProxy &entityProxy)
    {
        // Ignored
        /*
        if(entityProxy.GetTypeId() == m_classTypeID)
        {
            Safir::Logging::SendSystemLog(Safir::Logging::Informational,
                                          L"update Joystick entity: " + entityProxy.GetEntityId().ToString());
            
        } */
    }

    void JoystickEntityHandler::OnDeletedEntityInternal(const Safir::Dob::EntityProxy &entityProxy)
    {
        Safir::Logging::SendSystemLog(Safir::Logging::Informational,
                                      L"delete Joystick entity: " + entityProxy.GetEntityId().ToString());

        m_engine->DeleteJoystickCB(entityProxy.GetEntityId());
    }

} // namespace TankEngine
