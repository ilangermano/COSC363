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
#include <cstdlib>
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
const bool ANTI_ALIASING  = false;
const bool SOFT_SHADOWS   = true; 
const int  NUM_SHADOW_RAYS = 16;    
const float LIGHT_RADIUS   = 1.2f;
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
	glm::vec3 backgroundCol(0);
	glm::vec3 color(0);
	SceneObject* obj;

	glm::vec3 lights[] = {
		glm::vec3(  0, 14, -100),   // main light, centre
		glm::vec3(-14, 12,  -60)    // fill light, front-left
	};
	const int numLights = 2;

	ray.closestPt(sceneObjects);
	if(ray.index == -1) return backgroundCol;
	obj = sceneObjects[ray.index];

	if (ray.index == 0)
	{
		int tileSize = 5;
		int ix = (int)floor((ray.hit.x + 20) / tileSize);
		int iz = (int)floor((ray.hit.z + 200) / tileSize);
		int k = (ix + iz) % 2;
		if (k == 0) obj->setColor(glm::vec3(0, 0, 0));
		else        obj->setColor(glm::vec3(1, 1, 1));
	}

	for (int l = 0; l < numLights; l++)
	{
		glm::vec3 lightPos = lights[l];
		glm::vec3 lightColor = obj->lighting(lightPos, -ray.dir, ray.hit);

		if (SOFT_SHADOWS)
		{
			float shadowSum = 0.0f;
			for (int s = 0; s < NUM_SHADOW_RAYS; s++)
			{
				float r1 = ((float)rand()/RAND_MAX - 0.5f) * 2.0f * LIGHT_RADIUS;
				float r2 = ((float)rand()/RAND_MAX - 0.5f) * 2.0f * LIGHT_RADIUS;
				float r3 = ((float)rand()/RAND_MAX - 0.5f) * 2.0f * LIGHT_RADIUS;
				glm::vec3 jLight = lightPos + glm::vec3(r1, r2, r3);
				glm::vec3 jVec   = jLight - ray.hit;
				float     jDist  = glm::length(jVec);
				Ray jShadow(ray.hit, jVec);
				jShadow.closestPt(sceneObjects);
				if (jShadow.index > -1 && jShadow.dist < jDist)
				{
					SceneObject* h = sceneObjects[jShadow.index];
					shadowSum += (h->isTransparent() || h->isRefractive()) ? 0.3f : 1.0f;
				}
			}
			float sf = shadowSum / NUM_SHADOW_RAYS;
			lightColor = (1.0f - sf) * lightColor + sf * 0.2f * obj->getColor();
		}
		else
		{
			glm::vec3 lightVec = lightPos - ray.hit;
			float lightDist = glm::length(lightVec);
			Ray shadowRay(ray.hit, lightVec);
			shadowRay.closestPt(sceneObjects);
			if (shadowRay.index > -1 && shadowRay.dist < lightDist)
			{
				SceneObject* hitObj = sceneObjects[shadowRay.index];
				if (hitObj->isTransparent() || hitObj->isRefractive())
					lightColor = 0.7f * lightColor;
				else
					lightColor = 0.2f * obj->getColor();
			}
		}
		color += lightColor;
	}
	color /= (float)numLights;


	if (obj->isReflective() && step < MAX_STEPS)
	{
		float rho = obj->getReflectionCoeff();
		glm::vec3 normalVec = obj->normal(ray.hit);
		glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
		Ray reflectedRay(ray.hit, reflectedDir);
		glm::vec3 reflectedColor = trace(reflectedRay, step + 1);
		color = color + (rho * reflectedColor);
	}

	if (obj->isTransparent() && step < MAX_STEPS)
	{
		float tc = obj->getTransparencyCoeff();
		Ray transRay(ray.hit, ray.dir);
		glm::vec3 transColor = trace(transRay, step + 1);
		color = color + (tc * transColor);
	}

	if (obj->isRefractive() && step < MAX_STEPS)
	{
		float eta = obj->getRefractiveIndex();
		float rc  = obj->getRefractionCoeff();
		glm::vec3 n = obj->normal(ray.hit);

		
		glm::vec3 g = glm::refract(ray.dir, n, 1.0f / eta);
		Ray refractedRay(ray.hit, g);
		refractedRay.closestPt(sceneObjects);

		if (refractedRay.index != -1)
		{
			glm::vec3 m  = sceneObjects[refractedRay.index]->normal(refractedRay.hit);
			glm::vec3 g2 = glm::refract(g, -m, eta);
			if (glm::length(g2) > 0.001f)
			{
				Ray exitRay(refractedRay.hit, g2);
				glm::vec3 refractColor = trace(exitRay, step + 1);
				color = color + rc * refractColor;
			}
		}
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

			glm::vec3 col(0);
			if (ANTI_ALIASING)
			{
				float offsets[2] = { 0.25f, 0.75f };
				for (float ox : offsets)
					for (float oy : offsets)
					{
						glm::vec3 dir(xp + ox * cellX, yp + oy * cellY, -EDIST);
						col += trace(Ray(eye, dir), 1);
					}
				col /= 4.0f;
			}
			else
			{
				glm::vec3 dir(xp + 0.5f * cellX, yp + 0.5f * cellY, -EDIST);
				col = trace(Ray(eye, dir), 1);
			}
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

	
	// Refractive glass sphere — bottom left
	Sphere *glassSphere = new Sphere(glm::vec3(-7, -7, -70), 5.0);
	glassSphere->setColor(glm::vec3(0.9, 0.95, 1.0));
	glassSphere->setRefractivity(true, 0.85, 1.5);
	glassSphere->setReflectivity(true, 0.12);
	glassSphere->setSpecularity(true);
	glassSphere->setShininess(150.0);
	sceneObjects.push_back(glassSphere);

	// Transparent sphere — right side, mid distance
	Sphere *transpSphere = new Sphere(glm::vec3(7, -3, -90), 3.0);
	transpSphere->setColor(glm::vec3(0.1, 0.6, 0.2));   // deeper green
	transpSphere->setTransparency(true, 0.7);
	transpSphere->setSpecularity(true);
	transpSphere->setShininess(30.0);
	sceneObjects.push_back(transpSphere);

	Plane *mirror = new Plane(
		glm::vec3(-18, -14, -198),
		glm::vec3( 18, -14, -198),
		glm::vec3( 18,  10, -197),
		glm::vec3(-18,  10, -197));
	mirror->setColor(glm::vec3(0, 0, 0));
	mirror->setReflectivity(true, 1.0);
	mirror->setSpecularity(false);
	sceneObjects.push_back(mirror);

	Cylinder *cyl = new Cylinder(glm::vec3(0, 0, 0), 3, 22);
	cyl->setColor(glm::vec3(1, 0.5, 0));
	glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(-9, 3, -160));
	T = T * glm::rotate(glm::mat4(1.0f), glm::radians(105.0f), glm::vec3(1,0,0));
	cyl->setTransform(T);
	sceneObjects.push_back(cyl);

	Cone *cone = new Cone(glm::vec3(9, -15, -120), 4, 13);
	cone->setColor(glm::vec3(0.9, 0.2, 0.9));
	sceneObjects.push_back(cone);


	Torus *torus = new Torus(glm::vec3(0, 0, 0), 5, 2);
	torus->setColor(glm::vec3(0.2, 0.8, 0.8));
	glm::mat4 TT = glm::translate(glm::mat4(1.0f), glm::vec3(7, 5, -115));
	TT = TT * glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0, 0, 1));
	TT = TT * glm::rotate(glm::mat4(1.0f), glm::radians(-25.0f), glm::vec3(1, 0, 0));
	torus->setTransform(TT);
	sceneObjects.push_back(torus);
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
