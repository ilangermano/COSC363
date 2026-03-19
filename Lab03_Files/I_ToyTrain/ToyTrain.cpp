//  ========================================================================
//  COSC363: Computer Graphics (2025);  University of Canterbury.
//
//  FILE NAME: ToyTrain.cpp
//  See Lab03.pdf for details
//  ========================================================================

#include <math.h>
#include <GL/freeglut.h>
#include "RailModels.h"

float angle = 0.0;	//Rotation angle for the train

//-- Initialize OpenGL parameters ------------------------------------------
void initialize(void) {
	float white[4]  = {1., 1., 1., 1.};
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
//	Define light's diffuse, specular properties
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white);	//Default, only for LIGHT0
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);	//Default, only for LIGHT0
	glLightfv(GL_LIGHT1, GL_DIFFUSE, white);
	glLightfv(GL_LIGHT1, GL_SPECULAR, white);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0);
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 0);
	glMaterialfv(GL_FRONT, GL_SPECULAR, white);
 	glMaterialf(GL_FRONT, GL_SHININESS, 50);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glClearColor(0., 0., 0., 1.);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60., 1., 10., 1000.);
}

//-- Display: --------------------------------------------------------------
//-- This is the main display module containing function calls for generating
//-- the scene.
void display(void) {
	float light[] = {0., 50., 0., 1.};	//Light's position (directly above the origin)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt (-80., 100., 250., 0., 0., 0., 0., 1., 0.);
	glLightfv(GL_LIGHT0, GL_POSITION, light);

	floor();			//A tessellated floor plane
	tracks(120, 10);	//Circular tracks with mean radius 120 units, width 10 units
	glPushMatrix();
	    glRotatef(angle, 0., 1., 0.);	//Rotate the train about the y-axis
		
		glPushMatrix();
			glTranslatef(0, 1, -120);
			float light1_pos[] = {-10., 14., 0., 1.};	//Light1's position (headlight of the train)
			float light1_dir[] = {-1., -1., 0.};	//Light1's direction (pointing downwards)
			glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
			glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, light1_dir);
			engine();		//A simple locomotive model
		glPopMatrix();

		glPushMatrix();
			glRotatef(-10.5, 0, 1, 0);
			glTranslatef(0, 1, -120);
			wagon();		//A simple locomotive model
		glPopMatrix();

		glPushMatrix();
			glRotatef(-21, 0, 1, 0);
			glTranslatef(0, 1, -120);
			wagon();		//A simple locomotive model
		glPopMatrix();

		glPushMatrix();
			glRotatef(-31.5, 0, 1, 0);
			glTranslatef(0, 1, -120);
			wagon();		//A simple locomotive model
		glPopMatrix();

		glPushMatrix();
			glRotatef(-42, 0, 1, 0);
			glTranslatef(0, 1, -120);
			wagon();		//A simple locomotive model
		glPopMatrix();
	
	glPopMatrix();

	glutSwapBuffers();	//Double buffered animation
}

void myTimer(int value) {
	angle++;
	if (angle > 360) angle = 0;
	glutPostRedisplay(); // Request display update
	glutTimerFunc(50, myTimer, 0); // Register the timer callback again
}	

//-- Main: Initialize glut window and register call backs ------------------
int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(800, 800); 
	glutInitWindowPosition(5, 5);
	glutCreateWindow("Toy Train");
	initialize();
	glutTimerFunc(50, myTimer, 0); // Register timer callback function
	glutDisplayFunc(display); 
	glutMainLoop();
	return 0;
}
