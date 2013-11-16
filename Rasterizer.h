#ifndef _RASTERIZER_H_
#define _RASTERIZER_H_

#include <iostream>
#include <math.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include "Matrix4.h"
#include "objreader.h"
#include "Camera.h"
#include <vector>

using namespace std;

static int nVerts;
static vector<float> vertices;
static vector<float> colors;
static int nIndices;
static vector<int> indices;

// house
static float house_vertices[] = {  
            -4,-4,4, 4,-4,4, 4,4,4, -4,4,4,     // front face
            -4,-4,-4, -4,-4,4, -4,4,4, -4,4,-4, // left face
            4,-4,-4,-4,-4,-4, -4,4,-4, 4,4,-4,  // back face
            4,-4,4, 4,-4,-4, 4,4,-4, 4,4,4,     // right face
            4,4,4, 4,4,-4, -4,4,-4, -4,4,4,     // top face
            -4,-4,4, -4,-4,-4, 4,-4,-4, 4,-4,4, // bottom face

            -20,-4,20, 20,-4,20, 20,-4,-20, -20,-4,-20, // grass
            -4,4,4, 4,4,4, 0,8,4,                       // front attic wall
            4,4,4, 4,4,-4, 0,8,-4, 0,8,4,               // left slope
            -4,4,4, 0,8,4, 0,8,-4, -4,4,-4,             // right slope
            4,4,-4, -4,4,-4, 0,8,-4};                   // rear attic wall
// These are the RGB colors corresponding to the vertices, in the same order
static float house_colors[] = { 
            1,0,0, 1,0,0, 1,0,0, 1,0,0,  // front is red
            0,1,0, 0,1,0, 0,1,0, 0,1,0,  // left is green
            1,0,0, 1,0,0, 1,0,0, 1,0,0,  // back is red
            0,1,0, 0,1,0, 0,1,0, 0,1,0,  // right is green
            0,0,1, 0,0,1, 0,0,1, 0,0,1,  // top is blue
            0,0,1, 0,0,1, 0,0,1, 0,0,1,  // bottom is blue
  
            0,0.5,0, 0,0.5,0, 0,0.5,0, 0,0.5,0, // grass is dark green
            0,0,1, 0,0,1, 0,0,1,                // front attic wall is blue
            1,0,0, 1,0,0, 1,0,0, 1,0,0,         // left slope is green
            0,1,0, 0,1,0, 0,1,0, 0,1,0,         // right slope is red
            0,0,1, 0,0,1, 0,0,1,};              // rear attic wall is red
static int house_indices[] = {
            0,2,3,    0,1,2,      // front face
            4,6,7,    4,5,6,      // left face
            8,10,11,  8,9,10,     // back face
            12,14,15, 12,13,14,   // right face
            16,18,19, 16,17,18,   // top face
            20,22,23, 20,21,22,   // bottom face
                   
            24,26,27, 24,25,26,   // grass
            28,29,30,             // front attic wall
            31,33,34, 31,32,33,   // left slope
            35,37,38, 35,36,37,   // right slope
            39,40,41};            // rear attic wall
static int house_nVerts = sizeof(house_vertices)/sizeof(house_vertices[0]);
static int house_nIndices = sizeof(house_indices)/sizeof(house_indices[0]);

static float cube_vertices[] = {
			// Draw front face:
			-1.0,  1.0,  1.0,
			 1.0,  1.0,  1.0,
			 1.0, -1.0,  1.0,
			-1.0, -1.0,  1.0,
    
			// Draw left side:
			-1.0,  1.0,  1.0,
			-1.0,  1.0, -1.0,
			-1.0, -1.0, -1.0,
			-1.0, -1.0,  1.0,
    
			// Draw right side:
			 1.0,  1.0,  1.0,
			 1.0,  1.0, -1.0,
			 1.0, -1.0, -1.0,
			 1.0, -1.0,  1.0,
  
			// Draw back face:
			-1.0,  1.0, -1.0,
			 1.0,  1.0, -1.0,
			 1.0, -1.0, -1.0,
			-1.0, -1.0, -1.0,
  
			// Draw top side:
			-1.0,  1.0,  1.0,
			 1.0,  1.0,  1.0,
			 1.0,  1.0, -1.0,
			-1.0,  1.0, -1.0,
  
			// Draw bottom side:
			-1.0, -1.0, -1.0,
			 1.0, -1.0, -1.0,
			 1.0, -1.0,  1.0,
			-1.0, -1.0,  1.0};

static float cube_colors[] = {
			// Draw front face:
			1,0,0,		0,1,0,		1,1,1,		0,0,1,
    
			// Draw left side:
			1,0,0,		1,1,1,		0,1,0,		0,0,1,
    
			// Draw right side:
			0,1,0,		0,0,1,		1,0,0,		1,1,1,

			// Draw back face:
			1,1,1,		0,0,1,		1,0,0,		0,1,0,

			// Draw top side:
			1,0,0,		0,1,0,		0,0,1,		1,1,1,

			// Draw bottom side:
			0,1,0,		1,0,0,		1,1,1,		0,0,1};

static int cube_indices[] = {
			// Draw front face:
			0,1,3,		1,2,3,

			//Draw left side:
			4,5,7,		5,6,7,

			//Draw right side:
			8,9,11,		9,10,11,

			//Draw back face:
			12,13,15,		13,14,15,

			//Draw top side:
			16,17,19,		17,18,19,

			//Draw bottom side:
			20,21,23,		21,22,23};
static int cube_nVerts = sizeof(cube_vertices)/sizeof(cube_vertices[0]);
static int cube_nIndices = sizeof(cube_indices)/sizeof(cube_indices[0]);

// sphere
static int sphere_nVerts;
static float *sphere_vertices;
static float *sphere_normals;
static float *sphere_texcoords;
static int sphere_nIndices;
static int *sphere_indices;
		
// teddy
static int teddy_nVerts;
static float *teddy_vertices;
static float *teddy_normals;
static float *teddy_texcoords;
static int teddy_nIndices;
static int *teddy_indices;

// teapot
static int teapot_nVerts;
static float *teapot_vertices;
static float *teapot_normals;
static float *teapot_texcoords;
static int teapot_nIndices;
static int *teapot_indices;

// cow
static int cow_nVerts;
static float *cow_vertices;
static float *cow_normals;
static float *cow_texcoords;
static int cow_nIndices;
static int *cow_indices;

// bunny
static int bunny_nVerts;
static float *bunny_vertices;
static float *bunny_normals;
static float *bunny_texcoords;
static int bunny_nIndices;
static int *bunny_indices;

class Rasterizer { };

#endif