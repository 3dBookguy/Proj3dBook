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

// callback stuff		
		// The calling side (to ViewDW) of our callback interface.
//		void DoItA(void* pt2Object, void(*pt2Function)(void* pt2Object, float* color));
		void DoItA(void* pt2Object, void(*pt2Function)(void* pt2Object, glm::vec4* color));
		void Callback_Using_Argument();
	
		void create(HWND handle);
        bool createContext(HWND handle);
		void closeContext(HWND handle);
		bool hookUpShaders();
		void keyDown(int key);
		void leftButtonDown(int x, int y);
		int  mouseMove(int x, int y);
		void drawGL();
        void swapBuffers();
		void resizeWindow(int w, int h);
		void generateGL_Buffers();
		void hello_From_DW(int routine, std::wstring& link, int sourceFlag);
		void clear(int run);
		void box(int run);
		void rectangle(int run);
		void equilateral(int run);
		void equilateralFace(int run);
		void pentagonalFace(int run);
		void tetrahedron(int run);
		void octahedron(int run);
		void icosahedron(int run);
		void dodecahedron(int run);
		void faces(int run);
		void grid(int run);
		void cubeMesh(int run);
		void palette(int run);
		void xyzAxes();
		void tripod(float scale);
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

// Render loop 
		int windowWidth;
		int windowHeight;
		bool windowResized;
		// "wire frame" flags
		bool bLineFrame;
		bool bPointFrame;

	//	static int count;
		int count;

// GL declarations

		//	grid params
		UINT xCells;
		UINT yCells;

		//GLint n;  // Input for mesh functions
		//GLuint step;
		//GLuint numberOfTriangles;
		//GLuint triangleCount;

		int n;  // Input for mesh functions
		int step;
		int numberOfTriangles;
		int triangleCount;


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



// Shader 

		// Shader program
		UINT uiProg;

		// for shader switch - case block
		int glRoutineNumber;
		GLuint routineLocation;

		// uniform locations
		GLuint rotateLoc;
		GLuint offsetLocation;
		GLuint projectionMatrixLocation;

// Handles and pointers
		//to Windows device context
		// and openGL rendering context.
		HDC hdc;
		HGLRC hglrc;

		// Handle to glWin;
		HWND glWinHandle; 
		ModelGL* model;
    };
}

#endif
