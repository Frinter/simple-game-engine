#pragma once

#include <stdio.h>

#include "framework/applicationcontext.hh"
#include "framework/iwindowcontroller.hh"
#include "system/keycode.hh"

namespace System
{	
    class Window {
    public:
        virtual ~Window() {};
        virtual int DoMessageLoop() = 0;
        virtual void GetWindowSize(unsigned int *width, unsigned int *height) = 0;
        virtual bool SetMousePosition(unsigned int posX, unsigned int posY) = 0;
        virtual void Destroy() = 0;
		
        static Window *Create(Framework::ApplicationContext *applicationContext, Framework::ISystemWindowController *controller);
    };
}
