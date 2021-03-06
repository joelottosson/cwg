/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Björn Weström / bjws
*
* Hello and welcome to the game engine. This is the greatest code ever written.
* One could say it's the gods gift to computer science.
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

        /**
         * This is the main function in which the game gets updated.
         * this is the game "logic"...and we use logic in the most liberal sense of the word.
         */
        void Evaluate();

        /**
         * Function needed to remove missiles after the game has stopped.
         */
        void ScheduleMissileCleanup();

        bool collisonDetector(CWG::DudePtr& dude, CWG::TankPtr& tank);

        void detonateRedeemer(Consoden::TankGame::GameStatePtr game_ptr, Consoden::TankGame::RedeemerPtr redeemer_ptr, GameMap* gm, int radius);

        /*
         * Nullchecks everything and sets null fields to default values.*
         *
         * this might seem redundant but it keeps	 us from having to manually nullcheck everything 50000000 times in the evaluate loop
         * and get flippin nullpointer exceptions in the DOB which exits the engine and leaves you without a stack to trace.
         * This makes debuggingn about as enjoyable as slowly driving a rusty angle grinder trough your knee whilst having to listen
         * to someone who thinks Ayn Rand talk about their views.
         */
        void nullCheckEverything(Consoden::TankGame::GameStatePtr game_ptr);

        /*
         * Makes the dude choose a new direction based on random movement. alltough the dude will not return to its previous position
         * if there exists any other possible direction.
         *

         */
        void dudeUpdater(CWG::DudePtr& dude_ptr, GameMap gm,CWG::GameStatePtr game_ptr);
        bool fireTheLaser(CWG::TankPtr& own_tank, CWG::TankPtr& enemy_tank , GameMap gm,CWG::GameStatePtr game_ptr);

        std::pair<int,int> directionToVector(CWG::Direction::Enumeration dir);
        std::pair<int,int> addPair(std::pair<int,int> a, std::pair<int,int> b);
        std::pair<int,int> subPair(std::pair<int,int> a, std::pair<int,int> b);

        int wrap(int pos, int size);

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
