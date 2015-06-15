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

#include "KeyboardReader.h"
#include "JoystickEntityHandler.h"

#include <Consoden/TankGame/Direction.h>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <Safir/Logging/Log.h>

#include <iostream>
using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

namespace TankJoyPlayer
{
    KeyboardReader::KeyboardReader(boost::asio::io_service& io_service) :
      //input_handle(io_service),
      input_stream(io_service, ::dup(STDIN_FILENO))
    {
        fireState = false;
    }

    KeyboardReader::~KeyboardReader() 
    {
        reset_keypress();
    }

    void KeyboardReader::set_keypress()
    {
        struct termios currentTermParam;

        tcgetattr( 0, &originalTermParam );
        memcpy( &currentTermParam, &originalTermParam, sizeof( struct termios ) );

        /*
         * Disable canonical mode, and set buffer size to 1 byte
         */
        currentTermParam.c_lflag       &= ~ICANON;
        currentTermParam.c_lflag       &= ~ECHO;
        currentTermParam.c_cc[ VTIME ]  = 255;
        currentTermParam.c_cc[VMIN]     = 1;

        tcsetattr( 0, TCSANOW, &currentTermParam );
    }

    void KeyboardReader::reset_keypress()
    {
        tcsetattr( 0, TCSANOW, &originalTermParam );
    }

    void KeyboardReader::Init(JoystickEntityHandler* control) 
    {
        control_ptr = control;

        // Make stdin unbuffered
        set_keypress();

        // Read a char of input.
        boost::asio::async_read(input_stream, 
            boost::asio::buffer(input), 
            boost::bind( &KeyboardReader::handle_read, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));

    }

    void KeyboardReader::PrintUsage()
    {
        cout << "Keyboard control for joystick: (not case sensitive)" << endl;
        cout << "A     - Move direction left" << endl;
        cout << "S     - Move direction right" << endl;
        cout << "W     - Move direction up" << endl;
        cout << "Z     - Move direction down" << endl;
        cout << "X     - Move direction neutral" << endl;
        cout << "H     - Aim direction left" << endl;
        cout << "J     - Aim direction right" << endl;
        cout << "U     - Aim direction up" << endl;
        cout << "N     - Aim direction down" << endl;
        cout << "F     - Toggle fire button status (on/off)" << endl;

        cout << "Q     - Quit" << endl;
    }

    void KeyboardReader::handle_read(const boost::system::error_code& error, std::size_t length)
    {
        if (length == 1) {
            bool done = ReadNextChar();

            if (!done) {
                boost::asio::async_read(input_stream, 
                    boost::asio::buffer(input), 
                    boost::bind( &KeyboardReader::handle_read, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
            }

        } else {
            Safir::Logging::SendSystemLog(Safir::Logging::Critical,
                                          L"Keyboard reader error! ");
        }
    }

    bool KeyboardReader::ReadNextChar()
    {
        char c = input[0];
        //cin >> c;

        switch (c) 
        {
            case 'A':
            case 'a':
                cout << "Move direction left!" << endl;
                control_ptr->MoveDirection(Consoden::TankGame::Direction::Left);
                break;

            case 'S':
            case 's':
                cout << "Move direction right!" << endl;
                control_ptr->MoveDirection(Consoden::TankGame::Direction::Right);
                break;

            case 'W':
            case 'w':
                cout << "Move direction up!" << endl;
                control_ptr->MoveDirection(Consoden::TankGame::Direction::Up);
                break;

            case 'Z':
            case 'z':
                cout << "Move direction down!" << endl;
                control_ptr->MoveDirection(Consoden::TankGame::Direction::Down);
                break;

            case 'X':
            case 'x':
                cout << "Move direction neutral!" << endl;
                control_ptr->MoveNeutral();
                break;

            case 'H':
            case 'h':
                cout << "Aim direction left!" << endl;
                control_ptr->TowerDirection(Consoden::TankGame::Direction::Left);
                break;

            case 'J':
            case 'j':
                cout << "Aim direction right!" << endl;
                control_ptr->TowerDirection(Consoden::TankGame::Direction::Right);
                break;

            case 'U':
            case 'u':
                cout << "Aim direction up!" << endl;
                control_ptr->TowerDirection(Consoden::TankGame::Direction::Up);
                break;

            case 'N':
            case 'n':
                cout << "Aim direction down!" << endl;
                control_ptr->TowerDirection(Consoden::TankGame::Direction::Down);
                break;

            case 'F':
            case 'f':
                if (fireState) {
                    cout << "Fire button off!" << endl;
                    fireState = false;
                } else {
                    cout << "Fire button on!" << endl;
                    fireState = true;                    
                }
                control_ptr->Fire(fireState);
                break;

            case 'Q':
            case 'q':
                cout << "Keyboard reader stopped!" << endl;
                return true;
                break;

            default:
                cout << "unknown input, ignored." << endl;
                break;
        }

        return false;
    }

};
