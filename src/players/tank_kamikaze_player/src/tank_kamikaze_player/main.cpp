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
#include <Safir/Dob/Typesystem/Exceptions.h>
#include <Safir/Logging/Log.h>
#include <Safir/Application/CrashReporter.h>

int main(int argc, char** argv)
{
    Safir::Application::ScopedCrashReporter crashReporter;

    try
    {
        const char* playerName = "Default KamikazePlayer";
        if (argc > 1) {
          playerName = argv[1];
        }

        std::wstring wPlayerName;
        for (unsigned int i = 0; i < strlen(playerName); i++) {
          wPlayerName += playerName[i];
        }

        TankKamikazePlayer::App app(wPlayerName);
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
                    L" in TankKamikazePlayer");
        return 1;
   }
   catch (...)
   {
        Safir::Logging::SendSystemLog(
                    Safir::Logging::Critical,
                    L"catch(...) exception in TankKamikazePlayer");
       return 1; 
   }
}
