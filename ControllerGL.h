///////////////////////////////////////////////////////////////////////////////
// ControllerGL.h
// ==============
// Derived Controller class for OpenGL window
// It is the controller of OpenGL rendering window. It initializes DC and RC,
// when WM_CREATE called, then, starts new thread for OpenGL rendering loop.
//
// When this class is constructed, it gets the pointers to model and view
// components.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef WIN_CONTROLLER_GL_H
#define WIN_CONTROLLER_GL_H

#include "Controller.h"
#include "ViewGL.h"
#include "ModelGL.h"

namespace Win
{
    class ControllerGL : public Controller
    {
    public:
        ControllerGL(ModelGL* model, ViewGL* view);
        ~ControllerGL() {}; 
        int close();    // close the RC and OpenGL window
        int command(int id, int cmd, LPARAM msg);   // for WM_COMMAND
        int create(HWND handle);
	//	bool hookupShaders();
        int paint();
        int size(int w, int h, WPARAM wParam);
        int lButtonDown(WPARAM state, int x, int y);
        int lButtonUp(WPARAM state, int x, int y);
        int rButtonDown(WPARAM state, int x, int y);
        int rButtonUp(WPARAM state, int x, int y);
        int mouseMove(WPARAM state, int x, int y);
		int mouseWheel(int state, int delta, int x, int y);
		int mButtonDown(int state, int delta, int x, int y);
    private:
		// _beginThreadex stuff
		// static thread function, it will route to member function, runThread()
   //     static void threadFunction(void* arg); 
	//	HANDLE threadHandle;
    //    unsigned int threadId;
    //    void runThread();       // thread for OpenGL rendering
        volatile bool loopFlag; // rendering loop flag

		// Model and View pointers
		ModelGL* model;
        ViewGL* view;
    };
}

#endif
