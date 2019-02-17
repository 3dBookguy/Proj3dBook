///////////////////////////////////////////////////////////////////////////////
// ControllerGL.cpp
// ================
// Derived Controller class for OpenGL window
// It is the controller of OpenGL rendering window. It initializes DC and RC,
// when WM_CREATE called, then, start new thread for OpenGL rendering loop.
//
// When this class is constructed, it gets the pointers to model and view
// components.
//
///////////////////////////////////////////////////////////////////////////////

#define DEBUG_GB
#include <process.h>        // for _beginthreadex()
#include "ControllerGL.h"
#include "Log.h"
extern bool renderLoop;
extern HANDLE threadHandle;
using namespace Win;

ControllerGL::ControllerGL(ModelGL* model, ViewGL* view)
: model(model), view(view),  loopFlag(false)
{}


int ControllerGL::create(HWND hwnd){ // handle WM_CREATE
#ifdef DEBUG_GB
	log(L"ControllerGL::create()");
#endif

	view->create( hwnd);


    return 0;
}

int ControllerGL::paint(){ return 0; } // handle WM_PAINT
int ControllerGL::command(int id, int cmd, LPARAM msg) { return 0; } // handle 
int ControllerGL::keyDown(int key, LPARAM lParam)
{ 
	model->rotateParams(key);
//	view->keyBoardInput(key);
	return 0; 

}


//mButtonDown(wParam, LOWORD(lParam), HIWORD(lParam));
int ControllerGL::mButtonDown(int state, int delta, int x, int y){
//#ifdef DEBUG_GB
//	log(L"ControllerGL::mButtonDown(int state, int delta, int x, int y)");
//#endif
//returnValue = ctrl->mouseWheel((short)LOWORD(wParam), (short)HIWORD(wParam)/WHEEL_DELTA, (short)LOWORD(lParam), (short)HIWORD(lParam));   // state, delta, x, y
    return 0;
}


int ControllerGL::mouseWheel(int state, int delta, int x, int y){
//#ifdef DEBUG_GB
//	log(L"ControllerGL::mouseWheel(int state, int delta, int x, int y)");
//#endif
    return 0;
}

int ControllerGL::lButtonDown(WPARAM state, int x, int y){
#ifdef DEBUG_GB
	log(L"ControllerGL::lButtonDown(WPARAM state, int x = %i, int y = %i)", x,y);
#endif

	view->leftButtonDown( x, y);
    return 0;
}

int ControllerGL::lButtonUp(WPARAM state, int x, int y){
//#ifdef DEBUG_GB
//	log(L"ControllerGL::lButtonUp(WPARAM state, int x, int y)");
//#endif

    return 0;
}
int ControllerGL::rButtonDown(WPARAM state, int x, int y){
//#ifdef DEBUG_GB
//	log(L"ControllerGL::lButtonDown(WPARAM state, int x, int y)");
//#endif
    return 0;
}

int ControllerGL::rButtonUp(WPARAM state, int x, int y){
//#ifdef DEBUG_GB
//	log(L"ControllerGL::lButtonUp(WPARAM state, int x, int y)");
//#endif

    return 0;
}

int ControllerGL::mouseMove(WPARAM state, int x, int y){

#ifdef DEBUG_GB
//	log(L"ControllerGL::mouseMove(WPARAM state, int x, int y)");
#endif

//	view->mouseMove( x,  y );
	//if( cursorState != ARROW){
	//	SetCursor(LoadCursor(NULL, IDC_ARROW));
	//	cursorState = ARROW; }

 //   if(state == MK_LBUTTON)
 //   {
 ////       model->rotateCamera(x, y);
 //   }
 //   if(state == MK_RBUTTON)
 //   {
 ////       model->zoomCamera(y);
 //   }

    return 0;
}

int ControllerGL::size(int w, int h, WPARAM wParam){ // handle WM_SIZE
#ifdef DEBUG_GB
	log(L"ControllerGL::size(int x = %i  y  = %i  )", w,  h);
#endif
    view->resizeWindow(w, h);
//    Win::log(L"OpenGL window is resized: %dx%d.", w, h);
    return 0;
}

int ControllerGL::close(){ // handle WM_CLOSE

    renderLoop = FALSE;
	// wait for rendering thread is terminated
    ::WaitForSingleObject( threadHandle, INFINITE);
	::wglMakeCurrent(0, 0);             // unset RC
	::CloseHandle(threadHandle);
     view->closeContext(handle);    // close OpenGL Rendering Context (RC)
    ::DestroyWindow(handle);
    return 0;
}