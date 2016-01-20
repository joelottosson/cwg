/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Björn Weström / bjws
*
*******************************************************************************/

#ifndef __APP_H
#define __APP_H

#include "EntityHandler.h"
#include <Safir/Dob/Connection.h>
#include <Safir/Utilities/AsioDispatcher.h>

namespace TankEngine
{
    /** 
     * Main class. Controls startup, closedown and receives events.
     */
    class App :
        // Allows this class to receive a stop order.
        public Safir::Dob::StopHandler
    {
    public:

        App();

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
        Safir::Dob::Connection m_connection;

        // The Dispatcher class makes a thread switch from the calling 
        // Dob thread to this applications main thread. It performs a dispatch
        // on the Dob connection that will result in callbacks to all overidden 
        // Dob interface methods, for example OnCreateRequest call in EntityOwner.
        boost::asio::io_service m_ioService;
        Safir::Utilities::AsioDispatcher   m_dispatch;
        
        // DOB object handlers.
        EntityHandler entityHandler;

    };
}
#endif
