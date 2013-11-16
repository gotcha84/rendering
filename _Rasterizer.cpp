#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <math.h>
#include <GL/gl.h>
#include <GL/glut.h>
//#include "Shape.h"
#include "Matrix4.h"
#include "Camera.h"
static int window_width = 512, window_height = 512;
static float* pixels = new float[window_width * window_height * 3];
static float zbuffer[512][512];
static float zmax = 0;

Matrix4 model;
Camera camera;
Matrix4 projection;
// HAVE COLORS, INDICES

Matrix4 viewport;
Matrix4 tmp;

//static Shape shape;

float house_vertices[] = {  
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
float house_colors[] = { 
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

int house_indices[] = {
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

using namespace std;

struct Color    // generic color
{
  float r,g,b;
};

// projects each vertex of the house to image coordinates and sets 
// the color of the corresponding pixel to white using drawPoint.





void loadData()
{
  // put code to load data model here
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
void drawPoint(int x, int y, float r, float g, float b)
{
  int offset = y*window_width*3 + x*3;
  pixels[offset]   = r;
  pixels[offset+1] = g;
  pixels[offset+2] = b;
}

void update_zbuffer(int x, int y, float c1, float c2, float c3, float z)
{
	cout << "entered updating with: ", 
	cout << "x: " << x << ", y: " << y << '\n';
	cout << "c1: " << c1 << ", c2: " << c2 << ", c3: " << c3 << '\n';
	if (z < zbuffer[x][y] && x >= 0 && y >= 0)
	{
		cout << "drawing: " << "x: " << x << ", y: " << y << '\n';
		cout << "colors: " << "c1: " << c1 << ", c2: " << c2 << ", c3: " << c3 << '\n';
		zbuffer[x][y] = z;
		drawPoint(x, y, c1, c2, c3);
	}
}

int f01(int x, int y, int x0, int x1, int y0, int y1)
{
	int term1 = (y0-y1)*x;
	int term2 = (x1-x0)*y;
	int term3 = x0*y1;
	int term4 = x1*y0;

	return term1 + term2 + term3 - term4;
}

int f12(int x, int y, int x1, int x2, int y1, int y2)
{
	int term1 = (y1-y2)*x;
	int term2 = (x2-x1)*y;
	int term3 = x1*y2;
	int term4 = x2*y1;
	//if (term1 + term2 + term3 - term4 == 0)
	//{
	//	cout << term1 << ", " << term2 << ", " << term3 << ", " << term4 << '\n';
	//}

	return term1 + term2 + term3 - term4;
}

int f20(int x, int y, int x0, int x2, int y0, int y2)
{
	int term1 = (y2-y0)*x;
	int term2 = (x0-x2)*y;
	int term3 = x2*y0;
	int term4 = x0*y2;

	return term1 + term2 + term3 - term4;
}

bool test1(int ax, int ay, int bx, int by, int cx, int cy, int xleft, int xright, int ytop, int ybottom)
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

bool test2(int ax, int ay, int bx, int by, int cx, int cy, int xleft, int xright, int ytop, int ybottom)
{
	float alpha;
	float beta;
	float gamma;
	bool tmp = false;
			/*
			if (f12(ax, ay, bx, cx, by, cy) == 0)
			{
				cout << "2f12\n";
			}
			if (f20(bx, by, ax, cx, ay, cy) == 0)
			{
				cout << "2f20\n";
			}
			//cout << "HI";
			if (f01(cx, cy, ax, bx, ay, by) == 0)
			{
				cout << "2f01\n";
			}
			*/
	alpha = f12(xleft, ytop, bx, cx, by, cy)/f12(ax, ay, bx, cx, by, cy);
	beta = f20(xleft, ytop, ax, cx, ay, cy)/f20(bx, by, ax, cx, ay, cy);
	gamma = f01(xleft, ytop, ax, bx, ay, by)/f01(cx, cy, ax, bx, ay, by); 
	
	if (alpha > 0 && beta > 0 && gamma > 0) {
		tmp = true;
	}
	cout << "HI";
	alpha = f12(xleft, ybottom, bx, cx, by, cy)/f12(ax, ay, bx, cx, by, cy);
	beta = f20(xleft, ybottom, ax, cx, ay, cy)/f20(bx, by, ax, cx, ay, cy);
	gamma = f01(xleft, ybottom, ax, bx, ay, by)/f01(cx, cy, ax, bx, ay, by); 
	
	if (alpha > 0 && beta > 0 && gamma > 0) {
		tmp = true;
	}
	cout << "HI";
	alpha = f12(xright, ytop, bx, cx, by, cy)/f12(ax, ay, bx, cx, by, cy);
	beta = f20(xright, ytop, ax, cx, ay, cy)/f20(bx, by, ax, cx, ay, cy);
	gamma = f01(xright, ytop, ax, bx, ay, by)/f01(cx, cy, ax, bx, ay, by); 
	
	if (alpha > 0 && beta > 0 && gamma > 0) {
		tmp = true;
	}
	cout << "HI";
	alpha = f12(xright, ybottom, bx, cx, by, cy)/f12(ax, ay, bx, cx, by, cy);
	beta = f20(xright, ybottom, ax, cx, ay, cy)/f20(bx, by, ax, cx, ay, cy);
	gamma = f01(xright, ybottom, ax, bx, ay, by)/f01(cx, cy, ax, bx, ay, by); 
	cout << "HI";
	if (alpha > 0 && beta > 0 && gamma > 0) {
		tmp = true;
	}
	cout << "HI";
	return tmp;
}
/*
bool intersect(int xleft1, int xleft2, int xright1, int xright2, int ytop1, int ytop2, int ybottom1, int ybottom2)
{
	if ((A2-A0)*(B1-B0) + (B2-B0)*(A1-A0)) * ((A3-A0)*(B1-B0) + (B3-B0)*(A1-A0)) < 0
&&
((A0-A2)*(B3-B2) + (B0-B2)*(A3-A2)) * ((A1-A2)*(B3-B2) + (B1-B2)*(A3-A2)) < 0) {
	  return true;
	}
}*/

bool intersect(int A0, int A2, int A1, int A3, int B0, int B2, int B1, int B3)
{
	if (((A2-A0)*(B1-B0) + (B2-B0)*(A1-A0)) * ((A3-A0)*(B1-B0) + (B3-B0)*(A1-A0)) < 0
&&
((A0-A2)*(B3-B2) + (B0-B2)*(A3-A2)) * ((A1-A2)*(B3-B2) + (B1-B2)*(A3-A2)) < 0) {
	  return true;
	}
}

bool test3(int ax, int ay, int bx, int by, int cx, int cy, int xleft, int xright, int ytop, int ybottom)
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
	// bool intersect(int A0, int A2, int A1, int A3, int B0, int B2, int B1, int B3)
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

void rasterizeTriangle(Vector4 &a, Vector4 &b, Vector4 &c, int color1, int color2, int color3)
{
	float minx = 0;
	float miny = 0;
	float minz = 0;
	float maxx = 0;
	float maxy = 0;
	float maxz = 0;

	if (a.x < b.x)  {
		if (c.x < a.x) {
			minx = c.x;
		}
		else {
			minx = a.x;
		}
		if (c.x > b.x) {
			maxx = c.x;
		}
		else {
			maxx = b.x;
		}
	}
	else {
		if (c.x < b.x) {
			minx = c.x;
		}
		else {
			minx = b.x;
		}
		if (c.x > a.x) {
			maxx = c.x;
		}
		else {
			maxx = a.x;
		}
	}


	if (a.y < b.y)  {
		if (c.y < a.y) {
			miny = c.y;
		}
		else {
			miny = a.y;
		}
		if (c.y > b.y) {
			maxy = c.y;
		}
		else {
			maxy = b.y;
		}
	}
	else {
		if (c.y < b.y) {
			miny = c.y;
		}
		else {
			miny = b.y;
		}
		if (c.y > a.y) {
			maxy = c.y;
		}
		else {
			maxy = a.y;
		}
	}

	float alpha;
	float beta;
	float gamma;

	int a_offset;
	int b_offset;
	int c_offset;

	float cr;
	float cg;
	float cb;
	
	float z_inv_interpolated;
	float z_interpolated;

/*
void drawPoint(int x, int y, float r, float g, float b)
{
  int offset = y*window_width*3 + x*3;
  pixels[offset]   = r;
  pixels[offset+1] = g;
  pixels[offset+2] = b;
}

void update_zbuffer(int x, int y, float c1, float c2, float c3, float z)
{
	if (z < zbuffer[x][y])
	{
		zbuffer[x][y] = z;
		drawPoint(x, y, c1, c2, c3);
	}
}


*/
	/*
	for (int i = minx; i < maxx+1; i++)
	{
		for (int j = miny; j < maxy+1; j++)
		{
			alpha = f12(i, j, b.x, c.x, b.y, c.y)/f12(a.x, a.y, b.x, c.x, b.y, c.y);
			beta = f20(i, j, a.x, c.x, a.y, c.y)/f12(b.x, b.y, a.x, c.x, a.y, c.y);
			gamma = f01(i, j, a.x, b.x, a.y, b.y)/f12(c.x, c.y, a.x, b.x, a.y, b.y);

			a_offset = a.y*window_width*3 + a.x*3;
			b_offset = b.y*window_width*3 + b.x*3;
			c_offset = c.y*window_width*3 + c.x*3;

			z_inv_interpolated = alpha*(1/a.z) + beta*(1/b.z) + gamma*(1/c.z);
			z_interpolated = 1/z_inv_interpolated;

			if (alpha > 0 && beta > 0 && gamma > 0)
			{
				cr = alpha*pixels[a_offset] + beta*pixels[b_offset] + gamma*pixels[c_offset];
				cg = alpha*pixels[a_offset+1] + beta*pixels[b_offset+1] + gamma*pixels[c_offset+1];
				cb = alpha*pixels[a_offset+2] + beta*pixels[b_offset+2] + gamma*pixels[c_offset+2];
			
				update_zbuffer(i, j, cr, cg, cb, z_interpolated);
				//drawPoint(i, j, cr, cg, cb);
			}
		}
	}
	*/
	// CHANGE THIS TO SEE REUSLTS
	int divisor = 2;
	int x_interval = (maxx+1 - minx)/divisor; 
	int y_interval = (maxy+1 - miny)/divisor;

	/*
	cout << "divisor: \n";
	cout << divisor << '\n';
	cout << "x_interval: \n";
	cout << x_interval << '\n';
	cout << "y_interval: \n";
	cout << y_interval << '\n';
	*/
	bool check1;
	bool check2;
	bool check3;

	int xleft = minx;
	int ytop = miny;
	int xright = xleft+x_interval;
	int ybottom = ytop+y_interval;

	int counter = 0;


	for (int i = minx; i < maxx+1; i++)
	{
		for (int j = miny; j < maxy+1; j++)
		{
			//cout << "IS THIS IT: " << f12(a.x, a.y, b.x, c.x, b.y, c.y) << '\n';
			if (f12(a.x, a.y, b.x, c.x, b.y, c.y) == 0)
			{
				alpha = 0;
			}
			else
			{
				alpha = f12(i, j, b.x, c.x, b.y, c.y)/f12(a.x, a.y, b.x, c.x, b.y, c.y);
				if (alpha < 0)
				{
					alpha *= -1;
				}
			}
			if (f20(b.x, b.y, a.x, c.x, a.y, c.y) == 0)
			{
				beta = 0;
			}
			else
			{
				beta = f20(i, j, a.x, c.x, a.y, c.y)/f20(b.x, b.y, a.x, c.x, a.y, c.y);
				if (beta < 0)
				{
					beta *= -1;
				}
			}
			if (f01(c.x, c.y, a.x, b.x, a.y, b.y) == 0)
			{
				gamma = 0;
			}
			else
			{
				gamma = f01(i, j, a.x, b.x, a.y, b.y)/f01(c.x, c.y, a.x, b.x, a.y, b.y);
				if (gamma < 0)
				{
					gamma *= -1;
				}
			}
			/*
			if (f12(a.x, a.y, b.x, c.x, b.y, c.y) == 0)
			{
				cout << f12(a.x, a.y, b.x, c.x, b.y, c.y) << '\n';
			}
			if (f20(b.x, b.y, a.x, c.x, a.y, c.y) == 0)
			{
				cout << f20(b.x, b.y, a.x, c.x, a.y, c.y) << '\n';
			}
			//cout << "HI";
			if (f01(c.x, c.y, a.x, b.x, a.y, b.y) == 0)
			{
				cout << f01(c.x, c.y, a.x, b.x, a.y, b.y) << '\n';
			}
			*/
			a_offset = a.y*window_width*3 + a.x*3;
			b_offset = b.y*window_width*3 + b.x*3;
			c_offset = c.y*window_width*3 + c.x*3;

			z_inv_interpolated = alpha*(1/a.z) + beta*(1/b.z) + gamma*(1/c.z);
			z_interpolated = 1/z_inv_interpolated;
			cout << "alpha: " << alpha << ", beta: " << beta << ", gamma: " << gamma << '\n';
			if (alpha > 0 && beta > 0 && gamma > 0)
			{
				cout << "JOJO\n";
				//cr = alpha*pixels[a_offset] + beta*pixels[b_offset] + gamma*pixels[c_offset];
				//cg = alpha*pixels[a_offset+1] + beta*pixels[b_offset+1] + gamma*pixels[c_offset+1];
				//cb = alpha*pixels[a_offset+2] + beta*pixels[b_offset+2] + gamma*pixels[c_offset+2];
			
				update_zbuffer(i, j, color1, color2, color3, z_interpolated);
				//drawPoint(i, j, cr, cg, cb);
			}
		}
	}

	//uncomment below for true. above uncommented is jsut testing
	/*
	for (int xleft = minx; xleft < maxx+1-x_interval; xleft+=x_interval)
	{
		xright = xleft+x_interval;
		for (int ytop = miny; ytop < maxy+1-y_interval; ytop+=y_interval)
		{
			ybottom = ytop+y_interval;




			check1 = test1(a.x, a.y, b.x, b.y, c.x, c.y, xleft, xright, ytop, ybottom);
			check2 = test2(a.x, a.y, b.x, b.y, c.x, c.y, xleft, xright, ytop, ybottom);
			check3 = test3(a.x, a.y, b.x, b.y, c.x, c.y, xleft, xright, ytop, ybottom);

			// inside mini hierachy thing
			if (check1 == true || check2 == true || check3 == true)
			//if (check1 == true || check3 == true)
			{
				// for all pixels in the mini hiearchy thing
				for (int i = xleft; i < xright+1; i++)
				{
					for (int j = ytop; j < ybottom+1; j++)
					{
						alpha = f12(xleft, ytop, b.x, c.x, b.y, c.y)/f12(a.x, a.y, b.x, c.x, b.y, c.y);
						beta = f20(xleft, ytop, a.x, c.x, a.y, c.y)/f20(b.x, b.y, a.x, c.x, a.y, c.y);
						gamma = f01(xleft, ytop, a.x, b.x, a.y, b.y)/f01(c.x, c.y, a.x, b.x, a.y, b.y);

						if (f12(a.x, a.y, b.x, c.x, b.y, c.y) == 0)
						{
							cout << "f12\n";
						}
						if (f20(b.x, b.y, a.x, c.x, a.y, c.y) == 0)
						{
							cout << "f20\n";
						}
						//cout << "HI";
						if (f01(c.x, c.y, a.x, b.x, a.y, b.y) == 0)
						{
							cout << "f01\n";
						}

						a_offset = a.y*window_width*3 + a.x*3;
						b_offset = b.y*window_width*3 + b.x*3;
						c_offset = c.y*window_width*3 + c.x*3;

						if (a.z == 0)
						{
							cout << "az\n";
						}
						if (b.z == 0)
						{
							cout << "bz\n";
						}
						if (c.z == 0)
						{
							cout << "cz\n";
						}
						z_inv_interpolated = alpha*(1/a.z) + beta*(1/b.z) + gamma*(1/c.z);

						if (z_inv_interpolated == 0)
						{
							cout << "z_inv\n";
						}
						z_interpolated = 1/z_inv_interpolated;

						if (alpha > 0 && beta > 0 && gamma > 0)
						{
							cr = alpha*pixels[a_offset] + beta*pixels[b_offset] + gamma*pixels[c_offset];
							cg = alpha*pixels[a_offset+1] + beta*pixels[b_offset+1] + gamma*pixels[c_offset+1];
							cb = alpha*pixels[a_offset+2] + beta*pixels[b_offset+2] + gamma*pixels[c_offset+2];
			
							update_zbuffer(i, j, color1, color2, color3, z_interpolated);
							// MAYBE NEED LINE BELOW
							//update_zbuffer(i, j, cr, cg, cb, z_interpolated);
							
						}
					}
				}
			}
			counter++;
			cout << "in the loop: " << counter << " times\n";
		
		}
	}*/
	
}

void rasterizeVertex(Vector4& p1, Vector4& p2, Vector4& p3, int color1, int color2, int color3)
{
	// change below
	/*
	for (int i = 0; i < sizeof(house_vertices); i+=3)
	{
		Vector4 p = Vector4(house_vertices[i], house_vertices[i+1], house_vertices[i+2], 1);
		Vector4 tmp1 = *getModelMatrix().multiply(p);
		Vector4 tmp2 = *getCameraMatrix().c.multiply(tmp1);
		Vector4 tmp3 = *getProjectionMatrix().multiply(tmp2);
		Vector4 tmp4 = *getViewportMatrix().multiply(tmp3);

		drawPoint(tmp4.get(0)/tmp4.get(4), tmp4.get(1)/tmp4.get(4), 1, 1, 1);
	}*/
	// sizeof(house_vertices)

	/*
	for (int i = 0; i < sizeof(house_vertices)/sizeof(float); i+=3)
	{
		Vector4 p = Vector4(house_vertices[i], house_vertices[i+1], house_vertices[i+2], 1);
		Vector4 tmp1 = *model.multiply(p);
		Vector4 tmp2 = *camera.c.multiply(tmp1);
		Vector4 tmp3 = *projection.multiply(tmp2);
		Vector4 tmp4 = *viewport.multiply(tmp3);

		cout << int(tmp4.get(0)/tmp4.get(3)) << " " << int(tmp4.get(1)/tmp4.get(3)) << '\n';

		drawPoint(int(tmp4.get(0)/tmp4.get(3)), int(tmp4.get(1)/tmp4.get(3)), 1.0, 0.0, 0.0);
		//drawPoint(tmp4.getX()/tmp4.getW(), tmp4.getY()/tmp4.getW(), 1.0, 1.0, 1.0);
	}
	*/

	Vector4 tmp1 = model.multiply(p1);
	Vector4 tmp2 = camera.c.multiply(tmp1);
	Vector4 tmp3 = projection.multiply(tmp2);
	Matrix4 temp;
	temp.identity();
	temp.translate(0, 0, -20);
	tmp2 = temp.multiply(tmp3);

	Vector4 a = viewport.multiply(tmp2);

	// NEEDED?
	//cout << "a33\n";
	//cout << a.get(3) << '\n';
	a.dehomogenize();

	tmp1 = model.multiply(p2);
	tmp2 = camera.c.multiply(tmp1);
	tmp3 = projection.multiply(tmp2);
	Matrix4 temp2;
	temp2.identity();
	temp2.translate(0, 0, -20);
	tmp2 = temp2.multiply(tmp3);

	Vector4 b = viewport.multiply(tmp2);

	// NEEDED?
	//cout << "b33\n";
	//cout << b.get(3) << '\n';
	b.dehomogenize();

	tmp1 = model.multiply(p3);
	tmp2 = camera.c.multiply(tmp1);
	tmp3 = projection.multiply(tmp2);
	Matrix4 temp3;
	temp3.identity();
	temp3.translate(0, 0, -20);
	tmp2 = temp3.multiply(tmp3);

	Vector4 c = viewport.multiply(tmp2);

	// NEEDED?
	//cout << "c33\n";
	//cout << c.get(3) << '\n';
	c.dehomogenize();

	rasterizeTriangle(a, b, c, color1, color2, color3);
	zmax = zmax + 3;
	//cout << int(tmp4.get(0)/tmp4.get(3)) << " " << int(tmp4.get(1)/tmp4.get(3)) << '\n';

	// Vector4 p = Matrix4(house_vertices[i], );

	cout << "ZMAX -- " << zmax;

	
	
}

void rasterize()
{
  // Put your main rasterization loop here
  // It should go over the data model and call rasterizeTriangle for every triangle in it
  
  // example: draw diagonal line:
 // void rasterizeTriangle(Vector4 &a, Vector4 &b, Vector4 &c)
  for (int i=0; i < sizeof(house_indices)/sizeof(float); i+=3)
  {
		Vector4 a = Vector4(house_vertices[3*int(house_indices[i])], house_vertices[3*int(house_indices[i]+1)], house_vertices[3*int(house_indices[i])+2], 1);	
		Vector4 b = Vector4(house_vertices[3*int(house_indices[i+1])], house_vertices[3*int(house_indices[i+1])+1], house_vertices[3*int(house_indices[i+1])+2], 1);
		Vector4 c = Vector4(house_vertices[3*int(house_indices[i+2])], house_vertices[3*int(house_indices[i+2])+1], house_vertices[3*int(house_indices[i+2])+2], 1);

		// floats needed to deal with dark green grass
		int color1 = house_colors[3*int(house_indices[i])];
		int color2 = house_colors[3*int(house_indices[i])+1];
		int color3 = house_colors[3*int(house_indices[i])+2];
		//cout << "colors: " << color1 << ", " << color2 << ", " << color3 << '\n';
	  rasterizeVertex(a, b, c, color1, color2, color3);
  }
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
	//int x0 = GLUT_INIT_WINDOW_WIDTH;
	//int y0 = GLUT_INIT_WINDOW_HEIGHT;
	float x0 = 0;
	float y0 = 0;

	getViewportMatrix() = 
		Matrix4((x-x0)/2, 0, 0, (x+x0)/2,
				0, (y-y0)/2, 0, (y+y0)/2,
				0, 0, 0.5, 0.5,
				0, 0, 0, 1);
										
		
}

void setProjectionMatrix()
{
					
}

void setCameraMatrix(Vector3 &e, Vector3 &d, Vector3 &up)
{
	getCameraMatrix().set(e, d, up);	
}

void setModelMatrix()
{

}


// Called whenever the window size changes
void reshape(int new_width, int new_height)
{
  window_width  = new_width;
  window_height = new_height;
  delete[] pixels;
  setViewportMatrix();
  pixels = new float[window_width * window_height * 3];
}

void keyboard(unsigned char key, int, int)
{
  cerr << "Key pressed: " << key << endl;
}



void display()
{
  clearBuffer();
  rasterize();
  //rasterizeVertex();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  // glDrawPixels writes a block of pixels to the framebuffer
  glDrawPixels(window_width, window_height, GL_RGB, GL_FLOAT, pixels);

  glutSwapBuffers();
}

void printer(Matrix4& matrix)
{
  cout << matrix(0,  0) << ", " << matrix(0,  1) << ", " << matrix(0,  2) << ", " << matrix(0,  3) << '\n';
  cout << matrix(1,  0) << ", " << matrix(1,  1) << ", " << matrix(1,  2) << ", " << matrix(1,  3) << '\n';
  cout << matrix(2,  0) << ", " << matrix(2,  1) << ", " << matrix(2,  2) << ", " << matrix(2,  3) << '\n';
  cout << matrix(3,  0) << ", " << matrix(3,  1) << ", " << matrix(3,  2) << ", " << matrix(3,  3) << '\n';

}
int main(int argc, char** argv) {
  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(window_width, window_height);
  glutCreateWindow("CSE 167 Project 3");

  getModelMatrix().identity();

  getCameraMatrix().identity();
  getCameraMatrix().set(Vector3(-15, 5, 10), Vector3(-5, 0, 0), Vector3(0, 1, 0.5));
  //getCameraMatrix().set(Vector3(0, 10, 10), Vector3(0, 0, 0), Vector3(0, 1, 0));
  getCameraMatrix().c.transpose();
  getCameraMatrix().inverse();


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
  // MAYBE UNCMOMENT BELOW
  getProjectionMatrix().translate(0, 0, -20);

  //cout << "projection: \n";
  //printer(projection);
  //cout << '\n';
  
  setViewportMatrix();
  //viewport.identity();

  //cout << "viewport: \n";
  //printer(viewport);
  //cout << '\n';

  //glutKeyboardFunc(Window::processNormalKeys);
  //glutSpecialFunc(Window::processSpecialKeys);
  loadData();

  glutReshapeFunc(reshape);
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glutMainLoop();
}