/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#include <Consoden/TankGame/Match.h>
#include <Consoden/TankGame/Boards.h>
#include <Consoden/TankGame/GameState.h>
#include <Consoden/TankGame/Player.h>
#include <Safir/Dob/NotFoundException.h>
#include "MatchServer.h"

#include <boost/lexical_cast.hpp>
#include <boost/chrono.hpp>
namespace
{
    inline std::string TimeString()
    {
        auto time=boost::chrono::steady_clock::now();
        auto s=boost::chrono::duration_cast<boost::chrono::milliseconds>(time.time_since_epoch());
        return boost::lexical_cast<std::string>(s.count()/1000.0);
    }
}

MatchServer::MatchServer(boost::asio::io_service& ioService)
    :m_work(new boost::asio::io_service::work(ioService))
    ,m_startNewGameTimer(ioService)
    ,m_connection()
    ,m_dispatcher(m_connection, ioService)
    ,m_defaultHandler()
    ,m_boardHandler(m_connection)
    ,m_currentMatch()
{
    m_connection.Open(L"MatchServer", L"", 0, this, &m_dispatcher);
    m_connection.RegisterEntityHandler(cwg::Match::ClassTypeId, m_defaultHandler, sd::InstanceIdPolicy::HandlerDecidesInstanceId, this);
    m_connection.SubscribeEntity(cwg::GameState::ClassTypeId, this);
    m_boardHandler.Init();
    std::wcout<<L"tank_match_server is running"<<std::endl;
}

//-------------------------------------
// StopHandler interface
//-------------------------------------
void MatchServer::OnStopOrder()
{
    m_connection.Close();
    m_work.reset();
}

//-------------------------------------
// EntityHandler interface
//-------------------------------------
void MatchServer::OnCreateRequest(const sd::EntityRequestProxy entityRequestProxy, sd::ResponseSenderPtr responseSender)
{
    cwg::MatchPtr m=boost::dynamic_pointer_cast<cwg::Match>(entityRequestProxy.GetRequest());
    if (!m)
    {
        throw std::logic_error(std::string("MatchServer got a request of something else than a match. Got: ")+
                               sdt::Utilities::ToUtf8(entityRequestProxy.GetEntityId().ToString()));
    }

    //check if there is a match running already
    if (ExistMatchRunning())
    {
        //ongoing match, can't start new
        auto error=sd::ErrorResponse::CreateErrorResponse(sd::ResponseGeneralErrorCodes::SafirNoFreeInstance(), L"A match is already running!");
        responseSender->Send(error);
        return;
    }

    //verify request    
    if (!VerifyMatchRequest(m, responseSender))
    {
        //request not valid, error response has been sent
        return;
    }

    //everything seems to be ok, start a new match
    m_currentMatch=boost::make_shared<MatchStateMachine>(m, [=](cwg::GameStatePtr gs){OnStartNewGame(gs);}, [=]{OnMatchUpdate();});
    m_work->get_io_service().post([=]{m_currentMatch->Start();});

    auto ok=sd::SuccessResponse::Create();
    responseSender->Send(ok);
}

void MatchServer::OnUpdateRequest(const sd::EntityRequestProxy /*entityRequestProxy*/, sd::ResponseSenderPtr responseSender)
{
    //means restart match
    RestartMatch();

    auto ok=sd::SuccessResponse::Create();
    responseSender->Send(ok);
}

void MatchServer::OnDeleteRequest(const sd::EntityRequestProxy entityRequestProxy, sd::ResponseSenderPtr responseSender)
{
    //delete ongoing match
    DeleteMatch();
    auto ok=sd::SuccessResponse::Create();
    responseSender->Send(ok);
}

void MatchServer::OnRevokedRegistration(const sdt::TypeId /*typeId*/, const sdt::HandlerId &/*handlerId*/)
{
    std::wcout<<L"MatchServer was revoked!"<<std::endl;
}

//-------------------------------------
//EntitySubscriber interface
//-------------------------------------
void MatchServer::OnNewEntity(const sd::EntityProxy entityProxy)
{
    auto gameState=boost::dynamic_pointer_cast<cwg::GameState>(entityProxy.GetEntity());
    if (gameState && m_currentMatch)
    {
        m_currentMatch->OnNewGameState(gameState);
    }
}

void MatchServer::OnUpdatedEntity(const sd::EntityProxy entityProxy)
{
    auto gameState=boost::dynamic_pointer_cast<cwg::GameState>(entityProxy.GetEntity());
    if (gameState && m_currentMatch)
    {
        m_currentMatch->OnUpdatedGameState(gameState);
    }
}

void MatchServer::OnDeletedEntity(const sd::EntityProxy /*entityProxy*/, const bool /*del*/)
{
}

//-------------------------------------
//Requestor interface
//-------------------------------------
void MatchServer::OnResponse(const sd::ResponseProxy responseProxy)
{
    if (!responseProxy.IsSuccess())
    {
        std::wcout<<L"Received an error response"<<std::endl;
    }
}

void MatchServer::OnNotRequestOverflow()
{
}

void MatchServer::DeleteMatch()
{
    //Send delete request for game state
    for (auto it=m_connection.GetEntityIterator(cwg::GameState::ClassTypeId, false); it!=sd::EntityIterator(); ++it)
    {
        m_connection.DeleteRequest((*it).GetEntityId(), this);
    }

    //delete match
    m_connection.DeleteAllInstances(cwg::Match::ClassTypeId, m_defaultHandler);
    m_currentMatch.reset();
}

void MatchServer::RestartMatch()
{
    auto keep=m_currentMatch; //keep reference to prevent destruction
    DeleteMatch();
    keep->Reset();
    m_currentMatch=keep;
    m_work->get_io_service().post([=]{m_currentMatch->Start();});
}

bool MatchServer::ExistMatchRunning()
{
    try
    {
        if (!m_currentMatch)
        {
            return false;
        }

        auto proxy=m_connection.Read(m_currentMatch->MatchEntityId());
        auto ongoingMatch=boost::dynamic_pointer_cast<cwg::Match>(proxy.GetEntity());
        if (!ongoingMatch)
        {
            return false; //no existing match
        }
        else if (ongoingMatch->Winner().IsNull())
        {
            return true; //still no winner
        }
        else if (ongoingMatch->Winner().GetVal()!=cwg::Winner::Unknown)
        {
            return true; //still no winner
        }

        //There is an instance of a match that has finished. Remove it and continue with the request
        DeleteMatch();
    }
    catch (const sd::NotFoundException&) {/*no match exists*/}

    return false;
}

bool MatchServer::VerifyMatchRequest(cwg::MatchPtr m, sd::ResponseSenderPtr responseSender)
{
    if (m->PlayerOneId().IsNull() || m->PlayerTwoId().IsNull())
    {
        auto error=sd::ErrorResponse::CreateErrorResponse(sd::ResponseGeneralErrorCodes::SafirNullMember(), L"Both players are not set!");
        responseSender->Send(error);
        return false;
    }
    else if (m->GameTime().IsNull())
    {
        auto error=sd::ErrorResponse::CreateErrorResponse(sd::ResponseGeneralErrorCodes::SafirNullMember(), L"GameTime not set!");
        responseSender->Send(error);
        return false;
    }
    else if (m->RepeatBoardSequence().IsNull())
    {
        auto error=sd::ErrorResponse::CreateErrorResponse(sd::ResponseGeneralErrorCodes::SafirNullMember(), L"RepeatBoardSequence not set!");
        responseSender->Send(error);
        return false;
    }
    else if (m->Boards()[0].IsNull())
    {
        auto error=sd::ErrorResponse::CreateErrorResponse(sd::ResponseGeneralErrorCodes::SafirNullMember(), L"Boards not set!");
        responseSender->Send(error);
        return false;
    }
    else if (!m->TotalNumberOfGames().IsNull() ||
             !m->CurrentGameNumber().IsNull() ||
             !m->PlayerOneTotalPoints().IsNull() ||
             !m->PlayerTwoTotalPoints().IsNull() ||
             !m->Winner().IsNull())
    {
        auto error=sd::ErrorResponse::CreateErrorResponse(sd::ResponseGeneralErrorCodes::SafirPresentMember(),
                                                                  L"Match request has a member set that shall be null in requests!");
        responseSender->Send(error);
        return false;
    }
    else if (!m_connection.IsCreated(sdt::EntityId(cwg::Player::ClassTypeId, m->PlayerOneId().GetVal())))
    {
        auto error=sd::ErrorResponse::CreateErrorResponse(sd::ResponseGeneralErrorCodes::SafirInstanceErr(), L"PlayerOneId does not exist!");
        responseSender->Send(error);
        return false;
    }
    else if (!m_connection.IsCreated(sdt::EntityId(cwg::Player::ClassTypeId, m->PlayerTwoId().GetVal())))
    {
        auto error=sd::ErrorResponse::CreateErrorResponse(sd::ResponseGeneralErrorCodes::SafirInstanceErr(), L"PlayerTwoId does not exist!");
        responseSender->Send(error);
        return false;
    }

    return true;
}

void MatchServer::OnMatchUpdate()
{
    m_connection.SetAll(m_currentMatch->CurrentState(), m_currentMatch->MatchEntityId().GetInstanceId(), m_defaultHandler);
}

void MatchServer::OnStartNewGame(Consoden::TankGame::GameStatePtr gameState)
{
    if (m_currentMatch->CurrentState()->CurrentGameNumber()==1)
    {
        m_startNewGameTimer.expires_from_now(boost::chrono::milliseconds(500));
    }
    else
    {
        m_startNewGameTimer.expires_from_now(boost::chrono::milliseconds(5000));
    }

    m_startNewGameTimer.async_wait([=](const boost::system::error_code&)
    {
        //Send delete request for game state
        for (auto it=m_connection.GetEntityIterator(cwg::GameState::ClassTypeId, false); it!=sd::EntityIterator(); ++it)
        {
            m_connection.DeleteRequest((*it).GetEntityId(), this);
        }

        m_connection.CreateRequest(gameState, m_defaultHandler, this);
    });
}
