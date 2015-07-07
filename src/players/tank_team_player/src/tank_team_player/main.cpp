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

#include <Safir/Dob/Typesystem/Exceptions.h>
#include <Safir/Logging/Log.h>
#include <Safir/Application/CrashReporter.h>

#include "App.h"

int main(int argc, char** argv)
{
    Safir::Application::ScopedCrashReporter crashReporter;

    try
    {
        // Change this string to your team name!
        const char* playerName = "Default TeamPlayer";
        
        if (argc > 1) {
          playerName = argv[1];
        }

        std::wstring wPlayerName;
        for (unsigned int i = 0; i < strlen(playerName); i++) {
          wPlayerName += playerName[i];
        }

        TankTeamPlayer::App app(wPlayerName);
        app.Run();
        return 0;
    }
    catch (const std::exception & e)
    {
        std::string str(e.what());
        Safir::Logging::SendSystemLog(
                    Safir::Logging::Critical,
                    L"Unexpected exception " +
                    Safir::Dob::Typesystem::Utilities::ToWstring(str) +
                    L" in TankTeamPlayer");
        return 1;
   }
   catch (...)
   {
        Safir::Logging::SendSystemLog(
                    Safir::Logging::Critical,
                    L"catch(...) exception in TankTeamPlayer");
       return 1; 
   }
}
