/******************************************************************************
*
* Copyright Consoden AB, 2013-2014 (http://www.consoden.se)
*
* Created by: Björn Weström / bjws
*
*******************************************************************************
*
* This file is part of Consodens Programming Challenge 2014.
* This is the player template used by the competing teams.
* It is based on the Safir SDK Core framework. (http://safir.sourceforge.net)
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

#ifndef __APP_H
#define __APP_H

#include <Safir/Utilities/AsioDispatcher.h>
#include <Safir/Dob/Connection.h>

#include "PlayerEntityHandler.h"

namespace TankTeamPlayer
{
    /** 
    * This application is an example of how to use DOB producer mechanisms. 
    * The code is written to show the possibilities of the interfaces 
    * rather than showing a real world example. 
    */ 


    /** 
     * Main class. Controls startup, closedown and receives events.
     */
    class App :
        public Safir::Dob::StopHandler
    {
    public:

        App(const std::wstring& playerName);

        /** 
        * Startup application: open dob connection, register entity ownership
        * and service provider, attach message sender and start event loop.
        */
        int Run();

        /** 
         * Called by the stop handler.
         */
        void OnStopOrder();

    private:
        // Primary connection for Dob calls
        Safir::Dob::Connection m_Connection;
   
        // This application is event driven. It's triggered by events 
        // received in the main event loop. 
        // The event loop is started in the Run method.
        //
        // The Dispatcher class makes a thread switch from the calling 
        // Dob thread to this applications main thread. It performs a dispatch
        // on the Dob connection that will result in callbacks to all overidden 
        // Dob interface methods, for example OnCreateRequest call in EntityOwner.
        boost::asio::io_service m_IoService;
        Safir::Utilities::AsioDispatcher   m_Dispatch;
        
        // DOB object handlers.
        PlayerEntityHandler m_PlayerEntityHandler;

        std::wstring m_PlayerName;

    };
}
#endif
