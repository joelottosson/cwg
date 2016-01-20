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
#include <memory>
#include "GameMap.h"

#define TG Consoden::TankGame
#define Dir Consoden::TankGame::Direction

typedef boost::function<void(Consoden::TankGame::Direction::Enumeration,
                             Consoden::TankGame::Direction::Enumeration,
                             bool,
                             bool)> MoveJoystick;
class TankLogic
{
public:

    /**
     * Team or player name
     */
    static const std::wstring PlayerName;
    boost::shared_ptr<GameMap> m_gm;

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
     * @param fire True if tha tank wants to fire a missile, else false.
     */
    inline void SetJoystick(Consoden::TankGame::Direction::Enumeration moveDirection,
                            Consoden::TankGame::Direction::Enumeration towerDirection,
                            bool fire, bool dropMine)
    {
        m_joystick(moveDirection, towerDirection, fire, dropMine);
    }

    Consoden::TankGame::Direction::Enumeration isInSight();

    TG::Direction::Enumeration pickDirection(TG::Direction::Enumeration preffered_direction, TG::Direction::Enumeration ignore_direction,
    		bool conservative, bool priorize_coin);

    TG::Direction::Enumeration escape();
    std::pair<int,int> posFromOffs(int x,int y,std::pair<int,int> pos);

    bool aligned(TG::Direction::Enumeration a,TG::Direction::Enumeration b);

    const char* direction2String(Dir::Enumeration);

};


#endif
