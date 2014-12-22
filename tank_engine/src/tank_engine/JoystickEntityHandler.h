/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Björn Weström / bjws
*
*******************************************************************************/
#ifndef __JOYSTICKENTITYHANDLER_H
#define __JOYSTICKENTITYHANDLER_H

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

namespace TankEngine
{
    class JoystickEngineIF;


    class JoystickEntityHandler :
        // Allows this class to subscribe for entity changes
        public Safir::Dob::EntitySubscriber
    {

    public:

        /**
        *  Attach to Dob.
        */  
        JoystickEntityHandler(JoystickEngineIF* engineIf);
        
        /**
        *  Destructor
        */
        virtual ~JoystickEntityHandler();

        /**
        *  Do some initialization.
        */      
        void Init(Safir::Dob::Typesystem::InstanceId gameId);

        /**
        * Overrides Safir::Dob::JoystickEntityHandler. Called by Dob when a new 
        * entity is available.
        */
        virtual void OnNewEntity(const Safir::Dob::EntityProxy entityProxy);
    
        /**
        * Overrides Safir::Dob::JoystickEntityHandler. Called by Dob when an entity 
        * is updated.
        */
        virtual void OnUpdatedEntity(const Safir::Dob::EntityProxy entityProxy);

        /**
        * Overrides Safir::Dob::JoystickEntityHandler. Called when an entity is removed.
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
        Safir::Dob::SecondaryConnection m_secDobConnection;

        JoystickEngineIF* m_engine;
        Safir::Dob::Typesystem::InstanceId m_gameId;
    };

} // namespace TankEngine


#endif // __PLAYERENTITYHANDLER_H
