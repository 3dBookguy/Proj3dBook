///////////////////////////////////////////////////////////////////////////////
// ControllerDW.cpp
// ====================
// Derived Controller class for DirectWrite window
//
///////////////////////////////////////////////////////////////////////////////

//#include <process.h>       // for _beginthreadex()  why was this here?
#include "ControllerDW.h"
 #include "resource.h"
#include "Log.h"
#include "ViewDW.h"
 #define DEBUG_GB
using namespace Win;

ControllerDW::ControllerDW( ViewDW* view) : view(view) { }

int ControllerDW::create(HWND hwnd) // handle WM_CREATE
{
#ifdef DEBUG_GB
	log(L"ControllerDW::create(HWND hwnd)");
#endif
	handle = hwnd;
	view->create(hwnd);
    return 0;
}

int ControllerDW::size(int w, int h, WPARAM wParam )
{
#ifdef DEBUG_GB
	log(L"ControllerDW::size(int x = %i  y  = %i  )", w,  h);
#endif

	view->size( w,  h);
    return 0;
}

int ControllerDW::paint() // This function added psl 3 - 18 -14
{
#ifdef DEBUG_GB
	 log(L"ControllerDW::paint()");
#endif
	view->drawDW();
	return 0;
}

int ControllerDW::mouseMove(WPARAM state, int x, int y){
#ifdef DEBUG_GB
//	log(L"ControllerDW::mouseMove(WPARAM state = %i, int x = %i , int y = %i)", state, x, y);
#endif
	view->mouseMove( x,  y );
	return 0;
}

int ControllerDW::getChar(WPARAM message){
#ifdef DEBUG_GB
log(L"int ControllerDW::keyDown(int key= %i, LPARAM lParam )", message);
#endif

	view->getChar(message);
	return 0;
}

//  (WPARAM state, int x, int y
int ControllerDW::keyDown(int key, LPARAM lParam){
#ifdef DEBUG_GB
log(L"int ControllerDW::keyDown(int key= %i, LPARAM lParam )", key);
#endif

	view->keyDown( key, lParam);
	return 0;
}

int ControllerDW::lButtonDown(WPARAM state, int x, int y){
#ifdef DEBUG_GB
log(L"ControllerDW::lButtonDown(WPARAM state =%i, x = %i, y = %i)", state,  x,  y);
#endif

	if( y > MENU_HEIGHT ) return 0;  // mouse is out of menu do nothing 
	view->lButtonDown(x, y, -1);
	return 0;
}

int ControllerDW::rButtonDown(WPARAM state, int menu, int menuItem){ 
#ifdef DEBUG_GB
log(L"ControllerDW::rButtonDown(WPARAM state, int x, int y))", state,  menu, menuItem);
#endif

	view->openDW_file( 0 );
	return 0;
}


int ControllerDW::command(int id, int command, LPARAM msg, HWND handle ){ // handle WM_COMMAND
#ifdef DEBUG_GB
log(L"ControllerDW::command(int id, int command, LPARAM msg)");
#endif

    switch(id)
    {
	// CtrlMain sends main window handle to ControllerDW which send it to
	// modelDW>openDW_file so it can put the file name on the main window title
    case ID_FILE_OPEN:
        if(command == BN_CLICKED)
        {
	log(L"File open here");
	
			if( view->openDW_file(0) == TRUE )
			{			
				log(L"ControllerDW::ID_FILE_OPEN invalidate");
				InvalidateRect( handle, NULL, FALSE );
			}
			break;
		}
    case REOPEN_ID:
        if(command == BN_CLICKED)
        {
			if( view->openDW_file(1) == TRUE )
			{
				//view->openDW_file(handle, 1) == TRUE ) {
				//	log(L"ControllerMain::REOPEN_ID invalidate");
				InvalidateRect(handle, NULL, FALSE );
			}
			break;
		}       
	}
    return 0;	
}


