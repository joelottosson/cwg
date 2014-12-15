/******************************************************************************
*
* Copyright Consoden AB, 2014
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#include <fstream>
#include <boost/filesystem.hpp>
#include <Consoden/TankGame/Boards.h>
#include "BoardHandler.h"

BoardHandler::BoardHandler(Safir::Dob::Connection& con)
    :m_connection(con)
    ,m_defaultHandler()
    ,m_instance(Safir::Dob::Typesystem::InstanceId::GenerateRandom())
{
    srand(time(NULL));
}

void BoardHandler::Init()
{
    m_connection.RegisterEntityHandler(Consoden::TankGame::Boards::ClassTypeId, m_defaultHandler, Safir::Dob::InstanceIdPolicy::HandlerDecidesInstanceId, this);
    Refresh();
}

//-------------------------------------
// EntityHandler interface
//-------------------------------------
void BoardHandler::OnCreateRequest(const Safir::Dob::EntityRequestProxy entityRequestProxy, Safir::Dob::ResponseSenderPtr responseSender)
{
    auto error=Safir::Dob::ErrorResponse::CreateErrorResponse(Safir::Dob::ResponseGeneralErrorCodes::SafirNoPermission(), L"");
    responseSender->Send(error);
}

void BoardHandler::OnUpdateRequest(const Safir::Dob::EntityRequestProxy entityRequestProxy, Safir::Dob::ResponseSenderPtr responseSender)
{
    auto ok=Safir::Dob::SuccessResponse::Create();
    responseSender->Send(ok);
    Refresh();
}

void BoardHandler::OnDeleteRequest(const Safir::Dob::EntityRequestProxy entityRequestProxy, Safir::Dob::ResponseSenderPtr responseSender)
{
    auto error=Safir::Dob::ErrorResponse::CreateErrorResponse(Safir::Dob::ResponseGeneralErrorCodes::SafirNoPermission(), L"");
    responseSender->Send(error);
}

void BoardHandler::OnRevokedRegistration(const Safir::Dob::Typesystem::TypeId /*typeId*/, const Safir::Dob::Typesystem::HandlerId &/*handlerId*/)
{
    std::wcout<<L"MatchServer: Revoed registration of TankGame.Boards"<<std::endl;
}

//-------------------------------------

bool BoardHandler::FromFile(const std::string &file, int &xSize, int &ySize, std::vector<char> &board, Point &tank1, Point &tank2)
{
    if (file=="<generate_random>")
    {
        GenerateRandom(xSize, ySize, board, tank1, tank2);
        return true;
    }

    xSize=0;
    ySize=0;
    board.clear();
    tank1=Point();
    tank2=Point();
    int numberOfTanks=0;

    std::ifstream is;
    is.open(file.c_str());
    std::string line;
    while (std::getline(is, line))
    {
        if (line.empty())
        {
            break;
        }

        if (xSize==0)
        {
            xSize=line.size();
        }
        else if (static_cast<int>(line.size())!=xSize)
        {
            std::wcout<<L"All rows must have same size."<<std::endl;
            return false; //all rows must have same size
        }

        //check that line is correct
        for (size_t i=0; i<line.size(); ++i)
        {
            char c=line[i];
            if (c=='.' || c=='o' || c=='f' || c=='x')
            {
                continue;
            }
            else if (c=='t' && numberOfTanks<2)
            {
                Point& tankRef=numberOfTanks==0 ? tank1 : tank2;
                tankRef.x=i;
                tankRef.y=ySize;
                line[i]='.'; //remove tank
                ++numberOfTanks;
            }
            else
            {
                std::wcout<<L"Invalid character: "<<c<<L", numTanks: "<<numberOfTanks<<std::endl;
                return false;
            }
        }

        board.insert(board.end(), line.begin(), line.end());
        ++ySize;
    }

    is.close();

    if (xSize==0 || ySize==0 || numberOfTanks!=2)
    {
        std::wcout<<L"Invalid file xSize: "<<xSize<<L", ySize: "<<ySize<<L" numTanks: "<<numberOfTanks<<std::endl;
        return false;
    }

    assert(board.size()==static_cast<size_t>(xSize*ySize));

    return true;
}

void BoardHandler::GenerateRandom(int &xSize, int &ySize, std::vector<char> &board, Point &tank1, Point &tank2)
{
    xSize=10+rand()%5;
    ySize=10+rand()%3;
    size_t size=xSize*ySize;
    board.clear();
    board.resize(size, '.');
    int numWalls=size*static_cast<double>((rand()%50)/100.0); //at most 50% walls
    for (int i=0; i<numWalls; ++i)
    {
        board[rand()%size]='x';
    }

    int numFlags=size*static_cast<double>((rand()%25)/100.0); //at most 25% flags
    for (int i=0; i<numFlags; ++i)
    {
        board[rand()%size]='f';
    }

    // ensure that there always are two tanks
    for(;;)
    {
        int t1=rand()%size;
        int t2=rand()%size;
        if (t1==t2)
        {
            continue; //same start positions, generate new
        }

        //set tank start positions to be empty squares
        board[static_cast<size_t>(t1)]='.';
        board[static_cast<size_t>(t2)]='.';
        tank1=Point(t1%xSize, t1/xSize);
        tank2=Point(t2%xSize, t2/xSize);
        break;
    }
}

bool BoardHandler::ValidFile(const std::string &file) const
{
    int x, y;
    std::vector<char> board;
    Point tank1;
    Point tank2;
    return FromFile(file, x, y, board, tank1, tank2);
}

void BoardHandler::Refresh()
{
    using namespace boost::filesystem;

    Consoden::TankGame::BoardsPtr boards=Consoden::TankGame::Boards::Create();
    int index=0;
    boards->BoardList()[index++].SetVal(L"<generate_random>");

    recursive_directory_iterator it(Safir::Dob::Typesystem::Utilities::ToUtf8(Consoden::TankGame::Boards::Path()));
    recursive_directory_iterator end;
    while (it != end)
    {
        if (index>=Consoden::TankGame::Boards::BoardListArraySize())
        {
            std::wcout<<L"Too many board files. Can only show "<<Consoden::TankGame::Boards::BoardListArraySize()<<std::endl;
            break;
        }

        if (boost::filesystem::is_regular_file(*it) && it->path().extension()==".txt")
        {
            if (ValidFile(it->path().string()))
            {
                boards->BoardList()[index++].SetVal(Safir::Dob::Typesystem::Utilities::ToWstring(it->path().string()));
            }
            else
            {
                std::wcout<<L"Invalid board file: "<<it->path().string().c_str()<<std::endl;
            }
        }

        ++it;
    }

    m_connection.SetAll(boards, m_instance, m_defaultHandler);
}
