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

#ifndef __ENGINE_H
#define __ENGINE_H

#include <boost/asio.hpp>
#include <vector>
#include <Consoden/TankGame/Joystick.h>
#include <Consoden/TankGame/GameState.h>

#include "JoystickEntityHandler.h"
#include "JoystickEngineIF.h"

namespace TankEngine
{
    /** 
     * Defines a vehicle owner. This class handles the
     * registration as a data owner and processes request
     * on that data.
     */
    class Engine :
        public JoystickEngineIF
    {
    public:

        Engine(boost::asio::io_service& io);

        /** 
         * Initiates this class. 
         */
        void Init(Safir::Dob::Typesystem::EntityId gameEntityId,
                  Safir::Dob::Typesystem::HandlerId game_handler_id,
                  int gamePace,
                  int maxGameTime);

        /** 
         *
         */
        void InitTimerExpired(const boost::system::error_code& e);
        void JoystickTimerExpired(const boost::system::error_code& e);
        void UpdateTimerExpired(const boost::system::error_code& e);
        void MissileCleanupTimerExpired(const boost::system::error_code& e);

        void StopAll();

        enum Timing {
            INITIAL_TIMEOUT = 15 * 1000,
            JOYSTICK_TIMEOUT = 1 * 1000
        };


        virtual void NewJoystickCB(int tankId, Safir::Dob::Typesystem::EntityId entityId);
        virtual void DeleteJoystickCB(Safir::Dob::Typesystem::EntityId entityId);

    private:

        void StartGame();
        void StopGame();
        void CacheJoysticks();
        void UpdateState();
        void Evaluate();
        void ScheduleMissileCleanup();
        //void AddPoints(int points, Safir::Dob::Typesystem::InstanceId player_id, Consoden::TankGame::GameStatePtr game_ptr);
        void AddPoints(int points, int tank_id, Consoden::TankGame::GameStatePtr game_ptr);
        void SetWinner(Consoden::TankGame::GameStatePtr game_ptr);
        Consoden::TankGame::Winner::Enumeration TankIdToWinner(int tank_id) { if (tank_id == mPlayerOneTankId) { return Consoden::TankGame::Winner::PlayerOne; } else { return Consoden::TankGame::Winner::PlayerTwo; } }
        int  OpponentTankId(int tank_id) { if (tank_id == mPlayerOneTankId) { return mPlayerTwoTankId; } else { return mPlayerOneTankId; } }


        bool mGamePrepare;
        bool mGameRunning;
        bool mMissileCleanupRunning;
        boost::asio::deadline_timer mTimer;
        int mCounter;
        int mMaxGameTime;
        int mPlayerOneCounter;
        int mPlayerTwoCounter;
        int mPlayerOneTankId;
        int mPlayerTwoTankId;
        int mPlayerOneTankIndex;
        int mPlayerTwoTankIndex;
        Safir::Dob::Typesystem::EntityId m_GameEntityId;
        Safir::Dob::Typesystem::HandlerId m_HandlerId;
        JoystickEntityHandler m_JoystickHandler;

        std::vector<int> m_JoystickWaitIds;
        std::map<int, Safir::Dob::Typesystem::EntityId> m_JoystickEntityMap;

        // When the game timer expires, we read out all joysticks to this map, and then
        // treat them when the update time expires
        std::map<int, Consoden::TankGame::JoystickPtr> m_JoystickCacheMap; 

        Safir::Dob::SecondaryConnection m_connection;
        int m_cyclicTimeout;
    };
};
#endif
