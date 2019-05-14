/*
						ModelGL.h

Include file for ModelGL.cpp and ViewGL.cpp.

*/

#ifndef MODEL_GL_H
#define MODEL_GL_H

#include <vector>
#include <cmath>
// ViewGL includes this file so we need 
// glm matrix includes here.
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext.hpp>
#include <glm/matrix.hpp>
//#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
namespace mConst
{
// trig consts
	const float pi = 3.1415926535f;
//	const float pi = acos(1.0f);
	const float halfPi = pi/2.0f;
	const float thirdPi = pi/3.0f;
	const float twoPi_5 = 2*pi/5.0f;

	const float sinThirdPi = sin(pi/3.0f);
	const float sin3Pi_10 = sin(3*pi/10.0f);
	const float sinPi_5 = sin(pi/5.0f);
	const float tetDi = acos( 1.0f/3.0f );
	const float tetDiComp = pi - tetDi;
	const float octDi = acos( -1.0f/3.0f );
	const float octDiComp = pi - octDi;
	const float icosaDi = acos( -sqrt(5.0f)/3.0f );
	const float icosaDiComp = pi - icosaDi;
	const float dodecaDi = acos( -1/sqrt(5.0f));
	const float dodecaDiComp = pi - dodecaDi;
	const float dodecaSphere = static_cast<float>(sqrt(3.0)/4.0f)*(1+sqrt(5.0f));

	const glm::vec4 origin4v = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	const glm::vec4 red = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	const glm::vec4 green = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	const glm::vec4 gray = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
	const glm::vec4 blue = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	const glm::vec4 white = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	const glm::vec4 black = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	const glm::vec3 origin = glm::vec3(0.0f, 0.0f, 0.0f);
	const glm::vec3 size = glm::vec3(0.1f, 0.1f, 0.1f);

	const glm::vec3 xaxis  = glm::vec3(0.2f, 0.0f, 0.0f);
	const glm::vec3 yaxis  = glm::vec3(0.0f, 0.2f, 0.0f);
	const glm::vec3 zaxis  = glm::vec3(0.0f, 0.0f, 0.2f);
}
namespace Win
{
	struct Vertex
	{
		glm::vec4 xyzw;
		glm::vec4 rgba;

		inline Vertex& operator+(const Vertex& rhs) {
			xyzw.x += rhs.xyzw.x;
			xyzw.y += rhs.xyzw.y;
			xyzw.z += rhs.xyzw.z;
			xyzw.w = rhs.xyzw.w;

			rgba.r += rhs.rgba.r;
			rgba.g += rhs.rgba.g;
			rgba.b += rhs.rgba.b;
			rgba.a = rhs.rgba.a;
			return *this;
		}

		inline Vertex  operator- (void) const{ 
			
			Vertex u(*this);
			u.xyzw.x = -u.xyzw.x;
			u.xyzw.y = -u.xyzw.y;
			u.xyzw.z = -u.xyzw.z;
			u.xyzw.w =  u.xyzw.w;

			u.rgba.r =  u.rgba.r;
			u.rgba.g =  u.rgba.g;
			u.rgba.b =  u.rgba.b;
			u.rgba.a =  u.rgba.a;
			return u;
		}
	};

	struct Line
	{
		Vertex v[2];
	};

	struct Triangle
	{
		Vertex v[3];
	};
	struct Rectangle
	{
		Triangle T[2];
	};

	struct FanRectangle
	{
		Vertex v[4];
	};

	struct Matrx
	{
		float xAngle;
		float yAngle;
		float zAngle;
		float rotSpeed;
		float fOV;
		glm::vec3 size;
		glm::mat4 sizeM;
		glm::mat4 xRot;
		glm::mat4 yRot;
		glm::mat4 zRot;
	};

	class ModelGL
	{
	public: ModelGL();

		std::vector<glm::vec4> exampleN(int exampleNumber, int no_Vertices);
		void examplePointer(int exampleNumber, int no_Vertices, Win::Triangle* array);
		Win::Vertex point(float x, float y, float z, float w, float r, float g, float b, float a);
		Win::Line line( Win::Vertex start, Win::Vertex end);
		Win::Triangle triangle(Win::Vertex (&vert)[3]);
		float lengthOfVector( glm::vec4 v );
		float lengthOfVector(float a, float b);
		void exampleTri( Win::Triangle &triVerts);
		void equilateral(Triangle &T);
		void equilateralFace(Triangle *&T);
		void pentagonalFace(Triangle *&T);
		void faces(Triangle *&T);
		void tetrahedron(Triangle *&T);
		void octahedron(Triangle *&T);
		void icosahedron(Win::Triangle *&T);
		void dodecahedron(Win::Triangle *&T);
		void exampleSubData( Win::Triangle(&triVerts)[2]);
		void rectangle( Win::Rectangle &rect );
		void prism( Win::Vertex(&inSpec)[5], Win::Triangle(&outTri)[8]);
		void box( Win::Rectangle (&inoutR)[6] );
		void coord_System( Win::Vertex &inSpec, Win::Line(&outGrid)[3]);
//		void tripod( Win::Line(&axes)[3]);
		void xyzAxes(Win::Line(&axes)[3]);
		void colorPalette( Win::Rectangle(&rect)[1331]);
		void returnColor(int x, int y, int width, int height, glm::vec4 &color);
		void keyDown(int key);
		glm::mat4 turnTable();
		void setMsize(float scale);
//		std::vector< Win::Triangle> cube(int grid, Win::Vertex &spec);
		void cubeMesh(Win::Rectangle *&rect);
		void grid(Win::Rectangle *&rect);
	private:

		std::vector<glm::vec4> vertices;
		Matrx Mem;

		// color palette params
		static const  UINT rows = 36;
		static const  UINT cols = 37;
		glm::vec4 colorArray[rows][cols]{};



	};
}

# endif
