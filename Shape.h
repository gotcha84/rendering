/*#ifndef _SHAPE_H_
#define _SHAPE_H_

#include "Vector3.h"
#include "Camera.h"
#include "Matrix4.h"
//#include "cube.h"
#include "objreader.h"

class Shape
{
	public:
		Camera camera;
		Matrix4 translation;
		Matrix4 scale;
		double angle;
		double x, y, z; // center
		float scaling_x, scaling_y, scaling_z;
		
		// house
		int house_nVerts;
		float *house_vertices;
		float *house_colors;
		int *house_indices;

		// sphere
		int sphere_nVerts;
		float *sphere_vertices;
		float *sphere_normals;
		float *sphere_texcoords;
		int sphere_nIndices;
		int *sphere_indices;
		
		// teddy
		int teddy_nVerts;
		float *teddy_vertices;
		float *teddy_normals;
		float *teddy_texcoords;
		int teddy_nIndices;
		int *teddy_indices;

		// teapot
		int teapot_nVerts;
		float *teapot_vertices;
		float *teapot_normals;
		float *teapot_texcoords;
		int teapot_nIndices;
		int *teapot_indices;

		// cow
		int cow_nVerts;
		float *cow_vertices;
		float *cow_normals;
		float *cow_texcoords;
		int cow_nIndices;
		int *cow_indices;

		// bunny
		int bunny_nVerts;
		float *bunny_vertices;
		float *bunny_normals;
		float *bunny_texcoords;
		int bunny_nIndices;
		int *bunny_indices;
		
		Shape();
		Camera& getCameraMatrix();
		double getAngle();
		void setAngle(double);
    void spin(double);      // spin shape [degrees]
		void drawHouse();
		void calculateStuff(int, float*);
};

class Window	  // output window related routines
{
  public:
    static int width, height; 	            // window size

    static void idleCallback(void);
    static void reshapeCallback(int, int);
    static void displayCallback(void);
		static void drawShape(int nVerts, float* vertices, 
			int nIndices, int* indices);
		static void processNormalKeys(unsigned char, int, int);
		static void processSpecialKeys(int, int, int);
		static void drawCube();
};

#endif*/