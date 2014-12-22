/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Björn Weström / bjws
*
*******************************************************************************/

#include "App.h"

#include <Safir/Logging/Log.h>

#include <iostream>

namespace TankEngine
{
    App::App()
        : m_dispatch(m_connection,m_ioService),
          entityHandler(m_ioService)
    {
    }

    int App::Run()
    {
        m_connection.Open(L"TankEngine", L"0", 0, this, &m_dispatch);

        entityHandler.Init();

        std::cout << "Started tank_engine!" << std::endl;
        Safir::Logging::SendSystemLog(Safir::Logging::Notice,
                                      L"Started TankEngine!");

        // Start the asio io-service loop in order to receive DOB callbacks
        // for example OnCreateRequest in EntityOwner.
        // We also need to define some dummy work in order for the io_service
        // to keep running until we tell it to stop.
        boost::asio::io_service::work keepRunning(m_ioService);
        m_ioService.run();
        return 0;
    }

    void App::OnStopOrder()
    {
        m_ioService.stop();
    }
}
