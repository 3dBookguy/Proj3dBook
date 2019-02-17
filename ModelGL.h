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
#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
namespace mConst
{
	const double pi = 2 * asin(1.0f);

	const glm::vec4 origin4v = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	const glm::vec4 red = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	const glm::vec4 green = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	const glm::vec4 blue = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

	const glm::vec3 origin = glm::vec3(0.0f, 0.0f, 0.0f);
	const glm::vec3 size = glm::vec3(0.1f, 0.1f, 0.1f);

	const glm::vec3 xaxis  = glm::vec3(0.2f, 0.0f, 0.0f);
	const glm::vec3 yaxis  = glm::vec3(0.0f, 0.2f, 0.0f);
	const glm::vec3 zaxis  = glm::vec3(0.0f, 0.0f, 0.2f);
}
namespace Win
{
//	const double pi = 2*asin(1.0f);

	struct Point
	{
		glm::vec4 xyzw;
		glm::vec4 rgba;

		inline Point& operator+(const Point& rhs) {
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

		inline Point  operator- (void) const{ 
			
			Point u(*this);
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
		Point p[2];
	};

	struct Triangle
	{
		Point p[3];
	};

	struct Rectangle
	{
		Triangle T[2];
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
		void examplePointer(int exampleNumber, int no_Vertices, Triangle* array);

		Point point(float x, float y, float z, float w, float r, float g, float b, float a);
		Line line(Point start, Point end);
		Triangle triangle(Point A, Point B, Point C);
//		std::vector<glm::vec4> rectangle(Point rect);
//		std::vector<Triangle> box(Point box);
		void exampleTri( Triangle &triVerts);
		void exampleSubData(Triangle(&triVerts)[2]);
		void rectangle(Point specs, Triangle(&tri)[2]);
		void prism(Point(&specs)[5], Triangle(&tri)[8]);
		void box(Point(&specs)[6], Triangle(&tri)[12]);
		void coord_System(Point specs, Line(&axis)[3]);
		void rgbTriAxis(Line(&axes)[3]);
		void colorPalette(Rectangle(&rect)[1331]);
		void returnColor(int x, int y, int width, int height, glm::vec4 &color);
//		void matricesFromInputs();
		void rotateParams(int key);
		glm::mat4 rotate();
	private:
		std::vector<glm::vec4> vertices;
		Matrx Mem;

		// color palette stuff
		static const  UINT rows = 36;
		static const  UINT cols = 37;
		glm::vec4 colorArray[rows][cols]{};
	};
}

# endif
