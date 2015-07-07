/******************************************************************************
*
* Copyright Saab AB, 2008-2013 (http://safir.sourceforge.net)
*
* Created by: Petter Lönnstedt / stpeln
*
*******************************************************************************
*
* This file is part of Safir SDK Core.
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

//#include "PlayerEntityHandler.h"

namespace TankRandomPlayer
{

    class GameStateHandler :
        // Allows this class to subscribe for entity changes
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
        *  Setup vehicle table and subscribe for Vehicle entity changes.
        */      
        void Init(Safir::Dob::Typesystem::InstanceId player_id);

        /**
        * Overrides Safir::Dob::GameStateHandler. Called by Dob when a new 
        * entity is available.
        */
        virtual void OnNewEntity(const Safir::Dob::EntityProxy entityProxy);
    
        /**
        * Overrides Safir::Dob::GameStateHandler. Called by Dob when an entity 
        * is updated.
        */
        virtual void OnUpdatedEntity(const Safir::Dob::EntityProxy entityProxy);

        /**
        * Overrides Safir::Dob::GameStateHandler. Called when an entity is removed.
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
        Safir::Dob::Typesystem::TypeId  m_classTypeID;
        Safir::Dob::SecondaryConnection m_secDobConnection;
        Safir::Dob::Typesystem::EntityId m_activeEntity;
        Safir::Dob::Typesystem::InstanceId m_playerId;
        bool m_activeEntityValid;

        PlayerEntityHandlerGameStateHandlerIF* m_playerEntityHandlerIF;
    };

} // namespace TankEngine


#endif // __GAMESTATEHANDLER_H
