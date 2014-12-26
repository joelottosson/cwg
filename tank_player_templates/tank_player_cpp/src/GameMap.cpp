/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Björn Weström / bjwe
*
*******************************************************************************/

#include <iostream>
#include <utility>
#include <vector>
#include <map>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "GameMap.h"

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

bool GameMap::IsWall(const std::pair<int, int>& pos) const
{
    return m_gamePtr->Board().GetVal()[Index(pos)]=='x';
}

bool GameMap::IsMine(const std::pair<int, int>& pos) const
{
    return m_gamePtr->Board().GetVal()[Index(pos)]=='o';
}

bool GameMap::IsCoin(const std::pair<int, int>& pos) const
{
    return m_gamePtr->Board().GetVal()[Index(pos)]=='$';
}

bool GameMap::IsPoisonGas(const std::pair<int, int>& pos) const
{
    return m_gamePtr->Board().GetVal()[Index(pos)]=='p';
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

int GameMap::TimeUntilNextJoystickReadout(int timestamp)
{
    // Get current time from the clock, using microseconds resolution
    const boost::posix_time::ptime now = 
        boost::posix_time::microsec_clock::local_time();

    // Get the time offset in current day
    const boost::posix_time::time_duration td = now.time_of_day();

    int total_milliseconds = td.total_milliseconds();

    // We ignore midnight to keep it simple
    return timestamp - total_milliseconds;
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
        std::cout << "Missile direction ";

        switch (direction) {
        case Consoden::TankGame::Direction::Left:
            std::cout << "Left";
            break;
        case Consoden::TankGame::Direction::Right:
            std::cout << "Right";
            break;
        case Consoden::TankGame::Direction::Up:
            std::cout << "Up";
            break;
        case Consoden::TankGame::Direction::Down:
            std::cout << "Down";
            break;
        case Consoden::TankGame::Direction::Neutral:
            std::cout << "Neutral";
            break;
        }
        std::cout << std::endl;
    }

    std::cout << std::endl;
}

