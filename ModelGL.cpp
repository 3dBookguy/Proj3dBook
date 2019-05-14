/*
						ModelGL.cpp

Handles calls from ViewGL for rendering data.


*/

#define DEBUG_GB

#include "Log.h"
#include "ModelGL.h"

using namespace Win;

ModelGL::ModelGL()
{ 

	std::srand(static_cast<unsigned int>(std::time(nullptr)));
	Mem.xAngle = Mem.yAngle = Mem.zAngle = 0.0f;
	Mem.rotSpeed = 0.01f;
	Mem.size.x = Mem.size.y = Mem.size.z = 1.0f;
	Mem.sizeM = glm::mat4(1.0f);
	Mem.xRot = glm::mat4(1.0f);
	Mem.yRot = glm::mat4(1.0f);	
	Mem.zRot == glm::mat4(1.0f);
}

//  Rotate - scale model using keyboard input 
void ModelGL::keyDown(int key) {

	if( key == 'S' || key == 'W' )
	{ 
		if( key == 'S' ) Mem.xAngle += Mem.rotSpeed;
		else Mem.xAngle -= Mem.rotSpeed;
	}

	else if( key == 'D' || key == 'A' )
	{
		if( key == 'D' ) Mem.yAngle += Mem.rotSpeed;
		else Mem.yAngle -= Mem.rotSpeed;
	}

	else if( key == 'Z' || key == 'X' )
	{
		if( key == 'Z' ) Mem.zAngle += Mem.rotSpeed;
		else Mem.zAngle -= Mem.rotSpeed;
	}

// Note: Enabling swizzle expressions will massively increase
// the size of your binaries and the time it takes to compile them!
	else if( key == 'B' || key == 'V' )
	{
		if( key == 'B' ) Mem.size.x += Mem.rotSpeed;
		else Mem.size.x -= Mem.rotSpeed;
		Mem.size.y = Mem.size.z = Mem.size.x;
	}
}

// Set the scale to display the model.
void ModelGL::setMsize(float scale){

		Mem.size.y = Mem.size.z = Mem.size.x = scale;
}

// Sculptors turntable to rotate and scale the model.
glm::mat4 ModelGL::turnTable(){

	Mem.sizeM = glm::scale(glm::mat4(1.0f),  Mem.size);
	Mem.xRot = glm::rotate(glm::mat4(1.0f), Mem.xAngle, mConst::xaxis);
	Mem.yRot = glm::rotate(glm::mat4(1.0f), Mem.yAngle, mConst::yaxis);
	Mem.zRot = glm::rotate(glm::mat4(1.0f), Mem.zAngle, mConst::zaxis);

	return Mem.sizeM*Mem.xRot*Mem.yRot*Mem.zRot;
}

// Model a point.
Vertex ModelGL::point(float x, float y, float z, float w, float r, float g, float b, float a) {

	Vertex point;
	point.xyzw  = glm::vec4( x,  y,  z,  w );
	point.rgba = glm::vec4( r, b, g, a);
	return point;
}

// Model a line.
Line ModelGL::line(Vertex begin, Vertex end) {

	Line line;
	line.v[0] = begin;
	line.v[1] = end;
	return line;
}


// T.v[0].rgba comes in specifying the color.
// Writes a unit side equilateral triangle into T 
// The triangle is in the XY plane with its center at the origin.
// The base is along the line Y = -tan(pi/6)/2  = -0.288675
// The X = 0 vertex is at Y = sin(pi/3) - tan(pi/6)/2 = 2sin(pi/3)/3 = 0.577350f
void ModelGL::equilateral(Triangle &T) {

	T.v[1].rgba = T.v[2].rgba = T.v[0].rgba;

	T.v[0].xyzw = glm::vec4(-0.5f, -0.288675f, 0.0f, 1.0f);
	T.v[1].xyzw = glm::vec4(0.0f, 0.577350f, 0.0f, 1.0f);
	T.v[2].xyzw = glm::vec4(0.5f, -0.288675f, 0.0f, 1.0f);
		
}

// Creates a grid of equilateral cells centered on the origin.
// n - The Triangles/Edge count of the grid is sent in as T[0].v[0].xyzw.x. 
// The color is sent in T[0].v[0].rgba.
// The grid is normalized to side = 1.0f
void ModelGL::equilateralFace( Triangle *&T) {

	int N = static_cast<int>(T[0].v[0].xyzw.x);
	if(N <= 0 ) return;

// seed Triangle base and height
    float h = 1.0f/T[0].v[0].xyzw.x*mConst::sinThirdPi;
	float b = 1.0f / T[0].v[0].xyzw.x; // for base
	float bHalf = b / 2.0f;

// Set the color and .xyzw.w components for the whole face
	T[0].v[0].xyzw = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	for( int t = 0; t < N*N;  t++ ){for( int v = 0; v < 3; v++){ T[t].v[v]  = T[0].v[0]; }}

// Set the seed triangle position
	T[0].v[1].xyzw.x = b / 2.0f; T[0].v[1].xyzw.y = h; T[0].v[1].xyzw.z = 0.0f;
	T[0].v[2].xyzw.x = b; T[0].v[2].xyzw.y = 0.0f; T[0].v[2].xyzw.z = 0.0f;

	glm::mat4 Spin = glm::rotate( glm::mat4(1.0), mConst::pi/3.0f, mConst::zaxis );
	glm::mat4 x_Lat;

	int count(0);	 
	for (int row = 0; row < N; row++) {
		count = 0;
		for (int n = 2 * N*row - row * row; n < 2 * N*row - row * row + 2 * N - 2 * row - 1; n++) {
			if (count % 2 == 0 ) { // triangle is even in the row - just translate triangle is even in the row				
				x_Lat = glm::translate(glm::mat4(1.0f), glm::vec3((count + row)*bHalf, row*h, 0.0f));
				for (int i = 0; i < 3; i++){ T[n].v[i].xyzw = x_Lat*T[0].v[i].xyzw; }
			}
			else { // triangle is odd in the row - rotate then translate 
				x_Lat = glm::translate(glm::mat4(1.0f), glm::vec3((count + 1 + row)*bHalf, row*h, 0.0f));
				for (int i = 0; i < 3; i++){ T[n].v[i].xyzw = x_Lat*Spin*T[0].v[i].xyzw; }
			}
			count++;
		}		
	}
}

// Creates a pentagonal grid of triangular cells centered on the origin.
// n - The Triangles/Edge count of the grid is sent in as T[0].v[0].xyzw.x. 
// The color is sent in T[0].v[0].rgba.
// The grid is normalized to side = 1.0f
void ModelGL::pentagonalFace( Triangle *&T) {

	int N = static_cast<int>(T[0].v[0].xyzw.x);
	int step = static_cast<int>(T[0].v[0].xyzw.y);
	if (N <= 0) return;

// Create F0  -  First we get an equilateral grid	
	equilateralFace(T);
	if( step == 1 ) return;
    // We have an equilateral grid with a base on the X axis and a vertex at the 
	// origin. We want an isoceles grid with two equal angles of 3pi/10 and
	// the other angle, 2pi/5 at the origin.  A rotation of the equilateral grid
	// around Z by -pi/6 and a scaling along the X axis will accomplish this. 

    // We rotate the equilateral grid around the Z axis then scale it 
	// symetrically along the X axis. By the scale factor.

	float isoScaleFactor = 0.5f*mConst::sin3Pi_10/(mConst::sinPi_5*mConst::sinThirdPi);

	glm::mat4 Spin = glm::rotate( glm::mat4(1.0),  -mConst::pi/6.0f, mConst::zaxis );
	glm::vec3 scaleVec = glm::vec3( isoScaleFactor, 1.0f  , 1.0f );
	glm::mat4 isoScale = glm::scale( glm::mat4(1.0f),  scaleVec );
	for( int n = 0; n < N*N; n++ ){
		for( int i = 0; i < 3; i++ ){
			T[n].v[i].xyzw = isoScale*Spin*T[n].v[i].xyzw;
		}
	}
	if( step == 2 ) return;

//  A series of four rotate/copies by 2pi/5 around Z completes the the first face:
//	F0 centered at the origin.  
	for( int rot = 1; rot < 5; rot++ ){
		glm::mat4 Spin = glm::rotate( glm::mat4(1.0),
						static_cast<float>(rot)*mConst::twoPi_5, mConst::zaxis );
		for(int n = 0; n < N*N; n++){
			for(int i = 0; i < 3; i++){
				T[ n + rot*N*N ].v[i].xyzw = Spin*T[n].v[i].xyzw;
				T[ n + rot*N*N ].v[i].rgba = T[n].v[i].rgba;
			}
		}
	}
}

// Creates a dodecahedron mesh centered on the origin with 5n² triangle/face.
// Calls ModelGL::pentagonalFace( Triangle *&T ). 
// n -  is sent in as T[0].v[0].xyzw.x. 
// The color is sent in T[0].v[0].rgba.
// The edge length is normalized to 1.
void ModelGL::dodecahedron(Win::Triangle *&T){

	int N = static_cast<int>(T[0].v[0].xyzw.x);
	int step = static_cast<int>(T[0].v[0].xyzw.y);
	if (N <= 0) return;

// F0 --- First Face
	pentagonalFace(T);
	 
	//  pentagonalFace(T);  gives us a pentagonal grid  centered on the origin, we need 
	//	to move the face so a vertex is centered on the origin. A bit of trig
	//	reveals a translation of 1/2sin(π/5)  along the X axis will do the trick.
    float transX = 1.0f/(2*mConst::sinPi_5);

	glm::mat4 x_Lat = glm::translate(glm::mat4(1.0f),  glm::vec3( transX, 0.0f, 0.0f));
	for(int n = 0; n < 5*N*N; n++){
		for(int i = 0; i < 3; i++){
			T[n].v[i].xyzw = x_Lat*T[n].v[i].xyzw;
		}
	}
 	if( step == 4 ) return;

// P0 ---  First Petal

	//A rotate/copy by 3π/5 around Z creates F1 attached to F0.  
	glm::mat4 Spin = glm::rotate( glm::mat4(1.0),
					3.0f*mConst::pi/5.0f, mConst::zaxis );
	for(int n = 0; n < 5*N*N; n++){
		for(int i = 0; i < 3; i++){
			T[ n + 5*N*N ].v[i].xyzw = Spin*T[n].v[i].xyzw;
			T[ n + 5*N*N ].v[i].rgba = T[n].v[i].rgba;
		}
	}
	if( step == 5 ) return;

// Rotating F1 by the dihedral angle around the common
// edge of F0 and F1  will give us P0.
	glm::vec3 petalAxis =  glm::vec3( cos(3.0f*mConst::pi/10.0f),  mConst::sin3Pi_10 , 0.0f);
	Spin = glm::rotate( glm::mat4(1.0), -mConst::dodecaDiComp, petalAxis );
	for(int n = 0; n < 5*N*N; n++){
		for(int i = 0; i < 3; i++){
			T[ n + 5*N*N ].v[i].xyzw = Spin*T[n + 5*N*N].v[i].xyzw;
//				T[ n + 5*N*N ].v[i].rgba = T[n].v[i].rgba;
		} 
	}

	if( step == 6 ) return;

// Alright, let's just go ahead and translate-rotate P0 to get a 
// vertex on the Z axis. edge of F0 and F1  will give us P0.
    transX = 0.5f/(sin(2.0f*mConst::pi/10.0f))*(1 + cos(2.0f*mConst::pi/10.0f));
    float transY = sin(2.0f*mConst::pi/10.0f)*sin(mConst::twoPi_5)*sin(mConst::pi/10.0f);
	x_Lat = glm::translate( glm::mat4(1.0f),  glm::vec3( -transX, 0.5f, 0.0f ));
	Spin = glm::rotate( glm::mat4(1.0), -3.0f*mConst::pi/10.0f, mConst::zaxis );
	float axAng = asin((2.0f/3.0f)*mConst::sinThirdPi/tan(mConst::pi/5.0f));

	//glm::mat4 Spin2 = glm::rotate( glm::mat4(1.0),
	//				-(mConst::pi/2.0f - axAng), mConst::xaxis );

	glm::mat4 Spin2 = glm::rotate( glm::mat4(1.0),
					(axAng - mConst::pi/2.0f), mConst::xaxis );

	for( int n = 0; n < 10*N*N; n++ ){
		for( int i = 0; i < 3; i++ ){
			T[n].v[i].xyzw = Spin2*Spin*x_Lat*T[n].v[i].xyzw;
		}   
	}

	if( step == 7 ) return;


	Spin = glm::rotate( glm::mat4(1.0), 2.0f*mConst::pi/3.0f, mConst::zaxis );

	for( int n = 0; n < 10*N*N; n++ ){
		for( int i = 0; i < 3; i++ ){
		T[n + 10*N*N].v[i].xyzw = Spin*T[n].v[i].xyzw;
		T[n + 10*N*N ].v[i].rgba = T[n].v[i].rgba;
		}
	}


	Spin = glm::rotate( glm::mat4(1.0), 4.0f*mConst::pi/3.0f, mConst::zaxis );
	for( int n = 0; n < 10*N*N; n++ ){
		for( int i = 0; i < 3; i++ ){
			T[n + 20*N*N].v[i].xyzw = Spin*T[n].v[i].xyzw;
			T[n + 20*N*N ].v[i].rgba = T[n].v[i].rgba;
		}
	}

	if( step == 8 ) return;

// B0 is created at this point now we center it.

	x_Lat = glm::translate( glm::mat4(1.0f),  glm::vec3( 0.0f, 0.0f, mConst::dodecaSphere ));
	for( int n = 0; n < 30*N*N; n++ ){
		for( int i = 0; i < 3; i++ ){
			T[n].v[i].xyzw = x_Lat*T[n].v[i].xyzw;
		}
	}

// B1 --- copy/invert B0 to create B1 and the complete dodecahedron.

	Win::Vertex swap{};
		for( int n = 0; n < 30*N*N; n++ ){
			for( int i = 0; i < 3; i++ ){
				T[n+ 30*N*N ].v[i] = -T[n].v[i];
				T[n + 30*N*N ].v[i].rgba = mConst::red;
			}
		swap = T[n+30*N*N].v[2];
		T[n+30*N*N].v[2] = T[n+30*N*N].v[0];
		T[n+30*N*N].v[0] = swap;
	}	
}


// Creates a icosahedron mesh centered on the origin with n² triangle/face.
// Calls ModelGL::equilateralFace( Triangle *&T ). 
// n -  is sent in as T[0].v[0].xyzw.x. 
// The color is sent in T[0].v[0].rgba.
// The edge length is normalized to 1.
void ModelGL::icosahedron(Win::Triangle *&T){
	 
	int N = static_cast<int>(T[0].v[0].xyzw.x);
	int step = static_cast<int>(T[0].v[0].xyzw.y);
	if (N <= 0) return;

	// F0 --- First Face
	equilateralFace(T);

// Create F1  from the F0 verts: rotate F0 pi/3 around z then 
// bend the dihedral to T0 icosaDiComp
	glm::vec3 axis = glm::vec3 (0.5f, mConst::sinThirdPi, 0.0f);
	glm::mat4 Spin = glm::rotate( glm::mat4(1.0),  mConst::thirdPi, mConst::zaxis );  //just rotate
	glm::mat4 Spin2 = glm::rotate( glm::mat4(1.0), -mConst::icosaDiComp, axis );
	for(int n = 0; n < N*N; n++){
		for(int i = 0; i < 3; i++){
			T[ n + N*N ].v[i].xyzw = Spin2*Spin*T[n].v[i].xyzw;
			T[ n + N*N ].v[i].rgba = T[n].v[i].rgba;
		}
	}

// xlate everything by -1.0 along x and bend down by epsilon from the xy plane
	float epsilon = 0.6523581f;
	glm::mat4 x_Lat = glm::translate(glm::mat4(1.0), glm::vec3(-1.0f, 0.0f, 0.0f));
	Spin2 = glm::rotate( glm::mat4(1.0), -mConst::thirdPi, mConst::zaxis );
	Spin = glm::rotate( glm::mat4(1.0), -epsilon, mConst::xaxis );  //just rotate
	for(int n = 0; n < 2*N*N; n++){
		for(int i = 0; i < 3; i++){
			T[n].v[i].xyzw = Spin*Spin2*x_Lat*T[n].v[i].xyzw;
		}						
	}

	if (step == 1) return;
// Create T2 (green) & T3 (brown) from the T0 & T1 verts: rotate 2pi/5 around z then 
	const float beta = 2.0f*mConst::pi/5.0f;
	Spin = glm::rotate( glm::mat4(1.0), beta, mConst::zaxis );  //just rotate
	for(int n = 0; n < 2*N*N; n++){
		for(int i = 0; i < 3; i++){
			T[n + 2*N*N].v[i].xyzw = Spin*T[n].v[i].xyzw;
			T[n + 2*N*N].v[i].rgba = T[n].v[i].rgba;
		}						
	}
	if (step == 2) return;
	Spin = glm::rotate( glm::mat4(1.0), 2*beta, mConst::zaxis );  //just rotate
	for(int n = 0; n < 2*N*N; n++){
		for(int i = 0; i < 3; i++){
			T[n + 4*N*N].v[i].xyzw = Spin*T[n].v[i].xyzw;
			T[n + 4*N*N].v[i].rgba = T[n].v[i].rgba;
		}						
	}
	if (step == 3) return;
	Spin = glm::rotate( glm::mat4(1.0), 3*beta,  mConst::zaxis );  //just rotate
	for(int n = 0; n < 2*N*N; n++){
		for(int i = 0; i < 3; i++){
			T[n + 6*N*N].v[i].xyzw = Spin*T[n].v[i].xyzw;
			T[n + 6*N*N].v[i].rgba = T[n].v[i].rgba;
		}						
	}
	if (step == 4) return;
	Spin = glm::rotate( glm::mat4(1.0), 4*beta, mConst::zaxis );  //just rotate
	for(int n = 0; n < 2*N*N; n++){
		for(int i = 0; i < 3; i++){
			T[n + 8*N*N].v[i].xyzw = Spin*T[n].v[i].xyzw;
			T[n + 8*N*N].v[i].rgba = T[n].v[i].rgba;
		}						
	}


	float radius = sin(2.0f*mConst::pi/5.0f);
	x_Lat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, radius ));
	for(int n = 0; n < 10*N*N; n++){
		for(int i = 0; i < 3; i++){		
			T[n].v[i].xyzw = x_Lat*T[n].v[i].xyzw;
		}						
	}
	if (step == 5) return;

//	Invert FL0
	Win::Vertex swap{};
	for(int n = 0; n < 10*N*N; n++){
		for(int i = 0; i < 3; i++){ // invert
			T[n+10*N*N].v[i] = -T[n].v[i];
		}
		swap = T[n+10*N*N].v[2];
		T[n+10*N*N].v[2] = T[n+10*N*N].v[0];
		T[n+10*N*N].v[0] = swap;
	}

	if (step == 6) return;

//	Spherify
	float length_of_v{};
	if (step == 7) {
		for (int t = 0; t < 20 * N*N; t++) {
				for (int v = 0; v < 3; v++) {
					length_of_v = lengthOfVector(T[t].v[v].xyzw);
					T[t].v[v].xyzw.x = T[t].v[v].xyzw.x / length_of_v;
					T[t].v[v].xyzw.y = T[t].v[v].xyzw.y / length_of_v;
					T[t].v[v].xyzw.z = T[t].v[v].xyzw.z / length_of_v;
			}
		}
	}
}

// Creates a octahedron mesh centered on the origin with n² triangle/face.
// Calls ModelGL::equilateralFace( Triangle *&T ). 
// n -  is sent in as T[0].v[0].xyzw.x. 
// The color is sent in T[0].v[0].rgba.
// The edge length is normalized to 1.
void ModelGL::octahedron(Triangle *&T){

	int N = static_cast<int>(T[0].v[0].xyzw.x);
	int step = static_cast<int>(T[0].v[0].xyzw.y);
	if (N <= 0) return;

		log(L"ModelGL::octahedron, N = %i,  step = %i", N, step);

	// F0 --- First Face
	equilateralFace(T);
	if (step == 0) return;

	// F1 --- Second Face  "rotate-copy"
	//The dihedral angle for the octahedron: acos(-1/3).
	//const float octDi = acos( -1.0f/3.0f );

	glm::mat4 Spin = glm::rotate(glm::mat4(1.0), -mConst::octDi, mConst::xaxis);
	Win::Vertex swap{};
	for (int n = 0; n < N*N; n++) {
		for (int i = 0; i < 3; i++) {
			T[N*N + n].v[i].xyzw = Spin * T[n].v[i].xyzw;
			T[N*N + n].v[i].rgba = T[n].v[i].rgba;
		}
		//The rotation gives CCW verts so need need to make them CW again
		swap = T[N*N + n].v[2];
		T[N*N + n].v[2] = T[N*N + n].v[0];
		T[N*N + n].v[0] = swap;
	}
	if (step == 1) return;

	// translate - rotate P0
	glm::mat4 x_Lat;
	float zTrans = cos(mConst::octDi / 2.0f)*sin(mConst::pi / 3.0f);
	glm::vec3 xlate = glm::vec3(-0.5f, 0.0f, zTrans);
	x_Lat = glm::translate(glm::mat4(1.0), xlate);
	Spin = glm::rotate(glm::mat4(1.0), (mConst::octDi - mConst::pi) / 2.0f, mConst::xaxis);
	for (int n = 0; n < 2 * N*N; n++) {
		for (int i = 0; i < 3; i++) T[n].v[i].xyzw = x_Lat * Spin*T[n].v[i].xyzw;
	}
	if (step == 2) return;

	//  Create P1  -  Rotate P0 pi/2 around Y axis - copy to P1. 
	Spin = glm::rotate(glm::mat4(1.0), mConst::pi / 2.0f, mConst::yaxis);
	for (int n = 0; n < 2 * N*N; n++) {
		for (int i = 0; i < 3; i++) {
			T[n + 2 * N*N].v[i].xyzw = Spin * T[n].v[i].xyzw;
			T[2 * N*N + n].v[i].rgba = T[n].v[i].rgba;
		}
	}
	if (step == 3) return;

	//  Create P2 and P3  -  Rotate P0 P1 pi around Y axis - copy to P2 P3.
	Spin = glm::rotate(glm::mat4(1.0), mConst::pi, mConst::yaxis);
	for (int n = 0; n < 4 * N*N; n++) {
		for (int i = 0; i < 3; i++) {
			T[n + 4 * N*N].v[i].xyzw = Spin * T[n].v[i].xyzw;
			T[n + 4 * N*N].v[i].rgba = T[n].v[i].rgba;
		}
	}
	if (step == 4) return;

	float col = 32767.0f/ static_cast<unsigned int>(std::rand());

	// texturing
	if (step == 6) {
		for (int n = 0; n < 8 * N*N; n++) {
			for (int i = 0; i < 3; i++) {
				col = static_cast<unsigned int>(std::rand())/ (4*32767.0f);
				T[n].v[i].rgba.r = 1.0f - col ;
			//	col = static_cast<unsigned int>(std::rand()) / (15*32767.0f);
				T[n].v[i].rgba.g = 0.7f - col;
			//	col = static_cast<unsigned int>(std::rand()) / (20*32767.0f);
				T[n].v[i].rgba.b = 0.4f - col;
			//	col = static_cast<unsigned int>(std::rand()) /(20*32767.0f);
				T[n].v[i].rgba.a = 1.0f;

			}
		}
	}

	// texturing
	if (step == 5) {
		for (int n = 0; n < 8 * N*N; n += 2) {
			for (int i = 0; i < 3; i++) {
				T[n].v[i].rgba = glm::vec4(0.45f, 0.45f, 0.45f, 1.0f);
				T[n + 1].v[i].rgba = glm::vec4(5.0f, 0.0f, 0.0f, 1.0f);
			}
		}
	}

	//	Spherify
	float length_of_v{};
	if (step == 7) {
		for (int t = 0; t < 8 * N*N; t++) {
				for (int v = 0; v < 3; v++) {
					length_of_v = lengthOfVector(T[t].v[v].xyzw);
					T[t].v[v].xyzw.x = T[t].v[v].xyzw.x / length_of_v;
					T[t].v[v].xyzw.y = T[t].v[v].xyzw.y / length_of_v;
					T[t].v[v].xyzw.z = T[t].v[v].xyzw.z / length_of_v;
			}
		}
	}
}

// Creates a series of animations showing 3, 4, 5, and 6
//  equilateral triangles coming together at a vertex.
void ModelGL::faces(Triangle *&T) {

	int N = static_cast<int>(T[0].v[0].xyzw.x);
	int J = static_cast<int>(T[0].v[0].xyzw.y);
	equilateralFace(T);

	float tetra_Dihedral = acos(1.0f / 3.0f);
	glm::mat4 Spin = glm::rotate(glm::mat4(1.0), mConst::pi/3.0f, mConst::xaxis);
	float angle = mConst::pi / 3.0f;

	for( int j = 0; j < J; j++ ){
		for( int n = 0; n < N*N; n++ ){
			for( int i = 0; i < 3; i++ ){
			//	if(j ==0) T[n].v[i].xyzw = mSize*T[n].v[i].xyzw;
				Spin = glm::rotate(glm::mat4(1.0), (j+1)*angle, mConst::zaxis);
				T[(j + 1)*N*N + n].v[i].xyzw =Spin*T[n].v[i].xyzw;
				T[(j + 1)*N*N + n].v[i].rgba = T[n].v[i].rgba;
			}
		}
	}
}


// Creates a tetrahedron mesh centered on the origin with n² triangle/face.
// Calls ModelGL::equilateralFace( Triangle *&T ). 
// n -  is sent in as T[0].v[0].xyzw.x. 
// The color is sent in T[0].v[0].rgba.
// The edge length is normalized to 1.
void ModelGL::tetrahedron(Triangle *&T){

	int N = static_cast<int>(T[0].v[0].xyzw.x);
	int step = static_cast<int>(T[0].v[0].xyzw.y);
	if (N <= 0) return;

	log(L"ModelGL::tetrahedron, N = %i,  step = %i", N, step);

// F0 --- First Face
	equilateralFace(T);

// F1 --- Second Face  "rotate-copy"
	//The dihedral angle for the tetrahedron: acos(1/3).
	float tetra_Dihedral = acos(1.0f/3.0f);
	glm::mat4 Spin = glm::rotate(glm::mat4(1.0), -tetra_Dihedral, mConst::xaxis);
	Win::Vertex swap{};
	for (int n = 0; n < N*N; n++) {
		for (int i = 0; i < 3; i++) {
			T[N*N + n].v[i].xyzw =  Spin*T[n].v[i].xyzw;
			T[N*N + n].v[i].rgba =  T[n].v[i].rgba;}
		//The rotation gives CCW verts so need need to make them CW again
		swap = T[N*N + n].v[2];
		T[N*N + n].v[2] = T[N*N + n].v[0];
		T[N*N + n].v[0] = swap;
	}
	if( step == 1 ) return;

// translate-rotate P0
	glm::mat4 x_Lat;
	float zTrans = cos(tetra_Dihedral / 2.0f)*sin(mConst::pi / 3.0f);
	glm::vec3 xlate = glm::vec3(-0.5f, 0.0f, zTrans);
	x_Lat = glm::translate(glm::mat4(1.0), xlate);
	Spin = glm::rotate(glm::mat4(1.0), (tetra_Dihedral - mConst::pi)/2.0f, mConst::xaxis);
	for (int n = 0; n < 2*N*N; n++) {
		for (int i = 0; i < 3; i++) T[ n].v[i].xyzw = x_Lat*Spin*T[n ].v[i].xyzw;}
	if (step == 2) return;

//  Create P1  -  Rotate P0 pi/2 around Z axis - copy to P1. 
	Spin = glm::rotate(glm::mat4(1.0),  mConst::pi / 2.0f, mConst::zaxis);
	for (int n = 0; n < 2 * N*N; n++) {
		for (int i = 0; i < 3; i++) {
			T[n + 2*N*N ].v[i].xyzw = Spin * T[n].v[i].xyzw;
			T[2*N*N + n].v[i].rgba = T[n].v[i].rgba;}}
	if (step == 3) return;

//	Invert P1 - Invert P1 through the origin.
	for (int n = 0; n < 2 * N*N; n++) {
		for (int i = 0; i < 3; i++) T[n + 2 * N*N].v[i] = -T[n + 2 * N*N].v[i];
		//The inversion gives CCW verts so need need to make them CW again
		swap = T[n + 2 * N*N].v[2];
		T[n + 2 * N*N].v[2] = T[n + 2 * N*N].v[0];
		T[n + 2 * N*N].v[0] = swap;}
	if (step == 4) return;

// Join P0-P1  -  Create the tetrahedron by joining P0 and P1.
	xlate = glm::vec3( 0.0f, 0.0f, zTrans/2.0f );
	x_Lat = glm::translate(glm::mat4( 1.0), xlate );
	for( int n = 0; n < 2 * N*N; n++ ){
		for( int i = 0; i < 3; i++ ){
			xlate = glm::vec3( 0.0f, 0.0f,  zTrans/2.0f );
			x_Lat = glm::translate( glm::mat4(1.0), xlate );
			T[n + 2*N*N].v[i].xyzw = x_Lat*T[n + 2 *N*N].v[i].xyzw;
			xlate = glm::vec3(0.0f, 0.0f, -zTrans / 2.0f);
			x_Lat = glm::translate( glm::mat4(1.0), xlate );
			T[n].v[i].xyzw = x_Lat*T[n ].v[i].xyzw;}}
	if( step == 5 ) return;

// Upright  -  Set the tetrahedron "upright"
// with the base in the XY, Y =0 plane.
	float angle = 0.61548f;  // = asin((1.0f/sqrt(3)));
	Spin = glm::rotate(glm::mat4(1.0), angle, mConst::xaxis);
	for( int n = 0; n < 4 * N*N; n++ ){
		for( int i = 0; i < 3; i++ ) T[n].v[i].xyzw = Spin*T[n].v[i].xyzw; }

// texturing
	if( step == 7 ){
		for( int n = 0; n < 4*N*N; n += 2 ){
			for( int i = 0; i < 3; i++ ){
				T[n].v[i].rgba = glm::vec4(1.0f, 0.8f, 0.6f, 1.0f);
				T[n + 1].v[i].rgba = glm::vec4(0.8f, 0.1f, 0.0f, 1.0f);}}}
}


void ModelGL::exampleTri(Triangle &triVerts) {

	triVerts.v[0].xyzw = glm::vec4(-0.5f, -0.33f, 0.0f, 1.0f);
	triVerts.v[1].xyzw = glm::vec4( 0.0f,  0.5f,  0.0f, 1.0f);
	triVerts.v[2].xyzw = glm::vec4( 0.5f, -0.33f, 0.0f, 1.0f);
	triVerts.v[0].rgba = triVerts.v[1].rgba = triVerts.v[2].rgba = glm::vec4( 0.3f, 0.2f, 0.9f, 1.0f);
}

void ModelGL::exampleSubData(Triangle(&triVerts)[2]) {

	triVerts[0].v[0].xyzw = glm::vec4(0.075f, 0.075f, 0.0f, 1.0f);
	triVerts[0].v[1].xyzw = glm::vec4(0.075f, -0.075f, 0.0f, 1.0f);
	triVerts[0].v[2].xyzw = glm::vec4(-0.075f, -0.075f, 0.0f, 1.0f);

	triVerts[1].v[0].xyzw = glm::vec4(0.05f, 0.05f, 0.0f, 1.0f);
	triVerts[1].v[1].xyzw = glm::vec4(0.05f, -0.05f, 0.0f, 1.0f);
	triVerts[1].v[2].xyzw = glm::vec4(-0.05f, -0.05f, 0.0f, 1.0f);

}


Triangle ModelGL::triangle(Vertex(&vert)[3]) {

	Triangle triangle;
	triangle.v[0] = vert[0];
	triangle.v[1] = vert[1];
	triangle.v[2] = vert[2];

	return triangle;
}

float ModelGL::lengthOfVector(glm::vec4 v){

	return	sqrt( v.x*v.x + v.y*v.y + v.z*v.z );

}

float ModelGL::lengthOfVector(float a, float b) {

	return	sqrt(a*a + b*b);

}

// Creates a grid of square cells centered on the origin.
// The xSize of the grid is sent in as R[0].T[0].v[0].xyzw.x. 
// The ySize of the grid is sent in as R[0].T[0].v[0].xyzw.y.
// The color is sent in R[0].T[0].v[0].rgba.
// Using 1D array R[xSize*ySize] as R[x][y] <-> R[ y + x*ySize ].
// The grid R[xSize*ySize] is created centered on the origin in the XY plane.
// The maximum edge dimension is normalized to 1.0f
void ModelGL::grid(Win::Rectangle *&R){

	float fScale(0.0f);
	if(R[0].T[0].v[0].xyzw.x >= R[0].T[0].v[0].xyzw.y) fScale = 1.0f/ R[0].T[0].v[0].xyzw.x;
	else fScale = 1.0f / R[0].T[0].v[0].xyzw.y;

//  Offsets to place our "seed" cell relative to the origin.
	float xOffset = (1.0f - R[0].T[0].v[0].xyzw.x) / 2.0f;
	float yOffset = (1.0f - R[0].T[0].v[0].xyzw.y) / 2.0f;

// Create our loop variables
	int xSize = static_cast<int>(R[0].T[0].v[0].xyzw.x);
	int ySize = static_cast<int>(R[0].T[0].v[0].xyzw.y);

//  Make all the rectangles 1x1 then scale them later.
//  Start with the seed rectangle R
	R[0].T[0].v[0].xyzw = glm::vec4(1.0f, 1.0f, 0.0f, 0.0f);
	rectangle(R[0]);

// Move the "seed" rect; R[0][0]  =  R[0]   ( 1 - xSize )/2 along x
// Move the "seed" rect; R[0][0]  =  R[0]   ( 1 - ySize )/2 along y
	for (int t = 0; t < 2; t++){ for (int v = 0; v < 3; v++){
		R[0].T[t].v[v].xyzw.x = R[0].T[t].v[v].xyzw.x + xOffset;
		R[0].T[t].v[v].xyzw.y = R[0].T[t].v[v].xyzw.y + yOffset;}}

// We use a 1D array such that R[r][c] -> R[ c + r * rSize]
// Create the rest of the cells from "seed" rect; R[0][0]
	for (int x = 0; x < xSize; x++) { for (int y = 0; y < ySize; y++) {
		for (int t = 0; t < 2; t++) { for (int v = 0; v < 3; v++) {
			R[y + x*ySize].T[t].v[v] = R[0].T[t].v[v];
			R[y + x*ySize].T[t].v[v].xyzw.x = R[0].T[t].v[v].xyzw.x + x; //xOffset;
			R[y + x*ySize].T[t].v[v].xyzw.y = R[0].T[t].v[v].xyzw.y + y; //yOffset;
	}}}}

// Normalize
	glm::vec3 size = glm::vec3( fScale, fScale, fScale );
	glm::mat4 scaleM = glm::scale( glm::mat4(1.0), size );
	
	for (int i = 0; i < xSize*ySize; i++){ for (int t = 0; t < 2; t++){
		for (int v = 0; v < 3; v++){ R[i].T[t].v[v].xyzw = scaleM*R[i].T[t].v[v].xyzw;}}}

}

// Creates a cube mesh of square cells centered on the origin.
// The number of cells (n) along an edge of the cube is sent in
// on R[0].T[0].v[0].xyzw.x. 
// The color is sent in on R[0].T[0].v[0].rgba.
// Using 1D array R[6*n*n] as R[Face][Rectangle]
//                         <-> R[f][r] <-> R[ f + r*n*n ].
//
// The 6 faces of the cube are written into R
// Later we can treat R as a three dimensional array to access
// The cell [x,y] on Face = f,  R[f][x][y] =  R[ f*n*n + x*n + y ]
//                                     
void ModelGL::cubeMesh(Win::Rectangle *&R) {

//Win::log(L"ModelGL:: cubeMesh(Win::Rectangle *&R) %f ", R[0].T[0].v[0].xyzw.x);

	int n = static_cast<int>(R[0].T[0].v[0].xyzw.x);
	int step = static_cast<int>(R[0].T[0].v[0].xyzw.y);

	// Color all the verts
	for (int r = 0; r < 6 * n*n; r++) { for (int t = 0; t < 2; t++) {
		for (int v = 0; v < 3; v++) R[r].T[t].v[v].rgba = R[0].T[0].v[0].rgba; }}

	// grid needs R[0].T[0].v[0].xyzw.y for the Y dimension of the grid. 
	R[0].T[0].v[0].xyzw.y = R[0].T[0].v[0].xyzw.x;

	// F0 - Front face
	grid(R);

	float offset = -R[0].T[0].v[0].xyzw.x;
	for (int r = 0; r < 2 * n*n; r++) { for (int t = 0; t < 2; t++) {
			for (int v = 0; v < 3; v++) R[r].T[t].v[v].xyzw.z += offset; }}

	// F1 - Right face copy/rotate front face pi/2 Y axis.
	glm::mat4 rot = glm::rotate(glm::mat4(1.0f), mConst::halfPi, mConst::yaxis);
	for (int r = 0; r < n*n; r++) { for (int t = 0; t < 2; t++) {
			for (int v = 0; v < 3; v++) 
				R[r + n*n].T[t].v[v].xyzw = rot * R[r].T[t].v[v].xyzw; }}

	// F2 - Bottom face copy/rotate front face pi/2 X axis.
	rot = glm::rotate(glm::mat4(1.0f), mConst::halfPi, mConst::xaxis);
	for( int r = 0; r < n*n; r++ ) { for( int t = 0; t < 2; t++ ){
			for (int v = 0; v < 3; v++) 
				R[r + 2*n*n].T[t].v[v].xyzw = rot * R[r].T[t].v[v].xyzw;}}

	// P0 - P1  Create P1(F3, F4, F5) by inverting P0(F0, F1, F2).
	for (int r = 0; r < 3 * n*n; r++) {for (int t = 0; t < 2; t++) {
		for (int v = 0; v < 3; v++) R[r + 3 * n*n].T[t].v[v] = -R[r].T[t].v[v]; }}

	// Restore CW winding order
	Win::Vertex temp{};
	for (int r = 3 * n*n; r < 6 * n*n; r++) { for (int t = 0; t < 2; t++) {
	//	for (int v = 0; v < 3; v++) {
	temp = R[r].T[t].v[1];  R[r].T[t].v[1] = R[r].T[t].v[2]; R[r].T[t].v[2] = temp;
//	}
	}}

//	Spherify
	float length_of_v{};
	if( step == 1 ){
		for (int r = 0; r < 6*n*n; r++){
			for (int t = 0; t < 2; t++) {
				for (int v = 0; v < 3; v++) {
					length_of_v = lengthOfVector(R[r].T[t].v[v].xyzw);
					R[r].T[t].v[v].xyzw.x = R[r].T[t].v[v].xyzw.x/length_of_v;
					R[r].T[t].v[v].xyzw.y = R[r].T[t].v[v].xyzw.y/length_of_v;
					R[r].T[t].v[v].xyzw.z = R[r].T[t].v[v].xyzw.z/length_of_v;
				}
			}
		}
	}

// Roofed Cylinder
	else if (step == 2) {
		for (int r = 0; r < n*n; r++) {
			for (int t = 0; t < 2; t++) {
				for (int v = 0; v < 3; v++) {
					length_of_v = lengthOfVector(R[r].T[t].v[v].xyzw.x, R[r].T[t].v[v].xyzw.z);
					R[r].T[t].v[v].xyzw.x = 0.5f*R[r].T[t].v[v].xyzw.x / length_of_v;
					R[r].T[t].v[v].xyzw.z = 0.5f*R[r].T[t].v[v].xyzw.z / length_of_v;

					length_of_v = lengthOfVector(R[r + n*n].T[t].v[v].xyzw.x, R[r + n * n].T[t].v[v].xyzw.z);
					R[r + n * n].T[t].v[v].xyzw.x = 0.5f*R[r + n * n].T[t].v[v].xyzw.x / length_of_v;
					R[r + n * n].T[t].v[v].xyzw.z = 0.5f*R[r + n * n].T[t].v[v].xyzw.z / length_of_v;

					length_of_v = lengthOfVector(R[r + 3*n * n].T[t].v[v].xyzw.x, R[r + 3 * n * n].T[t].v[v].xyzw.z);
					R[r + 3 * n * n].T[t].v[v].xyzw.x = 0.5f*R[r + 3 * n * n].T[t].v[v].xyzw.x / length_of_v;
					R[r + 3 * n * n].T[t].v[v].xyzw.z = 0.5f*R[r + 3 * n * n].T[t].v[v].xyzw.z / length_of_v;

					length_of_v = lengthOfVector(R[r + 4 * n * n].T[t].v[v].xyzw.x, R[r + 4 * n * n].T[t].v[v].xyzw.z);
					R[r + 4 * n * n].T[t].v[v].xyzw.x = 0.5f*R[r + 4 * n * n].T[t].v[v].xyzw.x / length_of_v;
					R[r + 4 * n * n].T[t].v[v].xyzw.z = 0.5f*R[r + 4 * n * n].T[t].v[v].xyzw.z / length_of_v;

					length_of_v = lengthOfVector(R[r + 5 * n * n].T[t].v[v].xyzw);
					R[r + 5 * n * n].T[t].v[v].xyzw.x = R[r + 5 * n * n].T[t].v[v].xyzw.x / length_of_v;
					R[r + 5 * n * n].T[t].v[v].xyzw.y = R[r + 5 * n * n].T[t].v[v].xyzw.y / length_of_v;
					R[r + 5 * n * n].T[t].v[v].xyzw.z = R[r + 5 * n * n].T[t].v[v].xyzw.z / length_of_v;
				}
			}
		}
	}


//R[face][row][col] = R[face*n*n + row*n + col]
// Paint an outside border on each face of the cube:
// R(0) - row 0, R(n-1) - row n-1, C(0) - column 0, C(n-1) column n-1.
// R[face][row][col] = R[face*n*n + row*n + col]

	else if (step == 3) {
		for (int f = 0; f < 6; f++) {
			for (int r = 0; r < n; r++) {
				for (int j = 0; j < 3; j++){
					for (int t = 0; t < 2; t++){
						// R[f][row ][col = 0] = R[f*n*n +   row*n]
						R[f*n*n + r*n ].T[t].v[j].rgba = mConst::gray;

						// R[f][row ][col = n - 1] = R[f*n*n +   row*n]
						R[f*n*n + r*n  + n - 1 ].T[t].v[j].rgba = mConst::gray;
					}}}}
		for (int f = 0; f < 6; f++) {
			for (int c = 0; c < n; c++) {
				for (int j = 0; j < 3; j++) {
					for (int t = 0; t < 2; t++){
						// R[f][row = 0 ][col] = R[ f*n*n +   col]
						R[f*n*n + c ].T[t].v[j].rgba = mConst::gray;

					// R[f][row = n - 1 ][col] = R[f*n*n + n(n-1)  + col ]
					R[f*n*n + n *( n - 1) + c].T[t].v[j].rgba = mConst::gray;
					}}}}
	}

// I don't know what this is?

//	else if( step == 3) {
//		for(int i = 0; i < n; i++ ){
//			for (int j = 0; j < 3; j++){
//				R[i].T[0].v[j].rgba = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
//				R[i].T[1].v[j].rgba = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
//				R[(n - 1)*n + i].T[0].v[j].rgba = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
//				R[(n - 1)*n + i].T[1].v[j].rgba = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
//
////				R[ f=0 ][row ][col = 0] = R[ row*n]
//				R[i].T[0].v[j].rgba = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
//				R[i].T[1].v[j].rgba = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
//				R[(n - 1)*n + i].T[0].v[j].rgba = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
//				R[(n - 1)*n + i].T[1].v[j].rgba = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
//
//			}
//		}
//	}

}

//  Right Hand Coordinate System X Y X axes 
void ModelGL::xyzAxes(Win::Line(&axes)[3]){

	for( int l = 0; l < 3; l++ )
		axes[l].v[0].rgba = axes[l].v[1].rgba = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);// mConst::white;

		axes[0].v[0].xyzw = glm::vec4(4.0f*mConst::xaxis, 1.0f);
		axes[0].v[1] = -axes[0].v[0];
		axes[1].v[0].xyzw = glm::vec4(4.0f*mConst::yaxis, 1.0f);
		axes[1].v[1] = -axes[1].v[0];
		axes[2].v[0].xyzw = glm::vec4(4.0f*mConst::zaxis, 1.0f);
		axes[2].v[1] = -axes[2].v[0];

}

// The palette is a 36 row x 37 column grid which covers GLwin.
// The verts and colors for the grid are written to 
// Rectangle(&rect)[1331] which is passed in.  The colors are also 
// stored in row column order in colorArray[row][col] so they can
// be accessed by mouse click in void ModelGL::returnColor(...
void ModelGL::colorPalette(Win::Rectangle(&rect)[1331]){

	float red(0.0f);
	float green(0.0f);
	float blue(0.0f);

	float x(0.0f);
	float y(0.0f);

	float cellWidth = (2.0f) / static_cast<float>(cols);
	float cellHeight = (2.0f) / static_cast<float>(rows);

	int count(0); int row(0); int col(0);

	glm::vec4 color{};

	for (int R = 0; R < 11; R++)
	{
		red = 0.1f*R;
		for (int G = 0; G < 11; G++)
		{
			green = 0.1f*G;
			for (int B = 0; B < 11; B++)
			{
				blue = 0.1f*B;
				//glm::vec4 color;
				color.r = red; color.g = green; color.b = blue; color.a = 1.0f;
			//	Win::log(L"color.r = %f",  color.r);
				for (int t = 0; t < 2; t++)
				{
					for (int p = 0; p < 3; p++)
					{
						rect[count].T[t].v[p].rgba.r = red;
						rect[count].T[t].v[p].rgba.g = green;
						rect[count].T[t].v[p].rgba.b = blue;
						rect[count].T[t].v[p].rgba.a = 1.0f;

						rect[count].T[t].v[p].xyzw.z = 0.0f;
						rect[count].T[t].v[p].xyzw.w = 1.0f;
					}
				}

//	We have a grid of 36 rows x 37 cols = 1332 = 11^3 -1 ;
//	We will fill 11^3 = 1331 of these cells [0 - 1330] 
//  numbered as below.  Leaving the last cell, 1331, empty.
//    R\C 0  1  2   ...   36
//    35|1295|     ...  | 1331 |
//    35|35*36|    ...  | 35*37 - 1|--empty.
//		...
//     2|74|        ...  |110|
//     1|37|        ...  |73|
//	   0| 0| 1| 2|  ...  |36|

//       Using  RH CS
//
//     Orientation of 2 CW triangles             Y   -Z
//            T0        T1                       ^  /
//			1 --2         0                      | /
//			|  /   .   /  |       . = COM        |/
//			0         2   1                      ----> X 

		//		row = count / 37;
		//		col = count - row * 37;
		//		col = count - (count/37) * 37;

				row = count/cols;
				// col = count - row*37;
				col = count - (count/cols)*cols;

				x = static_cast<float>(col)*cellWidth - 1.0f;
				y = static_cast<float>(row)*cellHeight - 1.0f;
				rect[count].T[0].v[0].xyzw.x = x;
				rect[count].T[0].v[0].xyzw.y = y;

				rect[count].T[0].v[1].xyzw.x = x;
				rect[count].T[0].v[1].xyzw.y = y + cellHeight;

				rect[count].T[0].v[2].xyzw.x = x + cellWidth;
				rect[count].T[0].v[2].xyzw.y = y + cellHeight;


				rect[count].T[1].v[0].xyzw = rect[count].T[0].v[2].xyzw;
				rect[count].T[1].v[2].xyzw = rect[count].T[0].v[0].xyzw;

				rect[count].T[1].v[1].xyzw.x = x + cellWidth;;
				rect[count].T[1].v[1].xyzw.y = y;

				// don't go out of colorArray bounds!
				if( row < rows && col < cols )
				colorArray[row][col] = color;
				++count;
			} // B
		} // G            
	} // R
}

// Writes the color of whatever palette grid cell 
// the x,y mouse coordinates are in, to vec4 color;  
void ModelGL::returnColor(int x, int y, int w , int h , glm::vec4 &color) {

	//   Pallete Grid
	//
	//    R\C 0  1  2   ...   36
	//    35|1295|     ...  | 1331 |
	//    35|35*36|    ...  | 35*37 - 1|--empty.
	//		...
	//     2|74|        ...  |110|
	//     1|37|        ...  |73|
	//	   0| 0| 1| 2|  ...  |36|

// Need to find row( x, y, w, h ) col( x, y, w, h )
//
//	0 ------------------ w
//  |
//  |      x,y       
//  |  
//  |
//  h

	int cellCol(-1);
	int cellRow(-1);
	float cellWidth = ((float)w / (float)cols);
 	cellCol = (x / cellWidth);
	float cellHeight = ((float)h / (float)rows);
	cellRow = (rows - y / cellHeight);
//	Win::log(L"cellCol = %i cellRow = %i", cellCol , cellRow);

	 if (cellRow < rows && cellCol < cols) {
		 if (cellRow > -1  && cellCol >- 1 ) color = colorArray[cellRow][cellCol];
	 }
}

void ModelGL::coord_System(Vertex &specs, Line(&axis)[3]) {

	//  specs.xyzw  = glm::vec4(origin.x, origin.w, origin.z, length of axes);
	//  specs.rgba = color of axes
	//  axis[0] = X axis
	//  axis[1] = Y axis
	//  axis[2] = Z axis


	for (int i = 0; i < 3; i++) {

		axis[i].v[0].rgba = axis[i].v[1].rgba = specs.rgba;
		axis[i].v[0].xyzw.w = axis[i].v[1].xyzw.w = 1.0f;
	}

	axis[0].v[0].xyzw.x = -specs.xyzw.w + specs.xyzw.x;
	axis[0].v[1].xyzw.x = specs.xyzw.w;

	axis[1].v[0].xyzw.y = -specs.xyzw.w + specs.xyzw.y;
	axis[1].v[1].xyzw.y = specs.xyzw.w;

	axis[2].v[0].xyzw.z = 0.0f; // -specs.xyzw.w + specs.xyzw.z;
	axis[2].v[1].xyzw.z = specs.xyzw.w;
}

// Creates a prism with the triangular faces in the XY plane.
// The faces are as shown
//
//
//
//         /\ T1               Y   -Z
//        /  \/                |  /
//       /____\                | /
//      /\    /                |/
//     /  \  /                 ----> X 
//    /____\/
//      |
//     T0  
//
//       1
//
//    0     2
void ModelGL::prism(Vertex(&specs)[5], Triangle(&tri)[8]) {


	//for (int i = 0; i < 2; i++)
	//{
	//	for (int j = 0; j < 3; j++)
	//	{
	//		glm::vec4 color = specs[i].rgba;
	//		tri[i].p[j].rgba = color;
	//	//	tri[i].p[j].rgba = specs[i].rgba;
	//		Win::log(L"tri[%i].p[%i].rgbd = %f  %f  %f  %f ", i, j, tri[i].p[j].rgba.r,
	//			tri[i].p[j].rgba.g, tri[i].p[j].rgba.b, tri[i].p[j].rgba.a);

	//		tri[i].v[j].xyzw.w = 1.0f;
	//	}
	//}

	tri[0].v[0].xyzw.x = specs[0].xyzw.x;
	tri[0].v[0].xyzw.y = specs[0].xyzw.y;
	tri[0].v[0].xyzw.z = specs[0].xyzw.w/2.0f;

	tri[0].v[1].xyzw.x = specs[1].xyzw.x;
	tri[0].v[1].xyzw.y = specs[1].xyzw.y;
	tri[0].v[1].xyzw.z = specs[0].xyzw.w/2.0f;

	tri[0].v[2].xyzw.x = specs[2].xyzw.x;
	tri[0].v[2].xyzw.y = specs[2].xyzw.y;
	tri[0].v[2].xyzw.z = specs[0].xyzw.w/2.0f;

	tri[1] = tri[0];
	tri[1].v[0].xyzw.z = -specs[0].xyzw.w/2.0f;
	tri[1].v[1].xyzw.z = -specs[0].xyzw.w/2.0f;
	tri[1].v[2].xyzw.z = -specs[0].xyzw.w/2.0f;

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			glm::vec4 color = specs[i].rgba;
			tri[i].v[j].rgba = color;
			//	tri[i].v[j].rgba = specs[i].rgba;
			Win::log(L"tri[%i].v[%i].rgbd = %f  %f  %f  %f ", i, j, tri[i].v[j].rgba.r,
				tri[i].v[j].rgba.g, tri[i].v[j].rgba.b, tri[i].v[j].rgba.a);

			tri[i].v[j].xyzw.w = 1.0f;
		}
	}

}




// We call model->rectangle(  Win::Rectangle &R ) to model a rectangle.
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
//  The rectangle will be offset R.T[0].v[0].xyzw.w -0.05 from the origin.  In other words; 
//  it will lie in the z = -0.05 plane.
//
//  The back side of the rectangle is the side seen looking out from the 
//  origin.  It will have counter-clockwise winding order.
//
//  The  rectangle R.T[0].v[0] "out" Rectangle R
//


void ModelGL::rectangle( Win::Rectangle &R ) {

	glm::vec4 spex = R.T[0].v[0].xyzw;

	float x = spex.x / 2.0f;
	float y = spex.y / 2.0f;
	float z = spex.z / 2.0f;
	float w = spex.w;

	Win::Vertex v[3]{};

	for (int j = 0; j < 3; j++)
	{
		v[j].rgba = R.T[0].v[0].rgba;
	}

	//  X = 0   YZ plane
	//     Using  RH CS
	//     Orientation of 2 CW triangles looking down
	//     the positive Y axis towards the origin.
	//			1 --2         0
	//			|  /   .   /  |       Y
	//			0         2   1       |
	//           T0        T1         |
	//                          Z---- X
	if (x == 0.0f)
	{
		x = spex.w;
		if (w >= 0.0f)
		{
			v[0].xyzw = glm::vec4( x, -y, z, 1.0f);
			v[1].xyzw = glm::vec4( x,  y, z, 1.0f);
		}
		else
		{
			v[1].xyzw = glm::vec4( x, -y,  z, 1.0f);
			v[0].xyzw = glm::vec4( x,  y,  z, 1.0f);
		}

		v[2].xyzw = glm::vec4( x, y,  -z, 1.0f);
		R.T[0] = triangle(v);

		if (w >= 0.0f)
		{
			v[0].xyzw = glm::vec4( x, y, -z, 1.0f);
			v[1].xyzw = glm::vec4( x, -y, -z, 1.0f);
		}
		else
		{
			v[1].xyzw = glm::vec4( x, y, -z, 1.0f);
			v[0].xyzw = glm::vec4( x, -y, -z, 1.0f);
		}

		v[2].xyzw = glm::vec4( x, -y, z, 1.0f);
		R.T[1] = triangle(v);
		return;
	}

	//  Y = 0   ZX plane
	//     Using  RH CS
	//     Orientation of 2 CW triangles looking down
	//     the positive Y axis towards the origin.
	//			1 --2         0
	//			|  /   .   /  |       Y -----> X
	//			0         2   1       |
	//           T0        T1         |
	//                                Z
	if (y == 0.0f)
	{
		y = spex.w;

		if (w >= 0.0f)
		{
			v[0].xyzw = glm::vec4( -x,  y,  z, 1.0f);
			v[1].xyzw = glm::vec4( -x,  y, -z, 1.0f);
		}
		else
		{
			v[1].xyzw = glm::vec4( -x,  y,  z, 1.0f);
			v[0].xyzw = glm::vec4( -x,  y, -z, 1.0f);
		}

		v[2].xyzw = glm::vec4( x, y, -z, 1.0f);
		R.T[0] = triangle(v);

		if (w >= 0.0f)
		{
			v[0].xyzw = glm::vec4( x, y, -z, 1.0f);
			v[1].xyzw = glm::vec4( x, y,  z, 1.0f);
		}
		else
		{
			v[1].xyzw = glm::vec4( x,  y,  -z, 1.0f);
			v[0].xyzw = glm::vec4( x,  y,  z, 1.0f);
		}

		v[2].xyzw = glm::vec4( -x,  y,  z, 1.0f);
		R.T[1] = triangle(v);
		return;
	}

	//  Z = 0   XY plane
	//     Using  RH CS
	//     Orientation of 2 CW triangles looking down
	//     the positive Z axis towards the origin.
	//			1 --2         0
	//			|  /   .   /  |       Y
	//			0         2   1       |
	//           T0        T1         |
	//                                Z----->X

	if (z == 0.0f  )
	{
		z = spex.w;

		if ( w >= 0.0f ) 
		{
			v[0].xyzw = glm::vec4(-x, -y, z, 1.0f);
			v[1].xyzw = glm::vec4(-x,  y, z, 1.0f);
		}
		else
		{
			v[1].xyzw = glm::vec4(-x, -y, z, 1.0f);
			v[0].xyzw = glm::vec4(-x, y, z, 1.0f);
		}

		v[2].xyzw = glm::vec4( x,  y, z, 1.0f);
		R.T[0] = triangle(v);

		if ( w >= 0.0f)
		{
			v[0].xyzw = glm::vec4(x, y, z, 1.0f);
			v[1].xyzw = glm::vec4(x, -y, z, 1.0f);
		}
		else
		{
			v[1].xyzw = glm::vec4(x, y, z, 1.0f);
			v[0].xyzw = glm::vec4(x, -y, z, 1.0f);
		}

		v[2].xyzw = glm::vec4(-x, -y, z, 1.0f);
		R.T[1] = triangle(v);
		return;
	}	
}

// Creates a box centered on the origin.
// R[0]v[0].xyzw is sent in to specify the dimensions of the box.  
// R[0].T[0].v[0].xyzw - xyz are the xyz
// 
// The color of each face is sent in on R[i]v[0].rgba 
// R[0] front, R[1] back, R[2] top, R[3] bottom, R[4] right ,R[5] left
void ModelGL::box(Win::Rectangle(&R)[6]) {

 log(L" ModelGL::box(Win::Rectangle(&R)[6])");

	float x = R[0].T[0].v[0].xyzw.x;
	float y = R[0].T[0].v[0].xyzw.y;
	float z = R[0].T[0].v[0].xyzw.z;

// Do XY front face  R0

	R[0].T[0].v[0].xyzw = glm::vec4( x, y, 0.0f, z/2.0f);
	rectangle(R[0]);

// Do XY back face  R1

	R[1].T[0].v[0].xyzw = glm::vec4(x, y, 0.0f, -z / 2.0f);
	rectangle(R[1]);

// Do ZX top face  R2

	R[2].T[0].v[0].xyzw = glm::vec4(x, 0.0f, z, y / 2.0f);
	rectangle(R[2]);

// Do ZX bottom face  R3

	R[3].T[0].v[0].xyzw = glm::vec4(x, 0.0f, z, -y / 2.0f);
	rectangle(R[3]);

// Do YZ right face  R4

	R[4].T[0].v[0].xyzw = glm::vec4(0.0f, y, z, x / 2.0f);
	rectangle(R[4]);

// Do YZ left face  R5

	R[5].T[0].v[0].xyzw = glm::vec4(0.0f, y, z, -x / 2.0f);
	rectangle(R[5]);
}

