//  ========================================================================
//  COSC363: Computer Graphics (2025);  University of Canterbury.
//
//  FILE NAME: Earth.cpp
//  See Lab03.pdf for details.
//  ========================================================================

#include <iostream>
#include <GL/freeglut.h>
#include "loadBMP.h"
using namespace std;

GLuint txId[2];
GLUquadricObj*	q;
float rotnEarthAxis = 0.;  //Rotation of the Earth about its own axis

//--------------------------------------------------------------------------
void loadTexture() {
	glGenTextures(2, txId);   //Get 2 texture IDs 
	glBindTexture(GL_TEXTURE_2D, txId[0]);  //Use this texture name for the following OpenGL texture
	loadBMP("Earth.bmp");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// Linear Filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glBindTexture(GL_TEXTURE_2D, txId[1]);  //Use this texture name for the following OpenGL texture
	loadBMP("Sun.bmp");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

//-- Initialize OpenGL parameters ------------------------------------------
void initialise() {
	loadTexture();

	glClearColor(0., 0., 0., 1.); 	 //Background
	glClearDepth(1.);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);  //Default
	glEnable(GL_COLOR_MATERIAL);
	
	q = gluNewQuadric();
	gluQuadricDrawStyle(q, GLU_FILL);
	gluQuadricNormals(q, GLU_SMOOTH);
	glEnable(GL_TEXTURE_2D);
	gluQuadricTexture (q, GL_TRUE);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40., 2., 10., 100.);
}

//-- Display: --------------------------------------------------------------
//-- This is the main display module containing function calls for generating
//-- the scene.
void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();  	
	gluLookAt(0., 0., 40., 0., 0., 0., 0., 1., 0.);
	
	glColor4f(1., 1., 1., 1.);        //Base colour
	
	//Earth
	glBindTexture(GL_TEXTURE_2D, txId[0]);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);	
	glPushMatrix();
		glTranslatef(20., 0., 0.);			//Translate Earth along x-axis by 20 units	
		glRotatef(rotnEarthAxis, 0, 1, 0);	//Rotate about polar axis of the Earth
		glRotatef(-90., 1., 0., 0.);		//make the sphere axis vertical
		gluSphere(q, 3.0, 36, 17 );
	glPopMatrix();
	
	//Sun
	glBindTexture(GL_TEXTURE_2D, txId[1]);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);	
	glPushMatrix();		
		glRotatef(-90., 1., 0., 0.);   //make the sphere axis vertical
		gluSphere(q, 4.0, 36, 17);
	glPopMatrix();

	glutSwapBuffers();
}

//--------------------------------------------------------------------------
void timer(int value) {
	rotnEarthAxis += 2.;
	if (rotnEarthAxis > 360.) rotnEarthAxis = 0.;
	glutTimerFunc(50, timer, value);
	glutPostRedisplay();
}

//-- Main: Initialize glut window and register call backs ------------------
int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(800, 400); 
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);
	initialise();
	glutDisplayFunc(display);
	glutTimerFunc(50, timer, 0);
	glutMainLoop();
	return 0;
}
