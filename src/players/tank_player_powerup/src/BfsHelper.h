/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Björn Weström / bjwe
*
*******************************************************************************/

#ifndef __BFSHELPER_H
#define __BFSHELPER_H

#include <vector>
#include <map>
#include <Consoden/TankGame/GameState.h>

/**
  *Implementation of a breadth first algorithm.
 * Helper class that parses game board and calculates the shortest paths to each square.
 */
class BfsHelper
{
public:

    /**
     * Create a bfs graph based on startPos.
     */
    BfsHelper(const Consoden::TankGame::GameStatePtr& gamePtr, std::pair<int, int> start_pos);

    /**
     * Is it possible to reach the square pos?
     */
    bool CanReachSquare(std::pair<int, int> pos);

    /**
     * How many steps are required to reach square pos?
     */
    int StepsToSquare(std::pair<int, int> pos);

    /**
     * Calculates the next step towards the target along the shortest path. 
     * Note that there may be multiple possible shortest paths, this functions returns only one of them.
     */
    std::pair<int, int> BacktrackFromSquare(std::pair<int, int> target_pos);

    /**
     * Calculates the direction of movement from start_pos to target_pos.
     * start_pos and target_pos must be adjacent squares.
     */
    Consoden::TankGame::Direction::Enumeration FindDirection(std::pair<int, int> start_pos, std::pair<int, int> target_pos);

    /**
     * For debugging assistance, prints out the parsed map
     */
    void PrintMap() const;

    bool CanWrapX();
    bool CanWrapY();

private:
    /**
     * Get the x and y size of the game board.
     */
    int SizeX() const { return m_SizeX; }
    int SizeY() const { return m_SizeY; }

    /**
     * Is this square empty? Only checks for walls and mines.
     * Flags and missiles are ignored.
     */
    bool IsEmpty(const std::pair<int, int>& pos) const;

    /**
     * Helper functions that moves one step in the indicated direction
     */
    std::pair<int, int> MoveLeft(const std::pair<int, int>& pos) const;
    std::pair<int, int> MoveRight(const std::pair<int, int>& pos) const;
    std::pair<int, int> MoveUp(const std::pair<int, int>& pos) const;
    std::pair<int, int> MoveDown(const std::pair<int, int>& pos) const;

    void GenerateShortestPaths(std::pair<int, int> start_pos);
    void EvaluateShortestPathsHelper(std::pair<int, int> current_pos, std::pair<int, int> next_pos, int steps);
    void EvaluateShortestPaths(std::pair<int, int> pos, int steps);


    Consoden::TankGame::GameStatePtr m_GamePtr;
    int m_SizeX;
    int m_SizeY;
    // Shortest paths from a given position
    std::map<int, std::map<int, int> > m_GamePaths;

    inline int Index(int x, int y) const {return x+y*m_SizeX;}
    inline int Index(const std::pair<int, int>& pos) const {return Index(pos.first, pos.second);}
};

#endif
