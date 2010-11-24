/*
===========================================================================
Copyright (C) 2010 Willi Schinmeyer

This file is part of the Jedi Academy: Renaissance source code.

Jedi Academy: Renaissance source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Jedi Academy: Renaissance source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Jedi Academy: Renaissance source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#ifndef JAR_INPUT_INPUTDEVICEJOYSTICK_HPP
#define JAR_INPUT_INPUTDEVICEJOYSTICK_HPP

#include "jar/input/InputDevice.hpp"
#include "jar/input/API.hpp"

namespace jar {

class JARINPUTAPI InputDeviceJoystick : public InputDevice
{
    public:
        /** \brief Destructor **/
        virtual ~InputDeviceJoystick();

        virtual const DeviceType GetDeviceType() { return DT_Joystick; }

        unsigned int Index;

    protected:
        /** \brief Constructor **/
        InputDeviceJoystick();

        /** \brief Sends a JoyButtonPressed Event.
            \param button index of the pressed button **/
        void JoyButtonPressed(unsigned int button);

        /** \brief Sends a MouseButtonReleased Event.
            \param button index of the released button **/
        void JoyButtonReleased(unsigned int button);

        /** \brief Sends a JoyAxisMoved Event. **/
        void JoyAxisMoved(unsigned int axis, float position);

};

} // namespace jar

#endif // JAR_INPUTDEVICEJOYSTICK_HPP
