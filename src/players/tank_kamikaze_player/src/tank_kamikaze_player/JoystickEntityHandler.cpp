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
#include "GameMap.h"

#include <Consoden/TankGame/Joystick.h>

#include <Safir/Logging/Log.h>
#include <Safir/Dob/SuccessResponse.h>
#include <Safir/Dob/ErrorResponse.h>
#include <Safir/Dob/EntityIdResponse.h>
#include <Safir/Dob/NotFoundException.h>
#include <Safir/Dob/ResponseGeneralErrorCodes.h>

#include <iostream>

// For randomization
#include <time.h>
#include <stdlib.h>

namespace TankKamikazePlayer
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
    
    void JoystickEntityHandler::Init(Safir::Dob::Typesystem::InstanceId playerId, const Consoden::TankGame::GameStatePtr game_ptr, Safir::Dob::Typesystem::InstanceId gameId)
    {
        m_connection.Attach();

        // Register as game entity handler.
        //std::wstring tid;
        //tid += m_TankId;
        //m_HandlerId = Safir::Dob::Typesystem::HandlerId(L"Consoden::TankRandomPlayer(" + tid + L")"); //::GenerateRandom();


        CreateJoystick(m_TankId, playerId, game_ptr, gameId);

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

    void JoystickEntityHandler::CreateJoystick(int tankId, Safir::Dob::Typesystem::InstanceId playerId, const Consoden::TankGame::GameStatePtr game_ptr, Safir::Dob::Typesystem::InstanceId gameId)
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

            m_connection.SetAll(joystick, m_InstanceId, m_HandlerId);
            m_JoystickEntity = entityId;


        }

        MoveJoystick(game_ptr);

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

    void JoystickEntityHandler::MoveJoystick(const Consoden::TankGame::GameStatePtr game_ptr) 
    {
        // Safir::Logging::SendSystemLog(Safir::Logging::Critical,
        //                              L"JoystickEntityHandler::MoveJoystick");
     
        struct timespec start_time;
        clock_gettime(CLOCK_MONOTONIC_RAW, &start_time);
        //clock_t start_time = clock();

        Safir::Dob::EntityProxy entityProxy = m_connection.Read(m_JoystickEntity);
        Consoden::TankGame::JoystickPtr joystick = 
            boost::static_pointer_cast<Consoden::TankGame::Joystick>(entityProxy.GetEntity());

        int player_x = -1;
        int player_y = -1;
        int opponent_x = -1;
        int opponent_y = -1;

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

            if (tank_ptr->TankId().GetVal() == m_TankId) {
                player_x = tank_ptr->PosX().GetVal();
                player_y = tank_ptr->PosY().GetVal();
            } else {
                opponent_x = tank_ptr->PosX().GetVal();
                opponent_y = tank_ptr->PosY().GetVal();
            }
        }

        if (opponent_x == -1 || opponent_y == -1 || player_x == -1 || player_y == -1) {
            // Canot find opponent or player, give up
            Safir::Logging::SendSystemLog(Safir::Logging::Critical,
                                          L"MoveJoystick: Could not find coords of player or oppponent, giving up");
            return;
        }


        Consoden::TankGame::Direction::Enumeration newDirection;

        GameMap gm(game_ptr, m_TankId);

        gm.GenerateShortestPaths(std::make_pair(player_x, player_y));
        std::cout << "Map" << std::endl;
        gm.PrintMap();
        std::cout << "Shortest paths" << std::endl;
        gm.Print();
        gm.GenerateLongestPaths(std::make_pair(player_x, player_y));
        std::cout << "Longest paths" << std::endl;
        gm.Print();
        std::cout << "Missile map" << std::endl;
        gm.PrintMissiles();
        

        newDirection = gm.MoveToEnemy(player_x, player_y, opponent_x, opponent_y);

        MoveDirection(newDirection, joystick);
        TowerDirection(gm.FireToEnemy(), joystick);
        Fire(gm.Fire(), joystick);

        struct timespec end_time;
        clock_gettime(CLOCK_MONOTONIC_RAW, &end_time);

        double duration = double(end_time.tv_sec - start_time.tv_sec) + (double(end_time.tv_nsec - start_time.tv_nsec) / 1000000000.0);
        //std::cout << start_time << " " << end_time << std::endl;
        std::cout << "AI time " << duration << " seconds" << std::endl;
    }

    void JoystickEntityHandler::MoveNeutral(Consoden::TankGame::JoystickPtr& joystick)
    {
        // New state counter
        joystick->Counter().SetVal(joystick->Counter().GetVal() + 1);

        joystick->MoveDirection().SetNull();
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
