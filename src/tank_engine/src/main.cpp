/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Björn Weström / bjws
*
*******************************************************************************/

#include "App.h"
#include <Safir/Dob/Typesystem/Exceptions.h>
#include <Safir/Logging/Log.h>
#include <Safir/Application/CrashReporter.h>

int main(int /*argc*/, char* /*argv*/[])
{
    Safir::Application::ScopedCrashReporter crashReporter;

    try
    {
        TankEngine::App app;
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
                    L" in TankEngine");
        return 1;
   }
   catch (...)
   {
        Safir::Logging::SendSystemLog(
                    Safir::Logging::Critical,
                    L"catch(...) exception in TankEngine");
       return 1; 
   }
}
