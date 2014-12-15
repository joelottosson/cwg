/******************************************************************************
*
* Copyright Consoden AB, 2014
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#include "MatchStateMachine.h"
#include "BoardHandler.h"


MatchStateMachine::MatchStateMachine(cwg::MatchPtr matchRequest,
                       std::function<void (Consoden::TankGame::GameStatePtr)> onStartNewGame,
                       std::function<void ()> onMatchFinished)
    :m_state(matchRequest)
    ,m_matchEid(cwg::Match::ClassTypeId, sdt::InstanceId::GenerateRandom())
    ,m_onStartNewGame(onStartNewGame)
    ,m_onMatchFinished(onMatchFinished)
    ,m_games()
    ,m_repetion(0)
    ,m_lastGameState()
    ,m_running(true)
{
    CreateBoards();
    m_state->TotalNumberOfGames()=m_state->RepeatBoardSequence().GetVal()*static_cast<int>(m_games.size());
    m_state->CurrentGameNumber()=0;
    m_state->PlayerOneTotalPoints()=0;
    m_state->PlayerTwoTotalPoints()=0;
}

void MatchStateMachine::Start()
{
    StartNextGame();
}

void MatchStateMachine::Update(const Consoden::TankGame::GameStatePtr &updatedState)
{
    if (!m_running)
    {
        return;
    }

    UpdatePoints(updatedState);
    m_lastGameState=updatedState;
    if (!updatedState->Winner().IsNull() && updatedState->Winner()!=cwg::Winner::Unknown)
    {
        std::cout<<"game finished: "<<updatedState->Winner().GetVal()<<std::endl;
        if (m_state->CurrentGameNumber()==m_state->TotalNumberOfGames())
        {
            std::cout<<"match finished"<<std::endl;
            m_running=false;
            m_onMatchFinished();
            return;
        }

        StartNextGame();
    }
}

void MatchStateMachine::UpdatePoints(const Consoden::TankGame::GameStatePtr &updatedState)
{
    if (m_lastGameState)
    {
        int diffP1=updatedState->PlayerOnePoints()-m_lastGameState->PlayerOnePoints();
        int diffP2=updatedState->PlayerTwoPoints()-m_lastGameState->PlayerTwoPoints();
        m_state->PlayerOneTotalPoints()+=diffP1;
        m_state->PlayerTwoTotalPoints()+=diffP2;
    }
}

void MatchStateMachine::StartNextGame()
{
    auto gameIndex=static_cast<size_t>(m_state->CurrentGameNumber().GetVal())%m_games.size();
    m_state->CurrentGameNumber()++;
    auto gameState=m_games[gameIndex];
    m_lastGameState.reset();
    m_onStartNewGame(gameState);
}

void MatchStateMachine::CreateBoards()
{
    for (int i=0; i<m_state->BoardsArraySize(); ++i)
    {
        if (m_state->Boards()[i].IsNull())
        {
            break; //no more boards
        }

        auto file=sdt::Utilities::ToUtf8(m_state->Boards()[i].GetVal());
        auto gs1=CreateGameState(file, false);
        auto gs2=CreateGameState(file, true);
        m_games.push_back(gs1);
        m_games.push_back(gs2);
    }
}

Consoden::TankGame::GameStatePtr MatchStateMachine::CreateGameState(const std::string &boardFile, bool reversedPlayers) const
{
    std::vector<char> board;
    Point tankPos1, tankPos2;
    int width, height;

    if (!BoardHandler::FromFile(boardFile, width, height, board, tankPos1, tankPos2))
    {
        std::wcout<<L"Failed to read file "<<boardFile.c_str()<<std::endl;
        throw std::logic_error("Board file error");
    }

    auto tank1Direction=(tankPos1.x<width/2) ? cwg::Direction::Right : cwg::Direction::Left;
    auto tank2Direction=(tankPos2.x<width/2) ? cwg::Direction::Right : cwg::Direction::Left;

    cwg::GameStatePtr game=cwg::GameState::Create();
    game->Width()=width;
    game->Height()=height;
    game->GamePace()=1.0f; //1 second per move
    game->Board().SetVal(board);
    game->PlayerOneId()=m_state->PlayerOneId();
    game->PlayerTwoId()=m_state->PlayerTwoId();
    game->PlayerOnePoints()=0;
    game->PlayerTwoPoints()=0;

    //create tanks
    cwg::TankPtr tank1=cwg::Tank::Create();
    tank1->TankId()=0;
    tank1->PlayerId()=game->PlayerOneId();
    tank1->Fire()=false;
    tank1->InFlames()=false;
    tank1->HitMine()=false;
    tank1->Fire()=false;
    tank1->HitWall()=false;
    tank1->TookFlag()=false;

    cwg::TankPtr tank2=cwg::Tank::Create();
    tank2->TankId()=1;
    tank2->PlayerId()=game->PlayerTwoId();
    tank2->Fire()=false;
    tank2->InFlames()=false;
    tank2->HitMine()=false;
    tank2->Fire()=false;
    tank2->HitWall()=false;
    tank2->TookFlag()=false;

    if (!reversedPlayers)
    {
        tank1->PosX()=tankPos1.x;
        tank1->PosY()=tankPos1.y;
        tank1->MoveDirection()=tank1Direction;
        tank1->TowerDirection()=tank1Direction;
        tank2->PosX()=tankPos2.x;
        tank2->PosY()=tankPos2.y;
        tank2->MoveDirection()=tank2Direction;
        tank2->TowerDirection()=tank2Direction;
    }
    else
    {
        tank1->PosX()=tankPos2.x;
        tank1->PosY()=tankPos2.y;
        tank1->MoveDirection()=tank2Direction;
        tank1->TowerDirection()=tank2Direction;
        tank2->PosX()=tankPos1.x;
        tank2->PosY()=tankPos1.y;
        tank2->MoveDirection()=tank1Direction;
        tank2->TowerDirection()=tank1Direction;
    }

    game->Tanks()[0].SetPtr(tank1);
    game->Tanks()[1].SetPtr(tank2);

    return game;
}
