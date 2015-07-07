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

#include <Consoden/TankGame/GameState.h>
#include <Consoden/TankGame/Tank.h>

#include "GameStateHandler.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning (disable: 4127)
#pragma warning (disable: 4251)
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace TankTeamPlayer
{

    GameStateHandler::GameStateHandler(PlayerEntityHandlerGameStateHandlerIF* playerEntityHandlerIF) :
        m_ClassTypeID(Consoden::TankGame::GameState::ClassTypeId),
        m_ActiveEntityValid(false),
        m_PlayerEntityHandlerIF(playerEntityHandlerIF)
    {
    }


    GameStateHandler::~GameStateHandler()
    {
        // Dob will automatically be disconnected.
    }


    void GameStateHandler::Init(Safir::Dob::Typesystem::InstanceId playerId)
    {
        m_PlayerId = playerId;

        // Attach to the secondary Dob connection.
        m_SecDobConnection.Attach();

        // Subscribe for Game state.
        m_SecDobConnection.SubscribeEntity(m_ClassTypeID, this);
    }

    void GameStateHandler::OnNewEntity(const Safir::Dob::EntityProxy entityProxy)
    {
        OnNewEntityInternal(entityProxy);
    }
    
    void GameStateHandler::OnUpdatedEntity(const Safir::Dob::EntityProxy entityProxy)
    {
        OnUpdatedEntityInternal(entityProxy);
    }

    void GameStateHandler::OnDeletedEntity(const Safir::Dob::EntityProxy entityProxy, const bool /*deprecated*/)
    {
        OnDeletedEntityInternal(entityProxy);
    }

    void GameStateHandler::OnNewEntityInternal(const Safir::Dob::EntityProxy &entityProxy)
    {
        if(entityProxy.GetTypeId() == m_ClassTypeID)
        {
            Safir::Logging::SendSystemLog(Safir::Logging::Informational,
                                          L"New GameState entity: " + entityProxy.GetEntityId().ToString());

            // Check if we are playing this game
            if (m_ActiveEntityValid == false) {
                Safir::Logging::SendSystemLog(Safir::Logging::Informational,
                                              L"Attached to new game!");
                m_ActiveEntity = entityProxy.GetEntityId();
                m_ActiveEntityValid = true;

                const Consoden::TankGame::GameStatePtr gamePtr = 
                    boost::static_pointer_cast<Consoden::TankGame::GameState>(entityProxy.GetEntity());
                
                for (Safir::Dob::Typesystem::ArrayIndex tankIndex = 0; 
                     tankIndex < gamePtr->TanksArraySize(); 
                     tankIndex++) {

                    if (gamePtr->Tanks()[tankIndex].IsNull()) {
                        // Reached last tank
                        break;
                    }

                    const Consoden::TankGame::TankPtr tankPtr = 
                        boost::static_pointer_cast<Consoden::TankGame::Tank>(gamePtr->Tanks()[tankIndex].GetPtr());

                    if (tankPtr->PlayerId().GetVal() == m_PlayerId) {
                        // This is our tank!
                        int tankId = tankPtr->TankId();
                        Safir::Logging::SendSystemLog(Safir::Logging::Informational,
                                                      L"Found a game for us!");

                        // Callback to player entity handler that a new game was created
                        m_PlayerEntityHandlerIF->NewGameStateCB(tankId, gamePtr, entityProxy.GetInstanceId());
                        break;
                    }
                }
            } else {
                Safir::Logging::SendSystemLog(Safir::Logging::Informational,
                                              L"ERROR: Already attached to game... entity: " + m_ActiveEntity.ToString());
            }

        }
    }

    void GameStateHandler::OnUpdatedEntityInternal(const Safir::Dob::EntityProxy &entityProxy)
    {
        if(entityProxy.GetTypeId() == m_ClassTypeID && entityProxy.GetEntityId() == m_ActiveEntity)
        {
            const Consoden::TankGame::GameStatePtr gamePtr = 
                boost::static_pointer_cast<Consoden::TankGame::GameState>(entityProxy.GetEntity());

            // Callback to player entity handler that gamestate is updated
            m_PlayerEntityHandlerIF->UpdateGameStateCB(gamePtr);
        }
    }

    void GameStateHandler::OnDeletedEntityInternal(const Safir::Dob::EntityProxy &entityProxy)
    {
        if(entityProxy.GetTypeId() == m_ClassTypeID && entityProxy.GetEntityId() == m_ActiveEntity)
        {
            Safir::Logging::SendSystemLog(Safir::Logging::Informational,
                                          L"Delete game state entity: " + entityProxy.GetEntityId().ToString());

            // Deallocate game connection
            m_ActiveEntityValid = false;

            // Notify owner
            m_PlayerEntityHandlerIF->DeleteGameStateCB();
        }
    }

} // namespace TankEngine
