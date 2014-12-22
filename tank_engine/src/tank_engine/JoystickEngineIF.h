/******************************************************************************
*
* Copyright Consoden AB, 2015
*
* Created by: Björn Weström / bjws
*
*******************************************************************************/

#ifndef __JOYSTICKENGINEIF_H
#define __JOYSTICKENGINEIF_H

namespace TankEngine
{
    /** 
     * Defines a vehicle owner. This class handles the
     * registration as a data owner and processes request
     * on that data.
     */
    class JoystickEngineIF 
    {
    public:

        virtual void NewJoystickCB(int tankId, Safir::Dob::Typesystem::EntityId entityId) = 0;
        //virtual void UpdateJoystickCB() = 0;
        virtual void DeleteJoystickCB(Safir::Dob::Typesystem::EntityId entityId) = 0;
    };
};
#endif
