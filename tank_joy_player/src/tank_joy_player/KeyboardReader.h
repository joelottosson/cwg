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

#ifndef __KEYBOARD_READER_H
#define __KEYBOARD_READER_H

#include <boost/asio.hpp>
#include <termios.h>

namespace TankJoyPlayer
{
    class JoystickEntityHandler;


    /** 
     * Defines a message to be sent. This class sends
     * the message.
     */
    class KeyboardReader 
    {
    public:

        KeyboardReader(boost::asio::io_service& io_service);
        virtual ~KeyboardReader();

        void set_keypress();
        void reset_keypress();

        /** 
         * Initiates this class. Creates a secondary Dob
         * connection.
         */
        void Init(JoystickEntityHandler* control);

        void PrintUsage();

        void handle_read(const boost::system::error_code& error, std::size_t length);

        /** 
         * Methods derived from Safir::Dob::KeyboardReader.
         */
        //void OnNotMessageOverflow();

        /** 
         * Sends a message.
         */
         //void SendMaxNofVehicleMsg();

    private:

        bool ReadNextChar();

        bool fireState;

        char input[1];

        struct termios originalTermParam;

        //boost::asio::posix::stream_handle input_handle;
        boost::asio::posix::stream_descriptor input_stream;
        JoystickEntityHandler* control_ptr;
        // This class uses this secondary connection for Dob calls.
        //Safir::Dob::SecondaryConnection m_connection;
    };
};
#endif
