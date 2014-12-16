/******************************************************************************
*
* Copyright Consoden AB, 2014
*
* Created by: Björn Weström / bjwe
*
*******************************************************************************/

#ifndef __GAMEMAP_H
#define __GAMEMAP_H

#include <vector>
#include <map>
#include <boost/chrono.hpp>
#include <Consoden/TankGame/GameState.h>

/**
 * Helper class for the Strategy. Parses game board and locates the
 * player and enemy tanks.
 */
class GameMap
{
public:

    GameMap(int tankId, const Consoden::TankGame::GameStatePtr& gamePtr);

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
     * Check if there is a flag in this square.
     */
    bool IsFlag(const std::pair<int, int>& pos) const;

    /**
     * Location of the players tank
     */
    std::pair<int, int> OwnPosition() const {return m_OwnPos; }

    /**
     * Location of the enemys tank
     */
    std::pair<int, int> EnemyPosition() const {return m_EnemyPos; }

    /**
     * Is there a missile in this position right now?
     */
    bool IsMissileInPosition(const std::pair<int, int>& pos) const;

    /**
     * Helper functions that moves one step in the indicated direction
     */
    std::pair<int, int> MoveLeft(const std::pair<int, int>& pos) const;
    std::pair<int, int> MoveRight(const std::pair<int, int>& pos) const;
    std::pair<int, int> MoveUp(const std::pair<int, int>& pos) const;
    std::pair<int, int> MoveDown(const std::pair<int, int>& pos) const;

    /**
     * Returns number of milliseconds elapsed since this GameMap was created.
     */
    unsigned int Elapsed() const;

    /**
     * For debugging assistance, prints out the parsed map
     */
    void PrintMap() const;

    /**
     * For debugging assistance, prints out the current state
     */
    void PrintState() const;

private:
    int m_TankId; // The tank id for the player
    Consoden::TankGame::GameStatePtr m_GamePtr;
    int m_SizeX;
    int m_SizeY;
    // Current positions of player and enemy
    std::pair<int, int> m_OwnPos;
    std::pair<int, int> m_EnemyPos;
    boost::chrono::high_resolution_clock::time_point m_creationTime;

    inline int Index(int x, int y) const {return x+y*m_SizeX;}
    inline int Index(const std::pair<int, int>& pos) const {return Index(pos.first, pos.second);}
};

#endif
