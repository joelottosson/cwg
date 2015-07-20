/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Björn Weström / bjwe
*
*******************************************************************************/

#include "BfsHelper.h"

#include <iostream>
#include <map>


BfsHelper::BfsHelper(const Consoden::TankGame::GameStatePtr& gamePtr, std::pair<int, int> start_pos) :
    m_GamePtr(gamePtr),
    m_SizeX(gamePtr->Width().GetVal()),
    m_SizeY(gamePtr->Height().GetVal())
{
    GenerateShortestPaths(start_pos);
}

bool BfsHelper::IsEmpty(const std::pair<int, int>& pos) const
{
    char c=m_GamePtr->Board().GetVal()[Index(pos)];
    return c=='.' || c=='$' || c=='p' || c == 's' || c == 'l' || c == 'r';
}

void BfsHelper::GenerateShortestPaths(std::pair<int, int> start_pos)
{
    // Shortest path finder by BFS
    for (int y = 0; y < m_SizeY; y++) {
        for (int x = 0; x < m_SizeX; x++) {
            m_GamePaths[x][y] = 200000;
        }
    }   

    m_GamePaths[start_pos.first][start_pos.second] = 0; // Current coords
    EvaluateShortestPaths(start_pos, 1);
}

void BfsHelper::EvaluateShortestPathsHelper(std::pair<int, int> current_pos, std::pair<int, int> next_pos, int steps)
{
    if (IsEmpty(next_pos)) 
    {
        if (steps < m_GamePaths[next_pos.first][next_pos.second]) {
            m_GamePaths[next_pos.first][next_pos.second] = steps;
            EvaluateShortestPaths(next_pos, steps+1);                
        }
    }
}

void BfsHelper::EvaluateShortestPaths(std::pair<int, int> pos, int steps)
{
    EvaluateShortestPathsHelper(pos, MoveLeft(pos), steps);
    EvaluateShortestPathsHelper(pos, MoveRight(pos), steps);
    EvaluateShortestPathsHelper(pos, MoveUp(pos), steps);
    EvaluateShortestPathsHelper(pos, MoveDown(pos), steps);
}

std::pair<int, int> BfsHelper::BacktrackFromSquare(std::pair<int, int> target_pos)
{
    //std::cout << "Backtrack path: "  << x << y << std::endl;

    if (!CanReachSquare(target_pos)) {
        std::cerr << "ERROR: Attempted backtracking from unreachable square!" << std::endl;
        return std::make_pair(-1, -1);
    }

    // Steps to square
    int steps = StepsToSquare(target_pos);

    // one step - found the next square
    if (steps == 1) {
        return target_pos;
    }

    // Shortest path is one step less
    if (StepsToSquare(MoveLeft(target_pos)) == (steps-1)) {
        return BacktrackFromSquare(MoveLeft(target_pos));
    }

    if (StepsToSquare(MoveRight(target_pos)) == (steps-1)) {
        return BacktrackFromSquare(MoveRight(target_pos));
    }

    if (StepsToSquare(MoveUp(target_pos)) == (steps-1)) {
        return BacktrackFromSquare(MoveUp(target_pos));
    }

    if (StepsToSquare(MoveDown(target_pos)) == (steps-1)) {
        return BacktrackFromSquare(MoveDown(target_pos));
    }

    // How did this happend??
    return std::make_pair(-1, -1);
}

Consoden::TankGame::Direction::Enumeration BfsHelper::FindDirection(std::pair<int, int> start_pos, std::pair<int, int> target_pos) 
{
    if (MoveLeft(start_pos) == target_pos) {
        return Consoden::TankGame::Direction::Left;
    } else if (MoveRight(start_pos) == target_pos) {
        return Consoden::TankGame::Direction::Right;
    } else if (MoveUp(start_pos) == target_pos) {
        return Consoden::TankGame::Direction::Up;
    } else if (MoveDown(start_pos) == target_pos) {
        return Consoden::TankGame::Direction::Down;
    }

    std::cerr << "ERROR: FindDirection was called with a pair of squares which are not adjacent!" << std::endl;
    return Consoden::TankGame::Direction::Neutral;
}

bool BfsHelper::CanReachSquare(std::pair<int, int> pos) {
    if (pos.first >= m_SizeX || pos.first < 0 || pos.second >= m_SizeY || pos.second < 0) {
        return false;
    }

    return (m_GamePaths[pos.first][pos.second] < 200000);
}

int BfsHelper::StepsToSquare(std::pair<int, int> pos) {
    if (pos.first >= m_SizeX || pos.first < 0 || pos.second >= m_SizeY || pos.second < 0)
        return 200000;

    return m_GamePaths[pos.first][pos.second];
}

void BfsHelper::PrintMap() const
{
    for (int y = 0; y < m_SizeY; y++) {
        for (int x = 0; x < m_SizeX; x++) {
            std::cout << m_GamePtr->Board().GetVal()[Index(x, y)];
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

std::pair<int, int> BfsHelper::MoveLeft(const std::pair<int, int>& pos) const
{
    int next_x = (pos.first - 1 + m_SizeX) % m_SizeX;
    int next_y = pos.second;

    return std::make_pair(next_x, next_y);
}

std::pair<int, int> BfsHelper::MoveRight(const std::pair<int, int>& pos) const
{
    int next_x = (pos.first + 1) % m_SizeX;
    int next_y = pos.second;

    return std::make_pair(next_x, next_y);
}

std::pair<int, int> BfsHelper::MoveUp(const std::pair<int, int>& pos) const
{
    int next_x = pos.first;
    int next_y = (pos.second - 1 + m_SizeY) % m_SizeY;

    return std::make_pair(next_x, next_y);
}

std::pair<int, int> BfsHelper::MoveDown(const std::pair<int, int>& pos) const
{
    int next_x = pos.first;
    int next_y = (pos.second + 1) % m_SizeY;

    return std::make_pair(next_x, next_y);
}
