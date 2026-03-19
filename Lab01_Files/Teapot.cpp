//  ========================================================================
//  COSC363: Computer Graphics (2025);  University of Canterbury.
//
//  FILE NAME: Teapot.cpp
//  See Lab01.pdf for details
//  ========================================================================

#include <cmath>
#include <GL/freeglut.h>

int cam_hgt = 10; // Initial camera height
float theta = 0.0; // Initial angle for teapot rotation


//-- Draws a grid of lines on the floor plane ------------------------------
void drawFloor() {
	glColor3f(0., 0.5, 0.);     //Floor colour

	for(int i = -50; i <= 50; i++)  {
		glBegin(GL_LINES);      //A set of grid lines on the xz-plane
			glVertex3f(-50., -0.75, i);
			glVertex3f( 50., -0.75, i);
			glVertex3f(i, -0.75, -50.);
			glVertex3f(i, -0.75,  50.);
		glEnd();
	}
}

//-- Display: --------------------------------------------------------------
//-- This is the main display module containing function calls for generating
//-- the scene.
void display(void) { 
	float lpos[4] = {0., 10., 10., 1.0};  //light's position

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(12*sin(theta * M_PI / 180.0), cam_hgt, 12*cos(theta * M_PI / 180.0), 0., 0., 0., 0., 1., 0.);  //Camera position and orientation
	glLightfv(GL_LIGHT0,GL_POSITION, lpos);   //Set light position

	glDisable(GL_LIGHTING);         //Disable lighting when drawing floor.
	drawFloor();

	glEnable(GL_LIGHTING);          //Enable lighting when drawing the teapot
	glColor3f(0., 1., 1.);
	glutSolidTeapot(1);      

	glFlush(); 
} 

void myTimer(int value)
{
	theta++;
	glutPostRedisplay(); // Request display update
	glutTimerFunc(50, myTimer, 0); // Register the timer callback again
}

// Implement special keybboard callback function to move the camera up and down
void special(int key, int x, int y)
{
	if (key == GLUT_KEY_UP && cam_hgt < 20) cam_hgt++;
	else if (key == GLUT_KEY_DOWN && cam_hgt > 2) cam_hgt--;
	glutPostRedisplay(); // Request display update
}

//-- Initialize OpenGL parameters ------------------------------------------
void initialize(void) {
	glClearColor(1., 1., 1., 1.);

	glEnable(GL_LIGHTING);      //Enable OpenGL states
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(30., 1., 10., 1000.);   //Camera Frustum
}


//-- Main: Initialize glut window and register call backs ------------------
int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Teapot");
	initialize(); 
	glutDisplayFunc(display);
	glutSpecialFunc(special);  // Register special key callback function
	glutTimerFunc(50, myTimer, 0); // Register timer callback function
	glutMainLoop();
	return 0; 
}
