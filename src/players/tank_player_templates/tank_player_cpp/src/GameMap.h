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
     * Returns the amount of lasers the tank has
     */
    int LaserAmmoCount() const;

    /**
     * Location of the players tank.
     */
    std::pair<int, int> OwnPosition() const;

    /**
     * Location of the enemys tank.
     */
    std::pair<int, int> EnemyPosition() const;

    /**
     * checks if you have a smoke grenade
     */
    bool HasSmoke() const;

    /**
     * checks if you have a redeemer
     */
    bool HasRedeemer() const;


    /**
     * Check if square is a wall.
     */
    bool IsWall(const std::pair<int, int>& pos) const;

    /**
     * Check if there is a mine in this square.
     */
    bool IsMine(const std::pair<int, int>& pos) const;


    /**
     * Check if there is laser ammo in this square.
     */
    bool IsLaserAmmo(const std::pair<int, int>& pos) const;


    /**
	 * Check if there is redeemer ammo in this square.
	 */
	bool IsRedeemerAmmo(const std::pair<int, int>& pos) const;

    /**
     * Check if there is a smoke_grenade
     */
    bool IsSmokeGrenade(const std::pair<int, int>& pos) const;

    /**
     * Check if there is a coin in this square.
     */
    bool IsCoin(const std::pair<int, int>& pos) const;

    /**
     * Check if there is poison gas in this square.
     */
    bool IsPoisonGas(const std::pair<int, int>& pos) const;

    /**
	 * Check if there is a penguin.
	 */
	bool IsPenguin(const std::pair<int, int>& pos) const;


    /**
     * Is there a missile in this position right now?
     */
    bool IsMissileInPosition(const std::pair<int, int>& pos) const;

    /**
     * Is there a redeemer in this position right now?
     */
    bool IsRedeemerInPosition(const std::pair<int, int>& pos) const;

    /**
     * Helper functions that moves pos one step in the indicated direction.
     * Returns a new positon after the move operation.
     */
    std::pair<int, int> Move(const std::pair<int, int>& pos,
                             Consoden::TankGame::Direction::Enumeration direction) const;

    /**
     * Returns an objet with information about the missile. If enemy_tank is set to false
     * the missile of your own player will be returned otherwise its the enemys missile.
     *
     * If no missile exists the function will still return a missile object but it will be NULL.
     * To get the other field you need to use getter methods wich are just the name of the field you want to get.
     *
     * Se the game documentation for details about the different fields.
     *
     * example:
     *	Consoden::TankGame::MissilePtr missile = gm.GetMissile(true);
     *	if(missile != NULL){
     *		std::wcout<< "Missile head is at position " << missile->HeadPosX() << "," << missile->HeadPosY() << std::endl;
     *	}
     *
     */
    Consoden::TankGame::MissilePtr GetMissile(bool enemy_tank);

    /**
     * Functionality identical to GetMissile
     */
    Consoden::TankGame::RedeemerPtr GetRedeemer(bool enemy_tank);

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
