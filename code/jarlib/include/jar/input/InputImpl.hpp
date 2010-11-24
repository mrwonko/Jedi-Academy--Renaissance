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

#ifndef JAR_INPUT_INPUTIMPL_HPP
#define JAR_INPUT_INPUTIMPL_HPP

#include "jar/core/Component.hpp"

namespace jar {

class InputDeviceManager;

#ifdef _WIN32
namespace Windows
{
    class WinKeyboard;
}
#endif

class InputImpl : public Component
{
    public:
        InputImpl();
        virtual ~InputImpl();

        virtual const bool Init();
        virtual const bool Deinit();

        virtual void Update(TimeType deltaT);

    private:
        InputDeviceManager* mInputDeviceManager;
#ifdef WIN32
        Windows::WinKeyboard* mKeyboard;
#endif
};

} // namespace jar

#endif // JAR_INPUT_INPUTIMPL_HPP
