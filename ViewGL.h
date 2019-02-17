/*
						ViewGL.h

Handles calls from ViewGL for rendering data.


*/
#ifndef VIEW_GL_H
#define VIEW_GL_H

#include <windows.h>

#include <gl/glew.h>  //  =>   will  #include <GL/gl.h>  #include <GL/glu.h>
#include <string>
#include "modelGL.h"

namespace Win
{
    class ViewGL
    {
    public:
        ViewGL(ModelGL* model);
        ~ViewGL();
		
		// The calling side (to ViewDW) of our callback interface.
//		void DoItA(void* pt2Object, void(*pt2Function)(void* pt2Object, float* color));
		void DoItA(void* pt2Object, void(*pt2Function)(void* pt2Object, glm::vec4* color));

//		Triangle(&tri)[2]
		void Callback_Using_Argument();
	
		void create(HWND handle);
		// create OpenGL rendering context
        bool createContext(HWND handle);
		void closeContext(HWND handle);
		bool hookUpShaders();
		void keyBoardInput(int key);

		void leftButtonDown(int x, int y);
		int  mouseMove(int x, int y);
//		POINT mouseCoords();
//		glm::vec4 color_vec4(int x, int y);
		void drawGL();
        void swapBuffers();
		void resizeWindow(int w, int h);
		void generateGL_Buffers();
		void hello_From_DW(int routine);
		void example0_Run(int run);
		void example1_Run(int run);
		void example2_Run(int run);
		void example3_Run(int run);
		void example4_Run(int run);
		void example5_Run(int run);
		void example6_Run(int run);
		void example7_Run(int run);
		void example8_Run(int run);
		void example9_Run(int run);
		void example10_Run(int run);
		void example11_Run(int run);
		void example12_Run(int run);
		void palette(int run);
// shaders
	class ShaderLoad
	{
	public:
		bool LoadShader(std::string sFile, int a_iType);
		void DeleteShader();
		bool IsLoaded();
		UINT GetShaderID();
	private:

		GLuint uiShader;	// ID of shader 
		int iShaderType;		// GL_VERTEX_SHADER, GL_FRAGMENT_SHADER...
		bool bLoaded;	// Flag shader as compiled and loaded.
		std::string ShaderLog;
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
		// Holds color picked from the palette
		glm::vec4 paletteColor{};

		int mouseX; int mouseY;

		Triangle triangle[2]{};

		int windowWidth;
		int windowHeight;
		bool windowResized;

		int glRoutineNumber;

		GLuint uiVBO[25];	// Buffer handles
		GLuint uiVAO[10];	// Array handles

		//  void glVertexAttribPointer(vertShade, attribBytes, ...
		//  params
		const GLuint vertShade = 0;
		const GLuint fragShade = 1;
		const GLuint vec4_Size = 4;
		// Stride for 2 vec4 attributes close packed.
		const GLuint stride_2_vec4 = 32; 
//		GLuint attribOffset;
		const GLuint offset_1_vec4 = 16;
		const GLuint offset_Zero  = 0;

		// Shader uniform locations.
		GLuint offsetLocation;
		GLuint routineLocation;
		GLuint rotateLoc;
		GLuint projectionMatrixLocation;

		// Handles to Windows device context
		// and openGL rendering context.
		HDC hdc;
		HGLRC hglrc;

		// Handle to glWin;
		HWND glWinHandle; 
		ModelGL* model;
    };
}

#endif
