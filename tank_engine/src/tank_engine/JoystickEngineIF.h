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
