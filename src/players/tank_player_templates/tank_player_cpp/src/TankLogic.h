/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#ifndef __TANK_LOGIC_H__
#define __TANK_LOGIC_H__

#include <boost/function.hpp>
#include <Consoden/TankGame/GameState.h>

typedef boost::function<void(Consoden::TankGame::Direction::Enumeration,
                             Consoden::TankGame::Direction::Enumeration,
                             bool,bool,bool,bool,bool,int)> MoveJoystick;
class TankLogic
{
public:

    /**
     * Team or player name
     */
    static const std::wstring PlayerName;

    /**
     * TankLogic A new instance is created every time a new game is started.
     * @param tankId The tank id of our tank.
     * @param joystick Function object for joystick management.
     */
    TankLogic(int tankId, const MoveJoystick& joystick)
        :m_ownTankId(tankId)
        ,m_joystick(joystick) {}

    /**
     * MakeMove Called every time the game has changed and we are supposed to
     * calculate a new move.
     * This is the only method players really need to implement.
     * Before the function returns a call to SetJoystick should be made.
     * The method should return before a new GameState arrives (1 second)
     * @param gameState
     */
    void MakeMove(Consoden::TankGame::GameStatePtr gameState);

private:
    int m_ownTankId;
    MoveJoystick m_joystick;

    /**
     * Call this method to change the joystick state.
     * @param moveDirection The tanks movement direction.
     * @param towerDirection The tanks canon tower direction.
     * @param fire True if the tank wants to fire a missile, else false.
     */
    inline void SetJoystick(Consoden::TankGame::Direction::Enumeration moveDirection,
                            Consoden::TankGame::Direction::Enumeration towerDirection,
                            bool fire, bool dropMine)
    {
        m_joystick(moveDirection, towerDirection, fire, dropMine,false,false,false ,0);
    }


    /**
     * Call this method to change the joystick state.
     * @param moveDirection The tanks movement direction.
     * @param towerDirection The tanks canon tower direction.
     * @param fire True if the tank wants to fire a missile, else false.
     * @param fireLaser True if the tank should fire the laser. Has no effect if lasercount <= 0
     */
    inline void SetJoystick(Consoden::TankGame::Direction::Enumeration moveDirection,
                            Consoden::TankGame::Direction::Enumeration towerDirection,
                            bool fire, bool dropMine, bool fireLaser,bool deploySmoke
							,bool fireRedeemer, int redeemerTimer)
    {
        m_joystick(moveDirection, towerDirection, fire, dropMine, fireLaser,deploySmoke,fireRedeemer, redeemerTimer);
    }


    /**
     * Call this method to change the joystick state.
     * @param moveDirection The tanks movement direction.
     * @param towerDirection The tanks canon tower direction.
     * @param fire True if the tank wants to fire a missile, else false.
     * @param fireLaser True if the tank should fire the laser. Has no effect if lasercount <= 0
     */
    inline void SetJoystick(Consoden::TankGame::Direction::Enumeration moveDirection,
                            Consoden::TankGame::Direction::Enumeration towerDirection,
                            bool fire, bool dropMine, bool fireLaser)
    {
        m_joystick(moveDirection, towerDirection, fire, dropMine, fireLaser, false, false, 0);
    }

};

#endif
