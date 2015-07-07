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

#include <iostream>
#include <utility>
#include <vector>
#include <map>

#include <Consoden/TankGame/GameState.h>

#include "GameMap.h"

namespace TankTeamPlayer
{
    GameMap::GameMap(int tankId, Consoden::TankGame::GameStatePtr gamePtr) :
        m_TankId(tankId),
        m_GamePtr(gamePtr)
    {
        m_XSize = m_GamePtr->Width();
        m_YSize = m_GamePtr->Height();

        // Parse game board
        for (int y = 0; y < m_YSize; y++) {
            for (int x = 0; x < m_XSize; x++) {
                int index = (y * m_XSize) + x;
                m_GameMap[x][y] = m_GamePtr->Board().GetVal()[index];
            }
        }

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
                m_PlayerPos = pos;
            } else {
                // The enemy
                m_EnemyPos = pos;
            }
        }
    }

    bool GameMap::EmptySquare(std::pair<int, int> pos)
    {
        return (m_GameMap[pos.first][pos.second] == '.');
    }
 
    bool GameMap::MissileInPosition(std::pair<int, int> pos)
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

    void GameMap::PrintMap()
    {
        for (int y = 0; y < m_YSize; y++) {
            for (int x = 0; x < m_XSize; x++) {
                std::cout << m_GameMap[x][y];
            }
            std::cout << std::endl;
        }        
        std::cout << std::endl;
    }

    void GameMap::PrintState() 
    {
        std::cout << "Game board" << std::endl;
        PrintMap();

        std::cout << "Player position " << PlayerPosition().first << "," << PlayerPosition().second << std::endl;
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
            }
            std::cout << std::endl;
        }

        std::cout << std::endl;
    }

 
    std::pair<int, int> GameMap::Left(std::pair<int, int> pos)
    {
        int next_x = (pos.first - 1 + m_XSize) % m_XSize;
        int next_y = pos.second;

        return std::make_pair(next_x, next_y);
    }

    std::pair<int, int> GameMap::Right(std::pair<int, int> pos)
    {
        int next_x = (pos.first + 1) % m_XSize;
        int next_y = pos.second;

        return std::make_pair(next_x, next_y);
    }

    std::pair<int, int> GameMap::Up(std::pair<int, int> pos)
    {
        int next_x = pos.first;
        int next_y = (pos.second - 1 + m_YSize) % m_YSize;

        return std::make_pair(next_x, next_y);
    }

    std::pair<int, int> GameMap::Down(std::pair<int, int> pos)
    {
        int next_x = pos.first;
        int next_y = (pos.second + 1) % m_YSize;

        return std::make_pair(next_x, next_y);
    }

 };

