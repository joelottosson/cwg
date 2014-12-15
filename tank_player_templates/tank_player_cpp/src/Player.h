/******************************************************************************
*
* Copyright Consoden AB, 2014
*
* Created by: Joel Ottosson / joot
*
*******************************************************************************/
#ifndef __GAME_CONNECTION_H__
#define __GAME_CONNECTION_H__

#include <Safir/Dob/Connection.h>
#include <Safir/Utilities/AsioDispatcher.h>
#include "TankLogic.h"

/**
 * The Player class handles all communication with the game engine.
 */
class Player :
        public Safir::Dob::StopHandler,
        public Safir::Dob::EntityHandler,
        public Safir::Dob::EntitySubscriber
{
public:
    /**
     * The constructor starts the player.
     */
    Player(const std::wstring& name);

private:
    Safir::Dob::Connection m_connection;
    Safir::Dob::Typesystem::HandlerId m_myHandlerId;
    Safir::Dob::Typesystem::InstanceId m_myPlayerId;
    Safir::Dob::Typesystem::InstanceId m_myJoystickId;
    Safir::Dob::Typesystem::InstanceId m_currentGameId;
    int m_currentTankId;
    boost::asio::io_service m_ioService;
    Safir::Utilities::AsioDispatcher m_dispatcher;
    boost::shared_ptr<boost::asio::io_service::work> m_work;
    boost::shared_ptr<TankLogic> m_logic;

    //StopHandler interface
    void OnStopOrder();

    // EntitySubscriber interface
    void OnNewEntity(const Safir::Dob::EntityProxy entityProxy);
    void OnUpdatedEntity(const Safir::Dob::EntityProxy entityProxy);
    void OnDeletedEntity(const Safir::Dob::EntityProxy entityProxy, const bool);

    // EntityHandler interface
    void OnCreateRequest(const Safir::Dob::EntityRequestProxy entityRequestProxy, Safir::Dob::ResponseSenderPtr responseSender);
    void OnUpdateRequest(const Safir::Dob::EntityRequestProxy entityRequestProxy, Safir::Dob::ResponseSenderPtr responseSender);
    void OnDeleteRequest(const Safir::Dob::EntityRequestProxy entityRequestProxy, Safir::Dob::ResponseSenderPtr responseSender);
    void OnRevokedRegistration(const Safir::Dob::Typesystem::TypeId typeId, const Safir::Dob::Typesystem::HandlerId &handlerId);

    //Update joystick
    void SetJoystick(Consoden::TankGame::Direction::Enumeration moveDirection,
                     Consoden::TankGame::Direction::Enumeration towerDirection,
                     bool fire);
};

#endif
