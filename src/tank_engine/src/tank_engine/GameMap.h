/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Björn Weström / bjws
*
*******************************************************************************/

#ifndef __GAMEMAP_H
#define __GAMEMAP_H

#include <vector>
#include <Consoden/TankGame/GameState.h>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace TankEngine
{
    class GameMap 
    {
    public:

        GameMap(Consoden::TankGame::GameStatePtr game_ptr);

        bool EmptySquare(int pos_x, int pos_y);

        bool WallSquare(int pos_x, int pos_y);

        bool OnBoard(int pos_x, int pos_y);

        bool MineSquare(int pos_x, int pos_y);

        bool CoinSquare(int pos_x, int pos_y);

        bool PoisonSquare(int pos_x, int pos_y);

        bool LaserAmmo(int pos_x, int pos_y);

        void ClearSquare(int pos_x, int pos_y);

        //TODO: Crap added by me
        bool DudeSquare(int pos_x, int pos_y);

        bool IsTankHit(int pos_x, int pos_y);

        void AddMine(int pos_x, int pos_y);

        bool TankMoveAgainstMissile(int pos_x, int pos_y, Consoden::TankGame::Direction::Enumeration move_direction);

        void MoveMissiles();

        bool MissilesLeft();

        bool FireMissile(int pos_head_x, int pos_head_y, int pos_tail_x, int pos_tail_y, Consoden::TankGame::Direction::Enumeration direction, int tank_id);

        Consoden::TankGame::Direction::Enumeration InvertDirection(Consoden::TankGame::Direction::Enumeration d);
        
        void SetChanges();

        void Print();

        //-----------------------------------------------------------------------------
        // Gets the current time in milliseconds from midnight
        //-----------------------------------------------------------------------------
        static int TimerTimeout(boost::posix_time::time_duration time_left);


    private:

        Consoden::TankGame::GameStatePtr m_Game_ptr;
        std::map<int, std::map<int, char> > m_Game;
        static int m_missileCounter;

        int x_size;
        int y_size;
    };
};
#endif
