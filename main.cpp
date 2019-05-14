/*			main.cpp

Instantiate mainWin, our parent window and its' two child windows:

			dwWin - directWrite - d2D typesetting window
			glWin - openGL endering window

Instantiate controllers, views and models in this MVC style program

*/
// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN

#include <windows.h>		// WINAPI
#include "resource.h"		// Windows #defines used in .rc file

#include "constants.h"		// Our constants
#include "Window.h"			// Our Windows class

// Derived Controller classes
#include "ControllerMain.h"	
#include "ControllerGL.h"
#include "ControllerDW.h"

// Model and View classes
#include "ViewGL.h"
#include "ViewDW.h"
#include "ModelGL.h"

#include "Log.h"

extern bool  renderLoop;
extern bool  showGLwin;
// function declarations
int mainMessageLoop(HACCEL hAccelTable=0);


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdArgs, int cmdShow)
{
	Win::logMode(Win::LOG_MODE_BOTH); // other mode is (Win::LOG_MODE_BOTH) LOG_MODE_DIALOG;

	// Instantiate our MVCs
	Win::ModelGL modelGL; 
	Win::ViewGL viewGL(&modelGL);
	Win::ViewDW viewDW(&viewGL);
	Win::ControllerGL ctrlGL(&modelGL, &viewGL);
//	Win::ControllerDW ctrlDW( &modelGL, &viewDW);
	Win::ControllerDW ctrlDW( &viewDW);
	Win::ControllerMain ctrlMain(&ctrlGL, &ctrlDW);  

	// create mainWin
	Win::Window mainWin(hInst, L"3dBook Reader", 0, &ctrlMain, MAIN_WIN );
	HWND mainHandle = mainWin.create( MAIN_WIN, &ctrlGL);
	if(mainHandle) Win::log("in main: mainWin is created.");
	else Win::log("[ERROR] Failed to create main window.");
 
	// create glWin
	Win::Window glWin(hInst, L"WindowGL", mainHandle, &ctrlGL, GL_WIN );
	HWND glHandle = glWin.create( GL_WIN, &viewGL);

	while( !renderLoop );
	if(glHandle) Win::log("in main: OpenGL child window is created.");
	else Win::log("[ERROR] Failed to create OpenGL window.");

	// create dwWin
	Win::Window dwWin(hInst, L"WindowDW", mainHandle, &ctrlDW, DW_WIN );
	HWND dwHandle = dwWin.create( DW_WIN, &viewDW);
	if(dwHandle) Win::log("in main: DW child window is created.");
	else Win::log("[ERROR] Failed to create DW window.");

	// send window handles to mainCtrl, they are used for resizing window
	ctrlMain.setGLHandle(glHandle);
	ctrlMain.setDWHandle(dwHandle);

	glWin.show();
	dwWin.show();
	mainWin.show();
	// Unblock the render loop now that we have a window to render to
	showGLwin = TRUE;

	HACCEL hAccelTable = 0;
	return mainMessageLoop(hAccelTable);
}

int mainMessageLoop(HACCEL hAccelTable){

    MSG msg;

	// loop until WM_QUIT(0) received
    while(::GetMessage(&msg, 0, 0, 0) > 0)  
    {
        // now, handle window messages
        if(!::TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }

	// return nExitCode of PostQuitMessage()
	return (int)msg.wParam;                 
}
