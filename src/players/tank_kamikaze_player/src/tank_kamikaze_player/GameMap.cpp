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

#include "GameMap.h"

#include <iostream>
#include <utility>
#include <vector>
#include <map>

#include <Consoden/TankGame/GameState.h>

namespace TankKamikazePlayer
{
    GameMap::GameMap(Consoden::TankGame::GameStatePtr game_ptr, int tankId) :
        m_Game_ptr(game_ptr),
        m_TowerDirection(Consoden::TankGame::Direction::Left),
        m_Fire(false),
        m_TankId(tankId)
    {
        x_size = m_Game_ptr->Width();
        y_size = m_Game_ptr->Height();

        for (int y = 0; y < y_size; y++) {
            for (int x = 0; x < x_size; x++) {
                int index = (y * x_size) + x;
                m_Game[x][y] = m_Game_ptr->Board().GetVal()[index];
                m_MissileMap1[x][y] = -1; // No missile
                m_MissileMap2[x][y] = -1; // No missile
            }
        }

        Safir::Dob::Typesystem::ArrayIndex missile_index;

        for (missile_index = 0; 
             missile_index < m_Game_ptr->MissilesArraySize(); 
             missile_index++) {

            if (m_Game_ptr->Missiles()[missile_index].IsNull()) {
                // empty missile slot
                continue;
            }

            Consoden::TankGame::MissilePtr missile_ptr = 
                boost::static_pointer_cast<Consoden::TankGame::Missile>(m_Game_ptr->Missiles()[missile_index].GetPtr());

            if (missile_ptr->InFlames()) {
                continue;
            }

            std::pair<int, int> head = std::make_pair(missile_ptr->HeadPosX().GetVal(), missile_ptr->HeadPosY().GetVal());
            std::pair<int, int> tail = std::make_pair(missile_ptr->TailPosX().GetVal(), missile_ptr->TailPosY().GetVal());

            if (missile_ptr->TankId().GetVal() == m_TankId) {
                FollowMissile(m_MissileMap2, head, tail, missile_ptr->Direction().GetVal());
            } else {
                FollowMissile(m_MissileMap1, head, tail, missile_ptr->Direction().GetVal());
            }
        }
    } 

    void GameMap::FollowMissile(std::map<int, std::map<int, int> >& mapRef, std::pair<int, int> head, std::pair<int, int> tail, Consoden::TankGame::Direction::Enumeration direction)
    {
        // Follow missile til it leaves board
        bool on_board = true;
        int steps = 0;

        mapRef[head.first][head.second] = 0; // Missile in this turn
        mapRef[tail.first][tail.second] = 0; // Missile in this turn

        while (on_board) {
            steps++;
            if (direction == Consoden::TankGame::Direction::Left) {
                head.first -= 2;
                tail.first -= 2;
                if (tail.first < 0 || WallSquare(tail)) {
                    // Full wrap
                    on_board = false;
                } else if (head.first < 0 || WallSquare(head)) {
                    // half wrap
                    mapRef[tail.first][tail.second] = steps; // Missile will be here in this many steps
                    on_board = false;
                } else {
                    mapRef[head.first][head.second] = steps; // Missile will be here in this many steps
                    mapRef[tail.first][tail.second] = steps; // Missile will be here in this many steps     
                }
            } else if (direction == Consoden::TankGame::Direction::Right) {
                head.first += 2;
                tail.first += 2;
                if (tail.first >= x_size || WallSquare(tail)) {
                    // Full wrap
                    on_board = false;
                } else if (head.first >= x_size || WallSquare(head)) {
                    // half wrap
                    mapRef[tail.first][tail.second] = steps; // Missile will be here in this many steps
                    on_board = false;
                } else {
                    mapRef[head.first][head.second] = steps; // Missile will be here in this many steps
                    mapRef[tail.first][tail.second] = steps; // Missile will be here in this many steps     
                }
            } else if (direction == Consoden::TankGame::Direction::Up) {
                head.second -= 2;
                tail.second -= 2;
                if (tail.second < 0 || WallSquare(tail)) {
                    // Full wrap
                    on_board = false;
                } else if (head.second < 0 || WallSquare(head)) {
                    // half wrap
                    mapRef[tail.first][tail.second] = steps; // Missile will be here in this many steps
                    on_board = false;
                } else {
                    mapRef[head.first][head.second] = steps; // Missile will be here in this many steps
                    mapRef[tail.first][tail.second] = steps; // Missile will be here in this many steps     
                }
            } else if (direction == Consoden::TankGame::Direction::Down) {
                head.second += 2;
                tail.second += 2;
                if (tail.second >= y_size || WallSquare(tail)) {
                    // Full wrap
                    on_board = false;
                } else if (head.second >= y_size || WallSquare(head)) {
                    // half wrap
                    mapRef[tail.first][tail.second] = steps; // Missile will be here in this many steps
                    on_board = false;
                } else {
                    mapRef[head.first][head.second] = steps; // Missile will be here in this many steps
                    mapRef[tail.first][tail.second] = steps; // Missile will be here in this many steps     
                }
            }
        }
    }


    void GameMap::FakeMine(std::pair<int, int> pos) 
    {
        m_Game[pos.first][pos.second] = 'o';
    }

    bool GameMap::EmptySquare(int pos_x, int pos_y)
    {
        return (m_Game[pos_x][pos_y] == '.');
    }

    bool GameMap::EmptySquare(std::pair<int, int> pos)
    {
        return (m_Game[pos.first][pos.second] == '.');
    }

    bool GameMap::WallSquare(int pos_x, int pos_y)
    {
        return (m_Game[pos_x][pos_y] == 'x');
    }

    bool GameMap::WallSquare(std::pair<int, int> pos)
    {
        return (m_Game[pos.first][pos.second]  == 'x');
    }

    void GameMap::GenerateShortestPaths(std::pair<int, int> start_pos)
    {
        // Shortest path finder by BFS
        for (int y = 0; y < y_size; y++) {
            for (int x = 0; x < x_size; x++) {
                m_GamePaths[x][y] = 200000;
            }
        }   

        m_GamePaths[start_pos.first][start_pos.second] = 0; // Current coords
        EvaluateShortestPaths(start_pos, 1);
    }

    void GameMap::GenerateLongestPaths(std::pair<int, int> start_pos)
    {
        std::map<int, std::map<int, int> > stepMap;

        // Longest path finder by BFS
        for (int y = 0; y < y_size; y++) {
            for (int x = 0; x < x_size; x++) {
                m_GamePaths[x][y] = 200000;
                stepMap[x][y] = 0;
            }
        }   

        m_GamePaths[start_pos.first][start_pos.second] = 0; // Current coords
        EvaluateLongestPaths(start_pos, 1, stepMap);
    }

    void GameMap::GenerateOpponentShortestPaths(std::pair<int, int> start_pos)
    {
        // Shortest path finder by BFS
        for (int y = 0; y < y_size; y++) {
            for (int x = 0; x < x_size; x++) {
                m_OpponentPaths[x][y] = 200000;
            }
        }   

        m_OpponentPaths[start_pos.first][start_pos.second] = 0; // Current coords
        EvaluateOpponentShortestPaths(start_pos, 1);
    }

    void GameMap::GenerateNewMissileMap(std::pair<int, int> tank_next_pos, Consoden::TankGame::Direction::Enumeration direction)
    {
        for (int y = 0; y < y_size; y++) {
            for (int x = 0; x < x_size; x++) {
                m_NewMissileMap[x][y] = -1; // No missile
            }
        }

        std::pair<int, int> head = tank_next_pos;
        std::pair<int, int> tail = tank_next_pos;

        // Fake the starting position of missile as one step backwards
        if (direction == Consoden::TankGame::Direction::Left) {
            //head.first -= 2;
            tail.first += 1;
        } else if (direction == Consoden::TankGame::Direction::Right) {
            //head.first += 2;
            tail.first -= 1;
        } else if (direction == Consoden::TankGame::Direction::Up) {
            //head.second -= 2;
            tail.second += 1;
        } else {
            //head.second += 2;
            tail.second -= 1;
        }

        // Follow missile til it leaves board
        bool on_board = true;
        int steps = 0;
        while (on_board) {
            steps++;
            if (direction == Consoden::TankGame::Direction::Left) {
                head.first -= 2;
                tail.first -= 2;
                if (tail.first < 0 || WallSquare(tail))  {
                    // Full wrap
                    on_board = false;
                } else if (head.first < 0 || WallSquare(head)) {
                    // half wrap
                    m_NewMissileMap[tail.first][tail.second] = steps; // Missile will be here in this many steps
                    on_board = false;
                } else {
                    m_NewMissileMap[head.first][head.second] = steps; // Missile will be here in this many steps
                    m_NewMissileMap[tail.first][tail.second] = steps; // Missile will be here in this many steps     
                }
            } else if (direction == Consoden::TankGame::Direction::Right) {
                head.first += 2;
                tail.first += 2;
                if (tail.first >= x_size || WallSquare(tail)) {
                    // Full wrap
                    on_board = false;
                } else if (head.first >= x_size || WallSquare(head)) {
                    // half wrap
                    m_NewMissileMap[tail.first][tail.second] = steps; // Missile will be here in this many steps
                    on_board = false;
                } else {
                    m_NewMissileMap[head.first][head.second] = steps; // Missile will be here in this many steps
                    m_NewMissileMap[tail.first][tail.second] = steps; // Missile will be here in this many steps     
                }
            } else if (direction == Consoden::TankGame::Direction::Up) {
                head.second -= 2;
                tail.second -= 2;
                if (tail.second < 0 || WallSquare(tail)) {
                    // Full wrap
                    on_board = false;
                } else if (head.second < 0 || WallSquare(head)) {
                    // half wrap
                    m_NewMissileMap[tail.first][tail.second] = steps; // Missile will be here in this many steps
                    on_board = false;
                } else {
                    m_NewMissileMap[head.first][head.second] = steps; // Missile will be here in this many steps
                    m_NewMissileMap[tail.first][tail.second] = steps; // Missile will be here in this many steps     
                }
            } else {
                head.second += 2;
                tail.second += 2;
                if (tail.second >= y_size || WallSquare(tail)) {
                    // Full wrap
                    on_board = false;
                } else if (head.second >= y_size || WallSquare(head)) {
                    // half wrap
                    m_NewMissileMap[tail.first][tail.second] = steps; // Missile will be here in this many steps
                    on_board = false;
                } else {
                    m_NewMissileMap[head.first][head.second] = steps; // Missile will be here in this many steps
                    m_NewMissileMap[tail.first][tail.second] = steps; // Missile will be here in this many steps     
                }
            }
        }
       
    }

    int GameMap::MissileOverSquareInSteps(std::pair<int, int> pos, int steps)
    {
        return (m_MissileMap1[pos.first][pos.second] == steps) || (m_MissileMap2[pos.first][pos.second] == steps);
    }

    int GameMap::NewMissileOverSquareInSteps(std::pair<int, int> pos)
    {
        return m_NewMissileMap[pos.first][pos.second];
    }

    void GameMap::EvaluateShortestPathsHelper(std::pair<int, int> current_pos, std::pair<int, int> next_pos, Consoden::TankGame::Direction::Enumeration direction, int steps) 
    {
        if (EmptySquare(next_pos) && 
            (!MissileOverSquareInSteps(next_pos, steps)) &&
            (!AgainstMissilePath(current_pos, next_pos, direction, steps-1))) 
        {
            if (steps < m_GamePaths[next_pos.first][next_pos.second]) {
                m_GamePaths[next_pos.first][next_pos.second] = steps;
                EvaluateShortestPaths(next_pos, steps+1);                
            }
        }        
    }

    void GameMap::EvaluateShortestPaths(std::pair<int, int> pos, int steps)
    {
        EvaluateShortestPathsHelper(pos, Left(pos), Consoden::TankGame::Direction::Left, steps);
        EvaluateShortestPathsHelper(pos, Right(pos), Consoden::TankGame::Direction::Right, steps);
        EvaluateShortestPathsHelper(pos, Up(pos), Consoden::TankGame::Direction::Up, steps);
        EvaluateShortestPathsHelper(pos, Down(pos), Consoden::TankGame::Direction::Down, steps);
    }

    void GameMap::EvaluateLongestPathsHelper(std::pair<int, int> current_pos, std::pair<int, int> next_pos, Consoden::TankGame::Direction::Enumeration direction, int steps, std::map<int, std::map<int, int> >& stepMap) 
    {
        if (EmptySquare(next_pos) && 
            (stepMap[next_pos.first][next_pos.second] == 0) &&
            (!MissileOverSquareInSteps(next_pos, steps)) &&
            (!AgainstMissilePath(current_pos, next_pos, direction, steps-1))) 
        {
            if (steps > m_GamePaths[next_pos.first][next_pos.second] || m_GamePaths[next_pos.first][next_pos.second] == 200000) {
                m_GamePaths[next_pos.first][next_pos.second] = steps;
                EvaluateShortestPaths(next_pos, steps+1);                
            }
        }        
    }

    void GameMap::EvaluateLongestPaths(std::pair<int, int> pos, int steps, std::map<int, std::map<int, int> >& stepMap)
    {
        // mark step as used
        stepMap[pos.first][pos.second] = 1;

        EvaluateLongestPathsHelper(pos, Left(pos), Consoden::TankGame::Direction::Left, steps, stepMap);
        EvaluateLongestPathsHelper(pos, Right(pos), Consoden::TankGame::Direction::Right, steps, stepMap);
        EvaluateLongestPathsHelper(pos, Up(pos), Consoden::TankGame::Direction::Up, steps, stepMap);
        EvaluateLongestPathsHelper(pos, Down(pos), Consoden::TankGame::Direction::Down, steps, stepMap);

        // mark step as free
        stepMap[pos.first][pos.second] = 0;
    }

    void GameMap::EvaluateOpponentShortestPathsHelper(std::pair<int, int> current_pos, std::pair<int, int> next_pos, Consoden::TankGame::Direction::Enumeration direction, int steps) 
    {
        if (EmptySquare(next_pos) && 
            (!MissileOverSquareInSteps(next_pos, steps)) &&
            (!AgainstMissilePath(current_pos, next_pos, direction, steps-1))) 
        {
            if (steps < m_OpponentPaths[next_pos.first][next_pos.second]) {
                m_OpponentPaths[next_pos.first][next_pos.second] = steps;
                EvaluateOpponentShortestPaths(next_pos, steps+1);                
            }
        }                
    }

    void GameMap::EvaluateOpponentShortestPaths(std::pair<int, int> pos, int steps)
    {
        EvaluateOpponentShortestPathsHelper(pos, Left(pos), Consoden::TankGame::Direction::Left, steps);
        EvaluateOpponentShortestPathsHelper(pos, Right(pos), Consoden::TankGame::Direction::Right, steps);
        EvaluateOpponentShortestPathsHelper(pos, Up(pos), Consoden::TankGame::Direction::Up, steps);
        EvaluateOpponentShortestPathsHelper(pos, Down(pos), Consoden::TankGame::Direction::Down, steps);
    }

    bool GameMap::CanReachSquare(int x, int y) {
        return (m_GamePaths[x][y] < 200000);
    }

    bool GameMap::CanReachSquare(std::pair<int, int> pos) {
        return (m_GamePaths[pos.first][pos.second] < 200000);
    }

    bool GameMap::CanOpponentReachSquare(int x, int y) {
        return (m_OpponentPaths[x][y] < 200000);
    }

    int GameMap::StepsToSquare(int x, int y) {
        if (x >= x_size || x < 0 || y >= y_size || y < 0)
            return 200000;

        return m_GamePaths[x][y];
    }

    int GameMap::StepsToSquare(std::pair<int, int> pos) {
        return StepsToSquare(pos.first, pos.second);
    }

    std::pair<int, int> GameMap::BacktrackFromSquare(int x, int y)
    {
        //std::cout << "Backtrack path: "  << x << y << std::endl;

        // Steps to square
        int steps = m_GamePaths[x][y];

        // one step - found the next square
        if (steps == 1) {
            return std::make_pair(x,y);
        }

        // Shortest path is one step less
        if (StepsToSquare((x+1) % x_size, y) == (steps-1)) {
            return BacktrackFromSquare((x+1) % x_size, y);
        }

        if (StepsToSquare((x-1+x_size) % x_size, y) == (steps-1)) {
            return BacktrackFromSquare((x-1+x_size) % x_size, y);
        }

        if (StepsToSquare(x, (y+1) % y_size) == (steps-1)) {
            return BacktrackFromSquare(x, (y+1) % y_size);
        }

        if (StepsToSquare(x, (y-1+y_size) % y_size) == (steps-1)) {
            return BacktrackFromSquare(x, (y-1+y_size) % y_size);
        }

        // How did this happend??
        return std::make_pair(-1, -1);
    }

    std::vector< std::pair<int, int> > GameMap::BacktrackAllFromSquare(std::pair<int, int> fromSquare)
    {
        //std::cout << "Backtrack path: "  << x << y << std::endl;

        // Steps to square
        int steps = m_GamePaths[fromSquare.first][fromSquare.second];

        std::vector< std::pair<int, int> > result;
        std::vector< std::pair<int, int> > tmpResult;

        // one step - found the next square
        if (steps == 1) {
            result.push_back(fromSquare);
            return result;
        }

        // Shortest path is one step less
        if (StepsToSquare(Left(fromSquare)) == (steps-1)) {
            tmpResult = BacktrackAllFromSquare(Left(fromSquare));
            result.insert(result.end(), tmpResult.begin(), tmpResult.end());
        }

        if (StepsToSquare(Right(fromSquare)) == (steps-1)) {
            tmpResult = BacktrackAllFromSquare(Right(fromSquare));
            result.insert(result.end(), tmpResult.begin(), tmpResult.end());
        }

        if (StepsToSquare(Up(fromSquare)) == (steps-1)) {
            tmpResult = BacktrackAllFromSquare(Up(fromSquare));
            result.insert(result.end(), tmpResult.begin(), tmpResult.end());
        }

        if (StepsToSquare(Down(fromSquare)) == (steps-1)) {
            tmpResult = BacktrackAllFromSquare(Down(fromSquare));
            result.insert(result.end(), tmpResult.begin(), tmpResult.end());
        }

        return result;
    }

    std::pair<int, int> GameMap::BacktrackFromMax()
    {
        int max_steps = 0;
        std::pair<int, int> max_pos(-1, -1);

        // Shortest path finder by BFS
        for (int y = 0; y < y_size; y++) {
            for (int x = 0; x < x_size; x++) {
                if (CanReachSquare(x, y)) {
                    if (max_steps < m_GamePaths[x][y]) {
                        max_steps = m_GamePaths[x][y];
                        max_pos.first = x;
                        max_pos.second = y;
                    }
                }
            }
        }   

        return BacktrackFromSquare(max_pos.first, max_pos.second);
    }

    void GameMap::FindBestMoves(std::pair<int, int> target, std::map< std::pair<int, int>, int >& moveScoreMap)
    {
        std::vector<std::pair<int, int> > bestMoves = BacktrackAllFromSquare(target);

        std::vector<std::pair<int, int> >::const_iterator it;
        for (it = bestMoves.begin(); it != bestMoves.end(); it++) {
            if (moveScoreMap.find((*it)) != moveScoreMap.end()) {
                moveScoreMap[(*it)]++;
            } else {
                moveScoreMap[(*it)] = 1;
            }
        }
    }

    // Returns the direction to move towards the enemy
    Consoden::TankGame::Direction::Enumeration GameMap::MoveToEnemy(int player_x, int player_y, int opponent_x, int opponent_y) 
    {
        std::pair<int, int> next_square(-1,-1);

        // Place a fake mine out own square, since it will contain a mine after this move
        FakeMine(std::make_pair(player_x, player_y));
        
        // Place a fake mine in the square of the enemy tank, since it will contain a mine after this move
        FakeMine(std::make_pair(opponent_x, opponent_y));

        GenerateShortestPaths(std::make_pair(player_x, player_y));

        std::map< std::pair<int, int>, int> moveScoreMap;

        // The opponent may move in one of four directions (normally only three, but in the first move it is four)
        // Evaluate our best moves based on each opponent move, and then pick the move with best combined effect

        std::pair<int, int> opponentLeft = Left(std::make_pair(opponent_x, opponent_y));
        if (EmptySquare(opponentLeft) && CanReachSquare(opponentLeft)) {
            FindBestMoves(opponentLeft, moveScoreMap);
        }

        std::pair<int, int> opponentRight = Right(std::make_pair(opponent_x, opponent_y));
        if (EmptySquare(opponentRight) && CanReachSquare(opponentRight)) {
            FindBestMoves(opponentRight, moveScoreMap);
        }

        std::pair<int, int> opponentUp = Up(std::make_pair(opponent_x, opponent_y));
        if (EmptySquare(opponentUp) && CanReachSquare(opponentUp)) {
            FindBestMoves(opponentUp, moveScoreMap);
        }

        std::pair<int, int> opponentDown = Down(std::make_pair(opponent_x, opponent_y));
        if (EmptySquare(opponentDown) && CanReachSquare(opponentDown)) {
            FindBestMoves(opponentDown, moveScoreMap);
        }


        std::cout << "Tank in square " << player_x << "," << player_y << std::endl;
        std::cout << "Enemy in square " << opponent_x << "," << opponent_y << std::endl;
        if (!moveScoreMap.empty()) {
            int topScore = 0;
            std::map< std::pair<int, int>, int>::const_iterator it;
            for (it = moveScoreMap.begin(); it != moveScoreMap.end(); it++) 
            {
                std::cout << "Move: " << (*it).first.first << "," << (*it).first.second << "  score " << (*it).second << std::endl;
                if ((*it).second > topScore) {
                    topScore = (*it).second;
                    next_square = (*it).first;
                }
            }
            std::cout << std::endl;

        } else {
            // Can't reach opponent, just keep moving as long as possible
            GenerateLongestPaths(std::make_pair(player_x, player_y));
            next_square = BacktrackFromMax();
            std::cout << "Cannot reach opponent, move as long as possible. Next move: " << next_square.first << "," << next_square.second << std::endl;
        }

        Consoden::TankGame::Direction::Enumeration result;
        if (next_square.first == -1 && next_square.second == -1) {
            // Don't know what to do, go WEST!
            result = Consoden::TankGame::Direction::Left;
        }

        if (player_x == next_square.first) {
            if (player_y + 1 == next_square.second || ((player_y + 1) == y_size && next_square.second == 0)) {
                result = Consoden::TankGame::Direction::Down;
            } else {
                result = Consoden::TankGame::Direction::Up;                
            }
        } else if (player_y == next_square.second) {
            if (player_x + 1 == next_square.first || ((player_x + 1) == x_size && next_square.first == 0)) {
                result = Consoden::TankGame::Direction::Right;
            } else {
                result = Consoden::TankGame::Direction::Left; 
            }
        }

        // Evaluate enemy moves
        GenerateOpponentShortestPaths(std::make_pair(opponent_x, opponent_y));

        // Evaluate firing direction
        int x_distance;
        int x_wrap_distance;
        Consoden::TankGame::Direction::Enumeration x_direction;
        int y_distance;
        int y_wrap_distance;
        Consoden::TankGame::Direction::Enumeration y_direction;

        if (opponent_x > player_x) {
            // opponent to the right
            x_distance = opponent_x - player_x;
            x_wrap_distance = player_x - opponent_x + x_size;
            if (x_distance < x_wrap_distance || !CanWrapX()) {
                x_direction = Consoden::TankGame::Direction::Right; //Shortest linear path to enemy is right
            } else {
                x_direction = Consoden::TankGame::Direction::Left; //Shortest linear path to enemy is left
                x_distance = x_wrap_distance;
            }
        } else {
            // opponent to the left
            x_distance = player_x - opponent_x;
            x_wrap_distance = opponent_x - player_x + x_size;
            if (x_distance < x_wrap_distance || !CanWrapX()) {
                x_direction = Consoden::TankGame::Direction::Left; //Shortest linear path to enemy is left
            } else {
                x_direction = Consoden::TankGame::Direction::Right; //Shortest linear path to enemy is right
                x_distance = x_wrap_distance;
            }            
        }

        if (opponent_y > player_y) {
            // opponent is down
            y_distance = opponent_y - player_y;
            y_wrap_distance = player_y - opponent_y + y_size;
            if (y_distance < y_wrap_distance || !CanWrapY()) {
                y_direction = Consoden::TankGame::Direction::Down; //Shortest linear path to enemy is down
            } else {
                y_direction = Consoden::TankGame::Direction::Up; //Shortest linear path to enemy is up
                y_distance = y_wrap_distance;
            }
        } else {
            // opponent is up
            y_distance = player_y - opponent_y;
            y_wrap_distance = opponent_y - player_y + y_size;
            if (y_distance < y_wrap_distance || !CanWrapY()) {
                y_direction = Consoden::TankGame::Direction::Up; //Shortest linear path to enemy is up
            } else {
                y_direction = Consoden::TankGame::Direction::Down; //Shortest linear path to enemy is down
                y_distance = y_wrap_distance;
            }            
        }

        if (x_distance > y_distance) {
            m_TowerDirection = x_direction;
        } else {
            m_TowerDirection = y_direction;
        }

        // Evaluate if we fire        
        GenerateNewMissileMap(next_square, m_TowerDirection);        

        /*
        std::cout << "Opponent paths" << std::endl;
        PrintOpponent();
        
        std::cout << "New Missile map" << std::endl;
        PrintNewMissile();
        */

        m_Fire = false;
        for (int y = 0; y < y_size; y++) {
            for (int x = 0; x < x_size; x++) {
                if (m_OpponentPaths[x][y] == m_NewMissileMap[x][y]) 
                { // We can hit!
                    m_Fire = true;
                } 
                else if ((m_OpponentPaths[x][y]+1) == m_NewMissileMap[x][y]) 
                {
                    if (m_TowerDirection == Consoden::TankGame::Direction::Right && x > 0) {
                        if (m_OpponentPaths[x-1][y] == (m_NewMissileMap[x-1][y]+1)) {
                            // We can hit!
                            m_Fire = true;
                        }
                    } else if (m_TowerDirection == Consoden::TankGame::Direction::Left && x < (x_size-1)) {
                        if (m_OpponentPaths[x+1][y] == (m_NewMissileMap[x+1][y]+1)) {
                            // We can hit!
                            m_Fire = true;
                        }
                    } else if (m_TowerDirection == Consoden::TankGame::Direction::Down && y > 0) {
                        if (m_OpponentPaths[x][y-1] == (m_NewMissileMap[x][y-1]+1)) {
                            // We can hit!
                            m_Fire = true;
                        }
                    } else if (y < (y_size-1)) {
                        if (m_OpponentPaths[x][y+1] == (m_NewMissileMap[x][y+1]+1)) {
                            // We can hit!
                            m_Fire = true;
                        }
                    }
                }
            }
        }

        if (m_Fire) {
            // We have a chance to hit opponent
            std::cout << "We may hit opponent, fire!" << std::endl;
        }

        return result;
    }

    Consoden::TankGame::Direction::Enumeration GameMap::FireToEnemy()
    {
        return m_TowerDirection;
    }

    bool GameMap::Fire()
    {
        return m_Fire;
    }

    // To wrap over in x direction, we need a connected pair of squares on each side of the wrapping that are reachable
    bool GameMap::CanWrapX()
    {
        for (int y = 0; y < y_size; y++) {
            if (CanReachSquare(0, y) && CanReachSquare(x_size - 1, y) && CanOpponentReachSquare(0, y)) {
                return true;
            }
        }
        return false;
    }

    // To wrap over in y direction, we need a connected pair of squares on each side of the wrapping that are reachable
    bool GameMap::CanWrapY()
    {
        for (int x = 0; x < x_size; x++) {
            if (CanReachSquare(x, 0) && CanReachSquare(x, y_size - 1) && CanOpponentReachSquare(x, 0)) {
                return true;
            }
        }
        return false;
    }


    // Checks if we move against a missile (in between turns)
    bool GameMap::AgainstMissilePath(std::pair<int, int> first_pos, std::pair<int, int> second_pos, Consoden::TankGame::Direction::Enumeration direction, int first_steps) 
    {
        // If we wrap, we will not get caught by the missile going off the board
        if ((first_pos.first == 0 && second_pos.first == x_size) ||
            (first_pos.first == x_size && second_pos.first == 0) ||
            (first_pos.second == 0 && second_pos.second == y_size) ||
            (first_pos.second == y_size && second_pos.second == 0)) {
            return false;
        }

        if (MissileOverSquareInSteps(second_pos, first_steps) && MissileOverSquareInSteps(first_pos, (first_steps + 1))) {
            // Seems like we are moving against missile, confirm that it is the same one
            std::pair<int, int> second_missile_part;
            if (direction == Consoden::TankGame::Direction::Left) {
                second_missile_part = Left(second_pos);
            } else if (direction == Consoden::TankGame::Direction::Right) {
                second_missile_part = Right(second_pos);
            } else if (direction == Consoden::TankGame::Direction::Up) {
                second_missile_part = Up(second_pos);
            } else {
                second_missile_part = Down(second_pos);
            }

            if (MissileOverSquareInSteps(second_missile_part, first_steps)) {
                return true;
            }
        }
        return false;
    }

    bool GameMap::AgainstNewMissilePath(std::pair<int, int> first_pos, std::pair<int, int> second_pos, Consoden::TankGame::Direction::Enumeration direction, int first_steps) 
    {
        // If we wrap, we will not get caught by the missile going off the board
        if ((first_pos.first == 0 && second_pos.first == x_size) ||
            (first_pos.first == x_size && second_pos.first == 0) ||
            (first_pos.second == 0 && second_pos.second == y_size) ||
            (first_pos.second == y_size && second_pos.second == 0)) {
            return false;
        }

        if (NewMissileOverSquareInSteps(second_pos) == first_steps && NewMissileOverSquareInSteps(first_pos) == (first_steps + 1)) {
            // Seems like we are moving against missile, confirm that it is the same one
            std::pair<int, int> second_missile_part;
            if (direction == Consoden::TankGame::Direction::Left) {
                second_missile_part = Left(second_pos);
            } else if (direction == Consoden::TankGame::Direction::Right) {
                second_missile_part = Right(second_pos);
            } else if (direction == Consoden::TankGame::Direction::Up) {
                second_missile_part = Up(second_pos);
            } else {
                second_missile_part = Down(second_pos);
            }

            if (NewMissileOverSquareInSteps(second_missile_part) == first_steps) {
                return true;
            }
        }
        return false;
    }

    void GameMap::PrintMap()
    {
        for (int y = 0; y < y_size; y++) {
            for (int x = 0; x < x_size; x++) {
                std::cout << m_Game[x][y];
            }
            std::cout << std::endl;
        }        
    }
 
    void GameMap::Print()
    {
        for (int y = 0; y < y_size; y++) {
            for (int x = 0; x < x_size; x++) {
                //int index = (y * x_size) + x;
                if (m_GamePaths[x][y] == 200000)
                    std::cout  << ".";
                else
                    std::cout << m_GamePaths[x][y];
            }
            std::cout << std::endl;
        }        
    }

    void GameMap::PrintMissiles()
    {
        std::cout << "Enemy missile" << std::endl;
        for (int y = 0; y < y_size; y++) {
            for (int x = 0; x < x_size; x++) {
                //int index = (y * x_size) + x;
                if (m_MissileMap1[x][y] == -1)
                    std::cout  << ".";
                else
                    std::cout << m_MissileMap1[x][y];
            }
            std::cout << std::endl;
        }        

        std::cout << "Player missile" << std::endl;
        for (int y = 0; y < y_size; y++) {
            for (int x = 0; x < x_size; x++) {
                //int index = (y * x_size) + x;
                if (m_MissileMap2[x][y] == -1)
                    std::cout  << ".";
                else
                    std::cout << m_MissileMap2[x][y];
            }
            std::cout << std::endl;
        }        
    }

    void GameMap::PrintOpponent()
    {
        for (int y = 0; y < y_size; y++) {
            for (int x = 0; x < x_size; x++) {
                //int index = (y * x_size) + x;
                if (m_OpponentPaths[x][y] == 200000)
                    std::cout  << ".";
                else
                    std::cout << m_OpponentPaths[x][y];
            }
            std::cout << std::endl;
        }        
    }

    void GameMap::PrintNewMissile()
    {
        for (int y = 0; y < y_size; y++) {
            for (int x = 0; x < x_size; x++) {
                //int index = (y * x_size) + x;
                if (m_NewMissileMap[x][y] == -1)
                    std::cout  << ".";
                else
                    std::cout << m_NewMissileMap[x][y];
            }
            std::cout << std::endl;
        }        
    }
 
    std::pair<int, int> GameMap::Left(std::pair<int, int> pos)
    {
        int next_x = (pos.first - 1 + x_size) % x_size;
        int next_y = pos.second;

        return std::make_pair(next_x, next_y);
    }

    std::pair<int, int> GameMap::Right(std::pair<int, int> pos)
    {
        int next_x = (pos.first + 1) % x_size;
        int next_y = pos.second;

        return std::make_pair(next_x, next_y);
    }

    std::pair<int, int> GameMap::Up(std::pair<int, int> pos)
    {
        int next_x = pos.first;
        int next_y = (pos.second - 1 + y_size) % y_size;

        return std::make_pair(next_x, next_y);
    }

    std::pair<int, int> GameMap::Down(std::pair<int, int> pos)
    {
        int next_x = pos.first;
        int next_y = (pos.second + 1) % y_size;

        return std::make_pair(next_x, next_y);
    }

 };

