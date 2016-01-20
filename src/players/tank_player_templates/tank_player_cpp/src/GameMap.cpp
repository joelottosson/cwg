/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Björn Weström / bjwe
*
*******************************************************************************/

#include "GameMap.h"

#include <iostream>
#include <utility>
#include <vector>
#include <map>
#include <boost/date_time/posix_time/posix_time.hpp>


GameMap::GameMap(int tankId, const Consoden::TankGame::GameStatePtr& gamePtr)
    :m_TankId(tankId)
    ,m_gamePtr(gamePtr)
    ,m_sizeX(gamePtr->Width().GetVal())
    ,m_sizeY(gamePtr->Height().GetVal())
{
    // Locate tanks
    Safir::Dob::Typesystem::ArrayIndex tankIndex;

    for (tankIndex = 0;
         tankIndex < m_gamePtr->TanksArraySize();
         tankIndex++) {

        if (m_gamePtr->Tanks()[tankIndex].IsNull()) {
            // empty tank slot, found last tank
            break;
        }

        Consoden::TankGame::TankPtr tankPtr =
                boost::dynamic_pointer_cast<Consoden::TankGame::Tank>(m_gamePtr->Tanks()[tankIndex].GetPtr());

        std::pair<int, int> pos = std::make_pair(tankPtr->PosX().GetVal(), tankPtr->PosY().GetVal());

        if (tankPtr->TankId().GetVal() == m_TankId) {
            // This is our tank!
            m_ownPos = pos;
        } else {
            // The enemy
            m_enemyPos = pos;
        }
    }
}

/**
  * Location of the players tank.
  */
 std::pair<int, int> GameMap::OwnPosition() const {
	 return m_ownPos;
 }

 /**
  * Location of the enemys tank.
  */
 std::pair<int, int> GameMap::EnemyPosition() const {
	   Safir::Dob::Typesystem::ArrayIndex tankIndex;
	   for (tankIndex = 0;
	         tankIndex < m_gamePtr->TanksArraySize();
	         tankIndex++) {

	        if (m_gamePtr->Tanks()[tankIndex].IsNull()) {
	            // empty tank slot, found last tank
	            break;
	        }

	        Consoden::TankGame::TankPtr tankPtr =
	                boost::dynamic_pointer_cast<Consoden::TankGame::Tank>(m_gamePtr->Tanks()[tankIndex].GetPtr());

	        //std::pair<int, int> pos = std::make_pair(tankPtr->PosX().GetVal(), tankPtr->PosY().GetVal());

	        if (tankPtr->TankId().GetVal() == m_TankId) {
	        	continue;
	        } else {
	            // The enemy
	            if(!tankPtr->SmokeLeft().IsNull() && tankPtr->SmokeLeft().GetVal() > 0){
	            	return  std::make_pair(rand()%m_sizeX, rand()%m_sizeY);
	            }else{
	            	return  std::make_pair(tankPtr->PosX().GetVal(), tankPtr->PosY().GetVal());
	            }
	        }
	    }
	   return std::make_pair(-1, -1);
 }



int GameMap::LaserAmmoCount() const{
	Safir::Dob::Typesystem::ArrayIndex tankIndex;
    for (tankIndex = 0;
         tankIndex < m_gamePtr->TanksArraySize();
         tankIndex++) {

        if (m_gamePtr->Tanks()[tankIndex].IsNull()) {
            // empty tank slot, found last tank
            break;
        }

        Consoden::TankGame::TankPtr tankPtr =
                boost::dynamic_pointer_cast<Consoden::TankGame::Tank>(m_gamePtr->Tanks()[tankIndex].GetPtr());

        if (tankPtr->TankId().GetVal() == m_TankId) {
            // This is our tank!
            return tankPtr->Lasers().GetVal();

        } else {
            continue;
        }
    }
    return -1;
}

bool GameMap::HasSmoke() const{
	Safir::Dob::Typesystem::ArrayIndex tankIndex;
    for (tankIndex = 0;
         tankIndex < m_gamePtr->TanksArraySize();
         tankIndex++) {

        if (m_gamePtr->Tanks()[tankIndex].IsNull()) {
            // empty tank slot, found last tank
            break;
        }

        Consoden::TankGame::TankPtr tankPtr =
                boost::dynamic_pointer_cast<Consoden::TankGame::Tank>(m_gamePtr->Tanks()[tankIndex].GetPtr());

        if (tankPtr->TankId().GetVal() == m_TankId) {
            // This is our tank!
            return tankPtr->HasSmoke().GetVal();

        } else {
            continue;
        }
    }
    return false;
}

bool GameMap::HasRedeemer() const{
	Safir::Dob::Typesystem::ArrayIndex tankIndex;
    for (tankIndex = 0;
         tankIndex < m_gamePtr->TanksArraySize();
         tankIndex++) {

        if (m_gamePtr->Tanks()[tankIndex].IsNull()) {
            // empty tank slot, found last tank
            break;
        }

        Consoden::TankGame::TankPtr tankPtr =
                boost::dynamic_pointer_cast<Consoden::TankGame::Tank>(m_gamePtr->Tanks()[tankIndex].GetPtr());

        if (tankPtr->TankId().GetVal() == m_TankId) {
            // This is our tank!
            return tankPtr->HasRedeemer().GetVal();

        } else {
            continue;
        }
    }
    return false;
}


Consoden::TankGame::MissilePtr GameMap::GetMissile(bool enemy_tank){
	int tank_id;
	if(enemy_tank){
		tank_id = (m_TankId +1) % 2;
	}else{
		tank_id = m_TankId;
	}

    for (Safir::Dob::Typesystem::ArrayIndex missileIndex = 0;
         missileIndex < m_gamePtr->MissilesArraySize();
         missileIndex++) {

        if (m_gamePtr->Missiles()[missileIndex].IsNull()) {
            // empty missile slot
            continue;
        }

        if(m_gamePtr->Missiles()[missileIndex].GetPtr()->TankId() == tank_id){
        	return m_gamePtr->Missiles()[missileIndex].GetPtr();
        }
    }
	return NULL;
}

Consoden::TankGame::RedeemerPtr GameMap::GetRedeemer(bool enemy_tank){
	int tank_id;
	if(enemy_tank){
		tank_id = (m_TankId +1) % 2;
	}else{
		tank_id = m_TankId;
	}

    for (Safir::Dob::Typesystem::ArrayIndex redeemerIndex = 0;
         redeemerIndex < m_gamePtr->RedeemersArraySize();
         redeemerIndex++) {

        if (m_gamePtr->Redeemers()[redeemerIndex].IsNull()) {
            // empty missile slot
            continue;
        }

        if(m_gamePtr->Redeemers()[redeemerIndex].GetPtr()->TankId() == tank_id){
        	return m_gamePtr->Redeemers()[redeemerIndex].GetPtr();
        }
    }
	return NULL;
}


bool GameMap::IsWall(const std::pair<int, int>& pos) const
{
    return m_gamePtr->Board().GetVal()[Index(pos)]=='x';
}

bool GameMap::IsMine(const std::pair<int, int>& pos) const
{
    return m_gamePtr->Board().GetVal()[Index(pos)]=='o';
}

bool GameMap::IsLaserAmmo(const std::pair<int, int>& pos) const
{
    return m_gamePtr->Board().GetVal()[Index(pos)]=='l';
}

bool GameMap::IsRedeemerAmmo(const std::pair<int, int>& pos) const
{
    return m_gamePtr->Board().GetVal()[Index(pos)]=='r';
}

bool GameMap::IsCoin(const std::pair<int, int>& pos) const
{
    return m_gamePtr->Board().GetVal()[Index(pos)]=='$';
}

bool GameMap::IsSmokeGrenade(const std::pair<int, int>& pos) const{
	return m_gamePtr->Board().GetVal()[Index(pos)]=='s';
}

bool GameMap::IsPoisonGas(const std::pair<int, int>& pos) const
{
    return m_gamePtr->Board().GetVal()[Index(pos)]=='p';
	//return true;
}

bool GameMap::IsPenguin(const std::pair<int, int>& pos) const
{
   return pos.first == m_gamePtr->TheDude().GetPtr()->PosX() && pos.second == m_gamePtr->TheDude().GetPtr()->PosY();
}

bool GameMap::IsRedeemerInPosition(const std::pair<int, int>& pos) const{
    Safir::Dob::Typesystem::ArrayIndex redeemerIndex;

    for (redeemerIndex = 0;
    		redeemerIndex < m_gamePtr->RedeemersArraySize();
    		redeemerIndex++) {

        if (m_gamePtr->Redeemers()[redeemerIndex].IsNull()) {
            // empty missile slot
            continue;
        }

        Consoden::TankGame::RedeemerPtr redeemerPtr =
                boost::dynamic_pointer_cast<Consoden::TankGame::Redeemer>(m_gamePtr->Redeemers()[redeemerIndex].GetPtr());

        std::pair<int, int> redeemerPosition = std::make_pair(redeemerPtr->PosX().GetVal(), redeemerPtr->PosY().GetVal());


        if ((pos == redeemerPosition)) {
            return true;
        }
    }

    return false;
}




bool GameMap::IsMissileInPosition(const std::pair<int, int>& pos) const
{
    Safir::Dob::Typesystem::ArrayIndex missileIndex;

    for (missileIndex = 0;
         missileIndex < m_gamePtr->MissilesArraySize();
         missileIndex++) {

        if (m_gamePtr->Missiles()[missileIndex].IsNull()) {
            // empty missile slot
            continue;
        }

        Consoden::TankGame::MissilePtr missilePtr =
                boost::dynamic_pointer_cast<Consoden::TankGame::Missile>(m_gamePtr->Missiles()[missileIndex].GetPtr());

        std::pair<int, int> head = std::make_pair(missilePtr->HeadPosX().GetVal(), missilePtr->HeadPosY().GetVal());
        std::pair<int, int> tail = std::make_pair(missilePtr->TailPosX().GetVal(), missilePtr->TailPosY().GetVal());

        if ((pos == head) || (pos == tail)) {
            return true;
        }
    }

    return false;
}

std::pair<int, int> GameMap::Move(const std::pair<int, int>& pos,
                                  Consoden::TankGame::Direction::Enumeration direction) const
{
    switch (direction)
    {
    case Consoden::TankGame::Direction::Left:
        return std::make_pair((pos.first - 1 + m_sizeX) % m_sizeX, pos.second);

    case Consoden::TankGame::Direction::Right:
        return std::make_pair((pos.first + 1) % m_sizeX, pos.second);

    case Consoden::TankGame::Direction::Up:
        return std::make_pair(pos.first, (pos.second - 1 + m_sizeY) % m_sizeY);

    case Consoden::TankGame::Direction::Down:
        return std::make_pair(pos.first, (pos.second + 1) % m_sizeY);

    case Consoden::TankGame::Direction::Neutral:
        return pos;
    }

    return pos;
}

int GameMap::TimeToNextMove() const
{
    // Get current time from the clock, using microseconds resolution
    const boost::posix_time::ptime now =
        boost::posix_time::microsec_clock::local_time();

    // Get the time offset in current day
    const boost::posix_time::time_duration td = now.time_of_day();

    int total_milliseconds = td.total_milliseconds();

    // We ignore midnight to keep it simple
    return m_gamePtr->NextMove().GetVal() - total_milliseconds;
}

void GameMap::PrintMap() const
{
    for (int y = 0; y < m_sizeY; y++) {
        for (int x = 0; x < m_sizeX; x++) {
            std::cout << m_gamePtr->Board().GetVal()[Index(x, y)];
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void GameMap::PrintState() const
{
    std::cout << "Game board" << std::endl;
    PrintMap();

    std::cout << "Own position " << OwnPosition().first << "," << OwnPosition().second << std::endl;
    std::cout << "Enemy position  " << EnemyPosition().first << "," << EnemyPosition().second << std::endl;

    std::cout << "Active missiles" << std::endl;

    Safir::Dob::Typesystem::ArrayIndex missileIndex;

    for (missileIndex = 0;
         missileIndex < m_gamePtr->MissilesArraySize();
         missileIndex++) {

        if (m_gamePtr->Missiles()[missileIndex].IsNull()) {
            // empty missile slot
            continue;
        }

        Consoden::TankGame::MissilePtr missilePtr =
                boost::dynamic_pointer_cast<Consoden::TankGame::Missile>(m_gamePtr->Missiles()[missileIndex].GetPtr());

        std::pair<int, int> head = std::make_pair(missilePtr->HeadPosX().GetVal(), missilePtr->HeadPosY().GetVal());
        std::pair<int, int> tail = std::make_pair(missilePtr->TailPosX().GetVal(), missilePtr->TailPosY().GetVal());
        Consoden::TankGame::Direction::Enumeration direction = missilePtr->Direction().GetVal();

        std::cout << "Missile position - head " << head.first << "," << head.second << std::endl;
        std::cout << "Missile position - tail " << tail.first << "," << tail.second << std::endl;
        std::cout << "Missile direction "<<Consoden::TankGame::Direction::ToString(direction).c_str()<<std::endl;
    }

    std::cout << std::endl;
}

