/******************************************************************************
*
* Copyright Consoden AB, 2013-2014 (http://www.consoden.se)
*
* Created by: Björn Weström / bjws
*
*******************************************************************************
*
* This file is part of Consodens Programming Challenge 2014.
* This is the player template used by the competing teams.
* It is based on the Safir SDK Core framework. (http://safir.sourceforge.net)
*
* Safir SDK Core is free software: you can redistribute it and/or modify
* it under the terms of version 3 of the GNU General Public License as
* published by the Free Software Foundation.
*
* Safir SDK Core is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Safir SDK Core.  If not, see <http://www.gnu.org/licenses/>.
*
******************************************************************************/

#ifndef __GAMESTATEHANDLER_H
#define __GAMESTATEHANDLER_H

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning (disable: 4127)
#pragma warning (disable: 4251)
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <Safir/Dob/Typesystem/Defs.h>
#include <Safir/Dob/Consumer.h>
#include <Safir/Dob/Connection.h>

#include "PlayerEntityHandlerGameStateHandlerIF.h" 

namespace TankTeamPlayer
{

    class GameStateHandler :
        public Safir::Dob::EntitySubscriber
    {

    public:

        /**
        *  Attach to Dob.
        */  
        GameStateHandler(PlayerEntityHandlerGameStateHandlerIF* playerEntityHandlerIF);
        
        /**
        *  Destructor
        */
        virtual ~GameStateHandler();

        /**
        *  Setup subscription for Gamestate entity changes.
        */      
        void Init(Safir::Dob::Typesystem::InstanceId playerId);

        /**
        * Overrides Safir::Dob::EntitySubscriber. Called by Dob when a new 
        * entity is available.
        */
        virtual void OnNewEntity(const Safir::Dob::EntityProxy entityProxy);
    
        /**
        * Overrides Safir::Dob::EntitySubscriber. Called by Dob when an entity 
        * is updated.
        */
        virtual void OnUpdatedEntity(const Safir::Dob::EntityProxy entityProxy);

        /**
        * Overrides Safir::Dob::EntitySubscriber. Called when an entity is removed.
        */
        virtual void OnDeletedEntity(const Safir::Dob::EntityProxy entityProxy, const bool deprecated);

    private:
        /**
        * Internal method to handle a new entity
        */
        void OnNewEntityInternal(const Safir::Dob::EntityProxy &entityProxy);
        
        /**
        * Internal method to handle an updated entity
        */
        void OnUpdatedEntityInternal(const Safir::Dob::EntityProxy &entityProxy);
        
        /**
        * Internal method to handle a deleted entity
        */
        void OnDeletedEntityInternal(const Safir::Dob::EntityProxy &entityProxy);

    protected:
        

        // Member variables
        Safir::Dob::Typesystem::TypeId  m_ClassTypeID;
        Safir::Dob::SecondaryConnection m_SecDobConnection;
        Safir::Dob::Typesystem::EntityId m_ActiveEntity;
        Safir::Dob::Typesystem::InstanceId m_PlayerId;
        bool m_ActiveEntityValid;

        PlayerEntityHandlerGameStateHandlerIF* m_PlayerEntityHandlerIF;
    };

} // namespace TankEngine


#endif // __GAMESTATEHANDLER_H
