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
#include <boost/date_time/posix_time/posix_time.hpp>

namespace TankEngine
{
    /** 
     * Defines a vehicle owner. This class handles the
     * registration as a data owner and processes request
     * on that data.
     */
    class GameMap 
    {
    public:

        GameMap(Consoden::TankGame::GameStatePtr game_ptr);

        bool EmptySquare(int pos_x, int pos_y);

        bool WallSquare(int pos_x, int pos_y);

        bool OnBoard(int pos_x, int pos_y);

        bool MineSquare(int pos_x, int pos_y);

        bool FlagSquare(int pos_x, int pos_y);

        void TakeFlag(int pos_x, int pos_y);

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
