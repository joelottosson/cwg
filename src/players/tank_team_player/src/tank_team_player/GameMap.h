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

#ifndef __GAMEMAP_H
#define __GAMEMAP_H

#include <vector>
#include <map>

#include <Consoden/TankGame/GameState.h>

namespace TankTeamPlayer
{
    /** 
     * Helper class for the Strategy. Parses game board and locates the 
     * player and enemy tanks.
     */
    class GameMap 
    {
    public:

        GameMap(int tankId, Consoden::TankGame::GameStatePtr gamePtr);

        /**
         * Is this square empty? Only checks for walls and mines!
         */
        bool EmptySquare(std::pair<int, int> pos);

        /**
         * Location of the players tank 
         */
        std::pair<int, int> PlayerPosition() {return m_PlayerPos; }

        /**
         * Location of the enemys tank 
         */
        std::pair<int, int> EnemyPosition() {return m_EnemyPos; }

        /**
         * Is there a missile in this position right now?
         */
        bool MissileInPosition(std::pair<int, int> pos);

        /**
         * For debugging assistance, prints out the parsed map
         */
        void PrintMap();

        /**
         * For debugging assistance, prints out the current state
         */
        void PrintState();

        /**
         * Helper functions that moves one step in the indicated direction
         */ 
        std::pair<int, int> Left(std::pair<int, int> pos);
        std::pair<int, int> Right(std::pair<int, int> pos);
        std::pair<int, int> Up(std::pair<int, int> pos);
        std::pair<int, int> Down(std::pair<int, int> pos);

        int XSize() { return m_XSize; }
        int YSize() { return m_YSize; }

    private:
        int m_TankId; // The tank id for the player
        int m_XSize;
        int m_YSize;

        Consoden::TankGame::GameStatePtr m_GamePtr;

        // The parsed game board
        std::map<int, std::map<int, char> > m_GameMap;

        // Current positions of player and enemy
        std::pair<int, int> m_PlayerPos;
        std::pair<int, int> m_EnemyPos;
    };
};
#endif
