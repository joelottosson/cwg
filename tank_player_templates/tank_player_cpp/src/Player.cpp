/******************************************************************************
*
* Copyright Consoden AB, 2014
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#include <boost/bind.hpp>
#include <Safir/Dob/ErrorResponse.h>
#include <Safir/Dob/ResponseGeneralErrorCodes.h>
#include <Consoden/TankGame/GameState.h>
#include <Consoden/TankGame/Player.h>
#include <Consoden/TankGame/Joystick.h>
#include "Player.h"
#include <iostream>

Player::Player(const std::wstring &playerName, boost::asio::io_service& ioService)
    :m_work(new boost::asio::io_service::work(ioService))
    ,m_connection()
    ,m_myHandlerId(playerName+L"Handler")
    ,m_myPlayerId(playerName+L"Instance")
    ,m_myJoystickId()
    ,m_currentGameId()
    ,m_currentTankId(-1)
    ,m_dispatcher(m_connection, ioService)
    ,m_logic()
{
    //set up connection
    m_connection.Open(playerName, L"", 0, this, &m_dispatcher);
    m_connection.RegisterEntityHandler(Consoden::TankGame::Player::ClassTypeId, m_myHandlerId, Safir::Dob::InstanceIdPolicy::HandlerDecidesInstanceId, this);
    m_connection.RegisterEntityHandler(Consoden::TankGame::Joystick::ClassTypeId, m_myHandlerId, Safir::Dob::InstanceIdPolicy::HandlerDecidesInstanceId, this);
    m_connection.SubscribeEntity(Consoden::TankGame::GameState::ClassTypeId, this);

    //create our player entity
    Consoden::TankGame::PlayerPtr player=Consoden::TankGame::Player::Create();
    player->Name()=playerName;
    m_connection.SetAll(player, m_myPlayerId, m_myHandlerId);

    std::wcout<<playerName<<L" is running"<<std::endl;
}

void Player::OnStopOrder()
{
    m_connection.Close();
    m_work.reset();
}

void Player::OnNewEntity(const Safir::Dob::EntityProxy entityProxy)
{
    Consoden::TankGame::GameStatePtr gameState=boost::dynamic_pointer_cast<Consoden::TankGame::GameState>(entityProxy.GetEntity());
    if (gameState)
    {
        for (int i=0; i<gameState->TanksArraySize(); ++i)
        {
            if (!gameState->Tanks()[i].IsNull() && gameState->Tanks()[i].GetPtr()->PlayerId().GetVal()==m_myPlayerId)
            {
                //we participate in this game
                m_currentGameId=entityProxy.GetInstanceId();
                m_currentTankId=gameState->Tanks()[i].GetPtr()->TankId();
                m_myJoystickId=Safir::Dob::Typesystem::InstanceId::GenerateRandom();
                Consoden::TankGame::JoystickPtr joystick=Consoden::TankGame::Joystick::Create();
                joystick->PlayerId()=m_myPlayerId;
                joystick->GameId()=m_currentGameId;
                joystick->TankId()=m_currentTankId;
                joystick->Counter()=0;
                m_connection.SetAll(joystick, m_myJoystickId, m_myHandlerId);
                m_logic.reset(new TankLogic(m_currentTankId, boost::bind(&Player::SetJoystick, this, _1, _2, _3, _4)));
                /* Don't move, the game has not started yet
                try
                {
                    m_logic->MakeMove(gameState);
                }
                catch(...)
                {
                    std::cout<<"Caught unhandled exception in TankLogic!"<<std::endl;
                }
                */
                break;
            }
        }
    }
}

void Player::OnUpdatedEntity(const Safir::Dob::EntityProxy entityProxy)
{
    Consoden::TankGame::GameStatePtr gameState=boost::dynamic_pointer_cast<Consoden::TankGame::GameState>(entityProxy.GetEntity());
    if (gameState && m_logic && entityProxy.GetInstanceId()==m_currentGameId)
    {
        if (gameState->Winner().GetVal() == Consoden::TankGame::Winner::Unknown) {
            try
            {
                m_logic->MakeMove(gameState);
            }
            catch(...)
            {
                std::cout<<"Caught unhandled exception in TankLogic!"<<std::endl;
            }
        }
    }
}

void Player::OnDeletedEntity(const Safir::Dob::EntityProxy entityProxy, const bool)
{
    if (entityProxy.GetTypeId()==Consoden::TankGame::GameState::ClassTypeId && entityProxy.GetInstanceId()==m_currentGameId)
    {
        if (m_myJoystickId!=Safir::Dob::Typesystem::InstanceId())
        {
            m_connection.Delete(Safir::Dob::Typesystem::EntityId(Consoden::TankGame::Joystick::ClassTypeId, m_myJoystickId), m_myHandlerId);
        }

        m_logic.reset();
        m_myJoystickId=Safir::Dob::Typesystem::InstanceId();
        m_currentGameId=Safir::Dob::Typesystem::InstanceId();
        m_currentTankId=-1;
    }
}

void Player::OnCreateRequest(const Safir::Dob::EntityRequestProxy /*entityRequestProxy*/, Safir::Dob::ResponseSenderPtr responseSender)
{
    Safir::Dob::ErrorResponsePtr err=Safir::Dob::ErrorResponse::CreateErrorResponse(Safir::Dob::ResponseGeneralErrorCodes::SafirNoPermission(), L"");
    responseSender->Send(err);
}

void Player::OnUpdateRequest(const Safir::Dob::EntityRequestProxy /*entityRequestProxy*/, Safir::Dob::ResponseSenderPtr responseSender)
{
    Safir::Dob::ErrorResponsePtr err=Safir::Dob::ErrorResponse::CreateErrorResponse(Safir::Dob::ResponseGeneralErrorCodes::SafirNoPermission(), L"");
    responseSender->Send(err);
}

void Player::OnDeleteRequest(const Safir::Dob::EntityRequestProxy /*entityRequestProxy*/, Safir::Dob::ResponseSenderPtr responseSender)
{
    Safir::Dob::ErrorResponsePtr err=Safir::Dob::ErrorResponse::CreateErrorResponse(Safir::Dob::ResponseGeneralErrorCodes::SafirNoPermission(), L"");
    responseSender->Send(err);
}

void Player::OnRevokedRegistration(const Safir::Dob::Typesystem::TypeId typeId, const Safir::Dob::Typesystem::HandlerId &handlerId)
{
    if (handlerId==m_myHandlerId)
    {
        std::wcout<<"Revoked!"<<std::endl;
        OnStopOrder();
    }
}

void Player::SetJoystick(Consoden::TankGame::Direction::Enumeration moveDirection, Consoden::TankGame::Direction::Enumeration towerDirection, bool fire, bool dropMine)
{
    static int counter=0;

    if (m_myJoystickId.GetRawValue()==-1)
    {
        return;
    }

    Consoden::TankGame::JoystickPtr joystick=Consoden::TankGame::Joystick::Create();
    joystick->PlayerId()=m_myPlayerId;
    joystick->GameId()=m_currentGameId;
    joystick->TankId()=m_currentTankId;
    joystick->Counter()=++counter;
    joystick->MoveDirection()=moveDirection;
    joystick->TowerDirection()=towerDirection;
    joystick->Fire()=fire;
    joystick->MineDrop()=dropMine;
    m_connection.SetAll(joystick, m_myJoystickId, m_myHandlerId);
}
