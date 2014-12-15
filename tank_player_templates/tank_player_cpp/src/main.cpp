/******************************************************************************
*
* Copyright Consoden AB, 2014
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

    Player player(playerName);
    return 0;
}

