/******************************************************************************
*
* Copyright Consoden AB, 2014
*
* Created by: Björn Weström / bjwe
*
*******************************************************************************/

#include <iostream>
#include <utility>
#include <vector>
#include <map>
#include "GameMap.h"

GameMap::GameMap(int tankId, const Consoden::TankGame::GameStatePtr& gamePtr) :
    m_TankId(tankId),
    m_GamePtr(gamePtr),
    m_SizeX(gamePtr->Width().GetVal()),
    m_SizeY(gamePtr->Height().GetVal()),
    m_creationTime(boost::chrono::high_resolution_clock::now())
{
    // Locate tanks
    Safir::Dob::Typesystem::ArrayIndex tankIndex;

    for (tankIndex = 0;
         tankIndex < m_GamePtr->TanksArraySize();
         tankIndex++) {

        if (m_GamePtr->Tanks()[tankIndex].IsNull()) {
            // empty tank slot, found last tank
            break;
        }

        Consoden::TankGame::TankPtr tankPtr =
                boost::dynamic_pointer_cast<Consoden::TankGame::Tank>(m_GamePtr->Tanks()[tankIndex].GetPtr());

        std::pair<int, int> pos = std::make_pair(tankPtr->PosX().GetVal(), tankPtr->PosY().GetVal());

        if (tankPtr->TankId().GetVal() == m_TankId) {
            // This is our tank!
            m_OwnPos = pos;
        } else {
            // The enemy
            m_EnemyPos = pos;
        }
    }
}

bool GameMap::IsEmpty(const std::pair<int, int>& pos) const
{
    char c=m_GamePtr->Board().GetVal()[Index(pos)];
    return c=='.' || c=='f';
}

bool GameMap::IsFlag(const std::pair<int, int>& pos) const
{
    return m_GamePtr->Board().GetVal()[Index(pos)]=='f';
}

bool GameMap::IsMissileInPosition(const std::pair<int, int>& pos) const
{
    Safir::Dob::Typesystem::ArrayIndex missileIndex;

    for (missileIndex = 0;
         missileIndex < m_GamePtr->MissilesArraySize();
         missileIndex++) {

        if (m_GamePtr->Missiles()[missileIndex].IsNull()) {
            // empty missile slot
            continue;
        }

        Consoden::TankGame::MissilePtr missilePtr =
                boost::dynamic_pointer_cast<Consoden::TankGame::Missile>(m_GamePtr->Missiles()[missileIndex].GetPtr());

        std::pair<int, int> head = std::make_pair(missilePtr->HeadPosX().GetVal(), missilePtr->HeadPosY().GetVal());
        std::pair<int, int> tail = std::make_pair(missilePtr->TailPosX().GetVal(), missilePtr->TailPosY().GetVal());

        if ((pos == head) || (pos == tail)) {
            return true;
        }
    }

    return false;
}

void GameMap::PrintMap() const
{
    for (int y = 0; y < m_SizeY; y++) {
        for (int x = 0; x < m_SizeX; x++) {
            std::cout << m_GamePtr->Board().GetVal()[Index(x, y)];
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
         missileIndex < m_GamePtr->MissilesArraySize();
         missileIndex++) {

        if (m_GamePtr->Missiles()[missileIndex].IsNull()) {
            // empty missile slot
            continue;
        }

        Consoden::TankGame::MissilePtr missilePtr =
                boost::dynamic_pointer_cast<Consoden::TankGame::Missile>(m_GamePtr->Missiles()[missileIndex].GetPtr());

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


std::pair<int, int> GameMap::MoveLeft(const std::pair<int, int>& pos) const
{
    int next_x = (pos.first - 1 + m_SizeX) % m_SizeX;
    int next_y = pos.second;

    return std::make_pair(next_x, next_y);
}

std::pair<int, int> GameMap::MoveRight(const std::pair<int, int>& pos) const
{
    int next_x = (pos.first + 1) % m_SizeX;
    int next_y = pos.second;

    return std::make_pair(next_x, next_y);
}

std::pair<int, int> GameMap::MoveUp(const std::pair<int, int>& pos) const
{
    int next_x = pos.first;
    int next_y = (pos.second - 1 + m_SizeY) % m_SizeY;

    return std::make_pair(next_x, next_y);
}

std::pair<int, int> GameMap::MoveDown(const std::pair<int, int>& pos) const
{
    int next_x = pos.first;
    int next_y = (pos.second + 1) % m_SizeY;

    return std::make_pair(next_x, next_y);
}

unsigned int GameMap::Elapsed() const
{
    boost::chrono::high_resolution_clock::duration elapsed=boost::chrono::high_resolution_clock::now()-m_creationTime;
    return boost::chrono::duration_cast<boost::chrono::milliseconds>(elapsed).count();
}
