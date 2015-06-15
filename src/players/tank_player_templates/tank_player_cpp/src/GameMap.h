/******************************************************************************
*
* Copyright Consoden AB, 2015
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

    /**
     * Constructor, creates a new GameMap from a GameState.
     */
    GameMap(int tankId, const Consoden::TankGame::GameStatePtr& gamePtr);

    /**
     * Get the x and y size of the game board.
     */
    int SizeX() const {return m_sizeX;}
    int SizeY() const {return m_sizeY;}

    /**
     * Location of the players tank.
     */
    std::pair<int, int> OwnPosition() const {return m_ownPos;}

    /**
     * Location of the enemys tank.
     */
    std::pair<int, int> EnemyPosition() const {return m_enemyPos;}

    /**
     * Check if square is a wall.
     */
    bool IsWall(const std::pair<int, int>& pos) const;

    /**
     * Check if there is a mine in this square.
     */
    bool IsMine(const std::pair<int, int>& pos) const;

    /**
     * Check if there is a coin in this square.
     */
    bool IsCoin(const std::pair<int, int>& pos) const;

    /**
     * Check if there is poison gas in this square.
     */
    bool IsPoisonGas(const std::pair<int, int>& pos) const;

    /**
     * Is there a missile in this position right now?
     */
    bool IsMissileInPosition(const std::pair<int, int>& pos) const;

    /**
     * Helper functions that moves pos one step in the indicated direction.
     * Returns a new positon after the move operation.
     */
    std::pair<int, int> Move(const std::pair<int, int>& pos,
                             Consoden::TankGame::Direction::Enumeration direction) const;

    /**
     * Time left (milliseconds) until the joystick will be readout next time.
     */
    int TimeToNextMove() const;

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
    Consoden::TankGame::GameStatePtr m_gamePtr;
    int m_sizeX;
    int m_sizeY;
    // Current positions of player and enemy
    std::pair<int, int> m_ownPos;
    std::pair<int, int> m_enemyPos;

    inline int Index(int x, int y) const {return x+y*m_sizeX;}
    inline int Index(const std::pair<int, int>& pos) const {return Index(pos.first, pos.second);}
};

#endif
