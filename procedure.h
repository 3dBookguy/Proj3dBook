///////////////////////////////////////////////////////////////////////////////
// procedure.h
// ===========
// Windows will call this function whenever a event is triggered. It routes 
// the message to the controller associated with window handle.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef WIN_PROCEDURE_H
#define WIN_PROCEDURE_H

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400                    
#endif
#include <windows.h>
#include <winuser.h>
#include "resource.h"


namespace Win
{
    // window procedure router
    LRESULT CALLBACK windowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
}

#endif
