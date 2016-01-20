/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#include <memory>
#include "MatchServer.h"

int main(int argc, char *argv[])
{
    boost::asio::io_service ioService;
    MatchServer server(ioService);
    ioService.run();
    return 0;
}
