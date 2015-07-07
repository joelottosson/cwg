/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#include <iostream>
#include <locale>

#include "Player.h"

int main(int argc, const char** argv)
{
    setlocale(LC_ALL, "en_US.UTF-8");

    auto playerName=TankLogic::PlayerName;

    if (argc>1)
    {
        playerName=Safir::Dob::Typesystem::Utilities::ToWstring(argv[1]);
    }

    std::cout << "Starting player with name " << Safir::Dob::Typesystem::Utilities::ToUtf8(playerName) << std::endl;

    //start running the program
    boost::asio::io_service ioService;
    Player player(playerName, ioService);
    ioService.run();

    return 0;
}

