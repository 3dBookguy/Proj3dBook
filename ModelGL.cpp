/*
						ModelGL.cpp

Handles calls from ViewGL for rendering data.


*/

#define DEBUG_GB

#include "Log.h"
#include "ModelGL.h"

using namespace Win;

ModelGL::ModelGL() {}

Point ModelGL::point(float x, float y, float z, float w, float r, float g, float b, float a) {

	Point point;
	point.xyzw  = glm::vec4( x,  y,  z,  w );
	point.color = glm::vec4( r, b, g, a);
	return point;
}

Line ModelGL::line(Point begin, Point end) {

	Line line;
	line.p[0] = begin;
	line.p[1] = end;
	return line;
}

void ModelGL::exampleTri(Triangle &triVerts) {

	triVerts.p[0].xyzw = glm::vec4(-0.5f, -0.33f, 0.0f, 1.0f);
	triVerts.p[1].xyzw = glm::vec4( 0.0f,  0.5f,  0.0f, 1.0f);
	triVerts.p[2].xyzw = glm::vec4( 0.5f, -0.33f, 0.0f, 1.0f);

}

void ModelGL::exampleSubData(Triangle(&triVerts)[2]) {

	triVerts[0].p[0].xyzw = glm::vec4(0.075f, 0.075f, 0.0f, 1.0f);
	triVerts[0].p[1].xyzw = glm::vec4(0.075f, -0.075f, 0.0f, 1.0f);
	triVerts[0].p[2].xyzw = glm::vec4(-0.075f, -0.075f, 0.0f, 1.0f);

	triVerts[1].p[0].xyzw = glm::vec4(0.05f, 0.05f, 0.0f, 1.0f);
	triVerts[1].p[1].xyzw = glm::vec4(0.05f, -0.05f, 0.0f, 1.0f);
	triVerts[1].p[2].xyzw = glm::vec4(-0.05f, -0.05f, 0.0f, 1.0f);

}

Triangle ModelGL::triangle(Point A, Point B, Point C) {
	
	Triangle triangle;
	triangle.p[0] = A;
	triangle.p[1] = B;
	triangle.p[2] = C;

	return triangle;
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
						rect[count].T[t].p[p].color.r = red;
						rect[count].T[t].p[p].color.g = green;
						rect[count].T[t].p[p].color.b = blue;
						rect[count].T[t].p[p].color.a = 1.0f;

						rect[count].T[t].p[p].xyzw.z = 0.0f;
						rect[count].T[t].p[p].xyzw.w = 1.0f;
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
				rect[count].T[0].p[0].xyzw.x = x;
				rect[count].T[0].p[0].xyzw.y = y;

				rect[count].T[0].p[1].xyzw.x = x;
				rect[count].T[0].p[1].xyzw.y = y + cellHeight;

				rect[count].T[0].p[2].xyzw.x = x + cellWidth;
				rect[count].T[0].p[2].xyzw.y = y + cellHeight;


				rect[count].T[1].p[0].xyzw = rect[count].T[0].p[2].xyzw;
				rect[count].T[1].p[2].xyzw = rect[count].T[0].p[0].xyzw;

				rect[count].T[1].p[1].xyzw.x = x + cellWidth;;
				rect[count].T[1].p[1].xyzw.y = y;
// Win::log(L"%f    %f    %f    %i    %i", red, green, blue, row, col);

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

void ModelGL::coord_System(Point specs, Line(&axis)[3]) {

	//  specs.xyzw  = glm::vec4(origin.x, origin.w, origin.z, length of axes);
	//  specs.color = color of axes
	//  axis[0] = X axis
	//  axis[1] = Y axis
	//  axis[2] = Z axis


	for (int i = 0; i < 3; i++) {

		axis[i].p[0].color = axis[i].p[1].color = specs.color;
		axis[i].p[0].xyzw.w = axis[i].p[1].xyzw.w = 1.0f;
	}

	axis[0].p[0].xyzw.x = -specs.xyzw.w + specs.xyzw.x;
	axis[0].p[1].xyzw.x = specs.xyzw.w;

	axis[1].p[0].xyzw.y = -specs.xyzw.w + specs.xyzw.y;
	axis[1].p[1].xyzw.y = specs.xyzw.w;

	axis[2].p[0].xyzw.z = 0.0f; // -specs.xyzw.w + specs.xyzw.z;
	axis[2].p[1].xyzw.z = specs.xyzw.w;
}

// Monochromatic rectangle
void ModelGL::rectangle(Point rect, Triangle(&tri)[2] ){

// Set all the color attributes to rect.color.
// Set all the xyzw.w = 1.0f;

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			tri[i].p[j].color = rect.color;
			tri[i].p[j].xyzw.w = 1.0f;
		}
	}

//       Using  RH CS
//
//     Orientation of 2 CW triangles             Y   -Z
//            T0        T1                       ^  /
//			1 --2         0                      | /
//			|  /   .   /  |       . = COM        |/
//			0         2   1                      ----> X 

	if (rect.xyzw.z == 0.0f)  // Create rectangle in XY plane.
	{   

		tri[0].p[0].xyzw.x = -rect.xyzw.x / 2.0f;
		tri[0].p[0].xyzw.y = -rect.xyzw.y / 2.0f;
		tri[0].p[0].xyzw.z = rect.xyzw.w;

		tri[0].p[1].xyzw.x = -rect.xyzw.x / 2.0f;
		tri[0].p[1].xyzw.y = rect.xyzw.y / 2.0f;
		tri[0].p[1].xyzw.z = rect.xyzw.w;

		tri[0].p[2].xyzw.x = rect.xyzw.x / 2.0f;
		tri[0].p[2].xyzw.y = rect.xyzw.y / 2.0f;
		tri[0].p[2].xyzw.z = rect.xyzw.w;

		tri[1].p[0] = tri[0].p[2];
		tri[1].p[1] = -tri[0].p[1];

		tri[1].p[1].xyzw.z = rect.xyzw.w;

		tri[1].p[2] = tri[0].p[0];

	}

//     Using  RH CS
//
//     Orientation of 2 CW verticess             Y   X
//                                               ^  /
//			1 --2         0                      | /
//			|  /   .    / |       . = COM        |/
//			0          2  1                      ----> Z 

	if (rect.xyzw.x == 0.0f)  // Create rectangle in YZ plane.
	{
		tri[0].p[0].xyzw.y = -rect.xyzw.y / 2.0f;
		tri[0].p[0].xyzw.z = -rect.xyzw.z / 2.0f;
		tri[0].p[0].xyzw.x = rect.xyzw.w;

		tri[0].p[1].xyzw.y = rect.xyzw.y / 2.0f;
		tri[0].p[1].xyzw.z = -rect.xyzw.z / 2.0f;
		tri[0].p[1].xyzw.x = rect.xyzw.w;

		tri[0].p[2].xyzw.y = rect.xyzw.y / 2.0f;
		tri[0].p[2].xyzw.z = rect.xyzw.z / 2.0f;
		tri[0].p[2].xyzw.x = rect.xyzw.w;

		tri[1].p[0] = tri[0].p[2];

		tri[1].p[1] = -tri[0].p[1];
		tri[1].p[1].xyzw.x = rect.xyzw.w;

		tri[1].p[2] = tri[0].p[0];
	}

	//     Orientation of 2 CW triangles             Z   Y
	//                                               ^  /
	//			1 --2         0                      | /
	//			|  /   .   /  |       . = COM        |/
	//			0         2   1                      ----> X 

//	spec.xyzw = glm::vec4(x, 0.0f, z, y / 2.0f);
	if (rect.xyzw.y == 0.0f)  // Create rectangle in ZX plane.
	{
		tri[0].p[0].xyzw.z = -rect.xyzw.z / 2.0f;
		tri[0].p[0].xyzw.x = -rect.xyzw.x / 2.0f;
		tri[0].p[0].xyzw.y = rect.xyzw.w;

		tri[0].p[1].xyzw.z =  rect.xyzw.z / 2.0f;
		tri[0].p[1].xyzw.x = -rect.xyzw.x / 2.0f;
		tri[0].p[1].xyzw.y = rect.xyzw.w;

		tri[0].p[2].xyzw.z = rect.xyzw.z / 2.0f;
		tri[0].p[2].xyzw.x = rect.xyzw.x / 2.0f;
		tri[0].p[2].xyzw.y = rect.xyzw.w;

		tri[1].p[0] = tri[0].p[2];
		tri[1].p[1] = -tri[0].p[1];
		tri[1].p[1].xyzw.y = rect.xyzw.w;

		tri[1].p[2] = tri[0].p[0];

		//for (int i = 0; i < 2; i++)
		//{
		//	for (int j = 0; j < 3; j++)
		//	{

		//		log(L"tri[%i].p[%i].xyzw.x = %f",i,j, tri[i].p[j].xyzw.x);
		//		//tri[i].p[j].xyzw.z -= 3.0f;
		//	}
		//}

	}

// Restore CW winding order for outside faces.
	if (rect.xyzw.w < 0.0f)
	{
		Point temp{};
		temp = tri[0].p[0];
		tri[0].p[0] = tri[0].p[2];
		tri[0].p[2] = temp;

		temp = tri[1].p[0];
		tri[1].p[0] = tri[1].p[2];
		tri[1].p[2] = temp;
	}
}

// rect[6] specifies the dimensions, offset for origin and color 
// of the 6 faces.  Writes the verts to Triangle(&tri)[12] 
void ModelGL::box(Point(&rect)[6], Triangle(&tri)[12]) {

         //     _________                 Y -Z
         //    / 23(89) /|                | /
         //   /________/ |                |/__ X
         //   |    .   |-|--- 45(1011)	       
         //   | 01(67) | /	    .  = origin = COM
	     //   |________|/   

// 01(67) means triangles  T0 and T1 will be on the XY front face
//              triangles  T6 and T7 will be on the XY back face
// 

	Point spec{};
	Triangle face[2]{};
	float x = rect[0].xyzw.x;
	float y = rect[0].xyzw.y;
	float z = rect[0].xyzw.z;
//---------------------------------------------------------------

// Do XY front face  T0 T1
// spec.xyzw = glm::vec4(x, y, 0.0f, offset from xy plane);
	spec.xyzw = glm::vec4( x, y, 0.0f, z/2.0f);
	spec.color = rect[0].color;
	rectangle(spec, face);

	for (int i = 0; i < 2; i++) tri[i] = face[i];

// Do XY back face  T6  T7
// spec.xyzw = glm::vec4(x, y, 0.0f, offset from xy plane);
	spec.xyzw = glm::vec4(x, y, 0.0f, -z/2.0f);
	spec.color = rect[1].color;
	rectangle(spec, face);

	for (int i = 0; i < 2; i++) tri[i + 6] = face[i];

//---------------------------------------------------------------

// Do ZX top face  T2 T3
// spec.xyzw = glm::vec4(x, 0.0f, z, offset from ZX plane);
	spec.xyzw = glm::vec4(x, 0.0f, z, y / 2.0f);
	spec.color = rect[2].color;
	rectangle(spec, face);

	for (int i = 0; i < 2; i++) tri[i + 2] = face[i];

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 3; j++)
		{

			log(L" BOX tri[%i].p[%i].xyzw.x = %f", i, j, tri[i].p[j].xyzw.x);
			//tri[i].p[j].xyzw.z -= 3.0f;
		}
	}

// Do ZX bottom face  T8 T9
//	spec.xyzw = glm::vec4(x, 0.0f, z, offset from ZX plane);
	spec.xyzw = glm::vec4(x, 0.0f, z, -y / 2.0f);
	spec.color = rect[3].color;
	rectangle(spec, face);

	for (int i = 0; i < 2; i++) tri[i + 8] = face[i];

//--------------------------------------------------------------------
 
// Do YZ right face  T4 T5
// spec.xyzw = glm::vec4(0.0f, y, z, offset from YZ plane);
	spec.xyzw = glm::vec4(0.0f, y, z, x / 2.0f);
	spec.color = rect[4].color;
	rectangle(spec, face);

	for (int i = 0; i < 2; i++) tri[i + 4] = face[i];
	
// Do YZ left face  T10 T11
// spec.xyzw = glm::vec4(0.0f, y, z, offset from YZ plane);
	spec.xyzw = glm::vec4(0.0f, y, z, -x / 2.0f);
	spec.color = rect[5].color;
	rectangle(spec, face);

	for (int i = 0; i < 2; i++) tri[i + 10] = face[i];	
}

void ModelGL::examplePointer(int exampleNumber, int no_Vertices, Triangle *array) {

	switch (exampleNumber)
	{
	case 0: // Hello Triangle! 
	{
		array[0].p[0].xyzw = glm::vec4(0.75f, 0.75f, 0.0f, 1.0f);
		array[1].p[0].xyzw = glm::vec4(0.75f, -0.75f, 0.0f, 1.0f);
		array[2].p[0].xyzw = glm::vec4(-0.75f, -0.75f, 0.0f, 1.0f);
		break;
	}
	case 1: // Hello Triangle! 
	{
		array[0].p[0].xyzw = glm::vec4(0.75f, 0.75f, 0.0f, 1.0f);
		array[1].p[0].xyzw = glm::vec4(0.75f, -0.75f, 0.0f, 1.0f);
		array[2].p[0].xyzw = glm::vec4(-0.75f, -0.75f, 0.0f, 1.0f);
		break;
	}
		default: { } // None of the above
} // End switch(  exampleNumber )


}


std::vector<glm::vec4> ModelGL::exampleN(int exampleNumber, int no_Vertices)
{
#ifdef DEBUG_GB
//	log(L"std::vector<glm::vec4> ModelGL::exampleN");
#endif

	std::vector<glm::vec4> exampleN(no_Vertices);

	switch( exampleNumber )
	{
		case 0: // Hello Triangle! no colors here
		{

			exampleN[0] = glm::vec4(0.75f, 0.75f, 0.0f, 1.0f);
			exampleN[1] = glm::vec4(0.75f, -0.75f, 0.0f, 1.0f);
			exampleN[2] = glm::vec4(-0.75f, -0.75f, 0.0f, 1.0f);
			break;
		}
		case 1: // Using glBufferSubData to move two triangles independantly.
		{
			exampleN[2] = glm::vec4(-0.075f, -0.075f, 0.0f, 1.0f);
			exampleN[1] = glm::vec4(0.075f, -0.075f, 0.0f, 1.0f);
			exampleN[0] = glm::vec4(0.075f, 0.075f, 0.0f, 1.0f);
			exampleN[5] = glm::vec4(-0.05f, -0.05f, 0.0f, 1.0f);
			exampleN[4] = glm::vec4(0.05f, -0.05f, 0.0f, 1.0f);
			exampleN[3] = glm::vec4(0.05f, 0.05f, 0.0f, 1.0f);
			break;
		}
		case 2:  // Using vertex shader with offset uniform to move a triangle.
		{
			exampleN[2] = glm::vec4(-0.075f, -0.075f, 0.0f, 1.0f);
			exampleN[1] = glm::vec4(0.075f, -0.075f, 0.0f, 1.0f);
			exampleN[0] = glm::vec4(0.075f, 0.075f, 0.0f, 1.0f);
			break; 
		}
		case 3:  // Box vertices  
		{
			exampleN[0] = glm::vec4(0.25f, 0.25f, 0.75f, 1.0f);
			exampleN[1] = glm::vec4(0.25f, -0.25f, 0.75f, 1.0f);
			exampleN[2] = glm::vec4(-0.25f, 0.25f, 0.75f, 1.0f);

			exampleN[3] = glm::vec4(0.25f, -0.25f, 0.75f, 1.0f);
			exampleN[4] = glm::vec4(-0.25f, -0.25f, 0.75f, 1.0f);
			exampleN[5] = glm::vec4(-0.25f, 0.25f, 0.75f, 1.0f);

			exampleN[6] = glm::vec4(0.25f, 0.25f, -0.75f, 1.0f);
			exampleN[7] = glm::vec4(-0.25f, 0.25f, -0.75f, 1.0f);
			exampleN[8] = glm::vec4(0.25f, -0.25f, -0.75f, 1.0f);

			exampleN[9] = glm::vec4(0.25f, -0.25f, -0.75f, 1.0f);
			exampleN[10] = glm::vec4(-0.25f, 0.25f, -0.75f, 1.0f);
			exampleN[11] = glm::vec4(-0.25f, -0.25f, -0.75f, 1.0f);
			 
			exampleN[12] = glm::vec4(-0.25f, 0.25f, 0.75f, 1.0f);
			exampleN[13] = glm::vec4(-0.25f, -0.25f, 0.75f, 1.0f);
			exampleN[14] = glm::vec4(-0.25f, -0.25f, -0.75f, 1.0f);

			exampleN[15] = glm::vec4( -0.25f, 0.25f, 0.75f, 1.0f);
			exampleN[16] = glm::vec4( -0.25f, -0.25f, -0.75f, 1.0f);
			exampleN[17] = glm::vec4( -0.25f, 0.25f, -0.75f, 1.0f);

			exampleN[18] = glm::vec4(0.25f, 0.25f, 0.75f, 1.0f);
			exampleN[19] = glm::vec4(0.25f, -0.25f, -0.75f, 1.0f);
			exampleN[20] = glm::vec4(0.25f, -0.25f, 0.75f, 1.0f);

			exampleN[21] = glm::vec4(0.25f, 0.25f, 0.75f, 1.0f);
			exampleN[22] = glm::vec4(0.25f, 0.25f, -0.75f, 1.0f);
			exampleN[23] = glm::vec4(0.25f, -0.25f, -0.75f, 1.0f);

			exampleN[24] = glm::vec4(0.25f, 0.25f, -0.75f, 1.0f);
			exampleN[25] = glm::vec4(0.25f, 0.25f, 0.75f, 1.0f);
			exampleN[26] = glm::vec4(-0.25f, 0.25f, 0.75f, 1.0f);

			exampleN[27] = glm::vec4(0.25f, 0.25f, -0.75f, 1.0f);
			exampleN[28] = glm::vec4( -0.25f, 0.25f, 0.75f, 1.0f);
			exampleN[29] = glm::vec4( -0.25f, 0.25f, -0.75f, 1.0f);

			exampleN[30] = glm::vec4(0.25f, -0.25f, -0.75f, 1.0f);
			exampleN[31] = glm::vec4( -0.25f, -0.25f, 0.75f, 1.0f);
			exampleN[32] = glm::vec4(0.25f, -0.25f, 0.75f, 1.0f);

			exampleN[33] = glm::vec4(0.25f, -0.25f, -0.75f, 1.0f);
			exampleN[34] = glm::vec4( -0.25f, -0.25f, -0.75f, 1.0f);
			exampleN[35] = glm::vec4( -0.25f, -0.25f, 0.75f, 1.0f);

			// Box colors.

			exampleN[36] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
			exampleN[37] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
			exampleN[38] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

			exampleN[39] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
			exampleN[40] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
			exampleN[41] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

			exampleN[42] = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
			exampleN[43] = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
			exampleN[44] = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);

			exampleN[45] = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
			exampleN[46] = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
			exampleN[47] = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);

			exampleN[48] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
			exampleN[49] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
			exampleN[50] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

			exampleN[51] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
			exampleN[52] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
			exampleN[53] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

			exampleN[54] = glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);
			exampleN[55] = glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);
			exampleN[56] = glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);

			exampleN[57] = glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);
			exampleN[58] = glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);
			exampleN[59] = glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);

			exampleN[60] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
			exampleN[61] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
			exampleN[62] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

			exampleN[63] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
			exampleN[64] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
			exampleN[65] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

			exampleN[66] = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
			exampleN[67] = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
			exampleN[68] = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);

			exampleN[69] = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
			exampleN[70] = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
			exampleN[71] = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
			break;
		}

		default: { } // None of the above

	} // End switch(  exampleNumber )

	return exampleN;
}


