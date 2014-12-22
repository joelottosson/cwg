/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#include <iostream>
#include "Player.h"

int main(int argc, const char** argv)
{
    //start running the program
    auto playerName=TankLogic::PlayerName;
    if (argc>1)
    {
        playerName=Safir::Dob::Typesystem::Utilities::ToWstring(argv[1]);
    }

    boost::asio::io_service ioService;
    Player player(playerName, ioService);
    ioService.run();

    return 0;
}

