///////////////////////////////////////////////////////////////////////////////
// ControllerMain.h
// ================
// Derived Controller class for main window
//
///////////////////////////////////////////////////////////////////////////////

#ifndef WIN_CONTROLLER_MAIN_H
#define WIN_CONTROLLER_MAIN_H

#include "constants.h"
#include "Controller.h"
#include "ControllerDW.h" 
#include "ControllerGL.h" 
#include "ViewDW.h"
#include "ModelGL.h"
namespace Win
{
    class ControllerMain :public Controller
    {
    public:
        ControllerMain(ControllerGL* ctrlGL, ControllerDW* ctrlDW); 
        ~ControllerMain() {};

		// These are called at startup in main.cpp
        void setGLHandle(HWND handle){ glHandle = handle;}  
        void setDWHandle(HWND handle){ dwHandle = handle;} 

		// Contoller override
        int command(int id, int cmd, LPARAM msg, HWND handle); 
        int close();
        int create(HWND handle);
        int destroy();
        int size( int w, int h, WPARAM wParam);
		int keyDown(int key, LPARAM lParam);

    private:
		// Make variables members if they belong to the object you are designing. 
		// If you are working just with temporary variables - do not make them	
		// members, because in that case you will just pollute your object design.

		// Child handles and controllers.
        HWND glHandle; 
        HWND dwHandle; 
		ControllerDW* ctrlDW;
		ControllerGL* ctrlGL;

		// Set windows in size
		// int size( int w, int h, WPARAM wParam);
		RECT clientRect;
		int windowMode;
		int clientWidth; 
		int clientHeight;
		int dwWidth;
		int glWidth;
    };
}

#endif
