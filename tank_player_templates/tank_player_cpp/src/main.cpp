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
    std::cout << "Stargin player with name.. " << argv[1] << std::endl;
    std::wcout << "Starting player with name " << playerName << std::endl;

    int len = 0;
    for (const char *c = argv[1]; *c; c++, len++) {
        printf("%x ", (*c));
        fflush(stdout);
    }

    if (argc>1)
    {
        playerName=Safir::Dob::Typesystem::Utilities::ToWstring(argv[1]);
    }

    std::wcout << "Starting player with name " << playerName << std::endl;

    //start running the program
    boost::asio::io_service ioService;
    Player player(playerName, ioService);
    ioService.run();

    return 0;
}

