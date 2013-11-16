/*#ifdef _WIN32
  #include <windows.h>
#endif

#include <iostream>
#include <math.h>
#include <GL/gl.h>
#include <GL/glut.h>

static int window_width = 512, window_height = 512;
static float* pixels = new float[window_width * window_height * 3];

float house_vertices2[] = {  
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
float house_colors2[] = { 
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

int house_indices2[] = {
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

void rasterizeTriangle()
{
  // Add code to rasterize a triangle
}

void rasterize()
{
  // Put your main rasterization loop here
  // It should go over the data model and call rasterizeTriangle for every triangle in it
  
  // example: draw diagonal line:
  for (int i=0; i<min(window_width,window_height); ++i)
  {
    drawPoint(i, i, 1.0, 0.0, 0.0);
  }
}

// Called whenever the window size changes
void reshape(int new_width, int new_height)
{
  window_width  = new_width;
  window_height = new_height;
  delete[] pixels;
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

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // glDrawPixels writes a block of pixels to the framebuffer
  glDrawPixels(window_width, window_height, GL_RGB, GL_FLOAT, pixels);

  glutSwapBuffers();

		int tris = 0;
	glBegin(GL_QUADS);
	for (int i=0; i<7; i++) {
		glColor3f(house_colors2[12*i], house_colors2[12*i+1], house_colors2[12*i+2]);
		glNormal3f(house_indices2[i*6], house_indices2[i*6+1], house_indices2[i*6+2]);
		for (int v=0; v<4; v++) {
			glVertex3f(house_vertices2[12*i+3*v], house_vertices2[(12*i)+(3*v)+1], house_vertices2[(12*i)+(3*v)+2]);
		}
		glNormal3f(house_indices2[i*6+3], house_indices2[i*6+4], house_indices2[i*6+5]);
	}
	glEnd();

	glBegin(GL_TRIANGLES);
	for (int i=7; i<8; i++) {
		glColor3f(house_colors2[12*i], house_colors2[12*i+1], house_colors2[12*i+2]);
		glNormal3f(house_indices2[i*6], house_indices2[i*6+1], house_indices2[i*6+2]);
		for (int v=0; v<3; v++) {
			glVertex3f(house_vertices2[12*i+3*v], house_vertices2[(12*i)+(3*v)+1], house_vertices2[(12*i)+(3*v)+2]);
		}
		tris++;
	}
	glEnd();

	glBegin(GL_QUADS);
	for (int i=8; i<10; i++) {
		glColor3f(house_colors2[12*i-3*tris], house_colors2[12*i+1-3*tris], house_colors2[12*i+2-3*tris]);
		glNormal3f(house_indices2[i*6-3*tris], house_indices2[i*6+1-3*tris], house_indices2[i*6+2-3*tris]);
		for (int v=0; v<4; v++) {
			glVertex3f(house_vertices2[12*i+3*v-3*tris], house_vertices2[(12*i)+(3*v)+1-3*tris], house_vertices2[(12*i)+(3*v)+2-3*tris]);
		}
		glNormal3f(house_indices2[i*6+3-3*tris], house_indices2[i*6+4-3*tris], house_indices2[i*6+5-3*tris]);
	}
	glEnd();
	
	glBegin(GL_TRIANGLES);
	for (int i=10; i<11; i++) {
		glColor3f(house_colors2[12*i-3*tris], house_colors2[12*i+1-3*tris], house_colors2[12*i+2-3*tris]);
		glNormal3f(house_indices2[i*6]-3*tris, house_indices2[i*6+1]-3*tris, house_indices2[i*6+2]-3*tris);
		for (int v=0; v<3; v++) {
			glVertex3f(house_vertices2[12*i+3*v-3*tris], house_vertices2[(12*i)+(3*v)+1-3*tris], house_vertices2[(12*i)+(3*v)+2-3*tris]);
		}
		tris++;
	}
	glEnd();
}

int main(int argc, char** argv) {
  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(window_width, window_height);
  glutCreateWindow("CSE 167 Project 3");

  loadData();

  glutReshapeFunc(reshape);
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glutMainLoop();
}*/