///////////////////////////////////////////////////////////////////////////////
// ControllerDW.h
// ==================
// Derived Controller class for OpenGL dialog window
//
///////////////////////////////////////////////////////////////////////////////

#ifndef WIN_CONTROLLER_DW_H
#define WIN_CONTROLLER_DW_H

#include "Controller.h"    
#include "ViewDW.h"

namespace Win
{
    class ControllerDW : public Controller
    {
    public:
		ControllerDW( ViewDW* view);
        ~ControllerDW() {};

		int mouseMove(WPARAM state, int x, int y);
		int getChar( WPARAM message);
		int keyDown(int key, LPARAM lParam);
        int command(int id, int cmd, LPARAM msg, HWND handle);   // for WM_COMMAND
		int size(int w, int h, WPARAM wParam);
        int create(HWND handle);                               // for WM_CREATE
		int lButtonDown(WPARAM wParam, int x, int y ); 
		int rButtonDown(WPARAM wParam, int x, int y );
		int paint();  

    private:

        ViewDW* view;	// pointer to view component
    };
}

#endif
