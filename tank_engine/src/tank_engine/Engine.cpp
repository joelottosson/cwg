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

// Comment this out to deny null movements!
//#define ALLOW_NULL_MOVE

#include "Engine.h"

#include <Consoden/TankGame/GameState.h>

#include <Safir/Logging/Log.h>
#include <Safir/Dob/SuccessResponse.h>
#include <Safir/Dob/ErrorResponse.h>
#include <Safir/Dob/EntityIdResponse.h>
#include <Safir/Dob/NotFoundException.h>
#include <Safir/Dob/ResponseGeneralErrorCodes.h>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "GameMap.h"

namespace TankEngine
{
    Engine::Engine(boost::asio::io_service& io) :
        mGamePrepare(true),
        mGameRunning(false),
        mMissileCleanupRunning(false),
        mTimer(io),
        mCounter(0),
        m_JoystickHandler(this),
        m_cyclicTimeout(JOYSTICK_TIMEOUT)
    {
    }

    void Engine::Init(
        Safir::Dob::Typesystem::EntityId gameEntityId, 
        Safir::Dob::Typesystem::HandlerId game_handler_id, 
        int gamePace)
    {
        m_cyclicTimeout = gamePace;
        mGamePrepare = true;

        m_connection.Attach();

        m_GameEntityId = gameEntityId;
        m_HandlerId = game_handler_id;
        mCounter = 0;

        // Build wait list for tank_ids
        const Consoden::TankGame::GameStatePtr game_ptr =
            boost::static_pointer_cast<Consoden::TankGame::GameState>(m_connection.Read(m_GameEntityId).GetEntity());

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

            m_JoystickWaitIds.push_back(tank_ptr->TankId().GetVal());
        }

        m_JoystickHandler.Init(m_GameEntityId.GetInstanceId());

        mTimer.expires_from_now(boost::posix_time::milliseconds(Engine::INITIAL_TIMEOUT));
        mTimer.async_wait(boost::bind(&Engine::InitTimerExpired, this, boost::asio::placeholders::error));
    }

    void Engine::InitTimerExpired(const boost::system::error_code& e)
    {
        if (!e) 
        {
            // This means that all joysticks were not created during initial timeout - fail
            Safir::Logging::SendSystemLog(Safir::Logging::Critical,
                                          L"Game engine timed out before all required joystick were registered. Please restart.");
        }
    }

    void Engine::JoystickTimerExpired(const boost::system::error_code& e)
    {
        if (!e)
        {
            if (mCounter < 12000)
            {

                if (mGameRunning) {
                    // Cache joystick states
                    CacheJoysticks();

                    if (m_cyclicTimeout <= JOYSTICK_TIMEOUT) {
                        // Refresh immediately
                        UpdateState();
                    } else {
                        // Wait for refresh                
                        mTimer.expires_at(mTimer.expires_at() + boost::posix_time::milliseconds(m_cyclicTimeout - JOYSTICK_TIMEOUT));
                        mTimer.async_wait(boost::bind(&Engine::UpdateTimerExpired, this, boost::asio::placeholders::error));
                    }
                                
                }
            } else {
                Safir::Logging::SendSystemLog(Safir::Logging::Critical,
                                              L"Game timeout - stopping engine!");            
            }
        }
    }  

    void Engine::CacheJoysticks()
    {
        //Safir::Logging::SendSystemLog(Safir::Logging::Critical, L"Caching Joysticks");

        Consoden::TankGame::GameStatePtr game_ptr =
            boost::static_pointer_cast<Consoden::TankGame::GameState>(m_connection.Read(m_GameEntityId).GetEntity());

        // Find all tanks and cache their joysticks
        for (Safir::Dob::Typesystem::ArrayIndex tank_index = 0; 
             tank_index < game_ptr->TanksArraySize(); 
             tank_index++) {

            if (game_ptr->Tanks()[tank_index].IsNull()) {
                // Reached last tank
                break;
            }

            Consoden::TankGame::TankPtr tank_ptr = 
                boost::static_pointer_cast<Consoden::TankGame::Tank>(game_ptr->Tanks()[tank_index].GetPtr());
        
            Safir::Dob::Typesystem::EntityId joystickEntityId = m_JoystickEntityMap[tank_ptr->TankId().GetVal()];
            Consoden::TankGame::JoystickPtr joystick_ptr =
                boost::static_pointer_cast<Consoden::TankGame::Joystick>(m_connection.Read(joystickEntityId).GetEntity());
            m_JoystickCacheMap[tank_ptr->TankId().GetVal()] = joystick_ptr;
        }
    }

    void Engine::UpdateTimerExpired(const boost::system::error_code& e)
    {
        if (!e)
        {
            if (mCounter < 12000)
            {
                ++mCounter;

                if (mGameRunning) {
                    UpdateState();
                }

            } else {
                Safir::Logging::SendSystemLog(Safir::Logging::Critical,
                                              L"Game timeout - stopping engine!");            
            }
        }
    }

    void Engine::UpdateState()
    {
        // Evaluate and publish new state
        Evaluate();

        // Evaluate may stop the game
        if (mGameRunning) {
            // Wait for joystick readout            
            mTimer.expires_at(mTimer.expires_at() + boost::posix_time::milliseconds(JOYSTICK_TIMEOUT));
            mTimer.async_wait(boost::bind(&Engine::JoystickTimerExpired, this, boost::asio::placeholders::error));        
        }
    }  

    void Engine::ScheduleMissileCleanup()
    {
        // Wait unti next turn refresh to clean up exploded missile and move missiles off map
        mTimer.expires_at(mTimer.expires_at() + boost::posix_time::milliseconds(m_cyclicTimeout));
        mTimer.async_wait(boost::bind(&Engine::MissileCleanupTimerExpired, this, boost::asio::placeholders::error));                
    }

    void Engine::MissileCleanupTimerExpired(const boost::system::error_code& e)
    {
        if (!e)
        {
            if (mCounter < 12000)
            {
                ++mCounter;

                if (mMissileCleanupRunning) {
                    Consoden::TankGame::GameStatePtr game_ptr =
                        boost::static_pointer_cast<Consoden::TankGame::GameState>(m_connection.Read(m_GameEntityId).GetEntity());

                    GameMap gm(game_ptr);
                    gm.MoveMissiles();

                    game_ptr->Counter() = game_ptr->Counter() + 1;

                    m_connection.SetChanges(game_ptr, m_GameEntityId.GetInstanceId(), m_HandlerId);        

                    if (gm.MissilesLeft()) {
                        ScheduleMissileCleanup();
                    } else {
                        // We are done
                        mMissileCleanupRunning = false;
                    }
                }
            } else {
                Safir::Logging::SendSystemLog(Safir::Logging::Critical,
                                              L"Game timeout - stopping engine!");            
            }
        }
    }

    void Engine::StopGame()
    {
        mGameRunning = false;
    }

    // Called by Entity handler when gamestate is deleted - makes sure we don't try to clean up missiles afterwards.
    void Engine::StopAll()
    {
        mGameRunning = false;
        mMissileCleanupRunning = false;
    }

    void Engine::NewJoystickCB(int tankId, Safir::Dob::Typesystem::EntityId entityId)
    {
        if (mGameRunning) return; // Ignore new tanks when live
        if (!mGamePrepare) return; // Ignore new tanks when not in startup

        std::wstring tank_id_str;
        tank_id_str += tankId;

        Consoden::TankGame::GameStatePtr game_ptr =
            boost::static_pointer_cast<Consoden::TankGame::GameState>(m_connection.Read(m_GameEntityId).GetEntity());

        Consoden::TankGame::JoystickPtr joystick_ptr =
            boost::static_pointer_cast<Consoden::TankGame::Joystick>(m_connection.Read(entityId).GetEntity());

        std::string player_str("Player X");
        if (game_ptr->PlayerOneId().GetVal() == joystick_ptr->PlayerId().GetVal()) {
            player_str = "Player One";
        } else if (game_ptr->PlayerTwoId().GetVal() == joystick_ptr->PlayerId().GetVal()) {
            player_str = "Player Two";
        }

        std::vector<int>::iterator tit;
        for (tit = m_JoystickWaitIds.begin(); tit != m_JoystickWaitIds.end(); tit++)
        {
            if ((*tit) == tankId) {
                // New joystick matched a tank we are waiting for
                m_JoystickWaitIds.erase(tit);
                m_JoystickEntityMap[tankId] = entityId;

                std::cout << player_str << " Joystick joined our game. Tank id: " << tankId << std::endl;
                Safir::Logging::SendSystemLog(Safir::Logging::Informational,
                                      L"Joystick joined our game. Tank id: " + tank_id_str + L", joystick " + entityId.ToString());

                if (m_JoystickWaitIds.empty()) {
                    // All players are in
                    StartGame();
                }
                return;
            }
        }
        Safir::Logging::SendSystemLog(Safir::Logging::Critical,
                                      L"Found joystick with bad (or duplicate) tank id!" + tank_id_str);
    }

    void Engine::DeleteJoystickCB(Safir::Dob::Typesystem::EntityId entityId)
    {
        std::map<int, Safir::Dob::Typesystem::EntityId>::const_iterator it;
        for (it = m_JoystickEntityMap.begin(); it != m_JoystickEntityMap.end(); it++) {
            if ((*it).second == entityId) {
                // Deleted joystick in this game
                std::wstring tank_id_string;
                tank_id_string += (*it).first;

                if (mGameRunning) {
                    std::cout << "Joystick left before game was over! Stopping game. Tank id: " << (*it).first << std::endl;

                    Safir::Logging::SendSystemLog(Safir::Logging::Informational,
                                              L"Joystick (" + tank_id_string + L", " + (*it).second.ToString() + L") deleted, shutting down!");

                    StopGame();                
                }
            }
        }

    }

    void Engine::StartGame()
    {
        mTimer.cancel();
        mGamePrepare = false;
        mGameRunning = true;
        mMissileCleanupRunning = false;
        mTimer.expires_from_now(boost::posix_time::milliseconds(JOYSTICK_TIMEOUT));
        mTimer.async_wait(boost::bind(&Engine::JoystickTimerExpired, this, boost::asio::placeholders::error));        
    }

    void Engine::Evaluate()
    {
        Consoden::TankGame::GameStatePtr game_ptr =
            boost::static_pointer_cast<Consoden::TankGame::GameState>(m_connection.Read(m_GameEntityId).GetEntity());

        GameMap gm(game_ptr);

        gm.MoveMissiles();

        // Move all tanks according to rules and evaluate resutls
        for (Safir::Dob::Typesystem::ArrayIndex tank_index = 0; 
             (tank_index < game_ptr->TanksArraySize()) && (!game_ptr->Tanks()[tank_index].IsNull()); 
             tank_index++) {

            Consoden::TankGame::TankPtr tank_ptr = 
                boost::static_pointer_cast<Consoden::TankGame::Tank>(game_ptr->Tanks()[tank_index].GetPtr());
        
            if (tank_ptr->InFlames()) {
                // Dead tank, go to next
                continue;
            }

            Consoden::TankGame::JoystickPtr joystick_ptr = m_JoystickCacheMap[tank_ptr->TankId().GetVal()];

            int request_pos_x = tank_ptr->PosX();
            int request_pos_y = tank_ptr->PosY();

#ifndef ALLOW_NULL_MOVE
            if (joystick_ptr->MoveDirection().IsNull()) {
                joystick_ptr->MoveDirection().SetVal(Consoden::TankGame::Direction::Left);
            }
#endif                

            if (!joystick_ptr->MoveDirection().IsNull()) 
            {
                // move tank unless it is moving into a missile
                tank_ptr->MoveDirection() = joystick_ptr->MoveDirection();
                if (!(gm.TankMoveAgainstMissile(tank_ptr->PosX().GetVal(), tank_ptr->PosY().GetVal(), joystick_ptr->MoveDirection()))) 
                {
                    // Calculate move if nothing in the way
                    switch (joystick_ptr->MoveDirection()) {
                        case Consoden::TankGame::Direction::Left:
                        {
                            int new_pos = (tank_ptr->PosX() - 1);
                            if (new_pos < 0) new_pos = (game_ptr->Width().GetVal() - 1); // Wrapping
                            request_pos_x = new_pos;
                            break;
                        }
                        case Consoden::TankGame::Direction::Right:
                        {
                            int new_pos = (tank_ptr->PosX() + 1);
                            new_pos = new_pos % game_ptr->Width().GetVal(); // Wrapping
                            request_pos_x = new_pos;
                            break;
                        }
                        case Consoden::TankGame::Direction::Up:
                        {
                            int new_pos = (tank_ptr->PosY() - 1);
                            if (new_pos < 0) new_pos = (game_ptr->Height().GetVal() - 1); // Wrapping
                            request_pos_y = new_pos;
                            break;
                        }
                        case Consoden::TankGame::Direction::Down:
                        {
                            int new_pos = (tank_ptr->PosY() + 1);
                            new_pos = new_pos % game_ptr->Height().GetVal(); // Wrapping
                            request_pos_y = new_pos;
                            break;
                        }
                    }

                    // Check if something in the way!!
                    if (!gm.WallSquare(request_pos_x, request_pos_y)) {
                        // Moving,  place mine in last position
                        gm.AddMine(tank_ptr->PosX(), tank_ptr->PosY());
                        tank_ptr->PosX() = request_pos_x;
                        tank_ptr->PosY() = request_pos_y;
                    } else {
                        // Crashed into wall!
                        tank_ptr->InFlames() = true;
                        tank_ptr->HitWall() = true;
                    }
                }
            }
        }

        // Fire missiles - this must be done after movement, otherwise the "MoveAgainstMissile" logic is broken
        for (Safir::Dob::Typesystem::ArrayIndex tank_index = 0; 
             (tank_index < game_ptr->TanksArraySize()) && (!game_ptr->Tanks()[tank_index].IsNull()); 
             tank_index++) {

            Consoden::TankGame::TankPtr tank_ptr = 
                boost::static_pointer_cast<Consoden::TankGame::Tank>(game_ptr->Tanks()[tank_index].GetPtr());
        
            if (tank_ptr->InFlames()) {
                // Dead tank, go to next
                continue;
            }

            Consoden::TankGame::JoystickPtr joystick_ptr = m_JoystickCacheMap[tank_ptr->TankId().GetVal()];

            if (!joystick_ptr->TowerDirection().IsNull()) {
                tank_ptr->TowerDirection() = joystick_ptr->TowerDirection();
            } else {
                tank_ptr->TowerDirection().SetNull();
            }

            if (!joystick_ptr->Fire().IsNull() && joystick_ptr->Fire() && !joystick_ptr->TowerDirection().IsNull()) {
                int pos_head_x = -1;
                int pos_head_y = -1;
                int pos_tail_x = -1;
                int pos_tail_y = -1;

                switch (joystick_ptr->TowerDirection()) {
                    case Consoden::TankGame::Direction::Left:
                        pos_head_x = tank_ptr->PosX() - 2;
                        pos_head_y = tank_ptr->PosY();
                        pos_tail_x = tank_ptr->PosX() - 1;
                        pos_tail_y = tank_ptr->PosY();
                        break;

                    case Consoden::TankGame::Direction::Right:
                        pos_head_x = tank_ptr->PosX() + 2;
                        pos_head_y = tank_ptr->PosY();
                        pos_tail_x = tank_ptr->PosX() + 1;
                        pos_tail_y = tank_ptr->PosY();
                        break;

                    case Consoden::TankGame::Direction::Up:
                        pos_head_x = tank_ptr->PosX();
                        pos_head_y = tank_ptr->PosY() - 2;
                        pos_tail_x = tank_ptr->PosX();
                        pos_tail_y = tank_ptr->PosY() - 1;
                        break;

                    case Consoden::TankGame::Direction::Down:
                        pos_head_x = tank_ptr->PosX();
                        pos_head_y = tank_ptr->PosY() + 2;
                        pos_tail_x = tank_ptr->PosX();
                        pos_tail_y = tank_ptr->PosY() + 1;
                        break;

                    default:
                        break;
                }

                bool fired = gm.FireMissile(pos_head_x, pos_head_y, pos_tail_x, pos_tail_y, joystick_ptr->TowerDirection(), tank_ptr->TankId().GetVal());                
                tank_ptr->Fire() = fired; // Only indicate fire if firing was successful
            } else {
                tank_ptr->Fire() = false;
            }
        }


        // Evaluate hits & collissions
        for (Safir::Dob::Typesystem::ArrayIndex tank_index = 0; 
             (tank_index < game_ptr->TanksArraySize()) && (!game_ptr->Tanks()[tank_index].IsNull()); 
             tank_index++) {

            Consoden::TankGame::TankPtr tank_ptr = 
                boost::static_pointer_cast<Consoden::TankGame::Tank>(game_ptr->Tanks()[tank_index].GetPtr());

            // Stepped on mine=
            if (gm.MineSquare(tank_ptr->PosX().GetVal(), tank_ptr->PosY().GetVal())) {
                tank_ptr->InFlames() = true;
                tank_ptr->HitMine() = true;
            }

            // Hit by missile?
            if (gm.IsTankHit(tank_ptr->PosX(), tank_ptr->PosY())) {
                // BOOM!
                tank_ptr->InFlames() = true;
                tank_ptr->HitMissile() = true;
            }

            for (Safir::Dob::Typesystem::ArrayIndex tank2_index = tank_index + 1;
                 (tank2_index < game_ptr->TanksArraySize()) && (!game_ptr->Tanks()[tank2_index].IsNull());
                 tank2_index++) {

                Consoden::TankGame::TankPtr tank2_ptr = 
                    boost::static_pointer_cast<Consoden::TankGame::Tank>(game_ptr->Tanks()[tank2_index].GetPtr());

                if ((tank_ptr->PosX() == tank2_ptr->PosX()) && (tank_ptr->PosY() == tank2_ptr->PosY())) {
                    // x and y coords match for these two tanks - collision!
                    tank_ptr->InFlames() = true;
                    tank_ptr->HitTank() = true;
                    tank2_ptr->InFlames() = true;
                    tank2_ptr->HitTank() = true;
                }
            }
        }

        // Check for game termination states
        bool player_one_in_flames = true;
        bool player_two_in_flames = true;

        for (Safir::Dob::Typesystem::ArrayIndex tank_index = 0; 
             (tank_index < game_ptr->TanksArraySize()) && (!game_ptr->Tanks()[tank_index].IsNull()); 
             tank_index++) {

            Consoden::TankGame::TankPtr tank_ptr = 
                boost::static_pointer_cast<Consoden::TankGame::Tank>(game_ptr->Tanks()[tank_index].GetPtr());

            if (!tank_ptr->InFlames()) {
                if (tank_ptr->PlayerId() == game_ptr->PlayerOneId()) {
                    player_one_in_flames = false;
                } else {
                    player_two_in_flames = false;
                }
            }
        }       

        if (player_one_in_flames && player_two_in_flames) {
            // Draw game
            game_ptr->Winner().SetVal(Consoden::TankGame::Winner::Draw);
            StopGame();
            if (gm.MissilesLeft()) {
                mMissileCleanupRunning = true;
                ScheduleMissileCleanup();
            }

            std::cout << "The game is a draw." << std::endl;
            Safir::Logging::SendSystemLog(Safir::Logging::Informational, L"The game is a draw.");

        } else if (player_one_in_flames) {
            game_ptr->Winner().SetVal(Consoden::TankGame::Winner::PlayerTwo);
            StopGame();
            if (gm.MissilesLeft()) {
                mMissileCleanupRunning = true;
                ScheduleMissileCleanup();
            }

            std::cout << "Player two wins!" << std::endl;
            Safir::Logging::SendSystemLog(Safir::Logging::Informational, L"Player two wins!");

        } else if (player_two_in_flames) {
            game_ptr->Winner().SetVal(Consoden::TankGame::Winner::PlayerOne);
            StopGame();
            if (gm.MissilesLeft()) {
                mMissileCleanupRunning = true;
                ScheduleMissileCleanup();
            }

            std::cout << "Player one wins!" << std::endl;
            Safir::Logging::SendSystemLog(Safir::Logging::Informational, L"Player one wins!.");
        }

        game_ptr->Counter() = game_ptr->Counter() + 1;

        // Push new state
        //gm.Print();
        gm.SetChanges();

        m_connection.SetChanges(game_ptr, m_GameEntityId.GetInstanceId(), m_HandlerId);        
    }
 };
