/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#include <fstream>
#include <stdio.h>
#include <time.h>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
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
    namespace fs=boost::filesystem;
    auto base=Safir::Dob::Typesystem::Utilities::ToUtf8(Consoden::TankGame::Boards::Path());
    auto randomDir=fs::path(base)/fs::path("random");
    if (!fs::exists(randomDir))
    {
        boost::filesystem::create_directories(randomDir);
    }

    //boost::filesystem::remove_all()
    fs::directory_iterator it(randomDir), end;
    while (it!=end)
    {
        fs::remove_all(it->path());
        ++it;
    }

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

bool BoardHandler::FromFile(const std::string &file, int &xSize, int &ySize, std::vector<char> &board, Point &tank1, Point &tank2, Point &dude)
{
    xSize=0;
    ySize=0;
    board.clear();
    tank1 = Point();
    tank2 = Point();
    dude = Point();
    int numberOfTanks=0;
    bool hasDude = false;

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
            if (c=='.' || c=='o' || c=='$' || c=='p' || c=='x' || c == 'l' || c == 's' || c == 'r')
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
            }else if(c=='d' && !hasDude){
            	hasDude = true;
            	dude.x = i;
            	dude.y = ySize;
            }
            else
            {
                std::wcout<<L"Invalid character: "<<c<<L", numTanks: "<<numberOfTanks<< " has dude ? " << hasDude << std::endl;
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

std::string BoardHandler::GenerateRandomFile()
{
    std::ofstream os;
    std::string fileName=CreateFileName();
    os.open(fileName);

    int xSize=10+rand()%5;
    int ySize=10+rand()%3;
    size_t size=xSize*ySize;
    std::vector<char> board(size, '.');

    int numWalls=size*static_cast<double>((rand()%50)/100.0); //at most 50% walls
    for (int i=0; i<numWalls; ++i)
    {
        board[rand()%size]='x';
    }

    int numCoins=size*static_cast<double>((rand()%25)/100.0); //at most 25% coins
    for (int i=0; i<numCoins; ++i)
    {
        board[rand()%size]='$';
    }

    int numPoison=size*static_cast<double>((rand()%15)/100.0); //at most 15% poison
    for (int i=0; i<numPoison; ++i)
    {
        board[rand()%size]='p';
    }

    for(;;)
    {
        int t1=rand()%size;
        int t2=rand()%size;
        if (t1==t2)
        {
            continue; //same start positions, generate new
        }
        else
        {
            board[static_cast<size_t>(t1)]='t';
            board[static_cast<size_t>(t2)]='t';
            break;
        }
    }



    while(true){
    	int dude_start = rand()%size;
    	if(board[dude_start] != 't'){
    		board[dude_start] = 'd';
    		break;
    	}
    }

    for(int i = 0 ; i < 3 ; i++){
		int dude_start = rand()%size;
		if(board[dude_start] != 't' && board[dude_start] != 'd'){
			board[dude_start] = 'l';
			break;
		}
	}

    for(int i = 0 ; i < 3 ; i++){
		int dude_start = rand()%size;
		if(board[dude_start] != 't' && board[dude_start] != 'd' && board[dude_start] != 'l'){
			board[dude_start] = 's';
			break;
		}
	}

    for(int i = 0 ; i < 3 ; i++){
        	int dude_start = rand()%size;
        	if(board[dude_start] != 't' && board[dude_start] != 'd' && board[dude_start] != 'l' && board[dude_start] != 's'){
        		board[dude_start] = 'r';
        		break;
        	}
        }

    size_t index=0;
    for (int y=0; y<ySize; ++y)
    {
        for (int x=0; x<xSize; ++x)
        {
            os<<board[index++];
        }
        os<<std::endl;
    }

    os.flush();
    os.close();

    return fileName;
}

bool BoardHandler::ValidFile(const std::string &file) const
{
    int x, y;
    std::vector<char> board;
    Point tank1;
    Point tank2;
    Point dude;
    return FromFile(file, x, y, board, tank1, tank2, dude);
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

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
std::string BoardHandler::CreateFileName()
{
    static int counter=0;
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[100];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    auto base=Safir::Dob::Typesystem::Utilities::ToUtf8(Consoden::TankGame::Boards::Path());
    auto leaf=std::string("random_")+std::string(buf)+"_"+boost::lexical_cast<std::string>(++counter)+".txt";
    auto randomDir=boost::filesystem::path(base)/boost::filesystem::path("random")/boost::filesystem::path(leaf);
    return randomDir.string();
}



