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



namespace TankEngine
{
    Engine::Engine(boost::asio::io_service& io) :
    	m_config(std::string(getenv("SAFIR_RUNTIME")) + std::string("/data/tank_game/rules.cfg")),
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
                AddPoints(m_config.m_draw_points, mPlayerOneTankId, game_ptr);
                AddPoints(m_config.m_draw_points, mPlayerTwoTankId, game_ptr);

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
                    gm.MoveRedeemers();

                    game_ptr->Counter() = game_ptr->Counter() + 1;

                    m_connection.SetChanges(game_ptr, m_GameEntityId.GetInstanceId(), m_HandlerId);        

                    if (gm.MissilesLeft() || gm.RedeemersLeft()) {
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

        if(!joystick_ptr->FireLaser().IsNull()){
        	joystick_ptr->FireLaser() = false;
        }

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
                    AddPoints(m_config.m_walkover_penalty, OpponentTankId(loser_tank_id), game_ptr);
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
     * the order of execution in this function is critical.
     *
     * This function is very confusing so unfortunately I have added some inline comments.
     *
     * Some serious refactoring of this function would not hurt.
     */
    void Engine::Evaluate()
    {
        Consoden::TankGame::GameStatePtr game_ptr =
            boost::static_pointer_cast<Consoden::TankGame::GameState>(m_connection.Read(m_GameEntityId).GetEntity());

        GameMap gm(game_ptr);

        nullCheckEverything(game_ptr);



        gm.MoveMissiles();
       //gm.MoveRedeemers();

        bool tank_tank_collission = false;

        //Decrement redeemer counter
        //And detonate redeemer if necessary.
        for (Safir::Dob::Typesystem::ArrayIndex redeemer_index = 0;
             (redeemer_index < game_ptr->RedeemersArraySize());
             redeemer_index++) {

        	if(!game_ptr->Redeemers()[redeemer_index].IsNull()){
    			Consoden::TankGame::RedeemerPtr redeemer =
    			                boost::static_pointer_cast<Consoden::TankGame::Redeemer>(game_ptr->Redeemers()[redeemer_index].GetPtr());
    			if(redeemer->InFlames()){
    				game_ptr->Redeemers()[redeemer_index].SetNull();
    				break;
    			}

    			if(gm.WallSquare(redeemer->PosX(),redeemer->PosY())){
					detonateRedeemer(game_ptr, redeemer, &gm, 1);
					redeemer->InFlames() = true;
		            Consoden::TankGame::TankPtr tank_ptr =
		                boost::static_pointer_cast<Consoden::TankGame::Tank>(game_ptr->Tanks()[redeemer->TankId()].GetPtr());
		            tank_ptr->RedeemerTimerLeft() = 0;
		            redeemer->TimeToExplosion()=0;
					break;
    			}

    			gm.MoveRedeemer(redeemer_index);

        		if(game_ptr->Redeemers()[redeemer_index].GetPtr()->TimeToExplosion() <= 1){//Needs to be done when timer is 1 to mitigate for delayed updates
        			if(gm.OnBoard(redeemer->PosX(),redeemer->PosY())){
						detonateRedeemer(game_ptr, redeemer, &gm, 1);
						redeemer->InFlames() = true;

        			}else{
        				game_ptr->Redeemers()[redeemer_index].SetNull();
        			}
        		}else{
        			redeemer->TimeToExplosion() -= 1;
        		}
        	}
        }



        CWG::DudePtr dude_ptr = game_ptr->TheDude().GetPtr();
        dudeUpdater(dude_ptr,gm,game_ptr);

        // Check for frontal collission
        for (Safir::Dob::Typesystem::ArrayIndex tank_index = 0; 
             (tank_index < game_ptr->TanksArraySize()) && (!game_ptr->Tanks()[tank_index].IsNull()); 
             tank_index++) {

            Consoden::TankGame::TankPtr tank_ptr = 
                boost::static_pointer_cast<Consoden::TankGame::Tank>(game_ptr->Tanks()[tank_index].GetPtr());

            //We need to decremet the tanks own redeemer timer here to fix the stupid lagging in the gui.
            if(tank_ptr->RedeemerTimerLeft() > 0 ){
            	tank_ptr->RedeemerTimerLeft()--;
            }

            Consoden::TankGame::JoystickPtr joystick_ptr = m_JoystickCacheMap[tank_ptr->TankId().GetVal()];

            for (Safir::Dob::Typesystem::ArrayIndex tank2_index = tank_index + 1;
                 (tank2_index < game_ptr->TanksArraySize()) && (!game_ptr->Tanks()[tank2_index].IsNull());
                 tank2_index++) {

                Consoden::TankGame::TankPtr tank2_ptr = 
                    boost::static_pointer_cast<Consoden::TankGame::Tank>(game_ptr->Tanks()[tank2_index].GetPtr());

                Consoden::TankGame::JoystickPtr joystick2_ptr = m_JoystickCacheMap[tank2_ptr->TankId().GetVal()];

                if ((tank_ptr->PosX() == tank2_ptr->PosX()) && (tank_ptr->PosY() == tank2_ptr->PosY())) {
                    // x and y coords match for these two tanks - collision!
                    tank_ptr->InFlames() = true;
                    tank_ptr->HitTank() = true;
                    tank2_ptr->InFlames() = true;
                    tank2_ptr->HitTank() = true;
                    tank_ptr->Fire() = false;
                    tank_ptr->FireLaser() = false;
                    tank2_ptr->Fire() = false;
                    tank2_ptr->FireLaser() = false;
                    tank_tank_collission = true;
                }

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
                        tank_ptr->FireLaser() = false;
                        tank_ptr->TookCoin() = false;
                        tank_ptr->HitPoisonGas() = false;
                        tank2_ptr->InFlames() = true;
                        tank2_ptr->HitTank() = true;
                        tank2_ptr->MoveDirection() = joystick2_ptr->MoveDirection();
                        tank2_ptr->Fire() = false;
                        tank2_ptr->FireLaser() = false;
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
            tank_ptr->FireLaser() = false;
            tank_ptr->TookCoin() = false;
            tank_ptr->HitPoisonGas() = false;

            if (tank_ptr->InFlames()) {
                // Dead tank, go to next
                continue;
            }

            Consoden::TankGame::JoystickPtr joystick_ptr = m_JoystickCacheMap[tank_ptr->TankId().GetVal()];

            /***********************************
             * Get information from joysticks
             ***********************************/
            // Player one made a move?
            if (game_ptr->PlayerOneId().GetVal() == joystick_ptr->PlayerId().GetVal()) {
                if (mPlayerOneCounter == joystick_ptr->Counter().GetVal()) {
                    std::cout << "Player One (" << mPlayerOneName << ") didn't make a move in time! ElapsedTime: " << mCounter << std::endl;
                    Safir::Logging::SendSystemLog(Safir::Logging::Informational,
                                            L"Player one didn't make a move in time!");
                    // Treat as neutral move
                    joystick_ptr->MoveDirection().SetVal(Consoden::TankGame::Direction::Neutral);
                    joystick_ptr->TowerDirection().SetVal(Consoden::TankGame::Direction::Neutral);
                    joystick_ptr->Fire().SetVal(false);
                    joystick_ptr->FireLaser() = false;
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
                    joystick_ptr->TowerDirection().SetVal(Consoden::TankGame::Direction::Neutral);
                    joystick_ptr->Fire().SetVal(false);
                    joystick_ptr->FireLaser() = false;
                    joystick_ptr->MineDrop().SetVal(false);
                }
                mPlayerTwoCounter = joystick_ptr->Counter().GetVal();
            }

 
            int request_pos_x = tank_ptr->PosX();
            int request_pos_y = tank_ptr->PosY();

            /*********************************************************************
			 * Move tanks and do rudimentary collision checks
			 * this must be done after movement, otherwise the "MoveAgainstMissile" logic is broken
			 *********************************************************************/
            if (joystick_ptr->MoveDirection() == Consoden::TankGame::Direction::Neutral){
            	tank_ptr->MoveDirection() = joystick_ptr->MoveDirection(); //We still need to set the move direction even though we don't move!
            }else{
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

                    /***********************************
        			 * Palce mines
        			 ***********************************/
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

        /******************************************************************************************************
		 * Fire weapons or deploy smoke
		 ******************************************************************************************************/
        for (Safir::Dob::Typesystem::ArrayIndex tank_index = 0; 
             (tank_index < game_ptr->TanksArraySize()) && (!game_ptr->Tanks()[tank_index].IsNull()); 
             tank_index++) {

            Consoden::TankGame::TankPtr tank_ptr = 
                boost::static_pointer_cast<Consoden::TankGame::Tank>(game_ptr->Tanks()[tank_index].GetPtr());
        
            if (tank_ptr->InFlames() || tank_ptr->HitTank()) {
                // Dead tank, go to next
            	tank_ptr->Fire() = false;
                continue;
            }

            Consoden::TankGame::JoystickPtr joystick_ptr = m_JoystickCacheMap[tank_ptr->TankId().GetVal()];

			if(tank_ptr->SmokeLeft() <= 0){
				tank_ptr->SmokeLeft() = 0;
			}else{
				tank_ptr->SmokeLeft() = tank_ptr->SmokeLeft() - 1;
			}

            if(joystick_ptr->DeploySmoke() && tank_ptr->HasSmoke()){
            	tank_ptr->HasSmoke() = false;
            	tank_ptr->SmokeLeft() = tank_ptr->SmokeLeft() + m_config.m_smoke_timer;
            }


			if(joystick_ptr->TowerDirection() != CWG::Direction::Neutral){
				tank_ptr->TowerDirection() = joystick_ptr->TowerDirection();
			}

            if(joystick_ptr->Fire() &&	joystick_ptr->FireLaser() && joystick_ptr->TowerDirection() != CWG::Direction::Neutral && joystick_ptr->MoveDirection() == CWG::Direction::Neutral){
            	if(tank_ptr->Lasers() > 0){
            		tank_ptr->Lasers() = tank_ptr->Lasers() - 1;
            		tank_ptr->FireLaser() = true;
            		tank_ptr->Fire() = true;

            		Consoden::TankGame::TankPtr enemy_ptr =
            		                boost::static_pointer_cast<Consoden::TankGame::Tank>(game_ptr->Tanks()[(tank_index+1) % 2].GetPtr());
            		fireTheLaser(tank_ptr, enemy_ptr,gm,game_ptr);

            	}else{
            		tank_ptr->FireLaser() = false;
            		tank_ptr->Fire() = false;
            	}
            }

            if (joystick_ptr->Fire() && !joystick_ptr->FireLaser() && !joystick_ptr->FireRedeemer()) {

				bool fired = gm.FireMissile(tank_ptr->PosX(), tank_ptr->PosY(), joystick_ptr->TowerDirection(), tank_ptr->TankId().GetVal());

				tank_ptr->Fire() = fired; // Only indicate fire if firing was successful

			}else if (joystick_ptr->Fire() && !joystick_ptr->FireLaser() && joystick_ptr->FireRedeemer() && tank_ptr->HasRedeemer()) {
                bool fired = gm.FireRedeemer(tank_ptr->PosX(), tank_ptr->PosY(), joystick_ptr->TowerDirection(),
                		joystick_ptr->RedeemerTimer(), tank_ptr->TankId().GetVal());
                tank_ptr->Fire() = fired; // Only indicate fire if firing was successful

                if(fired){
                	tank_ptr->RedeemerTimerLeft() = joystick_ptr->RedeemerTimer();
                	tank_ptr->HasRedeemer() = false;
                }

			} else {
				tank_ptr->Fire() = false;
			}

        }

        /***********************************
		 * Evaluate hits and collisions
		 ***********************************/
        for (Safir::Dob::Typesystem::ArrayIndex tank_index = 0; 
             (tank_index < game_ptr->TanksArraySize()) && (!game_ptr->Tanks()[tank_index].IsNull()); 
             tank_index++) {

            Consoden::TankGame::TankPtr tank_ptr = 
                boost::static_pointer_cast<Consoden::TankGame::Tank>(game_ptr->Tanks()[tank_index].GetPtr());

            //We might need to have this before any movement for the detector to work
            if ((gm.DudeSquare(tank_ptr->PosX().GetVal(), tank_ptr->PosY().GetVal()) || collisonDetector(dude_ptr,tank_ptr) )&& !dude_ptr->Dying() ) {
            	dude_ptr->Dying().SetVal(true);
                AddPoints(m_config.m_dude_penalty, tank_ptr->TankId(), game_ptr);
            }
            /*
            //Here we do some horrible int and bool conversion :(
            int killer_id1 = gm.HitByMissile(dude_ptr->PosX(),dude_ptr->PosY());
            int killer_id2 = gm.MoveAgainstMissile(dude_ptr->PosX(),dude_ptr->PosY(),dude_ptr->Direction());
            if((bool)killer_id1 || (bool)killer_id2){
            	dude_ptr->Dying() = true;


            	if((bool)killer_id1){
            		AddPoints(m_config.m_dude_penalty, killer_id1, game_ptr);
            	}else if((bool)killer_id2){
            		AddPoints(m_config.m_dude_penalty, killer_id2, game_ptr);
            	}
            }
            */

            // Stepped on mine
            if (gm.MineSquare(tank_ptr->PosX().GetVal(), tank_ptr->PosY().GetVal())) {
                tank_ptr->InFlames() = true;
                tank_ptr->HitMine() = true;
            }

            // Hit by missile?
            if (gm.IsTankHit(tank_ptr->PosX(), tank_ptr->PosY())) {
                // Hit by missile awards two points to opponent
                AddPoints(m_config.m_hit_points, OpponentTankId(tank_ptr->TankId()), game_ptr);
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
                    AddPoints(m_config.m_coin_value, tank_ptr->TankId(), game_ptr);
                    tank_ptr->TookCoin() = true;

                }else if (gm.LaserAmmo(tank_ptr->PosX(), tank_ptr->PosY())) {
                    gm.ClearSquare(tank_ptr->PosX(), tank_ptr->PosY()); //take the coin

                    tank_ptr->Lasers() = tank_ptr->Lasers().GetVal() + 1;

                }else if (gm.PoisonSquare(tank_ptr->PosX(), tank_ptr->PosY())) {
                    // Give one point to the opponent for driving into poison gas. 
                    gm.ClearSquare(tank_ptr->PosX(), tank_ptr->PosY()); //remove poison
                    AddPoints(m_config.m_gas_penalty, OpponentTankId(tank_ptr->TankId()), game_ptr);
                    tank_ptr->HitPoisonGas() = true;

                } else if (gm.SmokeGrenadeSquare(tank_ptr->PosX(), tank_ptr->PosY())) {
                    // Only pickup smoke if we don't already have smoke
                    if(!tank_ptr->HasSmoke().IsNull() && !tank_ptr->HasSmoke()){
                    	gm.ClearSquare(tank_ptr->PosX(), tank_ptr->PosY());
                    	tank_ptr->HasSmoke() = true;
                    }

                }else if(gm.RedeemerAmmoSquare(tank_ptr->PosX(), tank_ptr->PosY())){
                	if(!tank_ptr->HasRedeemer()){
                	gm.ClearSquare(tank_ptr->PosX(), tank_ptr->PosY());
                	tank_ptr->HasRedeemer() = true;
                	}

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
            AddPoints(m_config.m_draw_points, mPlayerOneTankId, game_ptr);
            AddPoints(m_config.m_draw_points, mPlayerTwoTankId, game_ptr);

            std::cout << "Both players died." << std::endl;
            Safir::Logging::SendSystemLog(Safir::Logging::Informational, L"Both players died.");

            game_ptr->Survivor().SetVal(Consoden::TankGame::Winner::Draw);
            SetWinner(game_ptr);
            StopGame();
            if (gm.MissilesLeft() || gm.RedeemersLeft()) {
                mMissileCleanupRunning = true;
                ScheduleMissileCleanup();
            }


        } else if (player_one_in_flames) {
            // One player survives, three points
            AddPoints(m_config.m_survival_points, mPlayerTwoTankId, game_ptr);

            std::cout << "Player Two (" << mPlayerTwoName << ") survives!" << std::endl;
            Safir::Logging::SendSystemLog(Safir::Logging::Informational, L"Player two survives!");

            game_ptr->Survivor().SetVal(Consoden::TankGame::Winner::PlayerTwo);
            SetWinner(game_ptr);
            StopGame();
            if (gm.MissilesLeft()|| gm.RedeemersLeft()) {
                mMissileCleanupRunning = true;
                ScheduleMissileCleanup();
            }


        } else if (player_two_in_flames) {
            // One player survives, three points
            AddPoints(m_config.m_survival_points, mPlayerOneTankId, game_ptr);

            std::cout << "Player One (" << mPlayerOneName << ") survives!" << std::endl;
            Safir::Logging::SendSystemLog(Safir::Logging::Informational, L"Player one survives!.");

            game_ptr->Survivor().SetVal(Consoden::TankGame::Winner::PlayerOne);
            SetWinner(game_ptr);
            StopGame();
            if (gm.MissilesLeft() || gm.RedeemersLeft()) {
                mMissileCleanupRunning = true;
                ScheduleMissileCleanup();
            }
        }

        game_ptr->Counter() = game_ptr->Counter() + 1;
        game_ptr->ElapsedTime().SetVal(mCounter);

        // Push new state
        gm.SetChanges();

        if (mGameRunning) {
            // Wait for joystick readout            
            mTimer.expires_from_now(boost::posix_time::milliseconds(JOYSTICK_TIMEOUT));
            mTimer.async_wait(boost::bind(&Engine::JoystickTimerExpired, this, boost::asio::placeholders::error));        
        }
        game_ptr->NextMove().SetVal(GameMap::TimerTimeout(mTimer.expires_from_now()));

        m_connection.SetChanges(game_ptr, m_GameEntityId.GetInstanceId(), m_HandlerId);        
    }

    bool Engine::fireTheLaser(CWG::TankPtr& own_tank, CWG::TankPtr& enemy_tank , GameMap gm,CWG::GameStatePtr game_ptr){
    	int dx = 0;
    	int dy = 0;
    	int x_pos = own_tank->PosX();
    	int y_pos = own_tank->PosY();
    	switch (own_tank->TowerDirection()) {
			case CWG::Direction::Up:
				dy = -1;
				break;
			case CWG::Direction::Down:
				dy = 1;
				break;
			case CWG::Direction::Left:
				dx = -1;
				break;
			case CWG::Direction::Right:
				dx = 1;
				break;

			default:
				std::wcout << "tower direction was neutral" << std::endl;
				return false;
				break;
		}

    	while(true){
    		//lazer wrapper
    		x_pos = wrap(x_pos+dx,game_ptr->Width().GetVal());
    		y_pos = wrap(y_pos+dy,game_ptr->Height().GetVal());

    		if(gm.WallSquare(x_pos,y_pos)){
    			return false;

    		}else if(x_pos == enemy_tank->PosX() && y_pos == enemy_tank->PosY()){

    			enemy_tank->InFlames() = true;

    			enemy_tank->HitMissile() = true;
    			AddPoints(m_config.m_hit_points,own_tank->TankId(),game_ptr);
    			return true;
    		}else if(x_pos == own_tank->PosX() && y_pos == own_tank->PosY()){

    			own_tank->InFlames() = true;

    			own_tank->HitMissile() = true;
    			return true;
    		}else if(x_pos == own_tank->PosX() && y_pos == own_tank->PosY()){
    			return true;
    		}else if(x_pos == game_ptr->TheDude().GetPtr()->PosX() && y_pos == game_ptr->TheDude().GetPtr()->PosY()){
    			game_ptr->TheDude().GetPtr()->Dying() = true;
    			game_ptr->TheDude().GetPtr()->StopInstantly() = true;
    			AddPoints(m_config.m_dude_penalty,own_tank->TankId(),game_ptr);
    		}

    	}

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



    /*
     * We need to take into account that the dude already has been moved but not the tank.
     *
     */
    bool Engine::collisonDetector(CWG::DudePtr& dude,CWG::TankPtr& tank){
    	std::pair<int,int> dude_pos(dude->PosX(),dude->PosY());
    	std::pair<int,int> dude_old_pos(dude->OldX(),dude->OldY());
    	std::pair<int,int> tank_pos(tank->PosX(),tank->PosY());

    	std::pair<int,int> tank_old_pos = subPair(tank_pos,directionToVector(tank->MoveDirection()));


    	if(		tank_pos == dude_pos ||
				(tank_old_pos == dude_pos && dude_old_pos == tank_pos)
    	){
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
		for (i = 0; i < 4; i++) {
			int j = (rand() % 4-i) + i;
			int swap = permutation[i];
			permutation[i] = permutation[j];
			permutation[j] = swap;
		}
		return permutation;
    }

    void Engine::dudeUpdater(CWG::DudePtr& dude_ptr, GameMap gm,CWG::GameStatePtr game_ptr){

        int lame_x = dude_ptr->OldX();
        int lame_y = dude_ptr->OldY();
        int old_seed = std::rand();
        CWG::Direction::Enumeration candidate_direction = CWG::Direction::Neutral;
        CWG::Direction::Enumeration lame_direction = CWG::Direction::Neutral;
        bool found_new_way = false;
        if(!dude_ptr->Dying()){
			int dude_new_x ;
			int dude_new_y ;

			std::srand(dude_ptr->Seed());

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

				if(		gm.WallSquare(dude_new_x, dude_new_y)
						|| dude_new_x < 0
						|| dude_new_x >= game_ptr->Width()
						|| dude_new_y < 0
						|| dude_new_y >= game_ptr->Height()
					){
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
			dude_ptr->Seed() = std::rand();
        }
        std::srand(old_seed);

    }

    int Engine::wrap(int pos, int size){
    	if(pos > size){
    			return 0;
    	}else if(pos < 0){
    			return size - 1;
    	}else{
    		return pos;
    	}
    }

    void Engine::detonateRedeemer(Consoden::TankGame::GameStatePtr game_ptr, Consoden::TankGame::RedeemerPtr redeemer_ptr, GameMap* gm, int radius){
    	int center_x = redeemer_ptr->PosX();
    	int center_y = redeemer_ptr->PosY();
    	CWG::TankPtr own= game_ptr->Tanks()[redeemer_ptr->TankId()].GetPtr();
    	CWG::TankPtr enemy= game_ptr->Tanks()[(redeemer_ptr->TankId() + 1)  %2].GetPtr();
    	int own_x = own->PosX();
    	int own_y = own->PosY();
    	int enemy_x = enemy->PosX();
    	int enemy_y = enemy->PosY();

    	for(int y_pos = center_y - radius; y_pos <= center_y + radius; y_pos++){
        	for(int x_pos = center_x - radius; x_pos <= center_x + radius; x_pos++){

        		if(x_pos == own_x && y_pos == own_y && !own->InFlames()){
        			own->InFlames() = true;
         		}

        		if(x_pos == enemy_x && y_pos == enemy_y && !enemy->InFlames()){
					enemy->InFlames() = true;
					AddPoints(m_config.m_hit_points,redeemer_ptr->TankId(), game_ptr);
				}

        		if(gm->WallSquare(x_pos,y_pos) || gm->MineSquare(x_pos,y_pos) ){
        			gm->ClearSquare(x_pos,y_pos);

        		}

        		if(x_pos == game_ptr->TheDude().GetPtr()->PosX() && y_pos == game_ptr->TheDude().GetPtr()->PosY()){
        			game_ptr->TheDude().GetPtr()->Dying() = true;
        			game_ptr->TheDude().GetPtr()->StopInstantly() = true;
        			AddPoints(m_config.m_dude_penalty,redeemer_ptr->TankId(), game_ptr);
        		}

        	}
    	}

    }

    std::pair<int,int> Engine::directionToVector(CWG::Direction::Enumeration dir){
    	switch(dir){
    		case(CWG::Direction::Neutral):
    			return std::pair<int,int>(0,0);
    		case(CWG::Direction::Up):
				return std::pair<int,int>(0,-1);
    		case(CWG::Direction::Down):
				return std::pair<int,int>(0,1);
    		case(CWG::Direction::Left):
				return std::pair<int,int>(-1,0);
    		case(CWG::Direction::Right):
				return std::pair<int,int>(1,0);
    		default:
				return std::pair<int,int>(0,0); // just to shutup the compiler
    	}
    }

    std::pair<int,int> Engine::addPair(std::pair<int,int> a, std::pair<int,int> b){
    	return std::pair<int,int>(a.first + b.first , a.second+b.second);
    }

    std::pair<int,int> Engine::subPair(std::pair<int,int> a, std::pair<int,int> b){
    	return std::pair<int,int>(a.first - b.first , a.second - b.second);
    }

    void Engine::nullCheckEverything(Consoden::TankGame::GameStatePtr game_ptr){
        for (Safir::Dob::Typesystem::ArrayIndex tank_index = 0;
             (tank_index < game_ptr->TanksArraySize()) && (!game_ptr->Tanks()[tank_index].IsNull());
             tank_index++) {

            Consoden::TankGame::TankPtr tank_ptr =
                boost::static_pointer_cast<Consoden::TankGame::Tank>(game_ptr->Tanks()[tank_index].GetPtr());

            Consoden::TankGame::JoystickPtr joystick_ptr = m_JoystickCacheMap[tank_ptr->TankId().GetVal()];

            //Nullceck tank fields:
            if(tank_ptr->PlayerId().IsNull()){
            	std::wcout << "Player Id is null... No other option but to abort everything and silently sit and cry" << std::endl;
            	abort();
            }
            if(tank_ptr->PosX().IsNull() || tank_ptr->PosY().IsNull()){
            	std::wcout << "Player position is null... No other option but to abort everything and silently sit and cry" << std::endl;
            	abort();
            }
            if(tank_ptr->MoveDirection().IsNull()){
				std::wcout << "Move Direction is null. Setting to neutral" << std::endl;
				tank_ptr->MoveDirection()  = CWG::Direction::Neutral;
			}
            if(tank_ptr->TowerDirection().IsNull()){
				std::wcout << "Tower Direction is null. Setting to neutral" << std::endl;
				tank_ptr->TowerDirection()  = CWG::Direction::Neutral;
			}
            if(tank_ptr->Fire().IsNull()){
				std::wcout << "Fire is null. Setting to false" << std::endl;
				tank_ptr->Fire()  = false;
			}
            if(tank_ptr->FireLaser().IsNull()){
				std::wcout << "Fire Laser is null. Setting to false" << std::endl;
				tank_ptr->FireLaser()  = false;
			}
            if(tank_ptr->Lasers().IsNull()){
            	std::wcout << "Laser count is null. Setting to 0" << std::endl;
				tank_ptr->Lasers()  = 0;
            }
            if(tank_ptr->InFlames().IsNull()){
				std::wcout << "InFlames is null. Setting to false" << std::endl;
				tank_ptr->InFlames()  = false;
			}
            if(tank_ptr->HitMine().IsNull()){
				std::wcout << "HitMine is null. Setting to false" << std::endl;
				tank_ptr->HitMine()  = false;
			}
            if(tank_ptr->HitMissile().IsNull()){
				std::wcout << "HitMissile is null. Setting to false" << std::endl;
				tank_ptr->HitMissile()  = false;
			}
            if(tank_ptr->HitTank().IsNull()){
				std::wcout << "HitTank is null. Setting to false" << std::endl;
				tank_ptr->HitTank()  = false;
			}
            if(tank_ptr->HitPoisonGas().IsNull()){
				std::wcout << "HitPoison is null. Setting to false" << std::endl;
				tank_ptr->HitPoisonGas()  = false;
			}
            if(tank_ptr->HasSmoke().IsNull()){
				std::wcout << "HasSmoke is null. Setting to false" << std::endl;
				tank_ptr->HasSmoke()  = false;
			}
            if(tank_ptr->TookCoin().IsNull()){
				std::wcout << "TookCoin is null. Setting to false" << std::endl;
				tank_ptr->TookCoin()  = false;
			}
            if(tank_ptr->SmokeLeft().IsNull()){
				std::wcout << "SmokeLeft is null. Setting to 0" << std::endl;
				tank_ptr->SmokeLeft()  = 0;
			}
            if(tank_ptr->HasRedeemer().IsNull()){
				std::wcout << "HasRedeemer is null. Setting to false" << std::endl;
				tank_ptr->HasRedeemer()  = false;
			}
            if(tank_ptr->RedeemerTimerLeft().IsNull()){
				std::wcout << "RedeemerTimer is null. Setting to 0" << std::endl;
				tank_ptr->RedeemerTimerLeft()  = 0;
			}

            //Nullchecks for joystic
            if(joystick_ptr->GameId().IsNull()){
            	std::wcout << "Joystick Game Id is null. Can't be bothered to continue" << std::endl;
            	abort();
            }

            if(joystick_ptr->PlayerId().IsNull()){
				std::wcout << "Joystick Player Id is null. Can't be bothered to continue" << std::endl;
				abort();
			}
            if(joystick_ptr->TankId().IsNull()){
				std::wcout << "Joystick Tank Id is null. Can't be bothered to continue" << std::endl;
				abort();
			}
            if(joystick_ptr->MoveDirection().IsNull()){
				std::wcout << "Joystick Move Direction is null. Setting to neutral" << std::endl;
				joystick_ptr->MoveDirection() = CWG::Direction::Neutral;
			}
            if(joystick_ptr->TowerDirection().IsNull()){
				std::wcout << "Joystick Tower Direction is null. Setting to neutral" << std::endl;
				joystick_ptr->TowerDirection() = CWG::Direction::Neutral;
			}
            if(joystick_ptr->Fire().IsNull()){
				std::wcout << "Joystick Fire is null. Setting to false" << std::endl;
				joystick_ptr->Fire() = false;
			}
            if(joystick_ptr->FireLaser().IsNull()){
				std::wcout << "Joystick FireLaser is null. Setting to false" << std::endl;
				joystick_ptr->FireLaser() = false;
			}
            if(joystick_ptr->MineDrop().IsNull()){
				std::wcout << "Joystick MineDrop is null. Setting to false" << std::endl;
				joystick_ptr->MineDrop() = false;
			}
            if(joystick_ptr->DeploySmoke().IsNull()){
				std::wcout << "Joystick DeploySmoke is null. Setting to false" << std::endl;
				joystick_ptr->DeploySmoke() = false;
			}
            if(joystick_ptr->FireRedeemer().IsNull()){
				std::wcout << "Joystick FireRedeemer is null. Setting to false" << std::endl;
				joystick_ptr->FireRedeemer() = false;
			}
            if(joystick_ptr->RedeemerTimer().IsNull()){
				std::wcout << "Joystick RedeemerTimer is null. Setting to 0" << std::endl;
				joystick_ptr->RedeemerTimer() = 0;
			}

        }
    }

 };
