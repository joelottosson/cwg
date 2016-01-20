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
#include <Consoden/TankGame/GameState.h>

namespace TankKamikazePlayer
{
    /** 
     * Defines a vehicle owner. This class handles the
     * registration as a data owner and processes request
     * on that data.
     */
    class GameMap 
    {
    public:

        GameMap(Consoden::TankGame::GameStatePtr game_ptr, int tankId);

        void FollowMissile(std::map<int, std::map<int, int> >& mapRef, std::pair<int, int> head, std::pair<int, int> tail, Consoden::TankGame::Direction::Enumeration direction);

        void FakeMine(std::pair<int, int> pos);

        bool EmptySquare(int pos_x, int pos_y);
        bool EmptySquare(std::pair<int, int> pos);

        bool WallSquare(int pos_x, int pos_y);
        bool WallSquare(std::pair<int, int> pos);

        void GenerateNewMissileMap(std::pair<int, int> tank_next_pos, Consoden::TankGame::Direction::Enumeration direction);
        void GenerateShortestPaths(std::pair<int, int> start_pos);
        void GenerateLongestPaths(std::pair<int, int> start_pos);
        void GenerateOpponentShortestPaths(std::pair<int, int> start_pos);
    
        int MissileOverSquareInSteps(std::pair<int, int> pos, int steps);
        int NewMissileOverSquareInSteps(std::pair<int, int> pos);

        void EvaluateShortestPathsHelper(std::pair<int, int> current_pos, std::pair<int, int> next_pos, Consoden::TankGame::Direction::Enumeration direction, int steps);
        void EvaluateShortestPaths(std::pair<int, int> pos, int steps);

        void EvaluateLongestPathsHelper(std::pair<int, int> current_pos, std::pair<int, int> next_pos, Consoden::TankGame::Direction::Enumeration direction, int steps, std::map<int, std::map<int, int> >& stepMap);
        void EvaluateLongestPaths(std::pair<int, int> pos, int steps, std::map<int, std::map<int, int> >& stepMap);

        void EvaluateOpponentShortestPathsHelper(std::pair<int, int> current_pos, std::pair<int, int> next_pos, Consoden::TankGame::Direction::Enumeration direction, int steps);
        void EvaluateOpponentShortestPaths(std::pair<int, int> pos, int steps);

        bool CanReachSquare(int x, int y); 
        bool CanReachSquare(std::pair<int, int> pos); 
        bool CanOpponentReachSquare(int x, int y);

        int StepsToSquare(int x, int y);
        int StepsToSquare(std::pair<int, int> pos);

        void FindBestMoves(std::pair<int, int> target, std::map< std::pair<int, int>, int >& moveScoreMap);

        std::pair<int, int> BacktrackFromSquare(int x, int y);
        std::vector< std::pair<int, int> > BacktrackAllFromSquare(std::pair<int, int> fromSquare);
        std::pair<int, int> BacktrackFromMax();
        Consoden::TankGame::Direction::Enumeration MoveToEnemy(int player_x, int player_y, int opponent_x, int opponent_y);

        // The following two functions must be called after MoveToEnemy
        Consoden::TankGame::Direction::Enumeration FireToEnemy();
        bool Fire();

        bool CanWrapX();
        bool CanWrapY();

        void PrintMap();
        void Print();
        void PrintMissiles();

        void PrintOpponent();
        void PrintNewMissile();

    private:

        bool AgainstMissilePath(std::pair<int, int> first_pos, std::pair<int, int> second_pos, Consoden::TankGame::Direction::Enumeration direction, int first_steps);
        bool AgainstNewMissilePath(std::pair<int, int> first_pos, std::pair<int, int> second_pos, Consoden::TankGame::Direction::Enumeration direction, int first_steps);

        std::pair<int, int> Left(std::pair<int, int> pos);
        std::pair<int, int> Right(std::pair<int, int> pos);
        std::pair<int, int> Up(std::pair<int, int> pos);
        std::pair<int, int> Down(std::pair<int, int> pos);

        Consoden::TankGame::GameStatePtr m_Game_ptr;
        std::map<int, std::map<int, char> > m_Game;
        std::map<int, std::map<int, int> > m_GamePaths;
        std::map<int, std::map<int, int> > m_MissileMap1; // Enemy missile
        std::map<int, std::map<int, int> > m_MissileMap2; // My own missile
        std::map<int, std::map<int, int> > m_NewMissileMap;
        std::map<int, std::map<int, int> > m_OpponentPaths;
        int x_size;
        int y_size;

        Consoden::TankGame::Direction::Enumeration m_TowerDirection;
        bool m_Fire;

        int m_TankId;
    };
};
#endif
