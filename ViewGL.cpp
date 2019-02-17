///////////////////////////////////////////////////////////////////////////////
// ViewGL.cpp
// ==========
// View component of OpenGL window
//
///////////////////////////////////////////////////////////////////////////////
#define DEBUG_GB

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glew32.lib")

#include <process.h> 
#include <sstream>
#include <iostream>

#include "ViewGL.h"
#include "ViewDW.h"  // for call back
#include "Log.h"
#include "constants.h"

using namespace Win;
using namespace std;

//extern bool renderLoop;
std::vector<glm::vec4> vertices;

const int numberOfVertices = 36;
float perspectiveMatrix[16];
const float fFrustumScale = 1.0f;
#define RIGHT_EXTENT 0.8f
#define LEFT_EXTENT -RIGHT_EXTENT
#define TOP_EXTENT 0.20f
#define MIDDLE_EXTENT 0.0f
#define BOTTOM_EXTENT -TOP_EXTENT
#define FRONT_EXTENT -1.25f
#define REAR_EXTENT -1.75f

#define GREEN_COLOR 0.75f, 0.75f, 1.0f, 1.0f
#define BLUE_COLOR 	0.0f, 0.5f, 7.0f, 1.0f
#define RED_COLOR 1.0f, 0.0f, 0.0f, 1.0f
#define GREY_COLOR 0.8f, 0.8f, 0.8f, 1.0f
#define BROWN_COLOR 0.5f, 0.5f, 0.0f, 1.0f
const float vertexData[] = {
	//Object 1 positions
	LEFT_EXTENT,	TOP_EXTENT,		REAR_EXTENT,
	LEFT_EXTENT,	MIDDLE_EXTENT,	FRONT_EXTENT,
	RIGHT_EXTENT,	MIDDLE_EXTENT,	FRONT_EXTENT,
	RIGHT_EXTENT,	TOP_EXTENT,		REAR_EXTENT,

	LEFT_EXTENT,	BOTTOM_EXTENT,	REAR_EXTENT,
	LEFT_EXTENT,	MIDDLE_EXTENT,	FRONT_EXTENT,
	RIGHT_EXTENT,	MIDDLE_EXTENT,	FRONT_EXTENT,
	RIGHT_EXTENT,	BOTTOM_EXTENT,	REAR_EXTENT,

	LEFT_EXTENT,	TOP_EXTENT,		REAR_EXTENT,
	LEFT_EXTENT,	MIDDLE_EXTENT,	FRONT_EXTENT,
	LEFT_EXTENT,	BOTTOM_EXTENT,	REAR_EXTENT,

	RIGHT_EXTENT,	TOP_EXTENT,		REAR_EXTENT,
	RIGHT_EXTENT,	MIDDLE_EXTENT,	FRONT_EXTENT,
	RIGHT_EXTENT,	BOTTOM_EXTENT,	REAR_EXTENT,

	LEFT_EXTENT,	BOTTOM_EXTENT,	REAR_EXTENT,
	LEFT_EXTENT,	TOP_EXTENT,		REAR_EXTENT,
	RIGHT_EXTENT,	TOP_EXTENT,		REAR_EXTENT,
	RIGHT_EXTENT,	BOTTOM_EXTENT,	REAR_EXTENT,

	//Object 2 positions
	TOP_EXTENT,		RIGHT_EXTENT,	REAR_EXTENT,
	MIDDLE_EXTENT,	RIGHT_EXTENT,	FRONT_EXTENT,
	MIDDLE_EXTENT,	LEFT_EXTENT,	FRONT_EXTENT,
	TOP_EXTENT,		LEFT_EXTENT,	REAR_EXTENT,

	BOTTOM_EXTENT,	RIGHT_EXTENT,	REAR_EXTENT,
	MIDDLE_EXTENT,	RIGHT_EXTENT,	FRONT_EXTENT,
	MIDDLE_EXTENT,	LEFT_EXTENT,	FRONT_EXTENT,
	BOTTOM_EXTENT,	LEFT_EXTENT,	REAR_EXTENT,

	TOP_EXTENT,		RIGHT_EXTENT,	REAR_EXTENT,
	MIDDLE_EXTENT,	RIGHT_EXTENT,	FRONT_EXTENT,
	BOTTOM_EXTENT,	RIGHT_EXTENT,	REAR_EXTENT,

	TOP_EXTENT,		LEFT_EXTENT,	REAR_EXTENT,
	MIDDLE_EXTENT,	LEFT_EXTENT,	FRONT_EXTENT,
	BOTTOM_EXTENT,	LEFT_EXTENT,	REAR_EXTENT,

	BOTTOM_EXTENT,	RIGHT_EXTENT,	REAR_EXTENT,
	TOP_EXTENT,		RIGHT_EXTENT,	REAR_EXTENT,
	TOP_EXTENT,		LEFT_EXTENT,	REAR_EXTENT,
	BOTTOM_EXTENT,	LEFT_EXTENT,	REAR_EXTENT,

	//Object 1 colors
	GREEN_COLOR,
	GREEN_COLOR,
	GREEN_COLOR,
	GREEN_COLOR,

	BLUE_COLOR,
	BLUE_COLOR,
	BLUE_COLOR,
	BLUE_COLOR,

	RED_COLOR,
	RED_COLOR,
	RED_COLOR,

	GREY_COLOR,
	GREY_COLOR,
	GREY_COLOR,

	BROWN_COLOR,
	BROWN_COLOR,
	BROWN_COLOR,
	BROWN_COLOR,

	//Object 2 colors
	RED_COLOR,
	RED_COLOR,
	RED_COLOR,
	RED_COLOR,

	BROWN_COLOR,
	BROWN_COLOR,
	BROWN_COLOR,
	BROWN_COLOR,

	BLUE_COLOR,
	BLUE_COLOR,
	BLUE_COLOR,

	GREEN_COLOR,
	GREEN_COLOR,
	GREEN_COLOR,

	GREY_COLOR,
	GREY_COLOR,
	GREY_COLOR,
	GREY_COLOR,
};
ViewGL::ShaderLoad shVertex, shFragment;
ViewGL::Shader spMain;


ViewGL::ViewGL(ModelGL* model):
	model(model),  hdc(0), hglrc(0), glRoutineNumber(-1) {}

ViewGL::~ViewGL(){ }

void ViewGL::create(HWND hwnd){ 
#ifdef DEBUG_GB
	log(L"ViewGL::create((HWND hwnd)");
#endif
	glWinHandle = hwnd;
    return;
}

bool ViewGL::hookUpShaders(){ // initialize OpenGL states and scene

		// Note relative path is not the same for running in VS and from taskbar
	// so we use the explicit path here
shVertex.LoadShader("C:\\Users\\pstan\\source\\repos\\Proj3dBook\\src\\Shade\\shader.vert", GL_VERTEX_SHADER);
shFragment.LoadShader("C:\\Users\\pstan\\source\\repos\\Proj3dBook\\src\\Shade\\shader.frag", GL_FRAGMENT_SHADER);
// shVertex.LoadShader("..\\shaders\\shader.vert", GL_VERTEX_SHADER);
// shFragment.LoadShader("..\\shaders\\shader.frag", GL_FRAGMENT_SHADER);

//	Proj3dBook\\src\\Shade\\shader.frag
//	shVertex.LoadShader(".\\Shade\\shader.vert", GL_VERTEX_SHADER);
//	shFragment.LoadShader(".\\Shade\\shader.frag", GL_FRAGMENT_SHADER);

	spMain.CreateProgram();
	spMain.AddShaderToProgram(&shVertex);
	spMain.AddShaderToProgram(&shFragment);
	spMain.LinkProgram();
	spMain.UseProgram();
	shVertex.DeleteShader();  // once the shader objects have been
	shFragment.DeleteShader(); // linked no longer need them.
	return TRUE;
}

//void Win::ViewGL::DoItA(void* pt2Object, void(*pt2Function)(void* pt2Object, float* color)){
	void Win::ViewGL::DoItA(void* pt2Object, void(*pt2Function)(void* pt2Object, glm::vec4* color)) {

	Win::log(L"Win::ViewGL::DoItA");
	//float myFloat;
	//myFloat = paletteColor.r;
	//pt2Function(pt2Object, &myFloat); // make callback
	//myFloat = paletteColor.g;
	//pt2Function(pt2Object, &myFloat); // make callback
	//myFloat = paletteColor.b;
	//pt2Function(pt2Object, &myFloat); // make callback

	pt2Function(pt2Object, &paletteColor);
}


void ViewGL::keyBoardInput(int key) {
	Win::log(L"keyBoardInput key = %i", key);
// a = 65 d = 68 S = 83  w = 87  z= 90 x = 88
//   -x     x     - y      y      z     -z
//	if (key = 87)  model->rotateParams();

}
void Win::ViewGL::Callback_Using_Argument()
{
	Win::log(L"ViewGL::Callback_Using_Argument()");
	// 1. instantiate object of ViewDW
	ViewDW objA;		// need default ctor for this 
	// 2. call ’DoItA’ for <objA>
	DoItA((void*)&objA, ViewDW::Wrapper_To_Call_Display);
}
void Win::ViewGL::leftButtonDown(int x, int y)
{
#ifdef DEBUG_GB
	Win::log(L"ViewGL::leftButtonDown x = %i y = %i ", x, y);
#endif

// Let user pick a color from void ViewGL::palette(int run)
	if (glRoutineNumber == constants::PALETTE) {
		RECT rc;
		GetClientRect(glWinHandle, &rc);
		model->returnColor( x, y, rc.right, rc.bottom, paletteColor);
		Win::log(L"color.rgba =  %f  %f  %f  %f", paletteColor.r, paletteColor.g, paletteColor.b, paletteColor.a);
		Callback_Using_Argument();
	}
}

int ViewGL::mouseMove( int x, int y){
//	Win::log(L"ViewGL::mouseMove x = %i y = %i ", x, y);
	mouseX = x; mouseY = y;
	return 0;
}

bool ViewGL::ShaderLoad::LoadShader(string sFile, int a_iShaderType)
{
Win::log(L"ModelGL::LoadShader() a_iShaderType = %i", a_iShaderType);

	FILE* fp = fopen(sFile.c_str(), "rt"); // r = read  t = text

	if(!fp){
		Win::log(L"LoadShader() failed to load file");
		return FALSE;
	}

	// Get all lines from a file
	vector<string> sLines;
	char sLine[255];
	while(fgets(sLine, 255, fp))sLines.push_back(sLine);
	fclose(fp);

	const char** sProgram = new const char*[ (int)sLines.size() ];

	for( int i = 0; i <(int)sLines.size(); i++ )
		sProgram[i] = sLines[i].c_str(); 
// get a name for the shader	
	uiShader = glCreateShader(a_iShaderType); // vertex, geometry, fragment
// Loads source code into shader named uishader
	glShaderSource(uiShader, (int)sLines.size(), sProgram, NULL);
	glCompileShader(uiShader);

	delete[] sProgram;

	int iCompilationStatus;
	glGetShaderiv(uiShader, GL_COMPILE_STATUS, &iCompilationStatus);

	if(iCompilationStatus == GL_FALSE){
		if( a_iShaderType == GL_VERTEX_SHADER ){
			Win::log(L"LoadShader() VERTEX_SHADER iCompilationStatus == GL_FALSE");}
		if( a_iShaderType == GL_FRAGMENT_SHADER ){
			Win::log(L"LoadShader() FRAGMENT_SHADER iCompilationStatus == GL_FALSE");}
		GLint log_length = 0;
		glGetShaderiv(uiShader, GL_INFO_LOG_LENGTH, &log_length);
		Win::log(L"GL_INFO_LOG_LENGTH = %i",log_length);
		string glsl_Log;
		char* log = (char*)malloc(log_length);
		glGetShaderInfoLog(uiShader, log_length, NULL, log);
		glsl_Log.clear();
		glsl_Log.append(log);
		free(log);

		if(log_length > 60){ // break up glsl_Log into lines for Log Window
			string temp;  
			int index = 0;
			while( index < log_length - 60 ){
					temp.clear();
					temp.append( glsl_Log, index,  60 );
					index = index + 60;
					Win::log(temp.c_str());
			}
		temp.clear();
		temp.append( glsl_Log, glsl_Log.size() - 60,  60 );
		Win::log(temp.c_str());
		}
		else Win::log(glsl_Log.c_str()); // have one line ShaderInfoLog
		return false;
	}// if(iCompilationStatus == GL_FALSE)
	iShaderType = a_iShaderType;
	bLoaded = TRUE;

	return TRUE;
}
void ViewGL::ShaderLoad::DeleteShader()
{
	if(!IsLoaded())return;
	bLoaded = false;
	glDeleteShader(uiShader);
}
bool ViewGL::ShaderLoad::IsLoaded()
{
	return bLoaded;
}
UINT ViewGL::ShaderLoad::GetShaderID()
{
	return uiShader;
}
void ViewGL::Shader::CreateProgram()
{
	uiProgram = glCreateProgram();
 
}
void ViewGL::Shader::DeleteProgram()
{
	if(!bLinked)return;
	bLinked = false;
	glDeleteProgram(uiProgram);
}
bool ViewGL::Shader::AddShaderToProgram(ShaderLoad* shShader)
{
	if(!shShader->IsLoaded())return false;

	glAttachShader(uiProgram, shShader->GetShaderID());

	return true;
}
bool ViewGL::Shader::LinkProgram()
{
	glLinkProgram(uiProgram);
	int iLinkStatus;
	glGetProgramiv(uiProgram, GL_LINK_STATUS, &iLinkStatus);
	bLinked = iLinkStatus == GL_TRUE;
	return bLinked;
}
void ViewGL::Shader::UseProgram()
{
	if(bLinked)glUseProgram(uiProgram);

}
void ViewGL::generateGL_Buffers(){
	#ifdef DEBUG_GB
	log(L"ViewGL::generateGL_Buffers()");
#endif

	glGenVertexArrays(3, uiVAO);
	glGenBuffers(2, uiVBO);

}

// Renders a a full window color palette.
// 36 rows x 37 columns  RGB run from  [0.0 - 1.0] in .1 increments.
// That is 11 values of R G and B so 11^3 = 1331 colors in all.
void ViewGL::palette(int run) {
	if (!run)
	{
		Win::log(L"void ViewGL::palette ");
		Win::Rectangle colorPalette[1331];
		model->colorPalette(colorPalette);
		routineLocation = glGetUniformLocation(spMain.GetProgramID(),
			"routineNumber");
		glUniform1i(routineLocation, 14);
		glBindVertexArray(uiVAO[0]);
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(colorPalette), &colorPalette, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, 0);
		glVertexAttribPointer(fragShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)(offset_1_vec4));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);


		glClearColor(0.7f, 0.3f, 0.2f, 1.0);
	}
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(uiVAO[0]);
	glDrawArrays(GL_TRIANGLES, 0, 7986);
	glBindVertexArray(0);
}
// Hello Triangle!
// We ignore the color attribute of the Triangle struct here and
// just use the xyzw attribute.  We let the fragment shader do the 
// color.
void ViewGL::example0_Run(int run) {

	if (!run)
	{
		Triangle verts{};
		model->exampleTri(verts);
		routineLocation = glGetUniformLocation(spMain.GetProgramID(),
			"routineNumber");
		glUniform1i(routineLocation, 0);
		glBindVertexArray(uiVAO[0]);
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts), &verts, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, 0);
		glVertexAttribPointer(fragShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)(offset_1_vec4));
		glBindVertexArray(0);

		glClearColor(0.7f, 0.3f, 0.2f, 1.0);
	}

	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(uiVAO[0]);
	glDrawArrays(GL_TRIANGLES, offset_Zero, 3);
	glBindVertexArray(0);
}
// Hello Triangle!
// We ignore the color attribute of the Triangle struct here and
// just use the xyzw attribute.  We let the fragment shader do the 
// color.

void ViewGL::example12_Run(int run) {

	if (!run)
	{
		
		Line trivit[3]{};
		Triangle verts{};
		model->exampleTri(verts);
//		model->triangle(verts);
		model->rgbTriAxis(trivit);
		Win::log(L"model->rgbTriAxis(trivit)trivit[1].p[0].rgba.r = %f", trivit[1].p[0].rgba.g);
	
		routineLocation = glGetUniformLocation(spMain.GetProgramID(),
			"routineNumber");
		glUniform1i(routineLocation, 12);
		rotateLoc = glGetUniformLocation(spMain.GetProgramID(),"rotate");
		glBindVertexArray(uiVAO[0]);
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts), &verts, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, 0);
		glVertexAttribPointer(fragShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)(offset_1_vec4));
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);


		glBindVertexArray(uiVAO[1]);
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(trivit), &trivit, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, 0);
		glVertexAttribPointer(fragShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)(offset_1_vec4));
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glLineWidth(2.0f);
//		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glClearColor(0.7f, 0.3f, 0.2f, 1.0);
	}

	glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(model->rotate()));
	   
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(uiVAO[0]);
	glDrawArrays(GL_TRIANGLES, offset_Zero, 3);
	glBindVertexArray(uiVAO[1]);
	glDrawArrays(GL_LINES, offset_Zero, 6);
	glBindVertexArray(0);

}
// Using glBufferSubData to move two triangles independently.
void ViewGL::example1_Run(int run){

	if( !run ) 
	{
		model->exampleSubData(triangle);
		routineLocation = glGetUniformLocation(spMain.GetProgramID(), "routineNumber");
		glUniform1i(routineLocation, 1);

		glBindVertexArray(uiVAO[0]);
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), &triangle[0], GL_STREAM_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)offset_Zero);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0);
	}

	static float inc(0.0015f);
	for (int i = 0; i < 3; i++) {
		triangle[0].p[i].xyzw.x += inc;
		triangle[1].p[i].xyzw.y += inc;
	}

	if (triangle[0].p[0].xyzw.x > 1.0f) inc = -0.003f;
	if (triangle[0].p[0].xyzw.x < -1.0f) inc = 0.003f;

	glClear(GL_COLOR_BUFFER_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
	glBindVertexArray(uiVAO[0]);
	//  The glBufferSubData function can update only a portion of the
	//  buffer object's memory. The second parameter to the function is the byte
	//	offset into the buffer object to begin copying to, 
	//  and the third parameter is the number of bytes to copy.
	//  The fourth parameter is our array of
	//	bytes to be copied into that location of the buffer object.
	//	The last line of the function is simply unbinding the buffer object
	glBufferSubData(GL_ARRAY_BUFFER, offset_Zero, sizeof(triangle), &triangle[0]);
	glDrawArrays(GL_TRIANGLES, offset_Zero, 6);
	glBindVertexArray(0);
}

//offsetLocation = glGetUniformLocation(theProgram, "offset");
// Moves a triangle by updating offset uniform and having shader 
// compute the position offset.
void ViewGL::example2_Run(int run) {
#ifdef DEBUG_GB
	//	log(L"ViewGL::example2_Run");
#endif 

	if ( !run )
	{
		Triangle verts{};
		model->exampleTri(verts);
		routineLocation = glGetUniformLocation(spMain.GetProgramID(),
			"routineNumber");
		glUniform1i(routineLocation, 2);
		offsetLocation = glGetUniformLocation(spMain.GetProgramID(), "offset");

		glBindVertexArray(uiVAO[0]);
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts), &verts, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)offset_Zero);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glClearColor(0.9f, 0.5f, 0.4f, 1.0);
	}

	static float fxOffset(0.0f);	static float fyOffset(0.0f);
	static float inc(0.0015f);

	if (fyOffset > 1.0f) inc = -0.0015f;
	if (fyOffset < -1.0f) inc = 0.0015f;

	fxOffset += inc;	fyOffset += inc;
	glUniform2f(offsetLocation, fxOffset, fyOffset);

	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(uiVAO[0]);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
}

// C:\Users\pstan\Mckesson\McKessonSamples\McKesson
// \Tut 04 Objects at Rest\OrthoCube.cpp
void ViewGL::example3_Run(int run) {
#ifdef DEBUG_GB
	//	log(L"ViewGL::example3_Run");
#endif 

	if (!run)
	{
		Triangle tri[12]{};
		Point dim_color[6]{};

		// specify the dimensions of our box.
		dim_color[0].xyzw = glm::vec4(0.2f, 0.2f, 0.4f, 1.0f);

		// specify the colors of each face of our box.
		dim_color[0].rgba = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // front
		dim_color[1].rgba = glm::vec4(0.5f, 0.4f, 0.3f, 1.0f); // back
		dim_color[2].rgba = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // top
		dim_color[3].rgba = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); // bottom
		dim_color[4].rgba = glm::vec4(0.5f, 0.5f, 0.1f, 1.0f); // right
		dim_color[5].rgba = glm::vec4(0.1f, 0.2f, 0.9f, 1.0f); // left

		model->box(dim_color, tri);
		//glm::vec3 yaxis(1.0f, 0.0f, 0.0f);
		//glm::mat4 rot;
	 //   rot = glm::rotate(glm::mat4(1.0),  static_cast<float>(pi)/2.0f, yaxis);
//		rot = glm::rotate(glm::mat4(1.0), 0.2f, yaxis);

		//for( int i = 0; i < 12; i++ ){ for (int j = 0; j < 3; j++){
		//		tri[i].p[j].xyzw = rot * tri[i].p[j].xyzw;
		//	}}

		offsetLocation = glGetUniformLocation(spMain.GetProgramID(), "offset");
		glUniform2f(offsetLocation, 0.3f, 0.3f);
		routineLocation = glGetUniformLocation(spMain.GetProgramID(), "routineNumber");
		glUniform1i(routineLocation, 3);

		glBindVertexArray(uiVAO[0]);
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(tri), &tri[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, 0);
		glVertexAttribPointer(fragShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)(offset_1_vec4));
		glBindVertexArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CW);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	}
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(uiVAO[0]);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(uiVAO[0]);
}

// loading a glUniformMatrix4fv for perpective projection
// // C:\Users\pstan\Mckesson\McKessonSamples\McKesson\
// Tut 04 Objects at Rest\MatrixPerspective.cpp
void ViewGL::example4_Run(int run) {
#ifdef DEBUG_GB
	//	log(L"ViewGL::example4_Run");
#endif 

	if (!run)
	{
		Triangle tri[12]{};
		Point dim_color[6]{};
		
		// specify the x,y,z, dimensions of our box.
		dim_color[0].xyzw = glm::vec4(0.5f, 0.5f, 1.5f, 1.0f);

		// specify the colors of each face of our box.
		dim_color[0].rgba = glm::vec4(0.7f, 0.2f, 0.0f, 1.0f); // front
		dim_color[1].rgba = glm::vec4(0.5f, 0.4f, 0.3f, 1.0f); // back
		dim_color[2].rgba = glm::vec4(0.5f, 0.0f, 0.0f, 1.0f); // top
		dim_color[3].rgba = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); // bottom
		dim_color[4].rgba = glm::vec4(0.5f, 0.5f, 0.1f, 1.0f); // right
		dim_color[5].rgba = glm::vec4(0.1f, 0.2f, 0.6f, 1.0f); // left

		model->box(dim_color, tri);
		glm::mat4 trans;
		glm::vec3 move(0.0f, 0.0f, -3.0f);
		trans = glm::translate(glm::mat4(1.0), move);

        // push the box 3 units into the -z direction
		for (int i = 0; i < 12; i++)
		{
			for (int j = 0; j < 3 ; j++)
			{
			//	tri[i].p[j].xyzw.z -= 3.0f;
				tri[i].p[j].xyzw = trans*tri[i].p[j].xyzw;
			}
		}

		// Send x and y offsets to shader.vert
		offsetLocation = glGetUniformLocation(spMain.GetProgramID(), "offset");
		glUniform2f(offsetLocation, 0.6f, 0.6f);
		routineLocation = glGetUniformLocation(spMain.GetProgramID(), "routineNumber");
		glUniform1i(routineLocation, 4);

		float fFrustumScale = 1.0f; float fzNear = 0.5f; float fzFar = 9.0f;
		float theMatrix[16];
		memset(theMatrix, 0, sizeof(float) * 16);
		theMatrix[0] = fFrustumScale;
		theMatrix[5] = fFrustumScale;
		theMatrix[10] = (fzFar + fzNear) / (fzNear - fzFar);  
		theMatrix[14] = (2 * fzFar * fzNear) / (fzNear - fzFar);
		theMatrix[11] = -1.0f;
		projectionMatrixLocation = glGetUniformLocation(spMain.GetProgramID(), "projectionMatrix");
		glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, theMatrix);

		glBindVertexArray(uiVAO[0]);
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(tri), &tri[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, 0);
		glVertexAttribPointer(fragShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)(offset_1_vec4));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
//		glDisable(GL_CULL_FACE);
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CW);
		glCullFace(GL_BACK);
//		glFrontFace(GL_CW);
		//glCullFace(GL_BACK);
//		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//POINT, LINE, FILL
		glClearColor(0.0f, 0.3f, 0.0f, 0.0f);
	}
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(uiVAO[0]);
//	glDrawArrays(GL_TRIANGLES, 18, 6);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void ViewGL::example5_Run(int run) {
#ifdef DEBUG_GB
	//	log(L"ViewGL::example5_Run");
#endif 

	if (!run)
	{
		GLshort indexData[24] = {	0, 2, 1, 
									3, 2, 0,
									4, 5, 6,
									6, 7, 4,
									8, 9, 10,
									11, 13, 12,
									14, 16, 15,
									17, 16, 14,
		};

		offsetLocation = glGetUniformLocation(spMain.GetProgramID(), "offset2");
		routineLocation = glGetUniformLocation(spMain.GetProgramID(), "routineNumber");
		glUniform1i(routineLocation, 5);
		float fzNear = 1.0f; float fzFar = 3.0f;

		memset(perspectiveMatrix, 0, sizeof(float) * 16);
		perspectiveMatrix[0] = fFrustumScale;
		perspectiveMatrix[5] = fFrustumScale;
		perspectiveMatrix[10] = (fzFar + fzNear) / (fzNear - fzFar);
		perspectiveMatrix[14] = (2 * fzFar * fzNear) / (fzNear - fzFar);
		perspectiveMatrix[11] = -1.0f;
		projectionMatrixLocation = glGetUniformLocation(spMain.GetProgramID(), "projectionMatrix");
		glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, perspectiveMatrix);

// InitializeVertexBuffer()
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uiVBO[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


//InitializeVertexArrayObjects()
		glBindVertexArray(uiVAO[1]);
		size_t colorDataOffset = sizeof(float) * 3 * numberOfVertices;
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]); // vertex buffer 
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)colorDataOffset);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uiVBO[1]); // index buffer
		glBindVertexArray(0);

		glBindVertexArray(uiVAO[2]);
		size_t posDataOffset = sizeof(float) * 3 * (numberOfVertices / 2);
		colorDataOffset += sizeof(float) * 4 * numberOfVertices/2;
		//Use the same buffer object previously bound to GL_ARRAY_BUFFER.
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)posDataOffset);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)colorDataOffset);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uiVBO[1]);
		glBindVertexArray(0);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CW);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	}

	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(uiVAO[1]);
	glUniform3f(offsetLocation, 0.0f, 0.0f, 0.0f);
	glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(uiVAO[2]);
	glUniform3f(offsetLocation, 0.0f, 0.0f, -1.0f);
	glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);

}
 
//glDrawArrays(GL_POINTS, 0, 1);
void ViewGL::example6_Run(int run) {

	if (!run) // Init here - just once - on first call.
	{
		Point first_Point{};	    // coord               //color
		first_Point = model->point(0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);

		routineLocation = glGetUniformLocation(spMain.GetProgramID(),
			"routineNumber");
		glUniform1i(routineLocation, 6);
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Point), &first_Point, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glPointSize(125.0f);
		glClearColor(0.3f, 0.3f, 0.3f, 1.0);
	}

	glClear(GL_COLOR_BUFFER_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, 0);
	glVertexAttribPointer(fragShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)(offset_1_vec4));
	glDrawArrays(GL_POINTS, 0, 1);
	glDisableVertexAttribArray(0); 
	glDisableVertexAttribArray(1);
}

// glDrawArrays(GL_LINES, 0, 2) example
void ViewGL::example7_Run(int run) {

	if (!run)
	{
		Point point_0{};
		Point point_1{};			// pos					// color
		point_0 = model->point(0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 1.0f);

		// This will not touch the point.color component only the .xyzw part.
		point_1 = -point_0;

		Line first_Line{};
		first_Line = model->line(point_0, point_1);

		routineLocation = glGetUniformLocation(spMain.GetProgramID(),
			"routineNumber");
		glUniform1i(routineLocation, 7);

		glBindVertexArray(uiVAO[0]);
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Line), &first_Line, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, 0);
		glVertexAttribPointer(fragShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)(offset_1_vec4));
		glBindVertexArray(0);
		glLineWidth(5.0f);
		glClearColor(0.3f, 0.3f, 0.3f, 1.0);
	}
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(uiVAO[0]);
	glDrawArrays(GL_LINES, 0, 2);
	glBindVertexArray(0);
}


//void ModelGL::prism(Point(&specs)[5], Triangle(&tri)[8]) {
void ViewGL::example11_Run(int run) {

	if (!run)
	{
		Triangle tri[8]{};
		Point dim_color[5]{};

		// specify the x,y,z, dimensions of our box.
		dim_color[0].xyzw = glm::vec4(-0.2f, 0.0f, 0.0f, 1.6f);
		dim_color[1].xyzw = glm::vec4( 0.0f, 0.8f, 0.0f, 0.0f);
		dim_color[2].xyzw = glm::vec4(0.2f, 0.0f, 0.0f, 0.0f);

		// specify the colors of each face of our box.
		dim_color[0].rgba = glm::vec4(0.9f, 0.0f, 0.2f, 1.0f); // front
		dim_color[1].rgba = glm::vec4(0.5f, 0.4f, 0.7f, 1.0f); // back
		dim_color[2].rgba = glm::vec4(0.5f, 0.0f, 0.0f, 1.0f); // left
		dim_color[3].rgba = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); // right
		dim_color[4].rgba = glm::vec4(0.5f, 0.5f, 0.1f, 1.0f); // bottom


		model->prism(dim_color, tri);
		glm::mat4 trans;
		glm::vec3 move(0.0f, 0.0f, -3.0f);
		trans = glm::translate(glm::mat4(1.0), move);

		// push the box 3 units into the -z direction
		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				//Win::log(L"tri[%i].p[%i].xyzw = %f  %f  %f  %f ",i, j, tri[i].p[j].xyzw.x, 
				//	tri[i].p[j].xyzw.y, tri[i].p[j].xyzw.z, tri[i].p[j].xyzw.w);

				//Win::log(L"tri[%i].p[%i].rgbd = %f  %f  %f  %f ", i, j, tri[i].p[j].rgba.r,
				//	tri[i].p[j].rgba.g, tri[i].p[j].rgba.b, tri[i].p[j].rgba.a);

				tri[i].p[j].xyzw = trans * tri[i].p[j].xyzw;
			}
		}


		// Send x and y offsets to shader.vert
		offsetLocation = glGetUniformLocation(spMain.GetProgramID(), "offset");
		glUniform2f(offsetLocation, 0.6f, 0.6f);
		routineLocation = glGetUniformLocation(spMain.GetProgramID(), "routineNumber");
		glUniform1i(routineLocation, 11);

		float fFrustumScale = 1.0f; float fzNear = 0.5f; float fzFar = 9.0f;
		float theMatrix[16];
		memset(theMatrix, 0, sizeof(float) * 16);
		theMatrix[0] = fFrustumScale;
		theMatrix[5] = fFrustumScale;
		theMatrix[10] = (fzFar + fzNear) / (fzNear - fzFar);
		theMatrix[14] = (2 * fzFar * fzNear) / (fzNear - fzFar);
		theMatrix[11] = -1.0f;
		projectionMatrixLocation = glGetUniformLocation(spMain.GetProgramID(), "projectionMatrix");
		glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, theMatrix);

		glBindVertexArray(uiVAO[0]);
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(tri), &tri[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, 0);
		glVertexAttribPointer(fragShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)(offset_1_vec4));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
				glDisable(GL_CULL_FACE);
		//glEnable(GL_CULL_FACE);
		//glFrontFace(GL_CW);
		//glCullFace(GL_BACK);
		//		glFrontFace(GL_CW);
				//glCullFace(GL_BACK);
		//		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				//POINT, LINE, FILL
		glClearColor(0.0f, 0.3f, 0.0f, 0.0f);
	}
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(uiVAO[0]);
	//	glDrawArrays(GL_TRIANGLES, 18, 6);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}


// glDrawArrays(GL_TRIANGLES, 0, 3) example
void ViewGL::example8_Run(int run) {

	if (!run)
	{
		Point point_0{};
		Point point_1{};
		Point point_2{}; 

		// Check your winding order here.
		point_0.xyzw = glm::vec4(-0.5f, -0.33f, 0.0f, 1.0f);
		point_1.xyzw = glm::vec4( 0.0f,  0.5f, 0.0f, 1.0f);
		point_2.xyzw = glm::vec4( 0.5f, -0.33f, 0.0f, 1.0f);

		point_0.rgba = point_1.rgba = point_2.rgba = glm::vec4(0.7f, 0.3f, 0.0f, 1.0f);

		Triangle first_Triangle{};
		first_Triangle = model->triangle(point_0, point_1, point_2);

		routineLocation = glGetUniformLocation(spMain.GetProgramID(),
			"routineNumber");
		glUniform1i(routineLocation, 8);

		glBindVertexArray(uiVAO[0]); 
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Triangle), &first_Triangle, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, 0);
		glVertexAttribPointer(fragShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)(offset_1_vec4));
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glClearColor(0.1f, 0.2f, 0.3f, 1.0);
	}

	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(uiVAO[0]);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
}

// Coordinate system
void ViewGL::example10_Run(int run) {

	if (!run)
	{
		routineLocation = glGetUniformLocation(spMain.GetProgramID(),
			"routineNumber");
		glUniform1i(routineLocation, 10);
//		GLuint MatrixID = glGetUniformLocation(spMain.GetProgramID(), "PVM");

		Point rect{};
		rect.xyzw = glm::vec4(0.0f, 0.0f, 0.0f, 0.25f);
		rect.rgba = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

		Line coordArray[3]{};
		model->coord_System(rect, coordArray);


		// Load the coordArray.
		glBindVertexArray(uiVAO[0]);
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(coordArray), &coordArray[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, 0);
		glVertexAttribPointer(fragShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)(offset_1_vec4));
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glLineWidth(0.1f);
		glClearColor(0.3f, 0.2f, 0.1f, 1.0);
	}

	glm::vec3 yaxis(0.0f, 1.0f, 0.0f);
	glm::vec3 move(0.5f, 0.5f, -1.0f);
	glm::vec3 tdrScale(1.0f, 1.0f, 1.0f);
	float W = static_cast<float>(windowWidth);
	float H = static_cast<float>(windowHeight);
	float angle(0.0f);
//	glm::mat4 rot = glm::rotate(glm::mat4(1.0), glm::radians(angle), yaxis);
	glm::mat4 rot = glm::rotate(glm::mat4(1.0), 0.0F, yaxis);

	glm::mat4 trans = glm::translate(glm::mat4(1.0), move);
	glm::mat4 tdrSize = glm::scale(glm::mat4(1.0), tdrScale);
//	glm::mat4 tdrModel = trans*rot*tdrSize;
	glm::mat4 tdrModel = glm::mat4(1.0f);
	glm::mat4 View = glm::lookAt(
		glm::vec3(4, 3, 30), // Camera is at (4,3,3), in World Space
	//	glm::vec3(4, 3, 3), // Camera is at (4,3,3), in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)); // Head is up. (set to 0, -1, 0 to look upside - down)
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), W/H, 0.1f, 100.0f);
	glm::mat4 pvm = projection * View*tdrModel;
	GLuint MatrixID = glGetUniformLocation(spMain.GetProgramID(), "PVM");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &pvm[0][0]);

	glBindVertexArray(uiVAO[0]);
	glClear(GL_COLOR_BUFFER_BIT);
//	
	glDrawArrays(GL_LINES, 0, 6);
	glBindVertexArray(0);
}

// Rectangle example:
// Creates a monochromatic rectangle with its COM at the origin.
// 
// We use rect.xyzw = glm::vec4(0.25f, 0.5f, 0.0f, 0.0f);
// to specify to model->rectangle(rect, triAr); the size and 
// orientation of the rectangle.  One of the position coordinates
// x, y, or z must be 0.  If z = 0 the rect will be in the XY plane
// rect.xyzw.w gives the offset.  Ex: if rect.xyzw.z = 0; 
// rect.xyzw.w will be the offset of rectangle from the z = 0 plane.
//
//  specs.xyzw  = glm::vec4(x dimension, y dim, z dim, offset);
//  specs.color = color of rectangle
void ViewGL::example9_Run(int run) {

	if (!run)
	{
		Point rect{};
		rect.xyzw = glm::vec4( 0.5f, 0.25f, 0.0f, 0.0f); 
		rect.rgba = glm::vec4(1.0f, 0.0f, 0.3f, 1.0f);

		Triangle triAr[2]{};
		model->rectangle(rect, triAr);
		//		glm::vec3 xaxis(1.0f, 0.0f, 0.0f);
		//		glm::vec3 yaxis(0.0f, 1.0f, 0.0f);
		//		glm::vec3 zaxis(0.0f, 0.0f, 1.0f);
		//		glm::mat4 rot;
////	    rot = glm::rotate(glm::mat4(1.0), -static_cast<float>(pi)/2.0f, xaxis);
//		rot = glm::rotate(glm::mat4(1.0), 0.2f, yaxis); //just rotate - is CW
//		for( int i = 0; i < 2; i++ ){ for (int j = 0; j < 3; j++){
			//	triAr[i].p[j].xyzw = rot * triAr[i].p[j].xyzw;
				//log(L"triAr[%i].p[%i].xyzw.x = %f", i, j, triAr[i].p[j].xyzw.x);
				//log(L"triAr[%i].p[%i].xyzw.y = %f", i, j, triAr[i].p[j].xyzw.y);
				//log(L"triAr[%i].p[%i].xyzw.z = %f", i, j, triAr[i].p[j].xyzw.z);
//			}}

		//Line coordArray[3]{};
		//rect.xyzw = glm::vec4(0.0f, 0.0f, 0.0f, 0.25f);
		//rect.color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		//model->coord_System(rect, coordArray);
		//glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(coordArray), &coordArray[0], GL_STATIC_DRAW);

		routineLocation = glGetUniformLocation(spMain.GetProgramID(),
			"routineNumber");
		glUniform1i(routineLocation, 8);
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(triAr), &triAr[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glClearColor(0.3f, 0.2f, 0.1f, 1.0);
	}

	glClear(GL_COLOR_BUFFER_BIT);
	size_t colorOffset = sizeof(Triangle);
	glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, 0);
	glVertexAttribPointer(fragShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)(offset_1_vec4));
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

void ViewGL::hello_From_DW(int routine) 
{
	glRoutineNumber = routine;
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void ViewGL::drawGL(){

	if(glRoutineNumber == -1 ) return; // have dW_dW mode 

    if(windowResized)
    {
		glViewport(0, 0, windowWidth, windowHeight);
        windowResized = false;
    }

	static int count = -1;
	switch( glRoutineNumber )
	{	
		case 0: { if( count != 0 ){ count = 0; example0_Run(0); } example0_Run(1); break; }
		case 1: { if (count != 1) { count = 1; example1_Run(0); } example1_Run(1); break; }
		case 2: { if (count != 2) { count = 2; example2_Run(0); } example2_Run(1); break; }
		case 3: { if (count != 3) { count = 3; example3_Run(0); } example3_Run(1); break; }
		case 4: { if (count != 4) { count = 4; example4_Run(0); } example4_Run(1); break; }
		case 5: { if (count != 5) { count = 5; example5_Run(0); } example5_Run(1); break; }
		case 6: { if (count != 6) { count = 6; example6_Run(0); } example6_Run(1); break; }
		case 7: { if (count != 7) { count = 7; example7_Run(0); } example7_Run(1); break; }
		case 8: { if (count != 8) { count = 8; example8_Run(0); } example8_Run(1); break; }
		case 9: { if (count != 9) { count = 9; example9_Run(0); } example9_Run(1); break; }
		case 10: { if (count != 10) { count = 10; example10_Run(0); } example10_Run(1); break; }
		case 11: { if (count != 11) { count = 11; example11_Run(0); } example11_Run(1); break; }
		case 12: { if (count != 12) { count = 12; example12_Run(0); } example12_Run(1); break; }
		case 99: { if (count != 99) { count = 99; palette(0); } palette(1); break; }


		default: { // none of the above.
			count = -1;
			glClearColor(0.3f, 0.2f, 0.7f, 1.0);
			glClear(GL_COLOR_BUFFER_BIT );
			glFlush();
		} 
	} // End switch( glRoutineNumber )
}

void ViewGL::resizeWindow(int w, int h) // flag window for a glViewport call
{ 
	Win::log(L"void ViewGL::resizeWindow(int w = %i, int h = %i)", w, h );
    windowWidth = w;		// Get the width/height of the window
    windowHeight = h;
    windowResized = true;	// Flag viewport call
}

bool ViewGL::createContext(HWND handle)
{
	Win::log(L"createContext");
	hdc = ::GetDC(handle);
//	hglrc = ::wglGetCurrentContext();
	hglrc = ::wglCreateContext(hdc);
//	if( !hglrc) return FALSE;
	if( !::wglMakeCurrent(hdc, hglrc)) return FALSE;;
	return TRUE;

}

void ViewGL::closeContext(HWND handle){

    if(!hdc || !hglrc) return;

    // delete DC and RC
    ::wglMakeCurrent(0, 0);   
	::wglDeleteContext(hglrc);
    ::ReleaseDC(handle, hdc);

    hdc = 0;    hglrc = 0;
}

///////////////////////////////////////////////////////////////////////////////
// swap OpenGL frame buffers
///////////////////////////////////////////////////////////////////////////////
void ViewGL::swapBuffers()
{
    ::SwapBuffers(hdc);
}



