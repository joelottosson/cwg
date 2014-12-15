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

#include "GameStateHandler.h"
#include <Consoden/TankGame/GameState.h>
#include <Consoden/TankGame/Tank.h>
#include <Safir/Logging/Log.h>


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning (disable: 4127)
#pragma warning (disable: 4251)
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace TankJoyPlayer
{

    GameStateHandler::GameStateHandler(PlayerEntityHandlerGameStateHandlerIF* playerEntityHandlerIF) :
        m_classTypeID(Consoden::TankGame::GameState::ClassTypeId),
        m_activeEntityValid(false),
        m_playerEntityHandlerIF(playerEntityHandlerIF)
    {
    }


    GameStateHandler::~GameStateHandler()
    {
        // Dob will automatically be disconnected.
    }


    void GameStateHandler::Init(Safir::Dob::Typesystem::InstanceId player_id)
    {
        m_playerId = player_id;

        // Attach to the secondary Dob connection.
        m_secDobConnection.Attach();

        // Subscribe for Game state.
        m_secDobConnection.SubscribeEntity(m_classTypeID, this);
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
        if(entityProxy.GetTypeId() == m_classTypeID)
        {
            Safir::Logging::SendSystemLog(Safir::Logging::Informational,
                                          L"New GameState entity: " + entityProxy.GetEntityId().ToString());

            // Check if we are playing this game
            // - missing player id in gamestate...


            if (m_activeEntityValid == false) {
                Safir::Logging::SendSystemLog(Safir::Logging::Informational,
                                              L"Attached to new game!");
                m_activeEntity = entityProxy.GetEntityId();
                m_activeEntityValid = true;

                const Consoden::TankGame::GameStatePtr game_ptr = 
                    boost::static_pointer_cast<Consoden::TankGame::GameState>(entityProxy.GetEntity());
                
                //TankContainerArray::const_iterator tank_iterator;
                // = 
                //    game_ptr->Tanks().const_iterator();

                //for (tank_iterator = game_ptr->Tanks().begin(); tank_iterator != game_ptr->Tanks().end(); tank_iterator++) {
                for (Safir::Dob::Typesystem::ArrayIndex tank_index = 0; 
                     tank_index < game_ptr->TanksArraySize(); 
                     tank_index++) {
                    // Find the tanks that we will control

                    if (game_ptr->Tanks()[tank_index].IsNull()) {
                        // Reached last tank
                        break;
                    }

                    const Consoden::TankGame::TankPtr tank_ptr = 
                        boost::static_pointer_cast<Consoden::TankGame::Tank>(game_ptr->Tanks()[tank_index].GetPtr());

                    if (tank_ptr->PlayerId().GetVal() == m_playerId) {
                        // This is our tank!
                        int tank_id = tank_ptr->TankId();
                        std::wstring tank_id_str;
                        tank_id_str += tank_id;
                        Safir::Logging::SendSystemLog(Safir::Logging::Critical,
                                                      L"Dude, this game is for us, i can haz tank with id: " + tank_id_str);

                        m_playerEntityHandlerIF->NewGameStateCB(tank_id, entityProxy.GetInstanceId());
                        break;
                    }
                }

                 

                // If we have any tanks to control
                // Notify owner
                //m_playerEntityHandlerIF->NewGameStateCB(/* tanks to control */);
            } else {
                Safir::Logging::SendSystemLog(Safir::Logging::Critical,
                                              L"ERROR: Already attached to game... entity: " + m_activeEntity.ToString());
            }

        }
    }

    void GameStateHandler::OnUpdatedEntityInternal(const Safir::Dob::EntityProxy &entityProxy)
    {
        if(entityProxy.GetTypeId() == m_classTypeID && entityProxy.GetEntityId() == m_activeEntity)
        {
            Safir::Logging::SendSystemLog(Safir::Logging::Informational,
                                          L"update gamestate entity: " + entityProxy.GetEntityId().ToString());

            // A real player would react to new state here
            m_playerEntityHandlerIF->UpdateGameStateCB();
        }
    }

    void GameStateHandler::OnDeletedEntityInternal(const Safir::Dob::EntityProxy &entityProxy)
    {
        if(entityProxy.GetTypeId() == m_classTypeID && entityProxy.GetEntityId() == m_activeEntity)
        {
            Safir::Logging::SendSystemLog(Safir::Logging::Informational,
                                          L"delete game state entity: " + entityProxy.GetEntityId().ToString());

            // Deallocate game connection
            m_activeEntityValid = false;

            // Notify owner
            m_playerEntityHandlerIF->DeleteGameStateCB();
        }
    }

} // namespace TankEngine
