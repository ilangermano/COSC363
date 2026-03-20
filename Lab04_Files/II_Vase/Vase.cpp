//  ========================================================================
//  COSC363: Computer Graphics (2025);  University of Canterbury.
//
//  FILE NAME: Vase.cpp
//  See Lab04 (II) for details
//  ========================================================================
#define _USE_MATH_DEFINES 
#include <iostream>
#include <cmath> 
#include <GL/freeglut.h>
#include "loadBMP.h"
using namespace std;


const int N = 50;	// Total number of vertices on the base curve

float vx_init[N] = { 0, 8, 8, 7.5, 6.7, 5, 5.5, 4, 4, 5, 5.6, 6.1, 6.8, 7.1, 7.5, 8, 8.4, 8.7, 9, 9.3,
                      9.8, 10, 10.2, 10.4, 10.6, 10.9, 11, 11.1, 11.2, 11.3, 11.4, 11.3, 11.2, 11.1, 11, 10.5, 9.5, 8.2, 7, 6.2,
                      6, 6.2, 6.8, 7.6, 8.5, 7, 6.1, 5.3, 4.7, 4.5 };
float vy_init[N] = { 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
                      19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
                      39, 40, 41, 42, 43, 43, 42, 41, 40, 39 };
float nx_init[N] = { 0, 0.707107, 0.973249, 0.842335, 0.654779, 0.958896, 0.9665, 0.881675, 0.92388, 0.788206, 0.876606, 0.859152, 0.899972, 0.944087, 0.91224, 0.91224, 0.944087,
				0.957826, 0.957826, 0.92957, 0.945873, 0.980581, 0.980581, 0.980581, 0.970276, 0.980939, 0.995037, 0.995037, 0.995037, 0.995037, 1, 0.995037, 0.995037, 0.995037,
				0.960596, 0.811242, 0.659796, 0.625067, 0.714141, 0.906419, 1, 0.932722, 0.821032, 0.762403, 0.9135, -0.406839, -0.762403, -0.821032, -0.932722, -0.980581 };
float ny_init[N] = { -1, -0.707107, 0.229753, 0.538954, 0.755821, 0.283759, 0.256668, 0.471858, -0.382683, -0.615412, -0.481209, -0.511721, -0.435948, -0.329696, -0.409656, -0.409656,
			   -0.329696, -0.287348, -0.287348, -0.368646, -0.324536, -0.196116, -0.196116, -0.196116, -0.242, -0.194318, -0.0995041, -0.0995036, -0.0995036, -0.0995036,
				0, 0.0995036, 0.0995036, 0.0995036, 0.277949, 0.58471, 0.751445, 0.780571, 0.700001, 0.422379, 0, -0.360597, -0.570882, -0.647103, 0.406839, 0.9135, 0.647103,
				0.570882, 0.360597, 0.196116 };

float angle = 25., cam_hgt = 60.;	//Scene rotation angle, camera height
GLuint txId;

//--------------------------------------------------------------------------
void loadTexture() {
	glGenTextures(1, &txId);			// Create a Texture object
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, txId);	//Use this texture
	loadBMP("VaseTexture.bmp");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

//-- Ground Plane ----------------------------------------------------------
void drawFloor() {
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.7, 0.7, 0.7);		//Floor colour

	for(int i = -200; i <= 200; i += 5) {
		glBegin(GL_LINES);			//A set of grid lines on the xz-plane
			glVertex3f(-200, 0, i);
			glVertex3f(200, 0, i);
			glVertex3f(i, 0, -200);
			glVertex3f(i, 0, 200);
		glEnd();
	}
}

//--------------------------------------------------------------------------
void initialise(void) {
    float grey[4] = {0.2, 0.2, 0.2, 1.0};
    float white[4] = {1.0, 1.0, 1.0, 1.0};

	loadTexture();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, grey);
	glEnable(GL_COLOR_MATERIAL);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 100.);

    glClearColor(1., 1., 1., 0.);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(40., 1., 20., 500.);
}

//--------------------------------------------------------------------------
void display(void) {
	float toRadians = M_PI/180.0;		//Conversion from degrees to radians
	float angStep = 10.0*toRadians;		//Rotate base curve in 10 deg steps
	int nSlices = 36;					//36 slices at 10 deg intervals
	float lgt_pos[] = {5., 50., 100., 1.};	//light0 position (above the origin)
	float vx[N], vy[N], vz[N];	//vertex positions
	float wx[N], wy[N], wz[N];
	float nx[N], ny[N], nz[N];	//normal vectors
	float mx[N], my[N], mz[N];

	for (int i = 0; i < N; i++)	{	//Initialize data everytime the frame is refreshed
		vx[i] = vx_init[i];
		vy[i] = vy_init[i];
		vz[i] = 0;
		nx[i] = nx_init[i];
		ny[i] = ny_init[i];
		nz[i] = 0;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0., cam_hgt, 100., 0., 20., 0., 0., 1., 0.);

	glRotatef(angle, 0, 1, 0);	//Rotate the entire scene

	glLightfv(GL_LIGHT0, GL_POSITION, lgt_pos);	//light position

	drawFloor();

	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glColor3f(1., 0.75, 0.5);

	for (int j = 0; j < nSlices; j++) { // Transform vert and normals
		for (int i = 0; i < N; i++) {
			wx[i] = cos(angStep) * vx[i] + sin(angStep) * vz[i];
			wy[i] = vy[i];
			wz[i] = -sin(angStep) * vx[i] + cos(angStep) * vz[i];
			mx[i] = cos(angStep) * nx[i] + sin(angStep) * nz[i];  // transform normals
			my[i] = ny[i];
			mz[i] = -sin(angStep) * nx[i] + cos(angStep) * nz[i];
		}

		glBegin(GL_QUAD_STRIP); // Generate quad strip
		for (int i = 0; i < N; i++) {

			float s = (float)j/nSlices;	//Texture coordinate s
			float t = (float)i/(N-1);	//Texture coordinate t


			glNormal3f(nx[i], ny[i], nz[i]);   // normal for left vertex
			glTexCoord2f(s, t);				//Texture coordinate for left vertex
			glVertex3f(vx[i], vy[i], vz[i]);

			glNormal3f(mx[i], my[i], mz[i]);   // normal for right vertex
			glTexCoord2f(s + 1.0/nSlices, t);	//Texture coordinate for right vertex
			glVertex3f(wx[i], wy[i], wz[i]);
		}
		glEnd();

		for (int i = 0; i < N; i++) {   // update both verts and normals
			vx[i] = wx[i];
			vy[i] = wy[i];
			vz[i] = wz[i];
			nx[i] = mx[i];
			ny[i] = my[i];
			nz[i] = mz[i];
		}
	}



	glFlush();
}

//--------------------------------------------------------------------------
void special(int key, int x, int y) {
	if		(key == GLUT_KEY_LEFT)	angle --;	//Rotate wagon
	else if (key == GLUT_KEY_RIGHT) angle ++;
	else if (key == GLUT_KEY_UP)	cam_hgt ++;	//Change camera height
	else if (key == GLUT_KEY_DOWN)  cam_hgt --;

	if		(cam_hgt < 10.)  cam_hgt = 10.;
	else if (cam_hgt > 100.) cam_hgt = 100.;

	glutPostRedisplay();
}

//--------------------------------------------------------------------------
int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_DEPTH);
	glutInitWindowSize(500, 500); 
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Vase");
	initialise();

	glutDisplayFunc(display);
	glutSpecialFunc(special);
	glutMainLoop();
	return 0;
}
