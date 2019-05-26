///////////////////////////////////////////////////////////////////////////////
// ViewGL.cpp
// ==========
// View component of 3dBook-Reader OpenGL window
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

ViewGL::ShaderLoad shVertex, shFragment;
ViewGL::Shader spMain;

ViewGL::ViewGL(ModelGL* model):
	model(model),  
	hdc(0), 
	hglrc(0), 
	glRoutineNumber(-1),
	count(-1),
	bLineFrame(FALSE),
	bPointFrame(FALSE),
	n(1),
	triangleCount(0),
	paletteColor(0.0f)
{}
	
ViewGL::~ViewGL(){ }

void ViewGL::create(HWND hwnd){ 
#ifdef DEBUG_GB
	log(L"ViewGL::create((HWND hwnd)");
#endif
	glWinHandle = hwnd;

	char path[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, path);
	workDir = path;

 	log(L"ViewGL::create((HWND hwnd) CurrentDirectory is ...");
	log(path);

    return;
}

bool ViewGL::hookUpShaders(){ 

	std::string shadeFile = workDir;
	shadeFile.append("\\Shade\\shader.vert");
	shVertex.LoadShader(shadeFile, GL_VERTEX_SHADER);

	shadeFile.clear();
	shadeFile = workDir;
	shadeFile.append("\\Shade\\shader.frag");

	shFragment.LoadShader(shadeFile, GL_FRAGMENT_SHADER);

	//shVertex.LoadShader(".\\Shade\\shader.vert", GL_VERTEX_SHADER);
	//shFragment.LoadShader(".\\Shade\\shader.frag", GL_FRAGMENT_SHADER);

	spMain.CreateProgram();
	uiProg = spMain.GetProgramID();
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

//	Win::log(L"Win::ViewGL::DoItA");
	//float myFloat;
	//myFloat = paletteColor.r;
	//pt2Function(pt2Object, &myFloat); // make callback
	//myFloat = paletteColor.g;
	//pt2Function(pt2Object, &myFloat); // make callback
	//myFloat = paletteColor.b;
	//pt2Function(pt2Object, &myFloat); // make callback

	pt2Function(pt2Object, &paletteColor);
}

void Win::ViewGL::Callback_Using_Argument()
{
//	Win::log(L"ViewGL::Callback_Using_Argument()");
	// 1. instantiate object of ViewDW
	ViewDW objA;		// need default ctor for this 
	// 2. call ’DoItA’ for <objA>
	DoItA((void*)&objA, ViewDW::Wrapper_To_Call_Display);
}

//  Toggel lineframe mode & pointframe mode
void Win::ViewGL::keyDown(int key){

	if( key == 'L' ){
		if( bLineFrame ){ bLineFrame = FALSE; bPointFrame = FALSE; }
		else            { bLineFrame = TRUE;  bPointFrame = FALSE; }
	}

	if (key == 'K') {
		if( bPointFrame ){ bPointFrame = FALSE; bLineFrame = FALSE; }
		else             { bPointFrame = TRUE;  bLineFrame = FALSE; }
	}
}

void Win::ViewGL::leftButtonDown(int x, int y)
{
#ifdef DEBUG_GB
//	Win::log(L"ViewGL::leftButtonDown x = %i y = %i ", x, y);
#endif
	static bool bToggle = TRUE;
// Let user pick a color from void ViewGL::palette(int run)
	if( glRoutineNumber == constants::PALETTE ){
		RECT rc = {0,0,0,0};
		GetClientRect(glWinHandle, &rc);
		model->returnColor( x, y, rc.right, rc.bottom, paletteColor);
//		Win::log(L"color.rgba =  %f  %f  %f  %f", paletteColor.r, paletteColor.g, paletteColor.b, paletteColor.a);
		Callback_Using_Argument();
		glRoutineNumber = 24;
		return;
	}
	if( glRoutineNumber == 24) {glRoutineNumber = constants::PALETTE;}

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
	}// End if(iCompilationStatus == GL_FALSE)

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

// Just clear the screen.
void ViewGL::clear(int run) {
	glClearColor(paletteColor.x, paletteColor.y ,paletteColor.z, paletteColor.a);
	glClear(GL_COLOR_BUFFER_BIT );
}

// Renders a a full window color palette.
// 36 rows x 37 columns  RGB run from  [0.0 - 1.0] in .1 increments.
// That is 11 values of R G and B so 11^3 = 1331 colors in all.
void ViewGL::palette(int run) {
	if (!run)
	{
//		Win::log(L"void ViewGL::palette ");
		Win::Rectangle colorPalette[1331];
		model->colorPalette(colorPalette);
		routineLocation = glGetUniformLocation(uiProg,
			"routineNumber");
		glUniform1i(routineLocation, 99);
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

// Renders an equilateral triangle face 
// Inputs: n integer > 0 : T[0].v[0].xyzw.x = static_cast<float>(n);
//         color T[0].v[0].rgba = glm::vec4(.8f, 0.6f, 0.0f, 1.0f);
//
// Output: equilateral triangle grid in the XY, Z=0 plane with n triangular 
// cells along an edge.   
void ViewGL::equilateralFace(int run){

	if( n < 1 ) return;
	
	static UINT triangles = 0;
	if (!run)
	{
//		Win::log(L"ViewGL::equilateralFace(int run) ");
		triangles = 0;
		//      n*n = triangles/face
		//		3*n*n =  verts/face

		numberOfTriangles =  n*n;
		Win::Triangle *T = new Win::Triangle[n*n]();

		// Specify number of cells / face
		T[0].v[0].xyzw.x =  static_cast<float>(n);

		// Specify the cube color

		T[0].v[0].rgba = glm::vec4(.8f, 0.3f, 1.0f, 1.0f);
		model->equilateralFace(T);
		model->setMsize(0.5f);

		routineLocation = glGetUniformLocation(uiProg, "routineNumber");
		glUniform1i(routineLocation, 14);
		rotateLoc = glGetUniformLocation(uiProg, "turnTable");
		glBindVertexArray(uiVAO[0]);
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, n*n * sizeof(Win::Triangle), &T[0], GL_STATIC_DRAW);
		delete[] T;
		glEnableVertexAttribArray(vertShade);
		glEnableVertexAttribArray(fragShade);
		glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, 0);
		glVertexAttribPointer(fragShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)(offset_1_vec4));
		glBindVertexArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glClearColor(0.5f, 0.3f, 0.2f, 1.0);
		glDisable(GL_CULL_FACE);
		//glCullFace(GL_BACK);
		//glFrontFace(GL_CW);
		bLineFrame = TRUE;
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(0.5f);
	}

	glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(model->turnTable()));
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(uiVAO[0]);
	if (triangles < numberOfTriangles) {
		++triangles;
		Sleep(100/n);
	}
//	glDrawArrays(GL_TRIANGLES, offset_Zero, 3 * numberOfTriangles);
	glDrawArrays(GL_TRIANGLES, offset_Zero, 3 * triangles);
	glBindVertexArray(0);
}

// Renders an pentagonal face 
// Inputs: n integer > 0 : T[0].v[0].xyzw.x = static_cast<float>(n);
//         color T[0].v[0].rgba = glm::vec4(.8f, 0.6f, 0.0f, 1.0f);
//
// Output: pentagonal grid in the XY, Z=0 plane with n triangular 
// cells along an edge. 
void ViewGL::pentagonalFace(int run){

	if( n < 1 ) return;
	
	static UINT triangles = 0;
	if (!run)
	{
//		Win::log(L"ViewGL::pentagonFace(int run) step = %i ", step);
		triangles = 0;

		//      n*n = triangles/face
		//		3*n*n =  verts/face

		numberOfTriangles =  5*n*n;
		Win::Triangle *T = new Win::Triangle[5*n*n];

		// Specify number of cells / face
		T[0].v[0].xyzw.x =  static_cast<float>(n);

		// Specify the cube color
		T[0].v[0].rgba = glm::vec4( 0.8f, 0.6f, 0.4f, 1.0f );

		// Specify the step 
		T[0].v[0].xyzw.y = static_cast<float>(step);

		model->pentagonalFace(T);
		model->setMsize(0.5f);

		routineLocation = glGetUniformLocation(uiProg, "routineNumber");
		glUniform1i(routineLocation, 14);
		rotateLoc = glGetUniformLocation(uiProg, "turnTable");
		glBindVertexArray(uiVAO[0]);
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, 5*n*n*sizeof(Win::Triangle), &T[0], GL_STATIC_DRAW);
		delete[] T;
		glEnableVertexAttribArray(vertShade);
		glEnableVertexAttribArray(fragShade);
		glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, 0);
		glVertexAttribPointer(fragShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)(offset_1_vec4));
		glBindVertexArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glClearColor(0.0f, 0.0f, 0.1f, 1.0);
		glDisable(GL_CULL_FACE);
		//glCullFace(GL_BACK);
		//glFrontFace(GL_CW);
		bLineFrame = TRUE;
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(0.5f);
	}

	glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(model->turnTable()));
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(uiVAO[0]);
	if (triangles < numberOfTriangles) {
		++triangles;
		Sleep(100/n);
	}
//	glDrawArrays(GL_TRIANGLES, offset_Zero, 3 * numberOfTriangles);
	glDrawArrays(GL_TRIANGLES, offset_Zero, 3 * triangles);
	glBindVertexArray(0);
}

// Creates a series of animations showing 3, 4, 5, and 6
//  equilateral triangles coming together at a vertex.
void ViewGL::faces(int run) {

	static UINT triangles = 0;
	static float inc = 0.0f;
	static float inc2 = 0.0f;
	int numVerts_Face = 3*n*n;
	if (!run)
	{
		triangles = 0;
		inc = 0;
		n = 6;
		numberOfTriangles = 6*n*n;
		Win::Triangle *T = new Win::Triangle[numberOfTriangles];

		// Specify number of cells / face
		T[0].v[0].xyzw.x = static_cast<float>(n);
		T[0].v[0].xyzw.y = static_cast<float>(step);

		// Specify the mesh color
		T[0].v[0].rgba = glm::vec4(.9f, 0.5f, 0.3f, 1.0f);
		model->faces(T);
		model->setMsize(0.4f);

		routineLocation = glGetUniformLocation(uiProg, "routineNumber");
		glUniform1i(routineLocation, 14);
		rotateLoc = glGetUniformLocation(uiProg, "turnTable");
		glBindVertexArray(uiVAO[0]);
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, 6 * n*n * sizeof(Win::Triangle), &T[0], GL_STATIC_DRAW);
		delete[] T;
		glEnableVertexAttribArray(vertShade);
		glEnableVertexAttribArray(fragShade);
		glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, 0);
		glVertexAttribPointer(fragShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)(offset_1_vec4));
		glBindVertexArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glClearColor(0.1f, 0.0f, 0.1f, 1.0);

			glDisable(GL_CULL_FACE);
			bLineFrame = TRUE;
			//			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glLineWidth(0.5f);

		xyzAxes();
	}
	glBindVertexArray(uiVAO[2]);
	glDrawArrays(GL_LINES, offset_Zero, 6);
	glBindVertexArray(0);

	glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(model->turnTable()));
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(uiVAO[0]);
//	float angle = mConst::pi - acos(1.0f/3.0f); // Tetrahedron - three faces

	float angle= 0.0f; 
	if (step == 2) angle = mConst::tetDiComp;   // Tetrahedron - 3 faces
	if( step == 3 ) angle = mConst::octDiComp;  // octahedron - 4 faces
	if (step == 4) angle = mConst::icosaDiComp; // icosahedron - 5 faces

	glm::vec3 axis[4];
	axis[0] = glm::vec3( 0.5f,  0.866025f, 0.0f);
	axis[1] = glm::vec3(-0.5f,  0.866025f, 0.0f);
	axis[2] = glm::vec3( 0.5f,  0.0f, 0.0f);
	axis[3] = glm::vec3(-0.5f, -0.866025f, 0.0f);


	if( inc < angle ) inc += 0.005f;
 

	glm::mat4 Spin = glm::rotate(glm::mat4(1.0), -inc, axis[0]);
	glm::mat4 Spin2 = glm::rotate(glm::mat4(1.0), -inc, axis[0]);


	if (step != 4 && step != 5 ){
		// F0
		Spin = model->turnTable()*Spin;
		glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(Spin));
		glDrawArrays(GL_TRIANGLES, offset_Zero, numVerts_Face);
	}

	if (step == 2) { 
		// F1
		glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(model->turnTable()));
		glDrawArrays(GL_TRIANGLES, numVerts_Face, numVerts_Face);
		//	F2 - tetra
		Spin = glm::rotate(glm::mat4(1.0), inc, axis[1]);
		Spin = model->turnTable()*Spin;
		glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(Spin));
		glDrawArrays(GL_TRIANGLES, 2* numVerts_Face, numVerts_Face);
	}

	if( step == 3 ){

		// F1
		glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(model->turnTable()));
		glDrawArrays(GL_TRIANGLES, numVerts_Face, numVerts_Face);

		// F2
		Spin = glm::rotate(glm::mat4(1.0), inc, axis[1]);
		Spin = model->turnTable()*Spin;
		glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(Spin));
		glDrawArrays(GL_TRIANGLES, 2*numVerts_Face, numVerts_Face);

		// F3
		Spin2 = glm::rotate(glm::mat4(1.0),  inc, axis[1]);
		Spin = glm::rotate(glm::mat4(1.0),  -inc, axis[2]);
		Spin = model->turnTable()*Spin2*Spin;
		glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(Spin));
		glDrawArrays(GL_TRIANGLES, 3*numVerts_Face, numVerts_Face);
	}

	if( step == 4 ) {

		// F2
		glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(model->turnTable()));
		glDrawArrays(GL_TRIANGLES, 2*numVerts_Face, numVerts_Face);

		// F1
		Spin = glm::rotate(glm::mat4(1.0),  inc, axis[1]); // - +
		Spin = model->turnTable()*Spin;
		glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(Spin));
		glDrawArrays(GL_TRIANGLES, numVerts_Face, numVerts_Face);

		// F0
		Spin2 = glm::rotate(glm::mat4(1.0),    inc, axis[1]); // + +
		Spin = glm::rotate(glm::mat4(1.0),    inc, axis[0]); //  - +
		Spin = model->turnTable()*Spin2*Spin;
		glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(Spin));
		glDrawArrays(GL_TRIANGLES, offset_Zero, numVerts_Face);

		// F3
		Spin = glm::rotate(glm::mat4(1.0), inc, axis[2]);
		Spin = model->turnTable()*Spin;
		glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(Spin));
		glDrawArrays(GL_TRIANGLES, 3* numVerts_Face, numVerts_Face);

		// F4
		Spin2 = glm::rotate(glm::mat4(1.0), inc, axis[2]);
		Spin = glm::rotate(glm::mat4(1.0), -inc, axis[3]);
		Spin = model->turnTable()*Spin2*Spin;
		glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(Spin));
		glDrawArrays(GL_TRIANGLES, 4* numVerts_Face, numVerts_Face);

	}

	if( step == 5 ){
		// Hexagon
		glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(model->turnTable()));
		glDrawArrays(GL_TRIANGLES, offset_Zero, 6*numVerts_Face);

	}
	
//	Draw fixed axes 
	Spin = glm::rotate(glm::mat4(1.0), 0.0f, mConst::yaxis);
	glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(Spin));
	glBindVertexArray(uiVAO[2]);
	glDrawArrays(GL_LINES, offset_Zero, 6);
	glBindVertexArray(0);
}

// Renders a dodecahedron with unit edges
// Inputs: n integer > 0 : T[0].v[0].xyzw.x = static_cast<float>(n);
//         color T[0].v[0].rgba = glm::vec4(.8f, 0.6f, 0.0f, 1.0f);
//
// Output: dodecahedron mesh - center at origin - with n trianglular 
// cells along each edge.
void ViewGL::dodecahedron(int run){

	if (n < 1) n = 1;

	if (!run)
	{
//		Win::log(L"ViewGL::dodecahedron(int run) step = %i ", step);
		numberOfTriangles = 60*n*n;  // 12 faces x 5*n*n/face 
		Win::Triangle *T = new Win::Triangle[numberOfTriangles + 1];
//		Win::Triangle *T = new Win::Triangle[numberOfTriangles]{};

//		Stick on an axis of symmetry
		//	Using the last triangle to draw a Z axis 
		for (int v = 0; v < 3; v++){
			T[numberOfTriangles].v[v].xyzw = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			T[numberOfTriangles].v[v].rgba = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
		}

		T[numberOfTriangles].v[0].xyzw.z  = 1.5f; 
		T[numberOfTriangles].v[1].xyzw.z  = -1.5f;  

		// Specify number of cells / face
		T[0].v[0].xyzw.x = static_cast<float>(n);
	//	T[0].v[0].xyzw.y = static_cast<float>(step);

		T[0].v[0].rgba = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);

		model->dodecahedron(T, step);
		model->setMsize(0.5f);

		routineLocation = glGetUniformLocation(uiProg, "routineNumber");
		glUniform1i(routineLocation, 14);
		rotateLoc = glGetUniformLocation(uiProg, "turnTable");
		glBindVertexArray(uiVAO[0]);
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, (60*n*n + 1)*sizeof(Win::Triangle), &T[0], GL_STATIC_DRAW);

		delete[] T;
		glEnableVertexAttribArray(vertShade);
		glEnableVertexAttribArray(fragShade);
		glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, 0);
		glVertexAttribPointer(fragShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)(offset_1_vec4));
		glBindVertexArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glClearColor(0.0f, 0.0f, 0.1f, 1.0);
		
		// Draw illustration for axial angle.
		if( step == 8 ){

			Win::Line L[4];

			float axAng = asin((2.0f/3.0f)*mConst::sinThirdPi/tan(mConst::pi/5.0f));
			float z = -sin(mConst::pi/5.0f )*cos(axAng);


			L[0].v[0].xyzw = glm::vec4(-cos(mConst::pi/5.0f ), (2.0f/3.0f)*mConst::sinThirdPi*cos(mConst::pi/5.0f), z,  1.0f );
			L[0].v[1].xyzw = glm::vec4( cos(mConst::pi/5.0f ), (2.0f/3.0f)*mConst::sinThirdPi*cos(mConst::pi/5.0f), z,  1.0f );

			L[1].v[0].xyzw = L[0].v[1].xyzw;
			L[1].v[1].xyzw = glm::vec4( 0, -(4.0f/3.0f)*mConst::sinThirdPi*cos(mConst::pi/5.0f), z,  1.0f );

			L[2].v[0].xyzw = L[1].v[1].xyzw;
			L[2].v[1].xyzw = L[0].v[0].xyzw;;

			L[0].v[0].rgba = L[0].v[1].rgba = mConst::red;
			L[1].v[0].rgba = L[1].v[1].rgba = mConst::red;
			L[2].v[0].rgba = L[2].v[1].rgba = mConst::red;

			L[3].v[0].xyzw = mConst::origin4v;;
			L[3].v[1].xyzw = glm::vec4( 0, (2.0f/3.0f)*mConst::sinThirdPi*cos(mConst::pi/5.0f), z,  1.0f );

			L[3].v[0].rgba = L[3].v[1].rgba = mConst::green;

			glBindVertexArray(uiVAO[1]);
			glBindBuffer(GL_ARRAY_BUFFER, uiVBO[1]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(L), &L, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, 0);
			glVertexAttribPointer(fragShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)(offset_1_vec4));
			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glLineWidth(0.5f);
		}
		glDisable(GL_CULL_FACE);
		bLineFrame = TRUE;
		glLineWidth(0.5f);
	}

	// Draw the octahedron mesh
	glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(model->turnTable()));
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(uiVAO[0]);

	if( step == 8 ){

		glDrawArrays(GL_TRIANGLES, offset_Zero, 3*(5*n*n ));
		glDrawArrays(GL_TRIANGLES, 3*(10*n*n ), 3*(5*n*n ));
		glDrawArrays(GL_TRIANGLES, 3*(20*n*n ), 3*(5*n*n ));
		glDrawArrays(GL_TRIANGLES, 3*(60*n*n), 3);

	}
	if( step == 9 ){	
		glDrawArrays(GL_TRIANGLES, offset_Zero, 3*(30*n*n ));
		glDrawArrays(GL_TRIANGLES, 3*(60*n*n), 3);
	}

    else glDrawArrays(GL_TRIANGLES, offset_Zero, 3*(60*n*n + 1));
	
	glBindVertexArray(0);

	if( step == 8 ){
		// draw the tripod
	glBindVertexArray(uiVAO[1]);
	glDrawArrays(GL_LINES, offset_Zero, 8);
	glBindVertexArray(0);
	}
}

// Renders a unit icosahedron 
// Inputs: n integer > 0 : T[0].v[0].xyzw.x = static_cast<float>(n);
//         color T[0].v[0].rgba = glm::vec4(.8f, 0.6f, 0.0f, 1.0f);
//
// Output: icosahedron mesh - center at origin - with n trianglular 
// cells along an edge. 
void ViewGL::icosahedron(int run){

	if (n < 1) n = 1;

	if (!run)
	{
		numberOfTriangles = 20*n*n;
		Win::Triangle *T = new Win::Triangle[numberOfTriangles + 1];

		//	Using the last triangle to draw a Z axis 
		for (int v = 0; v < 3; v++){
			T[numberOfTriangles].v[v].xyzw = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			T[numberOfTriangles].v[v].rgba = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
		}

		T[numberOfTriangles].v[0].xyzw.z  = 1.5f; 
		T[numberOfTriangles].v[1].xyzw.z  = -1.5f;  

		// Specify number of cells / face
		T[0].v[0].xyzw.x = static_cast<float>(n);
		T[0].v[0].xyzw.y = static_cast<float>(step);

		T[0].v[0].rgba = glm::vec4(1.0f, 0.0f, 0.3f, 1.0f);

		model->icosahedron(T);
		model->setMsize(0.5f);
		if( step == 0 ){
			model->setMsize(0.2f);
			for( int t = 0; t < n*n; t++ ){
			   for (int v = 0; v < 3; v++){ 				   
				   T[t].v[v].rgba = T[t + 10*n*n].v[v].rgba = mConst::red;
				   T[t + n*n].v[v].rgba = T[t + 11*n*n].v[v].rgba = mConst::green;
				   T[t + 2*n*n].v[v].rgba = T[t + 12*n*n].v[v].rgba = mConst::gray;
				   T[t + 3*n*n].v[v].rgba = T[t + 13*n*n].v[v].rgba = mConst::blue;
				   T[t + 4*n*n].v[v].rgba = T[t + 14*n*n].v[v].rgba = mConst::white;
				   T[t + 5*n*n].v[v].rgba = T[t + 15*n*n].v[v].rgba = mConst::black;
				   T[t + 6*n*n].v[v].rgba = T[t + 16*n*n].v[v].rgba = glm::vec4(0.5f,0.0f,0.0f,1.0f);
				   T[t + 7*n*n].v[v].rgba = T[t + 17*n*n].v[v].rgba = glm::vec4(0.0f,0.5f,0.0f,1.0f);
				   T[t + 8*n*n].v[v].rgba = T[t + 18*n*n].v[v].rgba = glm::vec4(0.0f,0.0f,0.5f,1.0f);
				   T[t + 9*n*n].v[v].rgba = T[t + 19*n*n].v[v].rgba = glm::vec4(0.4f,0.4f,0.1f,1.0f);
		}}}

		routineLocation = glGetUniformLocation(uiProg, "routineNumber");
		glUniform1i(routineLocation, 14);
		rotateLoc = glGetUniformLocation(uiProg, "turnTable");
		glBindVertexArray(uiVAO[0]);
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, (20*n*n + 1)*sizeof(Win::Triangle), &T[0], GL_STATIC_DRAW);
//		glBufferData(GL_ARRAY_BUFFER, 20*n*n *sizeof(Win::Triangle), &T[0], GL_STATIC_DRAW);

		delete[] T;
		glEnableVertexAttribArray(vertShade);
		glEnableVertexAttribArray(fragShade);
		glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, 0);
		glVertexAttribPointer(fragShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)(offset_1_vec4));
		glBindVertexArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0);

		if( step == 0) {  // texturing
			bLineFrame = FALSE;
			bPointFrame = FALSE;
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(GL_CW);
		}
		else {
			glDisable(GL_CULL_FACE);
			bLineFrame = TRUE;
			glLineWidth(0.5f);
		}
		// xyzAxes();  // Draw the fixed axes CS
	}

	// Draw the octahedron mesh
	glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(model->turnTable()));
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(uiVAO[0]);
	if( step == 7) glDrawArrays(GL_TRIANGLES, offset_Zero, 3*20*n*n);
	else glDrawArrays(GL_TRIANGLES, offset_Zero, 3*(20*n*n + 1));

	glBindVertexArray(0);
}

// Renders a unit octahedron 
// Inputs: n integer > 0 : T[0].v[0].xyzw.x = static_cast<float>(n);
//         color T[0].v[0].rgba = glm::vec4(.8f, 0.6f, 0.0f, 1.0f);
//
// Output: octahedron mesh - center at origin - with n trianglular 
// cells along an edge.
void ViewGL::octahedron(int run){
	if (n < 1) return;// n = 12;

	static int triangles = 0;
	if (!run)
	{

//		log(L"ViewGL::octahedron, n = %i,  step = %i", n, step);
		triangles = 0;
		numberOfTriangles = 8*n*n;
		Win::Triangle *T = new Win::Triangle[numberOfTriangles];

		// Specify number of cells / face
		T[0].v[0].xyzw.x = static_cast<float>(n);
	//	T[0].v[0].xyzw.y = static_cast<float>(step);

		// Specify the mesh color
		// T[0].v[0].rgba = glm::vec4(.8f, 0.0f, 1.0f, 1.0f);
		T[0].v[0].rgba = glm::vec4(0.3f, 0.9f, 1.0f, 1.0f);

		if( step == 7 ) T[0].v[0].rgba = glm::vec4(1.0f, 0.7f, 0.8f, 1.0f);

		model->octahedron(T, step);
		model->setMsize(0.5f);

		routineLocation = glGetUniformLocation(uiProg, "routineNumber");
		glUniform1i(routineLocation, 14);
		rotateLoc = glGetUniformLocation(uiProg, "turnTable");
		glBindVertexArray(uiVAO[0]);
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, 8*n*n*sizeof(Win::Triangle), &T[0], GL_STATIC_DRAW);
		delete[] T;
		glEnableVertexAttribArray(vertShade);
		glEnableVertexAttribArray(fragShade);
		glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, 0);
		glVertexAttribPointer(fragShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)(offset_1_vec4));
		glBindVertexArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glClearColor(0.3f, 0.3f, 0.3f, 1.0);
		if (step == 7) glClearColor(0.8f, 0.6f, 0.2f, 1.0);
		if (step == 5 || step == 6) {  // texturing
			glClearColor(0.0f, 0.0f, 0.0f, 1.0);
			bLineFrame = FALSE;
			bPointFrame = FALSE;
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(GL_CW);
		}
		else {
			glDisable(GL_CULL_FACE);
			bLineFrame = TRUE;
			glLineWidth(0.5f);
		}
		xyzAxes();  // Draw the fixed axes CS
	}

	// Draw the octahedron mesh
	glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(model->turnTable()));
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(uiVAO[0]);
	if (triangles < 8 * n*n && step != 5 && step != 6 ) {++triangles;
	glDrawArrays(GL_TRIANGLES, offset_Zero, 3 * triangles);}
	else glDrawArrays(GL_TRIANGLES, offset_Zero, 3 * 8 * n*n);
	// Draw the fixed axes CS
	if( step != 5 && step != 6 && step != 7){
		glm::mat4 Spin = glm::rotate(glm::mat4(1.0), 0.0f, mConst::yaxis);
		glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(Spin));
		glBindVertexArray(uiVAO[2]);
		glDrawArrays(GL_LINES, offset_Zero, 6);
	}
	glBindVertexArray(0);
}

// Renders a unit tetrahedron 
// Inputs: n integer > 0 : T[0].v[0].xyzw.x = static_cast<float>(n);
//         color T[0].v[0].rgba = glm::vec4(.8f, 0.6f, 0.0f, 1.0f);
//
// Output: tetrahedron mesh - center at origin - with n trianglular 
// cells along an edge. 
void ViewGL::tetrahedron(int run) {

	if( n < 1 ) n = 12;

	static UINT triangles = 0;
	if (!run)
	{
		triangles = 0;
		numberOfTriangles = 4*n*n;
		Win::Triangle *T = new Win::Triangle[numberOfTriangles];

	//	log(L"ViewGL::tetrahedron, n = %i,  step = %i", n, step);

		// Specify number of cells / face
		T[0].v[0].xyzw.x = static_cast<float>(n);
		T[0].v[0].xyzw.y = static_cast<float>(step);

		// Specify the mesh color
		T[0].v[0].rgba = glm::vec4(.5f, 0.4f, 0.7f, 1.0f);
		model->tetrahedron(T);
		model->setMsize(0.5f);

		routineLocation = glGetUniformLocation(uiProg, "routineNumber");
		glUniform1i(routineLocation, 14);
		rotateLoc = glGetUniformLocation(uiProg, "turnTable");
		glBindVertexArray(uiVAO[0]);
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, 4*n*n*sizeof(Win::Triangle), &T[0], GL_STATIC_DRAW);
		delete[] T;
		glEnableVertexAttribArray(vertShade);
		glEnableVertexAttribArray(fragShade);
		glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, 0);
		glVertexAttribPointer(fragShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)(offset_1_vec4));
		glBindVertexArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glClearColor(0.1f, 0.0f, 0.1f, 1.0);

		if( step ==7){  // texturing
			bLineFrame = FALSE;
			bPointFrame = FALSE;
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(GL_CW);
		}
		else{
			glDisable(GL_CULL_FACE);
			bLineFrame = TRUE;
			glLineWidth(0.5f);
		}		
		xyzAxes();  // Draw the fixed axes CS
	}

	glClear(GL_COLOR_BUFFER_BIT);

		// Draw the fixed axes CS
	glm::mat4 Spin = glm::rotate(glm::mat4(1.0), 0.0f, mConst::yaxis);
	glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(Spin));
	glBindVertexArray(uiVAO[2]);
	glDrawArrays(GL_LINES, offset_Zero, 6);
	glBindVertexArray(0);

	// Draw the tetrahedron mesh
	glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(model->turnTable()));
	glBindVertexArray(uiVAO[0]);
	if(  triangles < 4*n*n) ++triangles;
	glDrawArrays(GL_TRIANGLES, offset_Zero, 3 * triangles);
	glBindVertexArray(0);

}

// Renders a unit cube 
// Inputs: n integer > 0 : R[0].v[0].xyzw.x = static_cast<float>(n);
//         color R[0].v[0].rgba = glm::vec4(.8f, 0.6f, 0.0f, 1.0f);
//
// Output: unit cube mesh - center at origin - with n trianglular 
// cells along an edge. 
void ViewGL::cubeMesh(int run) {

	if( n < 1 )  return;

	static int triangles = 0;
	if (!run)
	{
		triangles = 0;
//		log(L"ViewGL::cubeMesh, n = %i,  step = %i", n);

// 		n*n = rects/face
//      2*n*n = triangles/face
//		3*2*n*n = 6n*n = verts/face

   		numberOfTriangles = 12*n*n;
		Rectangle *R = new Rectangle[6*n*n];
		
		// Specify number of cells / face
		R[0].T[0].v[0].xyzw.x = static_cast<float>(n);
		R[0].T[0].v[0].xyzw.y = static_cast<float>(step);
		// Specify the cube color
		R[0].T[0].v[0].rgba = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

		model->cubeMesh(R);
		model->setMsize(0.4f);

		routineLocation = glGetUniformLocation(uiProg, "routineNumber");
		glUniform1i(routineLocation, 14);
		rotateLoc = glGetUniformLocation(uiProg, "turnTable");
		glBindVertexArray(uiVAO[0]);
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, 6*n*n*sizeof(Win::Rectangle), &R[0], GL_STATIC_DRAW);
		delete[] R; 
		glEnableVertexAttribArray(vertShade);
		glEnableVertexAttribArray(fragShade);
		glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, 0);
		glVertexAttribPointer(fragShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)(offset_1_vec4));
		glBindVertexArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);		
		glClearColor(0.3f, 0.0f, 0.0f, 1.0);
		glDisable(GL_CULL_FACE);
		bLineFrame = TRUE;
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(0.5f);
		if (step == 3) {
			bLineFrame = FALSE;
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(GL_CW);
		}
	}

	glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(model->turnTable()));
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(uiVAO[0]);

	if (n < 6 && triangles < 12*n*n) ++triangles;
	else triangles = 12*n*n;

	glDrawArrays(GL_TRIANGLES, offset_Zero, 3*triangles);
	glBindVertexArray(0);
}

// Creates a grid of square cells centered on the origin.
// The xSize of the grid is sent in as R[0].T[0].v[0].xyzw.x. 
// The ySize of the grid is sent in as R[0].T[0].v[0].xyzw.y.
// The color is sent in R[0].T[0].v[0].rgba.
// Using 1D array R[xSize*ySize] as R[x][y] <-> R[ y + x*ySize ].
// The grid R[xSize*ySize] is created centered on the origin in the XY plane.
// The maximum edge dimension is normalized to 1.0f
void ViewGL::grid(int run) {

	if( n < 1 ) n = 1;

	if (!run)
	{
		xCells = n;
		if(step == 2) yCells = n;
		else yCells = 5;
		numberOfTriangles = 6*xCells*yCells;
		Rectangle *R = new Rectangle[ xCells*yCells ];
	 	R[0].T[0].v[0].xyzw.x = static_cast<float>(xCells);
		R[0].T[0].v[0].xyzw.y = static_cast<float>(yCells);
		R[0].T[0].v[0].rgba  = glm::vec4(0.4f, 0.4f, 0.3f, 1.0f);

		model->grid(R);
		model->setMsize(0.5f);
		routineLocation = glGetUniformLocation(uiProg, "routineNumber");
		glUniform1i(routineLocation, 15);
		rotateLoc = glGetUniformLocation(uiProg, "turnTable");
		glBindVertexArray(uiVAO[0]);
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);		
		glBufferData(GL_ARRAY_BUFFER, xCells*yCells *sizeof(Win::Rectangle), &R[0], GL_STATIC_DRAW);
		delete[] R;
		glEnableVertexAttribArray(vertShade);
		glEnableVertexAttribArray(fragShade);
		glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, 0);
		glVertexAttribPointer(fragShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)(offset_1_vec4));
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisable(GL_CULL_FACE);
		glClearColor(0.9f, 0.9f, 0.7f, 1.0);
	}

	glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(model->turnTable()));
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(uiVAO[0]);
	glDrawArrays(GL_TRIANGLES, offset_Zero, numberOfTriangles);
	glBindVertexArray(0);
}

// A fixed set of coordinate axes.
void ViewGL::xyzAxes() {

	Win::Line L[3];
	model->xyzAxes(L);
	glBindVertexArray(uiVAO[2]);
	glBindBuffer(GL_ARRAY_BUFFER, uiVBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(L), &L, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, 0);
	glVertexAttribPointer(fragShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)(offset_1_vec4));
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glLineWidth(0.5f);
}

// Creates a tripod of coordinate system axis vectors centered on the origin.
// The red, green, and blue vectors point along the X, Y and Z axes respectively.
// If the face is in the XY, Z = 0 plane, the Z axis should be pointing straight 
// out of the display and disappear from view.   
void ViewGL::tripod(float scale){ 

	Win::Line axes[3];

	axes[0].v[0].xyzw = mConst::origin4v;
	axes[1].v[0].xyzw = mConst::origin4v;
	axes[2].v[0].xyzw = mConst::origin4v;

	axes[0].v[1].xyzw = glm::vec4(scale*mConst::xaxis, 1.0f);
	axes[1].v[1].xyzw = glm::vec4(scale*mConst::yaxis, 1.0f);
	axes[2].v[1].xyzw = glm::vec4(scale*mConst::zaxis, 1.0f);

	axes[0].v[0].rgba = axes[0].v[1].rgba = mConst::red;
	axes[1].v[0].rgba = axes[1].v[1].rgba = mConst::green;
	axes[2].v[0].rgba = axes[2].v[1].rgba = glm::vec4(0.6f, 0.6f, 1.0f, 1.0f);

	glBindVertexArray(uiVAO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, uiVBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes), &axes, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, 0);
	glVertexAttribPointer(fragShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)(offset_1_vec4));
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glLineWidth(0.5f);
}

// T.v[0].rgba goes out specifying the color.
// Writes a unit side equilateral triangle into T 
// The triangle is in the XY plane with its center at the origin.
// The base is along the line Y = -tan(pi/6)/2  = -0.288675
// The X = 0 vertex is at Y = sin(pi/3) - tan(pi/6)/2 = 2sin(pi/3)/3 = 0.577350f
void ViewGL::equilateral(int run) {
#ifdef DEBUG_GB
//		log(L"ViewGL::equilateral(int run)");
#endif 

	if (!run)
	{
		Win::Triangle T;
		// specify the color .
		T.v[0].rgba = glm::vec4(0.2f, 0.1f, 0.5f, 1.0f);
		model->equilateral(T);

		routineLocation = glGetUniformLocation(uiProg, "routineNumber");
		glUniform1i(routineLocation, 3);
		rotateLoc = glGetUniformLocation(uiProg, "turnTable");
		glBindVertexArray(uiVAO[0]);
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(T), &T, GL_STATIC_DRAW);

		glEnableVertexAttribArray(vertShade);
		glEnableVertexAttribArray(fragShade);
		glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, 0);
		glVertexAttribPointer(fragShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)(offset_1_vec4));
		glBindVertexArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		float triPodScale = 1.0f;

		tripod(triPodScale);

		glDisable(GL_CULL_FACE);
		bLineFrame = FALSE;
		bPointFrame = FALSE;
		glClearColor(0.3f, 0.0f, 0.0f, 0.0f);
	}

	glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(model->turnTable()));
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(uiVAO[0]);
	glDrawArrays(GL_TRIANGLES, offset_Zero, 3);
	glBindVertexArray(0);
	// draw the tripod
	glBindVertexArray(uiVAO[1]);
	glDrawArrays(GL_LINES, offset_Zero, 6);
	glBindVertexArray(0);
}

// Creates a box centered on the origin.
// R[0]v[0].xyzw is sent to specify the dimensions of the box.  
// R[0].T[0].v[0].xyzw - xyz are the xyz
// 
// The color of each face is sent in on R[i]v[0].rgba 
// R[0] front, R[1] back, R[2] top, R[3] bottom, R[4] right ,R[5] left
void ViewGL::box(int run) {
#ifdef DEBUG_GB
	//	log(L"ViewGL::box(int run)");
#endif 
//	static UINT triangles = 0;
	if (!run)
	{
		Win::Rectangle R[6];
		// specify the dimensions of our box.
		R[0].T[0].v[0].xyzw = glm::vec4(0.6f, 0.6f, 1.2f, 1.0f);

		// specify the colors of each face of our box.
		R[0].T[0].v[0].rgba = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // front
		R[1].T[0].v[0].rgba = glm::vec4(0.5f, 0.4f, 0.3f, 1.0f); // back
		R[2].T[0].v[0].rgba = glm::vec4(0.7f, 0.5f, 0.5f, 1.0f); // top
		R[3].T[0].v[0].rgba = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); // bottom
		R[4].T[0].v[0].rgba = glm::vec4(0.5f, 0.5f, 0.1f, 1.0f); // right
		R[5].T[0].v[0].rgba = glm::vec4(0.1f, 0.6f, 0.6f, 1.0f); // left

		model->box(R);
		model->setMsize(0.5f);

		routineLocation = glGetUniformLocation(uiProg, "routineNumber");
		glUniform1i(routineLocation, 3);
		rotateLoc = glGetUniformLocation(uiProg, "turnTable");
		glBindVertexArray(uiVAO[0]);
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(R), &R[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(vertShade);
		glEnableVertexAttribArray(fragShade);
		glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, 0);
		glVertexAttribPointer(fragShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)(offset_1_vec4));
		glBindVertexArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CW);
		bLineFrame = FALSE;
		bPointFrame = FALSE;
	//	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	}

	glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(model->turnTable()));
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(uiVAO[0]);
	glDrawArrays(GL_TRIANGLES, offset_Zero, 3*12);
	glBindVertexArray(uiVAO[0]);
}


// Rectangle example:
// Draws a monochromatic rectangle.
// 
// We call model->rectangle( triAr) to model the rectangle.
//
// Win::Rectangle R{}; is an "in out" parameter 
// We specify the dimensions, color, orientation, and offset of 
// the rectangle in the "in" parameter: R.T[0].v[0]. 
// 
// color is specified in R.T[0].v[0].rgba = glm::vec4(1.0f, 0.0f, 0.3f, 1.0f);
//
// orientation, dimensions and offset are specified in
//
//    Ex:  R.T[0].v[0].xyzw = glm::vec4( 0.25f, 0.5f, 0.0f, -0.05f);
// 
// One of the x,y,z components must be zero; this determines the orientation.
//
// Ex: if R.T[0].v[0].xyzw.z = 0, the rectangle will be in the Z = 0, XY plane. 
// The rectangle will have dimension 
//
//         R.T[0].v[0].xyzw.x = 0.25  by   R.T[0].v[0].xyzw.y = 0.5
//
//  The Z axis will pass through its center.
//
//  The rectangle will be offset vert.xyzw.w -0.05 from the origin.  In other words; 
//  it will lie in the z = -0.05 plane.
//
//  The back side of the rectangle is the side seen looking out from the 
//  origin.  It will have counter-clockwise winding order.
//
//  The  rectangle R.T[0].v[0] "out" Rectangle R
//  
void ViewGL::rectangle(int run) {
	 
	if (!run)
	{
		
		Win::Rectangle R;

		R.T[0].v[0].xyzw = glm::vec4(0.5f, 0.25f, 0.0f, 0.0f);
		R.T[0].v[0].rgba = glm::vec4(0.7f, 0.4f, 0.1f, 1.0f);
		model->rectangle(R);
		model->setMsize(1.5f);

		routineLocation = glGetUniformLocation(uiProg,
			"routineNumber");
		glUniform1i(routineLocation, 9);
		rotateLoc = glGetUniformLocation(uiProg, "turnTable");

		glBindVertexArray(uiVAO[0]);
		glBindBuffer(GL_ARRAY_BUFFER, uiVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(R), &R, GL_STATIC_DRAW);
		glEnableVertexAttribArray(vertShade);
		glVertexAttribPointer(vertShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, 0);
		glEnableVertexAttribArray(fragShade);
		glVertexAttribPointer(fragShade, vec4_Size, GL_FLOAT, GL_FALSE, stride_2_vec4, (void*)(offset_1_vec4));

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisable(GL_CULL_FACE);
		bLineFrame = FALSE;
		bPointFrame = FALSE;
		//glEnable(GL_CULL_FACE);
		//glFrontFace(GL_CW);
		//glCullFace(GL_BACK);		
		glClearColor(0.5f, 0.5f, 0.5f, 1.0);
	}

	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(uiVAO[0]);
	glUniformMatrix4fv(rotateLoc, 1, GL_FALSE, glm::value_ptr(model->turnTable()));
	glDrawArrays(GL_TRIANGLES, offset_Zero, 6);
	glBindVertexArray(0);
}

void ViewGL::hello_From_DW(int routine, std::wstring& link, int source)
{
//	Win::log(L"ViewGL::hello_From_DW routine = %i  source = %i", routine, source);

	count = -1;

	if(source == 0 ){
		glRoutineNumber = routine;
		step = 0;
	}

	if (source == 1) {
	//	log(link);		
		if( link == L"rectangular box" ) {glRoutineNumber = 3; triangleCount = 12;  return; }
		if (link == L"equilateral") { glRoutineNumber = 16;  return; }
		if (link == L"equilateralFace"){ glRoutineNumber = 17;
			if(routine > 0 ) n = routine; else n = 1;  return; }
		if (link == L"pentagonalFace"){ glRoutineNumber = 27;
			if(routine > 0 ) n = routine; else n = 1;  return; }
		if (link == L"rectangle") { glRoutineNumber = 10;  return; }
		if (link == L"renderOrder") { glRoutineNumber = 17;  return; }
		if (link == L"rotate-copy") { glRoutineNumber = 18;	n = routine; step = 1;   return; }
		if (link == L"translate-rotate") {glRoutineNumber = 18; n = routine; step = 2;   return;}
		if (link == L"Create P1") { glRoutineNumber = 18; n = routine; step = 3;   return; }
		if (link == L"Invert P1") { glRoutineNumber = 18; n = routine; step = 4;  return; }
		if (link == L"Join P0-P1"){ glRoutineNumber = 18; n = routine; step = 5;  return; }
		if (link == L"Upright")    { glRoutineNumber = 18; n = routine; step = 6;  return; }
		if (link == L"texturing") { glRoutineNumber = 18; n = routine; step = 7;   return; }
		if (link == L"three equilateral faces") { glRoutineNumber = 19; step = 2;  return; }
		if (link == L"four faces") { glRoutineNumber = 19; step = 3;  return; }
		if (link == L"five faces") { glRoutineNumber = 19; step = 4;  return; }
		if (link == L"six faces") { glRoutineNumber = 19; step = 5;  return; }
		if (link == L"cubeMesh") { glRoutineNumber = 14; n = routine;  step = 0; return; }
		if (link == L"spherify") { glRoutineNumber = 14; n = routine;  step = 1; return; }
		if (link == L"roofedCylinder") { glRoutineNumber = 14; n = routine;  step = 2; return; }
		if (link == L"texture") { glRoutineNumber = 14; n = routine;  step = 3; return; }
		if (link == L"rectangle(R[0])") { glRoutineNumber = 10; n = routine; step = 2;  return; }
		if (link == L"grid(R)") { glRoutineNumber = 15; n = routine; step = 2;  return; }
		if (link == L"octahedron F0") { glRoutineNumber = 20; n = routine; step = 0;  return; }
		if (link == L"octahedron P0") { glRoutineNumber = 20; n = routine; step = 1;  return;}
		if (link == L"P0 rotate") { glRoutineNumber = 20; n = routine; step = 2;  return; }
		if (link == L"P0 copy") { 
			
			glRoutineNumber = 20; n = routine; step = 3; 
				//	log(L"P0 copy, n = %i,  step = %i", n, step);
			return; }
		if (link == L"P0 P1 copy") { glRoutineNumber = 20; n = routine; step = 4;  return; }
		if (link == L"tex1") { glRoutineNumber = 20; n = routine; step = 5;  return; }
		if (link == L"tex2") { glRoutineNumber = 20; n = routine; step = 6;  return; }
		if (link == L"octaSphere") { glRoutineNumber = 20; n = routine; step = 7;  return; }
		if (link == L"P0 attached") { glRoutineNumber = 21; n = routine; step = 1;  return; }
		if (link == L"2pi/5") { glRoutineNumber = 21; n = routine; step = 2;  return; }
		if (link == L"4pi/5") { glRoutineNumber = 21; n = routine; step = 3;  return; }
		if (link == L"6pi/5") { glRoutineNumber = 21; n = routine; step = 4;  return; }
		if (link == L"8pi/5") { glRoutineNumber = 21; n = routine; step = 5;  return; }
		if (link == L"invert") { glRoutineNumber = 21; n = routine; step = 6;  return; }
		if (link == L"icosasphere") { glRoutineNumber = 21; n = routine; step = 7;  return; }
		if (link == L"equilateral grid"){ glRoutineNumber = 27; n = routine; step = 1;  return; }
		if (link == L"scale it symmetrically"){ glRoutineNumber = 27; n = routine; step = 2;  return; }
		if (link == L"rotate/copies"){ glRoutineNumber = 27; n = routine; step = 3;  return; }
		if (link == L"sin(π/5)/2"){ glRoutineNumber = 28; n = routine; step = 4;  return; }
		if (link == L"3π/5"){

			glRoutineNumber = 28; n = routine; step = 5; 
		//	Win::log(L"ViewGL::3pi/5  step = %i", step);
		
			return; }
		if (link == L"Rotating F1"){ glRoutineNumber = 28; n = routine; step = 6;  return; }
		if (link == L"translate-rotate P0"){ glRoutineNumber = 28; n = routine; step = 7;  return; }
		if (link == L"three faces forming a vertex"){ glRoutineNumber = 28; n = routine; step = 8;  return; }
		if (link == L"translate B0"){ glRoutineNumber = 28; n = routine; step = 9;  return; }
		if (link == L"invert B0"){ glRoutineNumber = 28; n = routine; step = 10;  return; }

	}
}

void ViewGL::drawGL(){

	if( glRoutineNumber == -1 ) return; // have dW_dW mode

	// bPointFrame & bLineFrame Set in void Win::ViewGL::keyDown(int key){
	if( bPointFrame ){
		glPointSize(0.5f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}
	
	else if( bLineFrame ){
		glLineWidth(0.5f);
		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} 

//	else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );
    else{		
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CW);
		glPolygonMode(GL_FRONT, GL_FILL );
	}

    if(windowResized)
    {
		glViewport(0, 0, windowWidth, windowHeight);
        windowResized = false;
    }

	switch( glRoutineNumber )
	{	
		case 0: { if( count != 0){ count = 0; n = 10;  step = 2; cubeMesh(0);} cubeMesh(1); break; }	
		case 1: { if (count != 1){ count = 1; n = 20;  step = 6; octahedron(0);} octahedron(1); break; }
		case 3: { if (count != 3){ count = 3; box(0); } box(1); break; }
		case 4: { if (count != 4){ count = 4; n = 8; step = 7; tetrahedron(0);} tetrahedron(1); break; }
		case 5: { if (count != 5){ count = 5; step = 2; faces(0); } faces(1); break; }
		case 6: { if (count != 6){ count = 6; step = 3; faces(0); } faces(1); break; }
		case 7: { if (count != 7){ count = 7; step = 4; faces(0); } faces(1); break; }
		case 8: { if (count != 8){ count = 7; step = 4; faces(0); } faces(1); break; }
		case 9: { if (count != 9){ count = 9; n=20; step = 1; cubeMesh(0); } cubeMesh(1);; break; }
		case 10: { if (count != 10){ count = 10; rectangle(0); } rectangle(1); break; }
		case 14: { if (count != 14){ count = 14; cubeMesh(0); } cubeMesh(1); break; }
		case 15: { if (count != 15){ count = 15; grid(0); } grid(1); break; }
		case 16: { if (count != 16){ count = 16; equilateral(0); } equilateral(1); break; }
		case 17: { if (count != 17){ count = 17; equilateralFace(0); } equilateralFace(1); break; }
		case 18: { if (count != 18){ count = 18; tetrahedron(0); } tetrahedron(1); break; }
		case 19: { if (count != 19){ count = 19; faces(0); } faces(1); break; }
		case 20: { if (count != 20){ count = 20; octahedron(0); } octahedron(1); break; }
		case 21: { if (count != 21){ count = 21; icosahedron(0);  } icosahedron(1); break; }
		case 24: { if (count != 24){ count = 24; clear(0); } clear(1); break; }
		case 25: { if (count != 25){ count = 25; step = 4; faces(0); } faces(1); break; }
		case 26: { if (count != 26){ count = 26; step = 7; n = 8; octahedron(0); } octahedron(1); break; }
		case 27: { if (count != 27){ count = 27; pentagonalFace(0); } pentagonalFace(1); break; }
		case 28: { if (count != 28){ count = 28; dodecahedron(0); } dodecahedron(1); break; }
		case 99: { if (count != 99){ count = 99; palette(0); } palette(1); break; }

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
	//Win::log(L"void ViewGL::resizeWindow(int w = %i, int h = %i)", w, h );
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



