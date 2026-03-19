//  ========================================================================
//  COSC363: Computer Graphics (2025); CSSE  University of Canterbury.
//
//  FILE NAME: Yard.cpp
//  See Lab03.pdf for details.
//  ========================================================================
 
#include <iostream>
#include <cmath> 
#include <GL/freeglut.h>
#include "loadTGA.h"
using namespace std;

GLuint txId[2];	//Texture ids

float angle=0, look_x, look_z=-1., eye_x, eye_z;  //Camera parameters

//--------------------------------------------------------------------------
void loadTexture() {
	glGenTextures(2, txId); // Create 2 texture ids

	glBindTexture(GL_TEXTURE_2D, txId[0]);	//Use this texture
	loadTGA("Wall.tga");
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	

	glBindTexture(GL_TEXTURE_2D, txId[1]);	//Use this texture
	loadTGA("Floor.tga");
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	
	
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
}

//-- Special key event callback --------------------------------------------
void special(int key, int x, int y) {
	if 		(key == GLUT_KEY_LEFT)  angle -= 0.1;  //Change direction
	else if (key == GLUT_KEY_RIGHT) angle += 0.1;
	else if (key == GLUT_KEY_DOWN)  { //Move backward
		eye_x -= 0.1*sin(angle);
		eye_z += 0.1*cos(angle);
	}
	else if (key == GLUT_KEY_UP)	{ //Move forward
		eye_x += 0.1*sin(angle);
		eye_z -= 0.1*cos(angle);
	}

	look_x = eye_x + 100*sin(angle);
	look_z = eye_z - 100*cos(angle);
	glutPostRedisplay();
}

//-- Initialize OpenGL parameters ------------------------------------------
void initialise() { 
//	loadTexture();	
//	glEnable(GL_TEXTURE_2D);
	glClearColor(0., 1., 1., 1.);    //Background colour 	
	glEnable(GL_DEPTH_TEST);
}

//--------------------------------------------------------------------------
void walls() {
	glColor3f(0.8, 0.7, 0.3);   //replace with a texture

	glBegin(GL_QUADS);

	////////////////////// BACK WALL ///////////////////////

	glVertex3f(-15, 1, -15);
	glVertex3f(-15, -1, -15);
	glVertex3f(15, -1, -15);
	glVertex3f(15, 1, -15);

	////////////////////// FRONT WALL ///////////////////////

	glVertex3f(-15, 1, 15);
	glVertex3f(-15, -1, 15);
	glVertex3f(15, -1, 15);
	glVertex3f(15, 1, 15);

	////////////////////// LEFT WALL ///////////////////////

	glVertex3f(-15, 1, -15);
	glVertex3f(-15, -1, -15);
	glVertex3f(-15, -1, 15);
	glVertex3f(-15, 1, 15);


	////////////////////// RIGHT WALL ///////////////////////

	glVertex3f(15, 1, -15);
	glVertex3f(15, -1, -15);
	glVertex3f(15, -1, 15);
	glVertex3f(15, 1, 15);

	glEnd();
}

//--------------------------------------------------------------------------
void floor() {
	glColor3d(0.6, 0.25, 0.25);  //replace with a texture

	glBegin(GL_QUADS);
		glVertex3f(-15, -1, -15);
		glVertex3f(-15, -1, 15);
		glVertex3f(15, -1, 15);
		glVertex3f(15, -1, -15);
	glEnd();
}

//-- Display: --------------------------------------------------------------
//-- This is the main display module containing function calls for generating
//-- the scene.
void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glMatrixMode(GL_PROJECTION);						
	glLoadIdentity();
	gluPerspective(45., 1., 1., 100.);

	glMatrixMode(GL_MODELVIEW);								
	glLoadIdentity();
	gluLookAt(eye_x, 0., eye_z, look_x, 0., look_z, 0., 1., 0.);	
	
	walls();
	floor();

	glutSwapBuffers();									
}

//-- Main: Initialize glut window and register call backs ------------------
int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 800); 
	glutInitWindowPosition(10, 10);
	glutCreateWindow("The Yard");
	initialise();

	glutDisplayFunc(display); 
	glutSpecialFunc(special);
	glutMainLoop();
	return 0;
}
