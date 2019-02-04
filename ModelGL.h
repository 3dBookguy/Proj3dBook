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
#include <glm/gtc/type_ptr.hpp>

namespace Win
{
	const double pi = 2*asin(1.0f);

	struct Point
	{
		glm::vec4 xyzw;
		glm::vec4 color;

		inline Point& operator+(const Point& rhs) {
			xyzw.x += rhs.xyzw.x;
			xyzw.y += rhs.xyzw.y;
			xyzw.z += rhs.xyzw.z;
			xyzw.w = rhs.xyzw.w;

			color.r += rhs.color.r;
			color.g += rhs.color.g;
			color.b += rhs.color.b;
			color.a = rhs.color.a;
			return *this;
		}

		inline Point  operator- (void) const{ 
			
			Point u(*this);
			u.xyzw.x = -u.xyzw.x;
			u.xyzw.y = -u.xyzw.y;
			u.xyzw.z = -u.xyzw.z;
			u.xyzw.w =  u.xyzw.w;

			u.color.r =  u.color.r;
			u.color.g =  u.color.g;
			u.color.b =  u.color.b;
			u.color.a =  u.color.a;
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

	class ModelGL
	{
	public: ModelGL();
		
		std::vector<glm::vec4> exampleN(int exampleNumber, int no_Vertices);
		void examplePointer(int exampleNumber, int no_Vertices, Triangle* array);

		Point point(float x, float y, float z, float w, float r, float g, float b, float a);
		Line line(Point start, Point end);
		Triangle triangle(Point A, Point B, Point C);
		std::vector<glm::vec4> rectangle(Point rect);
		std::vector<Triangle> box(Point box);
		void exampleTri( Triangle &triVerts);
		void exampleSubData(Triangle(&triVerts)[2]);
		void rectangle(Point specs, Triangle(&tri)[2]);
		void box(Point(&rect)[6], Triangle(&tri)[12]);
		void coord_System(Point specs, Line(&axis)[3]);
		void colorPalette(Rectangle(&rect)[1331]);
		void returnColor(int x, int y, int width, int height, glm::vec4 &color);

	private:
		std::vector<glm::vec4> vertices;

		// color palette stuff
		static const  UINT rows = 36;
		static const  UINT cols = 37;
		glm::vec4 colorArray[rows][cols]{};
	};
}

# endif
