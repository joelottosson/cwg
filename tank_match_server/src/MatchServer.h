/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#pragma once

#include <boost/asio/steady_timer.hpp>
#include <Safir/Utilities/AsioDispatcher.h>
#include <Consoden/TankGame/Match.h>
#include <Consoden/TankGame/GameState.h>
#include "BoardHandler.h"
#include "MatchStateMachine.h"

class MatchServer
        :public sd::StopHandler
        ,public sd::EntityHandler
        ,public sd::EntitySubscriber
        ,public sd::Requestor
{
public:
    MatchServer(boost::asio::io_service& ioService);

private:
    boost::shared_ptr<boost::asio::io_service::work> m_work;
    boost::asio::steady_timer m_startNewGameTimer;
    sd::Connection m_connection;
    Safir::Utilities::AsioDispatcher m_dispatcher;
    sdt::HandlerId m_defaultHandler;
    BoardHandler m_boardHandler;
    boost::shared_ptr<MatchStateMachine> m_currentMatch;

    //StopHandler interface
    void OnStopOrder();

    // EntityHandler interface
    void OnCreateRequest(const sd::EntityRequestProxy entityRequestProxy, sd::ResponseSenderPtr responseSender);
    void OnUpdateRequest(const sd::EntityRequestProxy entityRequestProxy, sd::ResponseSenderPtr responseSender);
    void OnDeleteRequest(const sd::EntityRequestProxy entityRequestProxy, sd::ResponseSenderPtr responseSender);
    void OnRevokedRegistration(const sdt::TypeId typeId, const sdt::HandlerId &handlerId);

    //EntitySubscriber interface
    void OnNewEntity(const sd::EntityProxy entityProxy);
    void OnUpdatedEntity(const sd::EntityProxy entityProxy);
    void OnDeletedEntity(const sd::EntityProxy entityProxy, const bool /*del*/);

    //Requestor interface
    void OnResponse(const sd::ResponseProxy responseProxy);
    void OnNotRequestOverflow();

    //Helper methods
    bool ExistMatchRunning();
    bool VerifyMatchRequest(cwg::MatchPtr m, sd::ResponseSenderPtr responseSender);
    void DeleteMatch();
    void RestartMatch();
    void OnMatchUpdate();
    void OnStartNewGame(cwg::GameStatePtr gameState);

};
