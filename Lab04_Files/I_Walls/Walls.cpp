//  ========================================================================
//  COSC363: Computer Graphics (2025);  University of Canterbury.
//
//  FILE NAME: Walls.cpp
//  See Lab04.pdf for details.
//  ========================================================================
 
#include <iostream>
#include <cmath> 
#include <GL/freeglut.h>
#include "loadTGA.h"
using namespace std;

//-- Globals ---------------------------------------------------------------
GLuint txId[4];   //Texture ids
float xpts[13] = { 0, 0.75, 1.58, 2.7, 5.25, 7.5, 9.75, 11.25, 12.45, 13.2, 14.17, 14.625, 15 };
float zpts[13] = { -15, -11.25, -9, -7.5, -5.25, -3.75, -1.87, 0, 1.87, 3.75, 7.5, 11.25, 15 };
float angle=0., look_x=0., look_z=0., eye_x=0., eye_z=10.;	//View parameters

//-- Function to load textures ---------------------------------------------
void loadTexture() {
	glGenTextures(4, txId); 	// Create 4 texture ids

	glBindTexture(GL_TEXTURE_2D, txId[0]);
    loadTGA("Brick_Texture.tga");
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	

	glBindTexture(GL_TEXTURE_2D, txId[1]);
    loadTGA("Floor_Texture.tga");
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	

	glBindTexture(GL_TEXTURE_2D, txId[2]);
	loadTGA("Stone_Texture.tga");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, txId[3]);
	loadTGA("Tree_Texture.tga");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
}


//-- Special function callback ---------------------------------------------
void special(int key, int x, int y) {
	if		(key == GLUT_KEY_LEFT)  angle -= 0.1;	//Change direction
	else if (key == GLUT_KEY_RIGHT) angle += 0.1;
	else if (key == GLUT_KEY_DOWN)  {	//Move backward
		eye_x -= 0.1*sin(angle);
		eye_z += 0.1*cos(angle);
	}
	else if (key == GLUT_KEY_UP)	{	//Move forward
		eye_x += 0.1*sin(angle);
		eye_z -= 0.1*cos(angle);
	}

	look_x = eye_x + 10*sin(angle);
	look_z = eye_z - 10*cos(angle);
	glutPostRedisplay();
}

//-- Initialise OpenGL state parameters, load textures ---------------------
void initialise() { 
	glEnable(GL_TEXTURE_2D);
	loadTexture();

	glClearColor(0., 0., 0., 1.);  //Background colour 	 
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(50., 1., 1., 100.);
}


//-- Function to compute normal vectors at vertices of a polygonal line ----
void normal(int i) {
	float xdiff1, zdiff1, xdiff2, zdiff2;
	if (i == 0 || i == 12) glNormal3f(-1, 0, 0);   //default normal along -x direction
	else {
		xdiff1 = xpts[i] - xpts[i - 1];
		zdiff1 = zpts[i] - zpts[i - 1];
		xdiff2 = xpts[i+1] - xpts[i];
		zdiff2 = zpts[i+1] - zpts[i];
		glNormal3f(-(zdiff1 + zdiff2), 0, (xdiff1 + xdiff2));
	}
}

//-- Draws two straight walls ----------------------------------------------
void brickWalls() {
	glBindTexture(GL_TEXTURE_2D, txId[0]);
	glColor3f(1, 1, 1);
	////////////////////// FRONT WALL ///////////////////////
	glNormal3f(0, 0, 1);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex3f(-15, 0, -15);
	glTexCoord2f(4, 0);
	glVertex3f(0, 0, -15);
	glTexCoord2f(4, 2);
	glVertex3f(0, 4, -15);
	glTexCoord2f(0, 2);
	glVertex3f(-15, 4, -15);
	glEnd();

	////////////////////// LEFT WALL ///////////////////////
	glNormal3f(1, 0, 0);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex3f(-15, 0, 15);
	glTexCoord2f(8, 0);
	glVertex3f(-15, 0, -15);
	glTexCoord2f(8, 2);
	glVertex3f(-15, 4, -15);
	glTexCoord2f(0, 2);
	glVertex3f(-15, 4, 15);
	glEnd();
}

//---- Draws a curved wall ----
void curvedWall() {
	glBindTexture(GL_TEXTURE_2D, txId[2]);
	glColor3f(1, 1, 1);
	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i < 13; i++) {
		normal(i);
		glTexCoord2f((float)i * 4 / 12, 0);   // bottom, s repeated 4 times
		glVertex3f(xpts[i], 0, zpts[i]);
		glTexCoord2f((float)i * 4 / 12, 1);   // top, s repeated 4 times
		glVertex3f(xpts[i], 4, zpts[i]);
	}
	glEnd();
}


//-- Draws a simple floor plane --------------------------------------------
void floor() {
	glBindTexture(GL_TEXTURE_2D,  txId[1]);
	glColor3f(1, 1, 1);
	glNormal3f(0, 1, 0);
	glBegin(GL_QUADS);		//A single quad
		glTexCoord2f(0, 0);
		glVertex3f(-16, -0.1, 16);
		glTexCoord2f(4, 0);
		glVertex3f(16, -0.1, 16);
		glTexCoord2f(4, 4);
		glVertex3f(16, -0.1, -16);
		glTexCoord2f(0, 4);
		glVertex3f(-16, -0.1, -16);
	glEnd();
}

//-- A quad for displaying a tree ------------------------------------------
void tree() {
	glBindTexture(GL_TEXTURE_2D, txId[3]);
	glColor3f(1, 1, 1);
	glNormal3f(0, 0, 1);
	glBegin(GL_QUADS);	//A single quad
		glTexCoord2f(0, 0);
		glVertex3f(6, 0, -12);
		glTexCoord2f(1, 0);
		glVertex3f(14, 0, -8);
		glTexCoord2f(1, 1);
		glVertex3f(14, 10, -8);
		glTexCoord2f(0, 1);
		glVertex3f(6, 10, -12);
	glEnd();
}

//-- The display function --------------------------------------------------
void display() {
	float light_pos[4] = {0., 20., 0., 1.};	//above origin
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eye_x, 5, eye_z, look_x, 3, look_z, 0, 1, 0);

	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	
	brickWalls();
	curvedWall();
	floor();
	tree();

	glFlush();									
}

//-- Glut initialisation, registration of callbacks ------------------------
int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 800); 
	glutInitWindowPosition(10, 10);
	glutCreateWindow("Walls");
	initialise();
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0);
	glutDisplayFunc(display); 
	glutSpecialFunc(special);
	glutMainLoop();
	return 0;
}
