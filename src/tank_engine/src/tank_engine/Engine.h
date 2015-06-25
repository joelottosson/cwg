/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Björn Weström / bjws
*
*******************************************************************************/
#ifndef __ENGINE_H
#define __ENGINE_H

#include <boost/asio.hpp>
#include <vector>
#include <Consoden/TankGame/Joystick.h>
#include <Consoden/TankGame/GameState.h>

#include "JoystickEntityHandler.h"
#include "JoystickEngineIF.h"


#include "GameMap.h"

namespace SDob = Safir::Dob::Typesystem;
namespace CWG= Consoden::TankGame;

namespace TankEngine
{
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
        bool CollisionPredicter(CWG::DudePtr& dude, CWG::TankPtr& tank);

        void dudeUpdater(CWG::DudePtr& dude_ptr, GameMap gm,CWG::GameStatePtr game_ptr);


        /*
         * Creates a random list of the directions.(Each direction occurs only once).
         * the neutral direction is always last.
         *
         * returns: A array of length 5 containing integers corresponding to directions in a random order.
         * The neutral direction(0 in this case)  is always the last element
         *
         * note: Caller must free returned array
         */
        int* directionPermuter();


        std::string FindPlayerName(Safir::Dob::Typesystem::InstanceId playerId);

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
        std::string mPlayerOneName;
        std::string mPlayerTwoName;
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
