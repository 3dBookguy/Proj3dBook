///////////////////////////////////////////////////////////////////////////////
// Window.h
// ========
// A class of Window for MS Windows
// It registers window class(WNDCLASSEX) with RegisterClassEx() and creates a 
// window with CreateWindowEx() API call.
//
//  AUTHOR: Song Ho Ahn
// CREATED: 2005-03-16
// UPDATED: 2006-06-20
///////////////////////////////////////////////////////////////////////////////

#ifndef WIN_WINDOW_H
#define WIN_WINDOW_H

#include <windows.h>
#include "constants.h"
#include "Controller.h"
#include "ViewGL.h"
 
namespace Win
{
    class Window
    {
    public:

		//ctor will fill in WNDCLASSEX and whatever is needed for
		// the calls to win API RegisterClassEx, CreateWindowEx
        Window(HINSTANCE hInst, const wchar_t* name, HWND hParent,
			Controller* ctrl, WinType winType);
        ~Window();

		//calls win API RegisterClassEx, CreateWindowEx to create a window
		//windows will send WM_NCCREAT WM_CREATE to our winproc.
        HWND create(WinType winType, void *ptr);
		bool createContext(HWND handle);
        void show(int cmdShow=SW_SHOWDEFAULT);  // make the window visible
        HWND getHandle(){ return handle; };		// return window handle

        // setters for WNDCLASSEX, if not specified, use default values
        void setClassStyle(UINT style)          { winClass.style = style; };
        void setIcon(int id)                    { winClass.hIcon = loadIcon(id); };
        void setIconSmall(int id)               { winClass.hIconSm = loadIcon(id); };
        void setCursor(int id)                  { winClass.hCursor = loadCursor(id); };
		void setBackground(int color)           { winClass.hbrBackground = (HBRUSH)::GetStockObject(color); };
        void setMenuName(LPCTSTR name)          { winClass.lpszMenuName = name; };
//		void setProc(WNDPROC name)				{ winClass.lpfnWndProc = name; };
        // setters for CreateWindowEx()
        void setWindowStyle(DWORD style)        { winStyle = style; };
        void setWindowStyleEx(DWORD style)      { winStyleEx = style; };
        void setPosition(int x, int y)          { this->x = x; this->y = y; };
        void setWidth(int w)                    { width = w; };
        void setHeight(int h)                   { height = h; };
        void setParent(HWND handle)             { parentHandle = handle; };
        void setMenu(HMENU handle)              { menuHandle = handle; };


    private:
		ViewGL* view;
		static void threadFunction(void* arg); 
//		HANDLE threadHandle;
        unsigned int threadId;
        void runThread();       // thread for OpenGL rendering
      //  volatile bool wloopFlag; // rendering loop flag



        enum { MAX_STRING = 256 };              // local constants, max length of string

        // member functions
		BOOL bisTemp;
        void registerClass();                   // register window class with Windows system
        HICON loadIcon(int id);                 // load icon using resource id
        HCURSOR loadCursor(int id);             // load icon using resource id

        HWND handle;                            // handle to this window
		HGLRC hglrc;                            // handle to OpenGL rendering context
		HDC hdc;								// handle to device context
        WNDCLASSEX winClass;                    // window class information
        DWORD winStyle;                         // window style: WS_OVERLAPPEDWINDOW, WS_CHILD, ...
        DWORD winStyleEx;                       // extended window style
        wchar_t title[MAX_STRING];              // title of window
        wchar_t className[MAX_STRING];          // name of window class
        int x;                                  // window position X
        int y;                                  // window position Y
        int width;                              // window width
        int height;                             // window height
        HWND parentHandle;                      // handle to parent window
        HMENU menuHandle;                       // handle to menu
        HINSTANCE instance;                     // handle to instance
        Win::Controller *controller;            // pointer to controller
    };
}

#endif
