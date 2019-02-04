/*
ViewGL has methods and members to handle the tasks of presenting an openGL
image to the display
*/

#ifndef VIEW_GL_H
#define VIEWL_GL_H

// #pragma comment(lib, "opengl32.lib") will tell Windows to look 
// for the library opengl32.lib. If that is an import library when
// the program is loaded windows will search for OtherLib700.dll in
// the path. If OtherLib700.lib is a static library then that is all it needs.
// C:\Program Files (x86)\Windows Kits\8.0\Lib\win8\um\x86
//C:\Program Files (x86)\Windows Kits\8.0\Lib\win8\um\x64
#pragma comment(lib, "opengl32.lib")
//#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glew32.lib")

//C:\Users\Pete\Documents\glew-1.10.0-win32\glew-1.10.0
//C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\include\GL
#include <gl/glew.h>  //  =>   will  #include <GL/gl.h>  #include <GL/glu.h>

// see C:\Users\Pete\Documents\glm-0.9.5.2\glm\doc
#define GLM_SWIZZLE
#define GLM_FORCE_RADIANS

// See the glm manual to include fearture specific .h's
// reduce compile time and executable size.
//C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\include\glm
#include <glm/glm.hpp>  //
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Log.h"


#include "resource.h"
#include <vector>
#include <sstream>
#include <iostream>
#include <string>
#include <Xinput.h> // Gamecontroller API
//#include "Mesh_Proc.h"
using namespace std;
namespace Win
{

class ViewGL
{
public:
    ViewGL();
    void hookUpShaders();    // initialize OpenGL states
 //   void setViewport(int width, int height);
    void resizeWindow(int width, int height);
	void hiFromDW(int page);
    void drawGL();
	void generateGL_Buffers();

// shaders
	class ShaderLoad
	{
	public:
		bool LoadShader(string sFile, int a_iType);
		void DeleteShader();
		bool IsLoaded();
		UINT GetShaderID();
	private:
		UINT uiShader;	// ID of shader 
		int iType;		// GL_VERTEX_SHADER, GL_FRAGMENT_SHADER...
		bool bLoaded;	// Flag shader as compiled and loaded.
		string ShaderLog;
	};
	class Shader
	{
	public:
		void CreateProgram();
		void DeleteProgram();
		bool AddShaderToProgram(ShaderLoad* shShader);
		bool LinkProgram();
		void UseProgram();
		UINT GetProgramID(){ return uiProgram;}
		
	private:
		UINT uiProgram; // ID of program
		bool bLinked;	// Flag program as linked and ready to use.
	};

private:

    int windowWidth;
    int windowHeight;
    bool windowResized;
	int GLroutine;       // Used to switch between drawing routines
	float bgColor[4];  // Screen bacground color

};// end class ViewGL
}
#endif
