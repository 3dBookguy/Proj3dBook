///////////////////////////////////////////////////////////////////////////////
// ControllerMain.cpp
// ==================
// Derived Controller class for main window 
// main has two child child windows the Direct Write window and the OpenGL window.
//
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <commctrl.h>                   // common controls
#include <sstream>
#include "ControllerMain.h"


#include "resource.h"
#include "Log.h"
#define DEBUG_GB
using namespace Win;

// handle events(messages) on all child windows that belong to the parent window.
// For example, close all child windows when the parent got WM_CLOSE message.
// lParam can be used to specify a event or message.
bool CALLBACK enumerateChildren(HWND childHandle, LPARAM lParam);

ControllerMain::ControllerMain(ControllerGL* ctrlGL, ControllerDW* ctrlDW):	
glHandle(0), 
dwHandle(0),
ctrlDW(ctrlDW),
ctrlGL(ctrlGL),
clientWidth(0), 
clientHeight(0),
dwWidth(0),
glWidth(0)
{
	ZeroMemory( &clientRect , sizeof(clientRect));
} 

int ControllerMain::keyDown(int key, LPARAM lParam){
	log(L"ControllerMain::keyDown( key = %i LOWORD = %i HIWORD = %i)", key, LOWORD(lParam),  HIWORD(lParam));

	ctrlDW->keyDown(key, lParam);
	return 0;
}

int ControllerMain::command(int id, int cmd, LPARAM msg, HWND uhandle)
{ 
#ifdef DEBUG_GB
	log(L"ControllerMain::command");
#endif	

	// we send main window handle to ControllerDW which send it to
	// modelDW so it can put the file name on the main window title
    switch(id)
    {
    case ID_FILE_OPEN:
        log(L"ControllerMain::ID_FILE_OPEN ID_FILE_OPEN  ID_FILE_OPEN");
		ctrlDW->command(id, cmd, msg, handle );
        break;
    case REOPEN_ID:
        log(L"ControllerMain::REOPEN_ID REOPEN_ID REOPEN_ID");
		ctrlDW->command(id, cmd, msg, handle);
        break;
    case ID_FILE_EXIT:
        ::PostMessage(handle, WM_CLOSE, 0, 0);
        break;
    }
    return 0;
}

int ControllerMain::close()
{

    ::EnumChildWindows(handle, (WNDENUMPROC)enumerateChildren, (LPARAM)WM_CLOSE);
    ::DestroyWindow(handle);    // close itself
    return 0;
}

int ControllerMain::destroy()
{
    ::PostQuitMessage(0);       // exit the message loop
    return 0;
}

int ControllerMain::create(HWND hwnd)
{
#ifdef DEBUG_GB
	log(L"ControllerMain::create()");
#endif
	handle = hwnd;
    return 0;
}

// Called on sizing the window and by a SendMessage( page mode ) from
// drawDW() when we need to change the graphics mode. 
//		returnValue = ctrl->size(LOWORD(lParam), HIWORD(lParam), (int)wParam); 
//	SendMessage( mainHandle, WM_SIZE, Pages[pageNumber].mode,  pageNumber );
int ControllerMain::size( int x, int y, WPARAM wParam ){
#ifdef DEBUG_GB
log(L"ControllerMain::size() x = %i y = %i  wParam = %i", x, y, wParam);
#endif

	static float aspectRatio(0.5f);
	
	if( wParam == constants::dW_gL ){
		windowMode = constants::dW_gL;
		aspectRatio = 0.5f;
	}
	else if( wParam == constants::dW_dW ){
		windowMode = constants::dW_dW;
		aspectRatio = 1.0f;
	}
	else if( wParam == constants::gL_gL ){
		windowMode = constants::gL_gL;
		aspectRatio = 0.0f;
	}

    // get client dimension of mainWin
    ::GetClientRect( handle, &clientRect );
    clientWidth = clientRect.right - clientRect.left;
	clientHeight = clientRect.bottom - clientRect.top;

	// Set child windows	
	dwWidth = aspectRatio*clientWidth;
	glWidth = clientWidth - dwWidth;
	::SetWindowPos(glHandle, 0, dwWidth, 0, glWidth, clientHeight, SWP_NOZORDER);
	::SetWindowPos(dwHandle, 0, 0, 0, dwWidth, clientHeight, SWP_NOZORDER);

	//These also work to set the children, what's the diff?
	//MoveWindow (glHandle, dwWidth, 0, glWidth, y, TRUE) ;
	//MoveWindow (dwHandle, 0, 0, glWidth, y, TRUE) ;

	return 0;
}

bool CALLBACK enumerateChildren(HWND handle, LPARAM lParam)
{
    if(lParam == WM_CLOSE) ::SendMessage(handle, WM_CLOSE, 0, 0);      // close child windows
 
    return true;
}
