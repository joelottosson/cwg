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
#include "App.h"

#include <Safir/Logging/Log.h>

namespace TankTeamPlayer
{
    App::App(const std::wstring& playerName) :
          m_Dispatch(m_Connection, m_IoService),
          m_PlayerEntityHandler(playerName),
          m_PlayerName(playerName)

    {
    }

    int App::Run()
    {
        m_Connection.Open(L"TankTeamPlayer", m_PlayerName, 0, this, &m_Dispatch);

        m_PlayerEntityHandler.Init();

        Safir::Logging::SendSystemLog(Safir::Logging::Critical,
                                      L"Started TankTeamPlayer!");

        // Start the asio io-service loop in order to receive DOB callbacks
        // for example OnCreateRequest in EntityOwner.
        // We also need to define some dummy work in order for the io_service
        // to keep running until we tell it to stop.
        boost::asio::io_service::work keepRunning(m_IoService);
        m_IoService.run();
        return 0;
    }

    void App::OnStopOrder()
    {
        m_IoService.stop();
    }
}
