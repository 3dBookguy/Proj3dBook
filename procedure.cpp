///////////////////////////////////////////////////////////////////////////////
// procedure.cpp
// =============
// Window procedure and dialog procedure callback functions.
// Windows will call this function whenever an event is triggered. It routes
// the message to the controller associated with window handle.
//
///////////////////////////////////////////////////////////////////////////////

#include "procedure.h"
#include "constants.h"
#include "Controller.h"
#include "ControllerDW.h"
#include "Log.h"


///////////////////////////////////////////////////////////////////////////////
// Window Procedure
///////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK Win::windowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT returnValue = 0;        // return value

    // find controller associated with window handle ; set by Window::create()
	// in ::CreateWindowEx
    static Win::Controller *ctrl;
    ctrl = (Controller*)::GetWindowLongPtr(hwnd, GWL_USERDATA);

    if(msg == WM_NCCREATE)  // Non-Client Create
    {		
		 Win::log("in winProc rcvd WM_NCCREATE");
        // WM_NCCREATE message is called before non-client parts(border,
        // titlebar, menu,etc) are created. This message comes with a pointer
        // to CREATESTRUCT in lParam. The lpCreateParams member of CREATESTRUCT
        // actually contains the value of lpParam of CreateWindowEX().
        // First, retrieve the pointer to the controller specified when
        // Win::Window is setup.
        ctrl = (Controller*)(((CREATESTRUCT*)lParam)->lpCreateParams);
//        ctrl->setHandle(hwnd);

        // Second, store the pointer to the Controller into GWL_USERDATA,
        // so, other messege can be routed to the associated Controller.
        ::SetWindowLongPtr(hwnd, GWL_USERDATA, (LONG_PTR)ctrl);

        return ::DefWindowProc(hwnd, msg, wParam, lParam);
    }

    // check NULL pointer, because GWL_USERDATA is initially 0, and
    // we store a valid pointer value when WM_NCCREATE is called.
    if(!ctrl)
        return ::DefWindowProc(hwnd, msg, wParam, lParam);

    // route messages to the associated controller
    switch(msg)
    {
		
    case WM_CREATE:
		Win::log("in winProc rcvd WM_CREATE");
		// Send controller the handle to its' window.
		ctrl->create(hwnd);		
        break;

    case WM_SIZE:
	//	Win::log("in winProc rcvd WM_SIZE");
	//  Windows will send this when we resize, max or min the window.
	//	ViewDW will send this when changing the graphics mode or the OGL routine.
	//  The graphics mode will be in wParam GL_MODE = 40007, DW_MODE = 40012
    //  returnValue = ctrl->size(LOWORD(lParam), HIWORD(lParam), GL_WIN);
								// x			y                mode
		returnValue = ctrl->size(LOWORD(lParam), HIWORD(lParam), (int)wParam);   
        break;

    case WM_ENABLE:
        {
		Win::log("in winProc rcvd WM_SIZE");
        bool flag;
        if(wParam) flag = true;
        else flag = false;
        returnValue = ctrl->enable(flag);  // TRUE or FALSE
        break;
        }

    case WM_PAINT:
	//	Win::log("winProc WM_PAINT");			
        ctrl->paint();
        returnValue = ::DefWindowProc(hwnd, msg, wParam, lParam);
        break;

    case WM_COMMAND:  // Not sure when we get this
		Win::log("winProc WM_COMMAND");
        returnValue = ctrl->command(LOWORD(wParam), HIWORD(wParam), lParam, hwnd);   // id, code, msg
        break;

    case WM_CLOSE:
		Win::log("winProc WM_CLOSE");
        returnValue = ctrl->close();
        break;

    case WM_DESTROY:
		Win::log("winProc WM_DESTROY");
        returnValue = ctrl->destroy();
        break;

    case WM_SYSCOMMAND:  // Sent when File Open is clicked in main system menu
		Win::log("WM_SYSCOMMAND");
		if(LOWORD (wParam) == ID_FILE_OPEN){// Win::log("ID_FILE_OPEN ID_FILE_OPEN");
			returnValue = ctrl->command(LOWORD(wParam), HIWORD(wParam), lParam, hwnd);}
        else returnValue = ::DefWindowProc(hwnd, msg, wParam, lParam);
        break;

//  ****************  KEYBOARD MESSAGES **************************

	case WM_KEYDOWN:  // these go to ControllerMain 
	 //	Win::log("winProc( LOWORD(wParam) = %i   HIWORD(wParam) = %i ",LOWORD(wParam), HIWORD(wParam));
		//Win::log("winProc( LOWORD(lParam) = %i   HIWORD(lParam) = %i ", LOWORD(lParam), HIWORD(lParam));

		returnValue = ctrl->keyDown(static_cast<int>( wParam), lParam);		
		break;

    case WM_SYSKEYDOWN:  // ControllerMain sends this to ControllerDW when user hit Enter
	//	Win::log("winProc( .. case WM_SYSKEYDOWN:");
        returnValue = ctrl->keyDown(28, lParam);  // keyCode, 
        break;


    case WM_SYSKEYUP:
        returnValue = ctrl->keyUp((int)wParam, lParam);  // keyCode, keyDetail
        break;

//  ****************  MOUSE MESSAGES **************************

	case WM_MOUSEMOVE:  // ctrl->mouseMove in ControllerDW and  ControllerGL

//	   Win::log("winProc WM_MOUSEMOVE");
		   returnValue = ctrl->mouseMove(wParam, LOWORD(lParam), HIWORD(lParam));
	 break;
    case WM_RBUTTONDOWN:
		returnValue = ctrl->rButtonDown( wParam, LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_LBUTTONDOWN:
		returnValue = ctrl->lButtonDown( wParam, LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_RBUTTONUP:
        returnValue = ctrl->rButtonUp(wParam, LOWORD(lParam), HIWORD(lParam));   // state, x, y
        break;

    case WM_MBUTTONDOWN:
		Win::log("winProc WM_MBUTTONDOWN");
        returnValue = ctrl->mButtonDown(wParam, LOWORD(lParam), HIWORD(lParam)); // state, x, y
        break;

    case WM_MBUTTONUP:
        returnValue = ctrl->mButtonUp(wParam, LOWORD(lParam), HIWORD(lParam));   // state, x, y
        break;

    case WM_MOUSEWHEEL:  // WM_MOUSEWHEEL does not get sent to child windows
						// gies to ControllerMain, then forward
		///	   Win::log("winProc WM_MOUSEWHEEL");
       // returnValue = ctrl->mouseWheel(0, 0, 0, 0);   // state, delta, x, y
		    returnValue = ctrl->mouseWheel((short)LOWORD(wParam), (short)HIWORD(wParam)/WHEEL_DELTA, (short)LOWORD(lParam), (short)HIWORD(lParam));   // state, delta, x, y

        break;

    case WM_HSCROLL:
        returnValue = ctrl->hScroll(wParam, lParam);
        break;

    case WM_VSCROLL:
        returnValue = ctrl->vScroll(wParam, lParam, 0);
        break;

    case WM_TIMER:
        returnValue = ctrl->timer(LOWORD(wParam), HIWORD(wParam));
        break;

    case WM_NOTIFY:
        returnValue = ctrl->notify((int)wParam, lParam); // controllerID, lParam
        break;

    //case WM_CONTEXTMENU:
    //    returnValue = ctrl->contextMenu((HWND)wParam, LOWORD(lParam), HIWORD(lParam));    // handle, x, y (from screen coords)

    default:
        returnValue = ::DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return returnValue;
}
