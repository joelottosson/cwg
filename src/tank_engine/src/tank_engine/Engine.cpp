/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Björn Weström / bjws
*
*******************************************************************************/

// Comment this out to deny null movements!
//#define ALLOW_NULL_MOVE

#include "Engine.h"

#include <Consoden/TankGame/GameState.h>
#include <Consoden/TankGame/Player.h>

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
        mMaxGameTime(0),
        mPlayerOneCounter(0),
        mPlayerTwoCounter(0),
        mPlayerOneTankId(-1),
        mPlayerTwoTankId(-1),
        mPlayerOneTankIndex(-1),
        mPlayerTwoTankIndex(-1),
        m_JoystickHandler(this),
        m_cyclicTimeout(JOYSTICK_TIMEOUT)
    {
    }

    void Engine::Init(
        Safir::Dob::Typesystem::EntityId gameEntityId, 
        Safir::Dob::Typesystem::HandlerId game_handler_id, 
        int gamePace,
        int maxGameTime)
    {
        m_cyclicTimeout = gamePace;
        mGamePrepare = true;

        m_connection.Attach();

        m_GameEntityId = gameEntityId;
        m_HandlerId = game_handler_id;
        mCounter = 0;
        mMaxGameTime = maxGameTime;

        // Cleanup old games
        mTimer.cancel();
        m_JoystickWaitIds.clear();

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

            if (game_ptr->PlayerOneId() == tank_ptr->PlayerId()) {
                mPlayerOneTankId = tank_ptr->TankId();
                mPlayerOneTankIndex = tank_index;
            } else {
                mPlayerTwoTankId = tank_ptr->TankId();
                mPlayerTwoTankIndex = tank_index;
            }

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
                                          L"Game engine timed out before all required joystick were registered. Please make sure both players are running. Waiting...");
        }
    }

    void Engine::JoystickTimerExpired(const boost::system::error_code& e)
    {
        if (!e)
        {
            if (mCounter < mMaxGameTime)
            {

                if (mGameRunning) {
                    // Cache joystick states
                    CacheJoysticks();
                    mCounter++;

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
                Safir::Logging::SendSystemLog(Safir::Logging::Informational,
                                              L"Game timeout - stopping game!"); 
    
                Consoden::TankGame::GameStatePtr game_ptr =
                boost::static_pointer_cast<Consoden::TankGame::GameState>(m_connection.Read(m_GameEntityId).GetEntity());

                GameMap gm(game_ptr);

                // Draw game, both players get one point
                AddPoints(1, mPlayerOneTankId, game_ptr);
                AddPoints(1, mPlayerTwoTankId, game_ptr);

                std::cout << "Both tanks survived, game timeout." << std::endl;
                Safir::Logging::SendSystemLog(Safir::Logging::Informational, L"Both tanks survived, game timeout.");

                game_ptr->Survivor().SetVal(Consoden::TankGame::Winner::Draw);
                SetWinner(game_ptr);
                StopGame();
                if (gm.MissilesLeft()) {
                    mMissileCleanupRunning = true;
                    ScheduleMissileCleanup();
                }

                game_ptr->ElapsedTime().SetVal(mCounter);

                gm.SetChanges();

                m_connection.SetChanges(game_ptr, m_GameEntityId.GetInstanceId(), m_HandlerId);        

            }
        }
    }  

    void Engine::CacheJoysticks()
    {
        //Safir::Logging::SendSystemLog(Safir::Logging::Critical, L"Caching Joysticks");

        Consoden::TankGame::GameStatePtr game_ptr =
            boost::static_pointer_cast<Consoden::TankGame::GameState>(m_connection.Read(m_GameEntityId).GetEntity());

        Safir::Dob::Typesystem::EntityId joystickEntityId1 = m_JoystickEntityMap[mPlayerOneTankIndex];
        Consoden::TankGame::JoystickPtr joystick_ptr1 =
            boost::static_pointer_cast<Consoden::TankGame::Joystick>(m_connection.Read(joystickEntityId1).GetEntity());
        m_JoystickCacheMap[mPlayerOneTankIndex] = joystick_ptr1;

        Safir::Dob::Typesystem::EntityId joystickEntityId2 = m_JoystickEntityMap[mPlayerTwoTankIndex];
        Consoden::TankGame::JoystickPtr joystick_ptr2 =
            boost::static_pointer_cast<Consoden::TankGame::Joystick>(m_connection.Read(joystickEntityId2).GetEntity());
        m_JoystickCacheMap[mPlayerTwoTankIndex] = joystick_ptr2;
    }

    void Engine::UpdateTimerExpired(const boost::system::error_code& e)
    {
        if (!e)
        {
            if (mCounter < 12000)
            {
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

    std::string Engine::FindPlayerName(Safir::Dob::Typesystem::InstanceId playerId)
    {
        for (Safir::Dob::EntityIterator it = m_connection.GetEntityIterator
                 (Consoden::TankGame::Player::ClassTypeId,true);
             it != Safir::Dob::EntityIterator(); ++it)
        {
            if (playerId == (*it).GetInstanceId()) 
            {
                Consoden::TankGame::PlayerPtr player_ptr =
                   boost::static_pointer_cast<Consoden::TankGame::Player>((*it).GetEntity());
                return Safir::Dob::Typesystem::Utilities::ToUtf8(player_ptr->Name().GetVal().c_str());
            }
        }

        return "ERROR, could not find player instance";        
    }

    void Engine::NewJoystickCB(int tankId, Safir::Dob::Typesystem::EntityId entityId)
    {
        if (mGameRunning) return; // Ignore new tanks when live
        if (!mGamePrepare) return; // Ignore new tanks when not in startup

        Consoden::TankGame::GameStatePtr game_ptr =
            boost::static_pointer_cast<Consoden::TankGame::GameState>(m_connection.Read(m_GameEntityId).GetEntity());

        Consoden::TankGame::JoystickPtr joystick_ptr =
            boost::static_pointer_cast<Consoden::TankGame::Joystick>(m_connection.Read(entityId).GetEntity());

        std::string player_str("Player X");
        if (game_ptr->PlayerOneId().GetVal() == joystick_ptr->PlayerId().GetVal()) {
            player_str = "Player One";
            mPlayerOneCounter = joystick_ptr->Counter().GetVal();
            mPlayerOneName = FindPlayerName(game_ptr->PlayerOneId());
            player_str += " (" + mPlayerOneName + ")";

        } else if (game_ptr->PlayerTwoId().GetVal() == joystick_ptr->PlayerId().GetVal()) {
            player_str = "Player Two";
            mPlayerTwoCounter = joystick_ptr->Counter().GetVal();
            mPlayerTwoName = FindPlayerName(game_ptr->PlayerTwoId());
            player_str += " (" + mPlayerTwoName + ")";
        }

        std::vector<int>::iterator tit;
        for (tit = m_JoystickWaitIds.begin(); tit != m_JoystickWaitIds.end(); tit++)
        {
            if ((*tit) == tankId) {
                // New joystick matched a tank we are waiting for
                m_JoystickWaitIds.erase(tit);
                m_JoystickEntityMap[tankId] = entityId;

                std::cout << player_str << " Joystick joined our game. Tank id: " << boost::lexical_cast<std::string>(tankId) << std::endl;
                Safir::Logging::SendSystemLog(Safir::Logging::Informational,
                                      L"Joystick joined our game. Tank id: " + boost::lexical_cast<std::wstring>(tankId) + L", joystick " + entityId.ToString());

                if (m_JoystickWaitIds.empty()) {
                    // All players are in
                    StartGame();
                }
                return;
            }
        }
        Safir::Logging::SendSystemLog(Safir::Logging::Critical,
                                      L"Found joystick with bad (or duplicate) tank id: " + boost::lexical_cast<std::wstring>(tankId));
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
                    StopGame();                

                    std::cout << "Joystick left before game was over. Walk over by ";
                    if ((*it).first == mPlayerOneTankId) {
                        std::cout << "Player One (" << mPlayerOneName << ")" << std::endl;
                    } else {
                        std::cout << "Player Two (" << mPlayerTwoName << ")" << std::endl;                        
                    }

                    Safir::Logging::SendSystemLog(Safir::Logging::Informational,
                                              L"Joystick (" + tank_id_string + L", " + (*it).second.ToString() + L") deleted, walk over!");

                    Consoden::TankGame::GameStatePtr game_ptr =
                        boost::static_pointer_cast<Consoden::TankGame::GameState>(m_connection.Read(m_GameEntityId).GetEntity());
                    // Walk over, 5 point to opponent
                    int loser_tank_id = (*it).first;
                    AddPoints(5, OpponentTankId(loser_tank_id), game_ptr);
                    game_ptr->Survivor().SetVal(TankIdToWinner(OpponentTankId(loser_tank_id)));
                    SetWinner(game_ptr);
                    m_connection.SetChanges(game_ptr, m_GameEntityId.GetInstanceId(), m_HandlerId);        
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
        mPlayerOneCounter = 6000000; // Set to large value to ensure first value from player is not this
        mPlayerTwoCounter = 6000000;
        mCounter = 0;
        mTimer.expires_from_now(boost::posix_time::milliseconds(JOYSTICK_TIMEOUT));
        mTimer.async_wait(boost::bind(&Engine::JoystickTimerExpired, this, boost::asio::placeholders::error));        

        // Game started, set NextMove time
        Consoden::TankGame::GameStatePtr game_ptr =
            boost::static_pointer_cast<Consoden::TankGame::GameState>(m_connection.Read(m_GameEntityId).GetEntity());
        game_ptr->NextMove().SetVal(GameMap::TimerTimeout(mTimer.expires_from_now()));
        m_connection.SetChanges(game_ptr, m_GameEntityId.GetInstanceId(), m_HandlerId);        
    }



    /*
     * I think this actually is the game "logic"
     */
    void Engine::Evaluate()
    {
        Consoden::TankGame::GameStatePtr game_ptr =
            boost::static_pointer_cast<Consoden::TankGame::GameState>(m_connection.Read(m_GameEntityId).GetEntity());

        GameMap gm(game_ptr);

        gm.MoveMissiles();
        bool tank_tank_collission = false;


        //TODO: lets try to move the dudeinator
        CWG::DudePtr dude_ptr = game_ptr->TheDude().GetPtr();
        int lame_x = -1;
        int lame_y= -1;
        CWG::Direction::Enumeration candidate_direction = CWG::Direction::Neutral;
        CWG::Direction::Enumeration lame_direction = CWG::Direction::Neutral;
        bool found_new_way = false;
        if(!dude_ptr->Dying()){
			int dude_new_x ;
			int dude_new_y ;

			int* random_list = directionPermuter();
			for(int i = 0; i < 4; i++){
				dude_new_x = dude_ptr->PosX();
				dude_new_y = dude_ptr->PosY();
				switch (random_list[i]) {
					case 1:
						candidate_direction = CWG::Direction::Up;
						dude_new_y--;
						break;
					case 2:
						candidate_direction = CWG::Direction::Down;
						dude_new_y++;
						break;
					case 3:
						candidate_direction = CWG::Direction::Left;
						dude_new_x--;
						break;
					case 4:
						candidate_direction = CWG::Direction::Right;
						dude_new_x++;
						break;
					case 5:
						candidate_direction = CWG::Direction::Neutral;
						if(i != 4){
							std::wcout << "Neutral was not the last direction :(" << std::endl;
						}
						//dude_new_x++;
						break;
					default:
						break;
				}

				//Lets skip mines and wrapping for now.
				if(		gm.WallSquare(dude_new_x, dude_new_y)
						|| dude_new_x < 0
						|| dude_new_x >= game_ptr->Width()
						|| dude_new_y < 0
						|| dude_new_y >= game_ptr->Height()
					){

					//Direction leads to silly possition, do nothing.

				}else{
					if((dude_new_x == dude_ptr->OldX() && dude_new_y == dude_ptr->OldY()) || candidate_direction == CWG::Direction::Neutral){
						lame_x = dude_new_x;
						lame_y = dude_new_y;
						lame_direction = candidate_direction;


					}else{
						dude_ptr->OldX() = dude_ptr->PosX();
						dude_ptr->OldY() = dude_ptr->PosY();
						dude_ptr->PosX() = dude_new_x;
						dude_ptr->PosY() = dude_new_y;
						dude_ptr->Direction() = candidate_direction;
						found_new_way = true;
						delete random_list;
						break;

					}
				}
			}

			if(!found_new_way){
				dude_ptr->OldX() = dude_ptr->PosX();
				dude_ptr->OldY() = dude_ptr->PosY();
				dude_ptr->PosX() = lame_x;
				dude_ptr->PosY() = lame_y;
				dude_ptr->Direction() = lame_direction;
				delete random_list;

			}
        }

        // Check for frontal collission
        for (Safir::Dob::Typesystem::ArrayIndex tank_index = 0; 
             (tank_index < game_ptr->TanksArraySize()) && (!game_ptr->Tanks()[tank_index].IsNull()); 
             tank_index++) {

            Consoden::TankGame::TankPtr tank_ptr = 
                boost::static_pointer_cast<Consoden::TankGame::Tank>(game_ptr->Tanks()[tank_index].GetPtr());

            //We might need to have this before any movement for the detector to work
            if ((gm.DudeSquare(tank_ptr->PosX().GetVal(), tank_ptr->PosY().GetVal()) || CollisionPredicter(dude_ptr,tank_ptr) )&& !dude_ptr->Dying() ) {
            	std::wcout << "DUDE WAS IT BY TANK!!!!" << std::endl;
                dude_ptr->Dying().SetVal(true);
                AddPoints(-5, tank_ptr->TankId(), game_ptr);

            }


            Consoden::TankGame::JoystickPtr joystick_ptr = m_JoystickCacheMap[tank_ptr->TankId().GetVal()];

            for (Safir::Dob::Typesystem::ArrayIndex tank2_index = tank_index + 1;
                 (tank2_index < game_ptr->TanksArraySize()) && (!game_ptr->Tanks()[tank2_index].IsNull());
                 tank2_index++) {

                Consoden::TankGame::TankPtr tank2_ptr = 
                    boost::static_pointer_cast<Consoden::TankGame::Tank>(game_ptr->Tanks()[tank2_index].GetPtr());

                Consoden::TankGame::JoystickPtr joystick2_ptr = m_JoystickCacheMap[tank2_ptr->TankId().GetVal()];

                if (!tank_ptr->InFlames() && !tank2_ptr->InFlames()) {
                    int w = game_ptr->Width().GetVal();
                    int h = game_ptr->Height().GetVal();

                    if ((tank_ptr->PosX() == ((tank2_ptr->PosX() + 1) % w)) && 
                        (tank_ptr->PosY() == tank2_ptr->PosY()) && 
                        (joystick_ptr->MoveDirection().GetVal() == Consoden::TankGame::Direction::Left) &&
                        (joystick2_ptr->MoveDirection().GetVal() == Consoden::TankGame::Direction::Right)) {
                        tank_tank_collission = true;
                    }

                    if ((((tank_ptr->PosX() + 1) % w) == tank2_ptr->PosX()) && 
                        (tank_ptr->PosY() == tank2_ptr->PosY()) && 
                        (joystick_ptr->MoveDirection().GetVal() == Consoden::TankGame::Direction::Right) &&
                        (joystick2_ptr->MoveDirection().GetVal() == Consoden::TankGame::Direction::Left)) {
                        tank_tank_collission = true;
                    }

                    if ((tank_ptr->PosX() == tank2_ptr->PosX()) && 
                        (tank_ptr->PosY() == ((tank2_ptr->PosY() + 1) % h)) && 
                        (joystick_ptr->MoveDirection().GetVal() == Consoden::TankGame::Direction::Up) &&
                        (joystick2_ptr->MoveDirection().GetVal() == Consoden::TankGame::Direction::Down)) {
                        tank_tank_collission = true;
                    }

                    if ((tank_ptr->PosX() == tank2_ptr->PosX()) && 
                        (((tank_ptr->PosY() + 1) % h) == tank2_ptr->PosY()) && 
                        (joystick_ptr->MoveDirection().GetVal() == Consoden::TankGame::Direction::Down) &&
                        (joystick2_ptr->MoveDirection().GetVal() == Consoden::TankGame::Direction::Up)) {
                        tank_tank_collission = true;
                    }

                    if (tank_tank_collission) {
                        tank_ptr->InFlames() = true;
                        tank_ptr->HitTank() = true;
                        tank_ptr->MoveDirection() = joystick_ptr->MoveDirection();
                        tank_ptr->Fire() = false;
                        tank_ptr->TookCoin() = false;
                        tank_ptr->HitPoisonGas() = false;
                        tank2_ptr->InFlames() = true;
                        tank2_ptr->HitTank() = true;
                        tank2_ptr->MoveDirection() = joystick2_ptr->MoveDirection();
                        tank2_ptr->Fire() = false;
                        tank2_ptr->TookCoin() = false;
                        tank2_ptr->HitPoisonGas() = false;
                    }                    
                }
            }
        }


        for (Safir::Dob::Typesystem::ArrayIndex tank_index = 0; 
             (tank_index < game_ptr->TanksArraySize()) && (!game_ptr->Tanks()[tank_index].IsNull()); 
             tank_index++) {

            Consoden::TankGame::TankPtr tank_ptr = 
                boost::static_pointer_cast<Consoden::TankGame::Tank>(game_ptr->Tanks()[tank_index].GetPtr());
        
            tank_ptr->Fire() = false;
            tank_ptr->TookCoin() = false;
            tank_ptr->HitPoisonGas() = false;

            if (tank_ptr->InFlames()) {
                // Dead tank, go to next
                continue;
            }

            Consoden::TankGame::JoystickPtr joystick_ptr = m_JoystickCacheMap[tank_ptr->TankId().GetVal()];

            // Player one made a move?
            if (game_ptr->PlayerOneId().GetVal() == joystick_ptr->PlayerId().GetVal()) {
                if (mPlayerOneCounter == joystick_ptr->Counter().GetVal()) {
                    std::cout << "Player One (" << mPlayerOneName << ") didn't make a move in time! ElapsedTime: " << mCounter << std::endl;
                    Safir::Logging::SendSystemLog(Safir::Logging::Informational,
                                            L"Player one didn't make a move in time!");
                    // Treat as neutral move
                    joystick_ptr->MoveDirection().SetVal(Consoden::TankGame::Direction::Neutral);
                    joystick_ptr->TowerDirection().SetVal(Consoden::TankGame::Direction::Left);
                    joystick_ptr->Fire().SetVal(false);
                    joystick_ptr->MineDrop().SetVal(false);
                }
                mPlayerOneCounter = joystick_ptr->Counter().GetVal();
            }

            // Player two made a move?
            if (game_ptr->PlayerTwoId().GetVal() == joystick_ptr->PlayerId().GetVal()) {
                if (mPlayerTwoCounter == joystick_ptr->Counter().GetVal()) {
                    std::cout << "Player Two (" << mPlayerTwoName << ") didn't make a move in time! ElapsedTime: " << mCounter << std::endl;
                    Safir::Logging::SendSystemLog(Safir::Logging::Informational,
                                            L"Player two didn't make a move in time!");
                    // Treat as neutral move
                    joystick_ptr->MoveDirection().SetVal(Consoden::TankGame::Direction::Neutral);
                    joystick_ptr->TowerDirection().SetVal(Consoden::TankGame::Direction::Left);
                    joystick_ptr->Fire().SetVal(false);
                    joystick_ptr->MineDrop().SetVal(false);
                }
                mPlayerTwoCounter = joystick_ptr->Counter().GetVal();
            }

 
            int request_pos_x = tank_ptr->PosX();
            int request_pos_y = tank_ptr->PosY();

            if (joystick_ptr->MoveDirection().IsNull()) {
                joystick_ptr->MoveDirection().SetVal(Consoden::TankGame::Direction::Neutral);
            }

            if (joystick_ptr->MoveDirection() != Consoden::TankGame::Direction::Neutral) 
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
                        case Consoden::TankGame::Direction::Neutral:
                        {
                            /* Should not enter here, covered by earlier if clause. Just to silence compiler */
                            break;
                        }
                    }

                    // Check if something in the way!!
                    if (!gm.WallSquare(request_pos_x, request_pos_y)) {
                        // Moving,  place mine in last position?
                        if (joystick_ptr->MineDrop().IsNull() || joystick_ptr->MineDrop()) {
                            gm.AddMine(tank_ptr->PosX(), tank_ptr->PosY());
                        }
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

            // Stepped on mine
            if (gm.MineSquare(tank_ptr->PosX().GetVal(), tank_ptr->PosY().GetVal())) {
                tank_ptr->InFlames() = true;
                tank_ptr->HitMine() = true;
            }

            //TODO: dude detection
            // Killed the dude :'(
/*            if ((gm.DudeSquare(tank_ptr->PosX().GetVal(), tank_ptr->PosY().GetVal()) || CollisionPredicter(dude_ptr,tank_ptr) )&& !dude_ptr->Dying() ) {
            	std::wcout << "DUDE WAS IT BY TANK!!!!" << std::endl;
                dude_ptr->Dying().SetVal(true);
                AddPoints(-5, tank_ptr->TankId(), game_ptr);

            }*/

            // Hit by missile?
            if (gm.IsTankHit(tank_ptr->PosX(), tank_ptr->PosY())) {
                // BOOM!
                // Hit by missile awards two points to opponent
                AddPoints(2, OpponentTankId(tank_ptr->TankId()), game_ptr);

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
                    tank_tank_collission = true;
                }
            }

            if (!tank_tank_collission) {
                // Check for coin
                if (gm.CoinSquare(tank_ptr->PosX(), tank_ptr->PosY())) {
                    gm.ClearSquare(tank_ptr->PosX(), tank_ptr->PosY()); //take the coin

                    // One point for taking a coin
                    AddPoints(1, tank_ptr->TankId(), game_ptr);
                    tank_ptr->TookCoin() = true;

                } else if (gm.PoisonSquare(tank_ptr->PosX(), tank_ptr->PosY())) {
                    // Give one point to the opponent for driving into poison gas. 
                    gm.ClearSquare(tank_ptr->PosX(), tank_ptr->PosY()); //remove poison
                    AddPoints(1, OpponentTankId(tank_ptr->TankId()), game_ptr);
                    tank_ptr->HitPoisonGas() = true;

                } else {
                    // Clear took coin and gas
                    tank_ptr->TookCoin() = false;
                    tank_ptr->HitPoisonGas() = false;
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
            // Draw game, both players get one point
            AddPoints(1, mPlayerOneTankId, game_ptr);
            AddPoints(1, mPlayerTwoTankId, game_ptr);

            std::cout << "Both players died." << std::endl;
            Safir::Logging::SendSystemLog(Safir::Logging::Informational, L"Both players died.");

            game_ptr->Survivor().SetVal(Consoden::TankGame::Winner::Draw);
            SetWinner(game_ptr);
            StopGame();
            if (gm.MissilesLeft()) {
                mMissileCleanupRunning = true;
                ScheduleMissileCleanup();
            }


        } else if (player_one_in_flames) {
            // One player survives, three points
            AddPoints(3, mPlayerTwoTankId, game_ptr);

            std::cout << "Player Two (" << mPlayerTwoName << ") survives!" << std::endl;
            Safir::Logging::SendSystemLog(Safir::Logging::Informational, L"Player two survives!");

            game_ptr->Survivor().SetVal(Consoden::TankGame::Winner::PlayerTwo);
            SetWinner(game_ptr);
            StopGame();
            if (gm.MissilesLeft()) {
                mMissileCleanupRunning = true;
                ScheduleMissileCleanup();
            }


        } else if (player_two_in_flames) {
            // One player survives, three points
            AddPoints(3, mPlayerOneTankId, game_ptr);

            std::cout << "Player One (" << mPlayerOneName << ") survives!" << std::endl;
            Safir::Logging::SendSystemLog(Safir::Logging::Informational, L"Player one survives!.");

            game_ptr->Survivor().SetVal(Consoden::TankGame::Winner::PlayerOne);
            SetWinner(game_ptr);
            StopGame();
            if (gm.MissilesLeft()) {
                mMissileCleanupRunning = true;
                ScheduleMissileCleanup();
            }
        }

        game_ptr->Counter() = game_ptr->Counter() + 1;
        game_ptr->ElapsedTime().SetVal(mCounter);

        // Push new state
        //gm.Print();
        gm.SetChanges();

        if (mGameRunning) {
            // Wait for joystick readout            
            mTimer.expires_from_now(boost::posix_time::milliseconds(JOYSTICK_TIMEOUT));
            mTimer.async_wait(boost::bind(&Engine::JoystickTimerExpired, this, boost::asio::placeholders::error));        
        }
        game_ptr->NextMove().SetVal(GameMap::TimerTimeout(mTimer.expires_from_now()));

        m_connection.SetChanges(game_ptr, m_GameEntityId.GetInstanceId(), m_HandlerId);        
    }

    void Engine::AddPoints(int points, int tank_id, Consoden::TankGame::GameStatePtr game_ptr) {
        if (mPlayerOneTankId == tank_id) {
            game_ptr->PlayerOnePoints().SetVal(game_ptr->PlayerOnePoints().GetVal() + points);
            if (game_ptr->PlayerOnePoints()<0) {
                // Give the negative points to the other player
                game_ptr->PlayerTwoPoints() += -(game_ptr->PlayerOnePoints());
                game_ptr->PlayerOnePoints()=0;
            }

        } else {
            game_ptr->PlayerTwoPoints().SetVal(game_ptr->PlayerTwoPoints().GetVal() + points);
            if (game_ptr->PlayerTwoPoints()<0) {
                // Give the negative points to the other player
                game_ptr->PlayerOnePoints() += -(game_ptr->PlayerTwoPoints());
                game_ptr->PlayerTwoPoints()=0;
            }
        }
    }

    void Engine::SetWinner(Consoden::TankGame::GameStatePtr game_ptr) {
        std::cout << "Game score: " << game_ptr->PlayerOnePoints() << "-" << game_ptr->PlayerTwoPoints() << ". ";

        if (game_ptr->PlayerOnePoints().GetVal() > game_ptr->PlayerTwoPoints().GetVal()) {
            game_ptr->Winner().SetVal(Consoden::TankGame::Winner::PlayerOne);
            std::cout << "Player One (" << mPlayerOneName << ") wins!" << std::endl;

        } else if (game_ptr->PlayerOnePoints().GetVal() < game_ptr->PlayerTwoPoints().GetVal()) {
            game_ptr->Winner().SetVal(Consoden::TankGame::Winner::PlayerTwo);
            std::cout << "Player Two (" << mPlayerTwoName << ") wins!" << std::endl;

        } else {
            game_ptr->Winner().SetVal(Consoden::TankGame::Winner::Draw);
            std::cout << "The game is a draw." << std::endl;
        }
    }

    std::pair<int,int> Engine::WrappedPosition(std::pair<int,int> pos, CWG::Direction dir){

    }

    bool Engine::CollisionPredicter(CWG::DudePtr& dude,CWG::TankPtr& tank){
    	std::pair<int,int> own_pos(dude->PosX(),dude->PosY());
    	std::pair<int,int> others_pos(tank->PosX(),tank->PosY());
    	CWG::Direction::Enumeration own_direction = dude->Direction();
    	CWG::Direction::Enumeration other_direction = tank->MoveDirection();

    	if(own_pos.first == (others_pos.first - 1) && own_pos.second == others_pos.second
    			&& own_direction == CWG::Direction::Right && other_direction == CWG::Direction::Left){
    		//dude is to the left of the tank
    		std::wcout << "dude got hit by tank coming from the right" << std::endl;
    		return true;

    	}else if(own_pos.first == (others_pos.first + 1) && own_pos.second == others_pos.second
    				&& own_direction == CWG::Direction::Left && other_direction == CWG::Direction::Right){
    		//dude is to the right of the tank
    		std::wcout << "dude got hit by tank coming from the left" << std::endl;
    		return true;

    	}else if(own_pos.second == (others_pos.second + 1) && own_pos.first == others_pos.first
				&& own_direction == CWG::Direction::Up && other_direction == CWG::Direction::Down){
		//dude is below the tank
    		std::wcout << "dude got hit by tank coming from above" << std::endl;
    		return true;

    	}else if(own_pos.second == (others_pos.second - 1) && own_pos.first== others_pos.first
				&& own_direction == CWG::Direction::Down && other_direction == CWG::Direction::Up){
    		std::wcout << "dude got hit by tank coming from be low" << std::endl;
		//dude is above the tank
    		return true;
	}

    	return false;
    }


    int* Engine::directionPermuter(){

    	int* permutation = new int[5];
    	for(int i = 0; i < 4; i++){
    		permutation[i] = i+1;
    	}
    	permutation[4] = 0;
		unsigned i;
		for (i = 0; i < 3; i++) {
			int j = (rand() % 3-i) + i;
			int swap = permutation[i];
			permutation[i] = permutation[j];
			permutation[j] = swap;
		}

		return permutation;
    }

 };
