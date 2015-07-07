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

#include "App.h"

#include <Safir/Logging/Log.h>

namespace TankKamikazePlayer
{
    App::App(const std::wstring& playerName) :
          m_dispatch(m_connection,m_ioService),
          mPlayerEntityHandler(m_ioService, playerName),
          mPlayerName(playerName)

    {
    }

    int App::Run()
    {
        m_connection.Open(L"TankKamikazePlayer", mPlayerName, 0, this, &m_dispatch);

        mPlayerEntityHandler.Init();

        Safir::Logging::SendSystemLog(Safir::Logging::Critical,
                                      L"Started TankKamikazePlayer!");

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
