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

#include <Consoden/TankGame/GameState.h>

namespace TankEngine
{
    int GameMap::m_missileCounter = 0;

    GameMap::GameMap(Consoden::TankGame::GameStatePtr game_ptr) :
        m_Game_ptr(game_ptr)
    {
        x_size = m_Game_ptr->Width();
        y_size = m_Game_ptr->Height();

        for (int y = 0; y < y_size; y++) {
            for (int x = 0; x < x_size; x++) {
                int index = (y * x_size) + x;
                m_Game[x][y] = m_Game_ptr->Board().GetVal()[index];
            }
        }   
    }


    bool GameMap::EmptySquare(int pos_x, int pos_y)
    {
        return (m_Game[pos_x][pos_y] == '.');
    }

    bool GameMap::WallSquare(int pos_x, int pos_y)
    {
        return (m_Game[pos_x][pos_y] == 'x');
    }

    bool GameMap::OnBoard(int pos_x, int pos_y)
    {
        return (pos_x >= 0 && pos_x < x_size && pos_y >= 0 && pos_y < y_size);
    }

    bool GameMap::MineSquare(int pos_x, int pos_y)
    {
        return (m_Game[pos_x][pos_y] == 'o');
    }
 
    bool GameMap::IsTankHit(int pos_x, int pos_y)
    {
        bool tank_is_hit = false;

        for (Safir::Dob::Typesystem::ArrayIndex missile_index = 0; 
             missile_index < m_Game_ptr->MissilesArraySize(); 
             missile_index++) {

            if (m_Game_ptr->Missiles()[missile_index].IsNull()) {
                // empty slot
                continue;
            }

            Consoden::TankGame::MissilePtr missile_ptr = 
                boost::static_pointer_cast<Consoden::TankGame::Missile>(m_Game_ptr->Missiles()[missile_index].GetPtr());

            if ((missile_ptr->HeadPosX() == pos_x && missile_ptr->HeadPosY() == pos_y) ||
                (missile_ptr->TailPosX() == pos_x && missile_ptr->TailPosY() == pos_y)) {
                // Hit by missile
                missile_ptr->InFlames() = true;
                tank_is_hit = true;
            }
        }

        return tank_is_hit;
    }

    void GameMap::AddMine(int pos_x, int pos_y)
    {
        m_Game[pos_x][pos_y] = 'o';
    }

    bool GameMap::FireMissile(int pos_head_x, int pos_head_y, int pos_tail_x, int pos_tail_y, Consoden::TankGame::Direction::Enumeration direction, int tank_id) 
    {
        Safir::Dob::Typesystem::ArrayIndex missile_index;
        bool tank_missile_active = false;
        int empty_index = -1;

        for (missile_index = 0; 
             missile_index < m_Game_ptr->MissilesArraySize(); 
             missile_index++) {

            if (m_Game_ptr->Missiles()[missile_index].IsNull()) {
                // Reached empty missile slot
                if (empty_index == -1) {
                    empty_index = missile_index;
                }
                continue;
            }

            Consoden::TankGame::MissilePtr missile_ptr = 
                boost::static_pointer_cast<Consoden::TankGame::Missile>(m_Game_ptr->Missiles()[missile_index].GetPtr());
            if (missile_ptr->TankId().GetVal() == tank_id) {
                tank_missile_active = true;
            }
        }

        if (tank_missile_active || empty_index == -1) {
            // Only one missile per tank can be active, or array is full
            return false;
        }

        Consoden::TankGame::MissilePtr missile_ptr = Consoden::TankGame::Missile::Create();
        missile_ptr->MissileId() = m_missileCounter++;
        missile_ptr->TankId() = tank_id;
        missile_ptr->Direction() = direction;
        missile_ptr->InFlames() = false;

        if (OnBoard(pos_tail_x, pos_tail_y) && WallSquare(pos_tail_x, pos_tail_y)) {
            // Missile totally into wall, set in flames
            pos_head_x = pos_tail_x;
            pos_head_y = pos_tail_y;
            missile_ptr->InFlames() = true;            
        } else {
            if (OnBoard(pos_head_x, pos_head_y) && WallSquare(pos_head_x, pos_head_y)) {
                // Head in wall, set in flames
                missile_ptr->InFlames() = true;
            } else {
                // Free path
            }
        }

        missile_ptr->HeadPosX() = pos_head_x;
        missile_ptr->HeadPosY() = pos_head_y;
        missile_ptr->TailPosX() = pos_tail_x;
        missile_ptr->TailPosY() = pos_tail_y;
        m_Game_ptr->Missiles()[empty_index].SetPtr(missile_ptr);
        return true;
    }

    bool GameMap::TankMoveAgainstMissile(int pos_x, int pos_y, Consoden::TankGame::Direction::Enumeration move_direction)
    {
        for (Safir::Dob::Typesystem::ArrayIndex missile_index = 0; 
             missile_index < m_Game_ptr->MissilesArraySize(); 
             missile_index++) {

            if (m_Game_ptr->Missiles()[missile_index].IsNull()) {
                // No missile in this slot
                continue;
            }

            Consoden::TankGame::MissilePtr missile_ptr = 
                boost::static_pointer_cast<Consoden::TankGame::Missile>(m_Game_ptr->Missiles()[missile_index].GetPtr());

            // To move against missile, the move direction must be opposite the missile direction
            if (move_direction == InvertDirection(missile_ptr->Direction().GetVal())) {
                // If we move against the missile, our pos will be in the moved missiles tail
                if (pos_x == missile_ptr->TailPosX().GetVal() && pos_y == missile_ptr->TailPosY().GetVal()) {
                    return true;
                }
            }
        }

        return false;
    }

    void GameMap::MoveMissiles()
    {
        for (Safir::Dob::Typesystem::ArrayIndex missile_index = 0; 
             missile_index < m_Game_ptr->MissilesArraySize(); 
             missile_index++) {

            if (m_Game_ptr->Missiles()[missile_index].IsNull()) {
                // No missile in this slot
                continue;
            }

            Consoden::TankGame::MissilePtr missile_ptr = 
                boost::static_pointer_cast<Consoden::TankGame::Missile>(m_Game_ptr->Missiles()[missile_index].GetPtr());

            if (missile_ptr->InFlames()) {
                // Missile burned up last round, remove it
                m_Game_ptr->Missiles()[missile_index].SetNull();
                continue;
            }
        
            int pos_tail_x = missile_ptr->TailPosX().GetVal();
            int pos_tail_y = missile_ptr->TailPosY().GetVal();
            int pos_head_x = missile_ptr->HeadPosX().GetVal();
            int pos_head_y = missile_ptr->HeadPosY().GetVal();

            // Moved position
            switch (missile_ptr->Direction()) {
                case Consoden::TankGame::Direction::Left:
                    pos_head_x = pos_head_x - 2;
                    pos_tail_x = pos_tail_x - 2;
                    break;

                case Consoden::TankGame::Direction::Right:
                    pos_head_x = pos_head_x + 2;
                    pos_tail_x = pos_tail_x + 2;
                    break;

                case Consoden::TankGame::Direction::Up:
                    pos_head_y = pos_head_y - 2;
                    pos_tail_y = pos_tail_y - 2;
                    break;

                case Consoden::TankGame::Direction::Down:
                    pos_head_y = pos_head_y + 2;
                    pos_tail_y = pos_tail_y + 2;
                    break;

                default:
                    break;

            }

            if (!OnBoard(pos_tail_x, pos_tail_y)) {
                // Moved completely off board, remove it
                m_Game_ptr->Missiles()[missile_index].SetNull();
                continue;
            }

            if (WallSquare(pos_tail_x, pos_tail_y)) {
                // Missile totally into wall, set in flames
                pos_head_x = pos_tail_x;
                pos_head_y = pos_tail_y;
                missile_ptr->InFlames() = true;
            } else {
                if (WallSquare(pos_head_x, pos_head_y)) {
                    // Head in wall, set in flames
                    missile_ptr->InFlames() = true;
                } else {
                    // Free path
                }
            }

            missile_ptr->HeadPosX() = pos_head_x;
            missile_ptr->HeadPosY() = pos_head_y;
            missile_ptr->TailPosX() = pos_tail_x;
            missile_ptr->TailPosY() = pos_tail_y;
        }
    }

    bool GameMap::MissilesLeft()
    {
        for (Safir::Dob::Typesystem::ArrayIndex missile_index = 0; 
             missile_index < m_Game_ptr->MissilesArraySize(); 
             missile_index++) {

            if (m_Game_ptr->Missiles()[missile_index].IsNull()) {
                // No missile in this slot
                continue;
            } else {
                // Still missiles left
                return true;
            }
        }
        return false;
    }

    Consoden::TankGame::Direction::Enumeration GameMap::InvertDirection(Consoden::TankGame::Direction::Enumeration d)
    {
        if (d == Consoden::TankGame::Direction::Left) {
            return Consoden::TankGame::Direction::Right;
        } else if (d == Consoden::TankGame::Direction::Right) {
            return Consoden::TankGame::Direction::Left;
        } else if (d == Consoden::TankGame::Direction::Up) {
            return Consoden::TankGame::Direction::Down;
        } 
        
        return Consoden::TankGame::Direction::Up;
        
    }

    void GameMap::SetChanges()
    {
        Safir::Dob::Typesystem::Binary serial_map = m_Game_ptr->Board();
        serial_map.resize(static_cast<size_t>(x_size*y_size));
        //const char* serial_map = new char[(x_size * y_size) + 1];

        for (int y = 0; y < y_size; y++) {
            for (int x = 0; x < x_size; x++) {
                int index = (y * x_size) + x;
                serial_map[index] = m_Game[x][y];
                //m_Game_ptr->Board()[index] = m_Game[x][y];
            }
        }

        m_Game_ptr->Board() = serial_map; //.SetVal(serial_map);
        //delete serial_map;           
    }


    void GameMap::Print()
    {
        for (int y = 0; y < y_size; y++) {
            for (int x = 0; x < x_size; x++) {
                //int index = (y * x_size) + x;
                std::cout << m_Game[x][y];
            }
            std::cout << std::endl;
        }        
    }
 };
