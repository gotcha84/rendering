#ifdef _WIN32
#include <windows.h>
#endif

#include "Rasterizer.h"
#include <climits>
#include <iostream>
#include <math.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include "Matrix4.h"
#include "Camera.h"
#include <time.h>
#include <stdlib.h> // abs

static float spin_angle = 0.1;

static int window_width = 512, window_height = 512;
static float* pixels = new float[window_width * window_height * 3];
static float zbuffer[512][512];

static bool draw_tile_mode = false;
static bool toggle_boxes = false;

static Matrix4 model;
static Camera camera;
static Matrix4 projection;
static Matrix4 viewport;
static float angle = 0;
Matrix4 tmp;

static int divisor = 2;

using namespace std;

struct Color    // generic color
{
  float r,g,b;
};

void printObject() {
	cout << "vertices:\n";
	for (int i=0; i<nVerts; i++)
		cout << vertices[i] << " ";
	cout << "\n";
	cout << "colors:\n";
	for (int i=0; i<nVerts; i++)
		cout << colors[i] << " ";
	cout << "\n";
	cout << "indices:\n";
	for (int i=0; i<nIndices; i++)
		cout << indices[i] << " ";
	cout << "\n\n";
}

void spin(double deg)
{
  if (angle > 360.0 || angle < -360.0) angle = 0.0;
	camera.c.rotateY(deg);
}

// projects each vertex of the house to image coordinates and sets 
// the color of the corresponding pixel to white using drawPoint.
void drawPoint(int x, int y, float r, float g, float b)
{
  int offset = y*window_width*3 + x*3;
  pixels[offset]   = r;
  pixels[offset+1] = g;
  pixels[offset+2] = b;
}

void loadHouse() {
	nVerts = house_nVerts;
	nIndices = house_nIndices;
	
	vertices.resize(nVerts);
	colors.resize(nVerts);
	indices.resize(nIndices);
	

	for (int i=0; i<nVerts; i++) {
		vertices[i] = house_vertices[i];
		colors[i] = house_colors[i];
	}
	for (int i=0; i<nIndices; i++) {
		indices[i] = house_indices[i];
	}
	//cout << "nVerts: " << sizeof(vertices) << '\n';
	//cout << "nindices: " << sizeof(indices) << '\n';
}

void loadCube() {
	nVerts = cube_nVerts;
	nIndices = cube_nIndices;

	vertices.resize(nVerts);
	colors.resize(nVerts);
	indices.resize(nIndices);
	
	for (int i=0; i<nVerts; i++) {
		vertices[i] = cube_vertices[i];
		colors[i] = cube_colors[i];
	}
	for (int i=0; i<nIndices; i++) {
		indices[i] = cube_indices[i];
	}
}

void loadData()
{
  // put code to load data model here
	ObjReader::readObj("sphere.obj", sphere_nVerts, &sphere_vertices, &sphere_normals, &sphere_texcoords, sphere_nIndices, &sphere_indices);
	ObjReader::readObj("teddy.obj", teddy_nVerts, &teddy_vertices, &teddy_normals, &teddy_texcoords, teddy_nIndices, &teddy_indices);
	ObjReader::readObj("teapot.obj", teapot_nVerts, &teapot_vertices, &teapot_normals, &teapot_texcoords, teapot_nIndices, &teapot_indices);
	ObjReader::readObj("cow.obj", cow_nVerts, &cow_vertices, &cow_normals, &cow_texcoords, cow_nIndices, &cow_indices);
	ObjReader::readObj("bunny.obj", bunny_nVerts, &bunny_vertices, &bunny_normals, &bunny_texcoords, bunny_nIndices, &bunny_indices);

	loadHouse();
	//printObject();
}

// Clear frame buffer
void clearBuffer()
{
  Color clearColor = {0.0, 0.0, 0.0};   // clear color: black
  for (int i=0; i<window_width*window_height; ++i)
  {
    pixels[i*3]   = clearColor.r;
    pixels[i*3+1] = clearColor.g;
    pixels[i*3+2] = clearColor.b;
  }  
}

// Draw a point into the frame buffer
void initialize_zbuffer()
{
	for (int i = 0; i < window_width; i++)
	{
		for (int j = 0; j < window_width; j++)
		{
			zbuffer[i][j] = INT_MAX;
		}
	}
}

void update_zbuffer(int x, int y, float c1, float c2, float c3, float z)
{
	if (x >= 0 && y >= 0 && x < window_width && y < window_height && z < zbuffer[x][y] ) {
		zbuffer[x][y] = z;
		drawPoint(x, y, c1, c2, c3);
	}
}

float f01(float x, float y, float x0, float x1, float y0, float y1)
{
	float term1 = (y0-y1)*x;
	float term2 = (x1-x0)*y;
	float term3 = x0*y1;
	float term4 = x1*y0;

	return term1 + term2 + term3 - term4;
}

float f12(float x, float y, float x1, float x2, float y1, float y2)
{
	float term1 = (y1-y2)*x;
	float term2 = (x2-x1)*y;
	float term3 = x1*y2;
	float term4 = x2*y1;

	return term1 + term2 + term3 - term4;
}

float f20(float x, float y, float x0, float x2, float y0, float y2)
{
	float term1 = (y2-y0)*x;
	float term2 = (x0-x2)*y;
	float term3 = x2*y0;
	float term4 = x0*y2;

	return term1 + term2 + term3 - term4;
}

bool test1(float ax, float ay, float bx, float by, float cx, float cy, float xleft, float xright, float ytop, float ybottom)
{
	bool tmp = false;
	if (ax > xleft || bx > xleft || cx > xleft) {
		tmp = true;
	}
	if (ax < xright || bx < xright || cx < xright) {
		tmp = true;
	}
	if (ay > ybottom || by > ybottom || cy > ybottom) {
		tmp = true;
	}
	if (ay < ytop || by < ytop || cy < ytop) {
		tmp = true;
	}
	return tmp;
}

bool test2(float ax, float ay, float bx, float by, float cx, float cy, float xleft, float xright, float ytop, float ybottom)
{
	float alpha;
	float beta;
	float gamma;
	bool tmp = false;

	alpha = f12(xleft, ytop, bx, cx, by, cy)/f12(ax, ay, bx, cx, by, cy);
	beta = f20(xleft, ytop, ax, cx, ay, cy)/f20(bx, by, ax, cx, ay, cy);
	gamma = f01(xleft, ytop, ax, bx, ay, by)/f01(cx, cy, ax, bx, ay, by); 
	
	if (alpha > 0 && beta > 0 && gamma > 0) {
		tmp = true;
	}

	alpha = f12(xleft, ybottom, bx, cx, by, cy)/f12(ax, ay, bx, cx, by, cy);
	beta = f20(xleft, ybottom, ax, cx, ay, cy)/f20(bx, by, ax, cx, ay, cy);
	gamma = f01(xleft, ybottom, ax, bx, ay, by)/f01(cx, cy, ax, bx, ay, by); 
	
	if (alpha > 0 && beta > 0 && gamma > 0) {
		tmp = true;
	}

	alpha = f12(xright, ytop, bx, cx, by, cy)/f12(ax, ay, bx, cx, by, cy);
	beta = f20(xright, ytop, ax, cx, ay, cy)/f20(bx, by, ax, cx, ay, cy);
	gamma = f01(xright, ytop, ax, bx, ay, by)/f01(cx, cy, ax, bx, ay, by); 
	
	if (alpha > 0 && beta > 0 && gamma > 0) {
		tmp = true;
	}

	alpha = f12(xright, ybottom, bx, cx, by, cy)/f12(ax, ay, bx, cx, by, cy);
	beta = f20(xright, ybottom, ax, cx, ay, cy)/f20(bx, by, ax, cx, ay, cy);
	gamma = f01(xright, ybottom, ax, bx, ay, by)/f01(cx, cy, ax, bx, ay, by); 

	if (alpha > 0 && beta > 0 && gamma > 0) {
		tmp = true;
	}
	return tmp;
}

bool intersect(float A0, float A2, float A1, float A3, float B0, float B2, float B1, float B3)
{
	if (((A2-A0)*(B1-B0) + (B2-B0)*(A1-A0)) * ((A3-A0)*(B1-B0) + (B3-B0)*(A1-A0)) < 0 && 
		((A0-A2)*(B3-B2) + (B0-B2)*(A3-A2)) * ((A1-A2)*(B3-B2) + (B1-B2)*(A3-A2)) < 0) {
	  
		return true;
	}
}

bool test3(float ax, float ay, float bx, float by, float cx, float cy, float xleft, float xright, float ytop, float ybottom)
{
	bool tmp = false;
	// AB and top horizontal
	if (intersect(ax, xleft, bx, xright, ay, ytop, by, ytop) == true) {
		return true;
	}
	
	// AC and top horizontal
	if (intersect(ax, xleft, cx, xright, ay, ytop, cy, ytop) == true) {
		return true;
	}

	// BC and top horizontal
	if (intersect(bx, xleft, cx, xright, by, ytop, cy, ytop) == true) {
		return true;
	}
	
	//---------------------
		// AB and bottom horizontal
	if (intersect(ax, xleft, bx, xright, ay, ybottom, by, ybottom) == true) {
		return true;
	}
	
	
	// AC and top horizontal
	if (intersect(ax, xleft, cx, xright, ay, ybottom, cy, ybottom) == true) {
		return true;
	}

	// BC and top horizontal
	if (intersect(bx, xleft, cx, xright, by, ybottom, cy, ybottom) == true) {
		return true;
	}

	//------------------------------
	// bool intersect(float A0, float A2, float A1, float A3, float B0, float B2, float B1, float B3)
	// AB and left vertical
	if (intersect(ax, xleft, bx, xleft, ay, ytop, by, ybottom) == true) {
		return true;
	}
	
	// AC and left vertical
	if (intersect(ax, xleft, cx, xleft, ay, ytop, cy, ybottom) == true) {
		return true;
	}

	// BC and left vertical
	if (intersect(bx, xleft, cx, xright, by, ytop, cy, ybottom) == true) {
		return true;
	}
	
	//---------------------
	// AB and right vertical
	if (intersect(ax, xright, bx, xright, ay, ytop, by, ybottom) == true) {
		return true;
	}
	
	
	// AC and right vertical
	if (intersect(ax, xright, cx, xright, ay, ytop, cy, ybottom) == true) {
		return true;
	}

	// BC and right vertical
	if (intersect(bx, xright, cx, xright, by, ytop, cy, ybottom) == true) {
		return true;
	}
	return tmp;
}

void rasterizeTriangle(Vector4 &a, Vector4 &b, Vector4 &c, float color1, float color2, float color3)
{
	float minx = 0;
	float miny = 0;
	float minz = 0;
	float maxx = 0;
	float maxy = 0;
	float maxz = 0;

	minx = min(min(a.x, b.x), c.x);
	miny = min(min(a.y, b.y), c.y);
	maxx = max(max(a.x, b.x), c.x);
	maxy = max(max(a.y, b.y), c.y);

	float alpha;
	float beta;
	float gamma;

	float a_offset;
	float b_offset;
	float c_offset;

	float cr;
	float cg;
	float cb;
	
	float z_inv_interpolated;
	float z_interpolated;

	float x_interval = (maxx+1 - minx)/divisor;

	float y_interval = (maxy+1 - miny)/divisor;

	bool check1;
	bool check2;
	bool check3;

	float xleft = minx;
	float ytop = miny;
	int xright = xleft+x_interval;
	int ybottom = ytop+y_interval;

	float old_xleft;
	float old_xright;
	
	int counter = 0;
	if (toggle_boxes == true && x_interval >= 1 && y_interval >= 1) {

		for (int xleft = minx; xleft < maxx+1-x_interval; xleft+=x_interval)
		{
			xright = xleft+x_interval;
			for (int ytop = miny; ytop < maxy+1-y_interval; ytop+=y_interval)
			{
				ybottom = ytop+y_interval;
				//cout << "xleft: " << xleft <<  ", xright: " << xright << '\n';
				//cout << "ytop: " << ytop <<  ", ybottom: " << ybottom << '\n';
				

				check1 = test1(a.x, a.y, b.x, b.y, c.x, c.y, xleft, xright, ytop, ybottom);
				check2 = test2(a.x, a.y, b.x, b.y, c.x, c.y, xleft, xright, ytop, ybottom);
				check3 = test3(a.x, a.y, b.x, b.y, c.x, c.y, xleft, xright, ytop, ybottom);

				// inside mini hierachy thing
				if (check1 == true || check2 == true || check3 == true) {
					if (draw_tile_mode == true) {

						for (int m = xleft; m < xright+1; m++) {
							zbuffer[m][ytop] = 0;
							zbuffer[m][ybottom] = 0;
							drawPoint(m, ytop, 1.0, 0, 0);
			
							drawPoint(m, ybottom, 1.0, 0, 0);
						}

						for (int n = ytop; n < ybottom+1; n++) {
							zbuffer[xleft][n] = 0;
							zbuffer[xright][n] = 0;
							drawPoint(xleft, n, 1.0, 0, 0);
							drawPoint(xright, n, 1.0, 0, 0);
						}
					}
					// for all pixels in the mini hiearchy thing
					for (int i = xleft; i < xright+1; i++) {
						for (int j = ytop; j < ybottom+1; j++) {
							alpha = ((b.y - c.y)*(i - c.x) + (c.x - b.x)*(j - c.y)) / 
								((b.y - c.y)*(a.x - c.x) + (c.x - b.x)*(a.y - c.y));
							beta = ((c.y - a.y)*(i - c.x) + (a.x - c.x)*(j - c.y)) / 
								((b.y - c.y)*(a.x - c.x) + (c.x - b.x)*(a.y - c.y));
							gamma = 1.0f - alpha - beta;

							a_offset = a.y*window_width*3 + a.x*3;
							b_offset = b.y*window_width*3 + b.x*3;
							c_offset = c.y*window_width*3 + c.x*3;

							z_interpolated = alpha*a.z + beta*b.z + gamma*c.z;

							if (alpha > 0 && beta > 0 && gamma > 0) {
								update_zbuffer(i, j, color1, color2, color3, z_interpolated);
							}
						}
					}

				}
			}
		}
	}
	else {
		for (int i = minx; i < maxx+1; i++)
		{
	
			for (int j = miny; j < maxy+1; j++)
			{
			
				alpha = ((b.y - c.y)*(i - c.x) + (c.x - b.x)*(j - c.y)) / 
					((b.y - c.y)*(a.x - c.x) + (c.x - b.x)*(a.y - c.y));
				beta = ((c.y - a.y)*(i - c.x) + (a.x - c.x)*(j - c.y)) / 
					((b.y - c.y)*(a.x - c.x) + (c.x - b.x)*(a.y - c.y));
				gamma = 1.0f - alpha - beta;

				a_offset = a.y*window_width*3 + a.x*3;
				b_offset = b.y*window_width*3 + b.x*3;
				c_offset = c.y*window_width*3 + c.x*3;

				z_interpolated = alpha*a.z + beta*b.z + gamma*c.z;

				if (alpha > 0 && beta > 0 && gamma > 0) {
					//cout << "updating\n";
					update_zbuffer(i, j, color1, color2, color3, z_interpolated);
				}    

				
			}
		}
	}
	
}

void rasterizeVertex(Vector4& p1, Vector4& p2, Vector4& p3, float color1, float color2, float color3)
{
	Vector4 tmp1 = model.multiply(p1);
	Vector4 tmp2 = camera.c.multiply(tmp1);
	Vector4 tmp3 = projection.multiply(tmp2);
	Matrix4 temp;
	temp.identity();
	temp.translate(0, 0, -20);
	tmp2 = temp.multiply(tmp3);

	Vector4 a = viewport.multiply(tmp2);
	a.dehomogenize();

	tmp1 = model.multiply(p2);
	tmp2 = camera.c.multiply(tmp1);
	tmp3 = projection.multiply(tmp2);
	Matrix4 temp2;
	temp2.identity();
	temp2.translate(0, 0, -20);
	tmp2 = temp2.multiply(tmp3);

	Vector4 b = viewport.multiply(tmp2);
	b.dehomogenize();


	tmp1 = model.multiply(p3);
	tmp2 = camera.c.multiply(tmp1);
	tmp3 = projection.multiply(tmp2);
	Matrix4 temp3;
	temp3.identity();
	temp3.translate(0, 0, -20);
	tmp2 = temp3.multiply(tmp3);

	Vector4 c = viewport.multiply(tmp2);
	c.dehomogenize();

	rasterizeTriangle(a, b, c, color1, color2, color3);
}

void rasterize()
{
  // Put your main rasterization loop here
  // It should go over the data model and call rasterizeTriangle for every triangle in it
  unsigned long long Int64 = 0;
  clock_t Start = clock();

  for (int i=0; i < indices.size(); i+=3) {
		Vector4 a = Vector4(vertices[3*(indices[i])], vertices[3*(indices[i])+1], vertices[3*(indices[i])+2], 1);
		Vector4 b = Vector4(vertices[3*(indices[i+1])], vertices[3*(indices[i+1])+1], vertices[3*(indices[i+1])+2], 1);
		Vector4 c = Vector4(vertices[3*(indices[i+2])], vertices[3*(indices[i+2])+1], vertices[3*(indices[i+2])+2], 1);

		// floats needed to deal with dark green grass
		float color1 = colors[3*int(indices[i])];
		float color2 = colors[3*int(indices[i])+1];
		float color3 = colors[3*int(indices[i])+2];

		rasterizeVertex(a, b, c, color1, color2, color3);
  }
	if (toggle_boxes)
		cout << "miliseconds elapsed for rendering " << divisor << "x" << divisor << ": " << clock() - Start << '\n';
}

Matrix4& getViewportMatrix()
{
	return viewport;
}

Matrix4& getProjectionMatrix()
{
	return projection;
}

Camera& getCameraMatrix()
{
	return camera;
}

Matrix4& getModelMatrix()
{
	return model;
}

void setViewportMatrix()
{
	float x = window_width;
	float y = window_height;
	float x0 = 0;
	float y0 = 0;

	getViewportMatrix() = 
		Matrix4((x-x0)/2, 0, 0, (x+x0)/2,
				0, (y-y0)/2, 0, (y+y0)/2,
				0, 0, 0.5, 0.5,
				0, 0, 0, 1);
}

void setProjectionMatrix() {
  getProjectionMatrix().identity();

  double left = -10.0;
  double right = 10.0;
  double bottom = -10.0;
  double top = 10.0;
  double nearV = 10.0;
  double farV = 1000.0;

  projection = 
	  Matrix4(2*nearV/(right-left), 0, (right+left)/(right-left), 0,
	          0, 2*nearV/(top-bottom), (top+bottom)/(top-bottom), 0,
			  0, 0, -1*(farV+nearV)/(farV-nearV), -2*farV*nearV/(farV-nearV),
			  0, 0, -1, 0);
  getProjectionMatrix().translate(0, 0, -20);
}

void setCameraMatrix(Vector3 &e, Vector3 &d, Vector3 &up)
{
	getCameraMatrix().set(e, d, up);	
}

void display()
{
  clearBuffer();
  rasterize();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  // glDrawPixels writes a block of pixels to the framebuffer
  glDrawPixels(window_width, window_height, GL_RGB, GL_FLOAT, pixels);

  glutSwapBuffers();
}

// Called whenever the window size changes
void reshape(int new_width, int new_height)
{
  window_width  = new_width;
  window_height = new_height;

	delete[] pixels;

  setProjectionMatrix();
	setViewportMatrix();
  
	pixels = new float[window_width*window_height*3];
}

void idleCallback(void)
{
	spin(spin_angle);
	initialize_zbuffer();
  display();
}

void keyboard(unsigned char key, int, int)
{
  cerr << "Key pressed: " << key << endl;

	switch (key) {
		case 'o':
			toggle_boxes = !toggle_boxes;
			toggle_boxes ? cout << "using hierarchy\n" : cout << "not using hierarchy\n";
			
			break;
		case 'b':
			draw_tile_mode = !draw_tile_mode;
			if (toggle_boxes) {
				draw_tile_mode ? cout << "drawing boxes\n" : cout << "undrawing boxes\n";
			}
			else {
				cout << "press 'o' key to turn on hierarchy first\n";
			}
			break;
	}
	initialize_zbuffer();
	display();
}

void loadCustomShape(float *custom_vertices, int custom_nVerts, int *custom_indices, int custom_nIndices) {
	bool red = true;

	nVerts = custom_nVerts*3;
	nIndices = custom_nIndices;
	vertices.resize(nVerts);
	for (int i=0; i<nVerts; i++) {
		vertices[i] = custom_vertices[i];
	}
	
	colors.resize(nVerts);
	for (int i=0; i<nVerts; i+=3) {
		if (red) {
			colors[i] = 0;
			colors[i+1] = 0;
			colors[i+2] = 1;
		}
		else {
			colors[i] = 1;
			colors[i+1] = 1;
			colors[i+2] = 1;
		}

		if (i % 12 == 0)
			red = !red;
	}

	indices.resize(nIndices);
	for (int i=0; i<nIndices; i++)
		indices[i] = custom_indices[i];
}

void keyboard_fkeys(int key, int, int) {
	bool red = true;

	switch (key) {
		case GLUT_KEY_F1:
			// cube
			loadCube();

			getModelMatrix().identity();
			getModelMatrix().scale(150,150,150);

			getCameraMatrix().identity();
			getCameraMatrix().c.transpose();
			getCameraMatrix().inverse();
			setViewportMatrix();
			break;
		case GLUT_KEY_F2:
			// sphere
			loadCustomShape(sphere_vertices, sphere_nVerts, sphere_indices, sphere_nIndices);

			getModelMatrix().identity();
			getModelMatrix().scale(400,400,400);

			getCameraMatrix().identity();
			getCameraMatrix().c.transpose();
			getCameraMatrix().inverse();
			break;
		case GLUT_KEY_F3:
			// teddy
			loadCustomShape(teddy_vertices, teddy_nVerts, teddy_indices, teddy_nIndices);

			getModelMatrix().identity();
			getModelMatrix().scale(12,12,12);

			getCameraMatrix().identity();
			getCameraMatrix().c.transpose();
			getCameraMatrix().inverse();
			break;
		case GLUT_KEY_F4:
			// teapot
			loadCustomShape(teapot_vertices, teapot_nVerts, teapot_indices, teapot_nIndices);
			
			getModelMatrix().identity();
			getModelMatrix().scale(100, 100, 100);
			getModelMatrix().transpose();
			getModelMatrix().translate(0,-150,0);
			getModelMatrix().transpose();

			getCameraMatrix().identity();
			getCameraMatrix().c.transpose();
			getCameraMatrix().inverse();
			break;
		case GLUT_KEY_F5:
			// cow
			loadCustomShape(cow_vertices, cow_nVerts, cow_indices, cow_nIndices);

			getModelMatrix().identity();
			getModelMatrix().scale(50,50,50);

			getCameraMatrix().identity();
			getCameraMatrix().c.transpose();
			getCameraMatrix().inverse();
			break;
		case GLUT_KEY_F6:
			// bunny
			loadCustomShape(bunny_vertices, bunny_nVerts, bunny_indices, bunny_nIndices);

			getModelMatrix().identity();
			getModelMatrix().scale(200,200,200);
			getModelMatrix().transpose();
			getModelMatrix().translate(75,-275,0);
			getModelMatrix().transpose();

			getCameraMatrix().identity();
			getCameraMatrix().c.transpose();
			getCameraMatrix().inverse();
			break;
		case GLUT_KEY_F8:
			// house view 1
			loadHouse();

			getModelMatrix().identity();
			getModelMatrix().scale(12,12,12);

			getCameraMatrix().identity();
			getCameraMatrix().set(Vector3(0, 10, 10), Vector3(0, 0, 0), Vector3(0, 1, 0));
			getCameraMatrix().c.transpose();
			getCameraMatrix().inverse();

			break;
		case GLUT_KEY_F9:
			// house view 2
			loadHouse();
			
			getModelMatrix().identity();
			getModelMatrix().scale(12,12,12);

			getCameraMatrix().identity();
			getCameraMatrix().set(Vector3(-15, 5, 10), Vector3(-5, 0, 0), Vector3(0, 1, 0.5));
			getCameraMatrix().c.transpose();
			getCameraMatrix().inverse();
			break;
	}

	setProjectionMatrix();
	setViewportMatrix();
	initialize_zbuffer();
	display();
}

void printer(Matrix4& matrix)
{
  cout << matrix(0,  0) << ", " << matrix(0,  1) << ", " << matrix(0,  2) << ", " << matrix(0,  3) << '\n';
  cout << matrix(1,  0) << ", " << matrix(1,  1) << ", " << matrix(1,  2) << ", " << matrix(1,  3) << '\n';
  cout << matrix(2,  0) << ", " << matrix(2,  1) << ", " << matrix(2,  2) << ", " << matrix(2,  3) << '\n';
  cout << matrix(3,  0) << ", " << matrix(3,  1) << ", " << matrix(3,  2) << ", " << matrix(3,  3) << '\n';
}

int main(int argc, char** argv) {
  initialize_zbuffer();
  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(window_width, window_height);
  glutCreateWindow("CSE 167 Project 3");

  getModelMatrix().identity();
  getModelMatrix().scale(12, 12, 12);

  getCameraMatrix().identity();

  getCameraMatrix().set(Vector3(0, 10, 10), Vector3(0, 0, 0), Vector3(0, 1, 0));
  getCameraMatrix().c.transpose();
  getCameraMatrix().inverse();

	setProjectionMatrix();  
  setViewportMatrix();

	loadData();

  glutReshapeFunc(reshape);
  glutDisplayFunc(display);
	glutIdleFunc(idleCallback);
  glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboard_fkeys);
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glutMainLoop();
}