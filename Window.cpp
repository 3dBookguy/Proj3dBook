///////////////////////////////////////////////////////////////////////////////
// Window.cpp
// ==========
// A class of Window for MS Windows
// It registers window class(WNDCLASSEX) with RegisterClassEx() and creates a 
// window with CreateWindowEx() API call.
//
// Based on original code by AUTHOR: Song Ho Ahn 2005
///////////////////////////////////////////////////////////////////////////////

#pragma warning(disable : 4996)
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glew32.lib")

#include <sstream>
#include <iostream>
#include <cstring>
#include <process.h> 
#include "Window.h"
#include "procedure.h"
#include "Log.h"

#include <gl/glew.h>
#include <gl/wglew.h>

using std::wstringstream;
using namespace Win;
HANDLE threadHandle;
bool renderLoop(FALSE);
bool showGLwin(FALSE);

///////////////////////////////////////////////////////////////////////////////
// constructor with params
// NOTE: Windows does not clip a child window from the parent client's area.
// To prevent the parent window from drawing over its child window area, the
// parent window must have WS_CLIPCHILDREN flag.
///////////////////////////////////////////////////////////////////////////////
Window::Window(HINSTANCE hInst, const wchar_t* name, HWND hParent, Controller* ctrl, WinType winType):
	handle(0),
	menuHandle(0),
	view(0),
	instance(hInst),
	parentHandle(hParent),
	controller(ctrl),
	winStyle( WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN ),
//	winStyleEx(WS_EX_CLIENTEDGE),  this style will have a border
	winStyleEx(WS_EX_WINDOWEDGE),	
	x(CW_USEDEFAULT),
 	y(CW_USEDEFAULT),
    width(CW_USEDEFAULT),
	height(CW_USEDEFAULT)

{
    	// copy string
	wcsncpy(this->title, name, MAX_STRING-1);
	wcsncpy(this->className, name, MAX_STRING-1);

	// populate window class struct
	winClass.cbSize = sizeof(WNDCLASSEX);
	winClass.lpfnWndProc = Win::windowProcedure;
	winClass.style = CS_OWNDC;

	if(winType == MAIN_WIN)
	{
		winClass.style = 0;
		winStyle= WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN ;
		Win::log("Window::Window if (winType == MAIN_WIN)");
	}

	winClass.cbClsExtra    	= 0;
	winClass.cbWndExtra    	= 0;
	winClass.hInstance     	= instance;
	winClass.hIcon         	= LoadIcon(instance, IDI_APPLICATION);
	winClass.hIconSm       	= 0;
	winClass.hCursor 		= LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground 	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	winClass.lpszMenuName  	= 0;
    winClass.lpszClassName 	= className;
    winClass.hIconSm       	= LoadIcon(instance, IDI_APPLICATION);

}// End ctor

///////////////////////////////////////////////////////////////////////////////
// destructor
///////////////////////////////////////////////////////////////////////////////
Window::~Window()
{
    ::UnregisterClass(className, instance);
}

///////////////////////////////////////////////////////////////////////////////
// create a window, rendering thread, and OGL context
///////////////////////////////////////////////////////////////////////////////
HWND Window::create(WinType winType, void *ptr)
{
	Win::log("HWND Window::create(WinType winType)");
    // register a window class
    if(!::RegisterClassEx(&winClass)) return 0;

    handle = ::CreateWindowEx(winStyleEx,           // window border with a sunken edge
                              className,            // name of a registered window class
                              title,                // caption of window
                              winStyle,             // window style
                              x,                    // x position
                              y,                    // y position
                              width,                // witdh
                              height,               // height
                              parentHandle,         // handle to parent window
                              menuHandle,           // handle to menu
                              instance,             // application instance
                              (LPVOID)controller);  // window creation data

	if( winType == GL_WIN )
	{
		view  = static_cast<ViewGL*>(ptr);

		if(view) Win::log(L"HWND Window::create have Handel to viewGL");
		else 
		{   
			Win::log(L"HWND Window::create FAILED to get Handel to viewGL");
			return FALSE;
		}

		threadHandle = (HANDLE)_beginthreadex(0, 0, (unsigned (__stdcall *)(void *))
					threadFunction, this, 0, &threadId);

		if( threadHandle ) Win::log(L"Window::create have threadHandle.");
		else 
		{
			Win::log(L"Window::create [ERROR] Failed to create rendering thread.");

			// set flag to allow main thread to proceed.   See main.cpp
			// renderLoop = TRUE;
		}
	} // End if( winType == GL_WIN )
    return handle;
}

void Window::threadFunction(void* param)
{

	log(L"Window::threadFunction(void* param)");

    ((Window*)param)->runThread();
}

void Window::runThread(){

	log(L"Window::runThread()");
    if( !createContext( handle ))   // create a OpenGL rendering context
    {
        Win::log(L"[ERROR] Failed to create OpenGL rendering context from Window::runThread().");       
    }
    else 
		Win::log(L"Void Window::runThread()  OpenGL rendering context created");

	view->createContext( handle );
 	renderLoop = view->hookUpShaders();

	// Wait while main.cpp shows our window and gives us something
	// to render to.
	while(!showGLwin);
	 RECT rect;
    ::GetClientRect(handle, &rect);
    view->resizeWindow(rect.right, rect.bottom); 
	view->generateGL_Buffers();

    while( renderLoop )
    { 
        Sleep(10);      // 10 millisecond yield to other processes or threads
		view->drawGL();
		view->swapBuffers();
    }
}

bool Window::createContext(HWND handle){

	log(L"Window::createContext(HWND handle)");
		
		if (!handle)  return FALSE;
		HGLRC hglrc = 0; // handle to OpenGL rendering context
		hdc = ::GetDC(handle);
		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
		pfd.nSize		= sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion   = 1;
		pfd.dwFlags    = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 32;
		pfd.iLayerType = PFD_MAIN_PLANE;
		int iPixelFormat = ChoosePixelFormat(hdc, &pfd);
		if (iPixelFormat == 0) return FALSE;
		if(!SetPixelFormat(hdc, iPixelFormat, &pfd)) return FALSE;
		HGLRC hRCFake = ::wglCreateContext(hdc);
		::wglMakeCurrent(hdc, hRCFake);
		if(glewInit() != GLEW_OK)
		{
			Win::log("glewInit() failed"); 
			return FALSE;
		}
		else{
			const GLubyte* version;
			version = glewGetString( GLEW_VERSION );
			Win::log("Yeah! ... , we have GLEW version"); 
			Win::log( (const char*)version ); 
		}
		if (GLEW_ARB_vertex_program)
		{
		log("GLEW_ARB_vertex_program");
	  // Core profile OpenGL 1.3 is supported.
		}

		::wglMakeCurrent(NULL, NULL);
		::wglDeleteContext(hglrc);
		hdc = ::GetDC(handle);
		if (WGLEW_ARB_create_context && WGLEW_ARB_pixel_format)
		{
			const int iPixelFormatAttribList[] = { WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
				WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
				WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
				WGL_COLOR_BITS_ARB, 32,
				WGL_DEPTH_BITS_ARB, 24,
				WGL_STENCIL_BITS_ARB, 8, 0 };
			int iContextAttribs[] = { WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
				WGL_CONTEXT_MINOR_VERSION_ARB, 0,
				WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB, 0 };
			//int iPixelFormat, iNumFormats;
			int iPixelFormat;

			//wglChoosePixelFormatARB(hdc, iPixelFormatAttribList, NULL,
			//	1, &iPixelFormat, (UINT*)&iNumFormats);

			iPixelFormat = ChoosePixelFormat(hdc, &pfd);
			// PFD seems to be only redundant parameter now
			if (!SetPixelFormat(hdc, iPixelFormat, &pfd)) {
			Win::log("!SetPixelFormat(hdc, iPixelFormat, &pfd) failed");
			return FALSE;
			}
			hglrc = wglCreateContextAttribsARB(hdc, 0, iContextAttribs);
			// If everything went OK
			if( hglrc)
			{ 
				wglMakeCurrent(hdc, hglrc);
				const GLubyte* version;
				version = glGetString( GL_VERSION );
				Win::log("Yeah! ... , we have GL version");
				Win::log( (const char*)version ); 
				version = glGetString( GL_VENDOR );
				Win::log("Yeah! ... , we have GL vendor");
				Win::log( (const char*)version );
				version = glGetString( GL_RENDERER );
				Win::log("Yeah! ... , we have GPU"); 
				Win::log( (const char*)version ); 
				version = glGetString( GL_SHADING_LANGUAGE_VERSION );
				Win::log("Yeah! ... , we have GLSL");
				Win::log( (const char*)version );		
			} 
			else return FALSE;
		}
		else return FALSE;
		return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// show the window on the screen
///////////////////////////////////////////////////////////////////////////////
void Window::show(int cmdShow){
	Win::log("Window::show(int cmdShow)");
    ::ShowWindow(handle, cmdShow);
    ::UpdateWindow(handle);
}
