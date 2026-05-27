/*==================================================================================
* COSC 363  Computer Graphics
* Department of Computer Science and Software Engineering, University of Canterbury.
*
* A basic ray tracer
* See Lab07.pdf   for details.
*===================================================================================
*/
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "SceneObject.h"
#include "Ray.h"
#include "Plane.h"
#include "Cylinder.h"
#include "Cone.h"
#include "Torus.h"
#include "stb_texture.h"
#include <GL/freeglut.h>
using namespace std;

const float EDIST = 40.0;
const int NUMDIV = 500;
const int MAX_STEPS = 5;
const float XMIN = -10.0;
const float XMAX = 10.0;
const float YMIN = -10.0;
const float YMAX = 10.0;

vector<SceneObject*> sceneObjects;
STBTexture texture;


//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step) {
	glm::vec3 backgroundCol(0);						//Background colour = (0,0,0)
	glm::vec3 lightPos(0, 14, -100);					//Light's position
	glm::vec3 color(0);
	SceneObject* obj;

	ray.closestPt(sceneObjects);					//Compare the ray with all objects in the scene
	if(ray.index == -1) return backgroundCol;		//no intersection
	obj = sceneObjects[ray.index];					//object on which the closest point of intersection is found

	if (ray.index == 0)
	{
		int tileSize = 5;
		int ix = (int)floor((ray.hit.x + 20) / tileSize);
		int iz = (int)floor((ray.hit.z + 200) / tileSize);
		int k = (ix + iz) % 2;
		if (k == 0) obj->setColor(glm::vec3(0, 0, 0));
		else        obj->setColor(glm::vec3(1, 1, 1));
	}

	color = obj->lighting(lightPos, -ray.dir, ray.hit);				//Object's colour
	glm::vec3 lightVec = lightPos - ray.hit;
	float lightDist = glm::length(lightVec);
	Ray shadowRay(ray.hit, lightVec);
	shadowRay.closestPt(sceneObjects);
	if (shadowRay.index > -1 && shadowRay.dist < lightDist)
	{
		SceneObject* hitObj = sceneObjects[shadowRay.index];
		if (hitObj->isTransparent() || hitObj->isRefractive())
			color = 0.7f * color;   // lighter shadow
		else
			color = 0.2f * obj->getColor();  // full shadow
	}


	if (obj->isReflective() && step < MAX_STEPS)
	{
		float rho = obj->getReflectionCoeff();
		glm::vec3 normalVec = obj->normal(ray.hit);
		glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
		Ray reflectedRay(ray.hit, reflectedDir);
		glm::vec3 reflectedColor = trace(reflectedRay, step + 1);
		color = color + (rho * reflectedColor);
		float tc = obj->getTransparencyCoeff();
		Ray transRay(ray.hit, ray.dir);
		glm::vec3 transColor = trace(transRay, step + 1);
		color = color + (tc * transColor);
	}

	return color;
}

//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
void display() {
	float xp, yp;  //grid point
	float cellX = (XMAX - XMIN) / NUMDIV;  //cell width
	float cellY = (YMAX - YMIN) / NUMDIV;  //cell height
	glm::vec3 eye(0., 0., 0.);

	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBegin(GL_QUADS);  //Each cell is a tiny quad.

	for (int i = 0; i < NUMDIV; i++) {	//Scan every cell of the image plane
		xp = XMIN + i * cellX;
		for (int j = 0; j < NUMDIV; j++) {
			yp = YMIN + j * cellY;

			glm::vec3 dir(xp + 0.5 * cellX, yp + 0.5 * cellY, -EDIST);	//direction of the primary ray

			Ray ray = Ray(eye, dir);

			glm::vec3 col = trace(ray, 1); //Trace the primary ray and get the colour value
			glColor3f(col.r, col.g, col.b);
			glVertex2f(xp, yp);				//Draw each cell with its color value
			glVertex2f(xp + cellX, yp);
			glVertex2f(xp + cellX, yp + cellY);
			glVertex2f(xp, yp + cellY);
		}
	}

	glEnd();
	glFlush();
}

//---This function initializes the scene ------------------------------------------- 
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL 2D orthographc projection matrix for drawing the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize() {
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(XMIN, XMAX, YMIN, YMAX);
	glClearColor(0, 0, 0, 1);

	Plane *floorPlane = new Plane(glm::vec3(-20, -15, -40), glm::vec3(20, -15, -40),
							 glm::vec3(20, -15, -200), glm::vec3(-20, -15, -200));
	
	floorPlane->setColor(glm::vec3(1,1,1));
	floorPlane->setSpecularity(false);
	sceneObjects.push_back(floorPlane);

	Plane *ceiling = new Plane(glm::vec3(-20,15,-40), glm::vec3(-20,15,-200),
                               glm::vec3(20,15,-200), glm::vec3(20,15,-40));
	ceiling->setColor(glm::vec3(0.5, 0.5, 0.7));
	ceiling->setSpecularity(false);
	sceneObjects.push_back(ceiling);

	Plane *backWall = new Plane(glm::vec3(-20,-15,-200), glm::vec3(20,-15,-200),
                                glm::vec3(20,15,-200), glm::vec3(-20,15,-200));
    backWall->setColor(glm::vec3(0.2, 0.2, 0.9));
    backWall->setSpecularity(false);
    sceneObjects.push_back(backWall);
	
	Plane *leftWall = new Plane(glm::vec3(-20,-15,-40), glm::vec3(-20,-15,-200),
                                glm::vec3(-20,15,-200), glm::vec3(-20,15,-40));
    leftWall->setColor(glm::vec3(0.9, 0.2, 0.2));
    leftWall->setSpecularity(false);
    sceneObjects.push_back(leftWall);

	Plane *rightWall = new Plane(glm::vec3(20,-15,-40), glm::vec3(20,15,-40),
                                 glm::vec3(20,15,-200), glm::vec3(20,-15,-200));
    rightWall->setColor(glm::vec3(0.2, 0.9, 0.2));
    rightWall->setSpecularity(false);
    sceneObjects.push_back(rightWall);

	Plane *frontWall = new Plane(glm::vec3(-20,-15,5), glm::vec3(-20,15, 5),
                                 glm::vec3(20,15, 5), glm::vec3(20,-15, 5));
    frontWall->setColor(glm::vec3(0.9, 0.9, 0.2));
    frontWall->setSpecularity(false);
    sceneObjects.push_back(frontWall);

	Sphere *glassSphere = new Sphere(glm::vec3(0, -5, -100), 5.0);
	glassSphere->setColor(glm::vec3(0.8, 0.8, 0.8));
	glassSphere->setTransparency(true, 0.8);
	glassSphere->setSpecularity(true);
	sceneObjects.push_back(glassSphere);

	Plane *mirror = new Plane(
		glm::vec3(-18, -14, -198),
		glm::vec3( 18, -14, -198),
		glm::vec3( 18,  10, -197), 
		glm::vec3(-18,  10, -197));
	mirror->setColor(glm::vec3(0, 0, 0));
	mirror->setReflectivity(true, 1.0);
	mirror->setSpecularity(false);
	sceneObjects.push_back(mirror);

	Cylinder *cyl = new Cylinder(glm::vec3(-5, -5, -120), 3, 20);
	cyl->setColor(glm::vec3(1, 0.5, 0));
	glm::mat4 T = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0,0,1));
	cyl->setTransform(T);
	sceneObjects.push_back(cyl);


	Cone *cone = new Cone(glm::vec3(5, -15, -100), 4, 12);
	cone->setColor(glm::vec3(0.9, 0.2, 0.9));
	sceneObjects.push_back(cone);
}

int main(int argc, char *argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(20, 20);
	glutCreateWindow("Raytracing");

	glutDisplayFunc(display);
	initialize();

	glutMainLoop();
	return 0;
}
