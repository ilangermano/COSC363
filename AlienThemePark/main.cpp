#include <iostream>
#include <cmath>
#include "loadTGA.h"
#include "loadBMP.h"
#include <GL/freeglut.h>


int frame = 0;
GLUquadric *q;
float starX[200], starY[200], starZ[200];
float turnDir = 0.0f;
float moveDir = 0.0f;
float cartT = 0.0f;                                                            // Current position along track (0 to NPTS)
float cartSpeed = 0.15f;                                                       // Current speed
float angle = 0, look_x = 0, look_z = -1., eye_x = 0, eye_y = 20., eye_z = 20; // Camera parameters
bool wireframe = false;
GLuint txId[3];
const int NPTS = 20;
float trackX[] = {0, 26, 47, 50, 50, 50, 50, 50, 50, 47, 26, 0, -26, -47, -50, -50, -50, -50, -47, -26};
float trackY[] = {0, 0, 0, 0, 5, 20, 40, 42, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
float trackZ[] = {100, 96, 83, 65, 40, 10, -20, -45, -65, -83, -96, -100, -96, -83, -65, -20, 20, 65, 83, 96};
float alienColours[3][3] = {
    {1, 0, 0},
    {0, 1, 0},
    {0, 0, 1}
};
int alienColourIdx = rand() % 3;



void initialize()
{
    glClearColor(0., 0., 0., 1.);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_COLOR_MATERIAL);

    q = gluNewQuadric();
    gluQuadricTexture(q, GL_TRUE);
    gluQuadricNormals(q, GLU_SMOOTH);

    glGenTextures(3, txId);

    glBindTexture(GL_TEXTURE_2D, txId[0]);
    loadTGA("Moon.tga");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, txId[1]);
    loadBMP("Earth.bmp");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, txId[2]);
    loadBMP("Sun.bmp");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    for (int i = 0; i < 200; i++)
    {
        starX[i] = (rand() % 1000) - 500;
        starY[i] = (rand() % 1000);
        starZ[i] = (rand() % 1000) - 500;
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60., 1., 1., 1000);
}

void drawAlien(float x, float y, float z, int colourIndex)
{
    float r = alienColours[colourIndex][0];
    float g = alienColours[colourIndex][1];
    float b = alienColours[colourIndex][2];

    glDisable(GL_TEXTURE_2D);
    glPushMatrix();
    glTranslatef(x, y, z);

        glColor3f(r, g, b);
            glPushMatrix();
            glScalef(1.0, 0.75, 1.0);
            glutSolidSphere(3, 16, 16); // Head
        glPopMatrix();

        // Eye
        glColor3f(1, 1, 1);
        glPushMatrix();
            glTranslatef(0, 1, -2.5);
            glutSolidSphere(1.2, 12, 12);
        glPopMatrix();

        // Cyan Pupil
        glColor3f(0, 1, 1);
        glPushMatrix();
            glTranslatef(0, 1, -3.2);
            glutSolidSphere(0.6, 10, 10);
        glPopMatrix();

        // Left arm
        glColor3f(r, g, b);
        glPushMatrix();
            glTranslatef(-3, 0, 0);
            glRotatef(90, 0, 0, 1);
            gluCylinder(q, 0.5, 0.3, 2, 8, 1);
        glPopMatrix();

        // Right arm
        glPushMatrix();
            glTranslatef(3, 0, 0);
            glRotatef(-90, 0, 0, 1);
            gluCylinder(q, 0.5, 0.3, 2, 8, 1);
        glPopMatrix();

        // Mouth
        glColor3f(0, 0, 0);
        glPushMatrix();
            glTranslatef(0, -1, -2.8);
            glScalef(1.5, 0.3, 0.3);
            glutSolidCube(1);
        glPopMatrix();
    glPopMatrix();
    glEnable(GL_TEXTURE_2D);
}

void drawTracks()
{
    glDisable(GL_TEXTURE_2D);
    float gauge = 3.0;
    float rw = 0.4;
    float rh = 0.5;

    // Pre-compute averaged sideways vector at each waypoint
    float sx[NPTS], sz[NPTS];
    for (int i = 0; i < NPTS; i++)
    {
        int prev = (i - 1 + NPTS) % NPTS;
        int next = (i + 1) % NPTS;

        float dx_in = trackX[i] - trackX[prev];
        float dz_in = trackZ[i] - trackZ[prev];
        float dx_out = trackX[next] - trackX[i];
        float dz_out = trackZ[next] - trackZ[i];

        float dx_avg = dx_in + dx_out;
        float dz_avg = dz_in + dz_out;
        float len = sqrt(dx_avg * dx_avg + dz_avg * dz_avg);
        if (len < 0.001)
            len = 1.0;

        sx[i] = dz_avg / len;
        sz[i] = -dx_avg / len;
    }

    for (int i = 0; i < NPTS; i++)
    {
        int j = (i + 1) % NPTS;
        float ax = trackX[i], ay = trackY[i], az = trackZ[i];
        float bx = trackX[j], by = trackY[j], bz = trackZ[j];

        // Left rail
        glColor3f(0.6, 0.6, 0.6);
        glNormal3f(0, 1, 0);
        glBegin(GL_QUADS);
        glVertex3f(ax + (-gauge - rw) * sx[i], ay + rh, az + (-gauge - rw) * sz[i]);
        glVertex3f(ax + (-gauge + rw) * sx[i], ay + rh, az + (-gauge + rw) * sz[i]);
        glVertex3f(bx + (-gauge + rw) * sx[j], by + rh, bz + (-gauge + rw) * sz[j]);
        glVertex3f(bx + (-gauge - rw) * sx[j], by + rh, bz + (-gauge - rw) * sz[j]);
        glEnd();

        // Right rail
        glBegin(GL_QUADS);
        glVertex3f(ax + (gauge - rw) * sx[i], ay + rh, az + (gauge - rw) * sz[i]);
        glVertex3f(ax + (gauge + rw) * sx[i], ay + rh, az + (gauge + rw) * sz[i]);
        glVertex3f(bx + (gauge + rw) * sx[j], by + rh, bz + (gauge + rw) * sz[j]);
        glVertex3f(bx + (gauge - rw) * sx[j], by + rh, bz + (gauge - rw) * sz[j]);
        glEnd();

        // Sleeper
        glColor3f(0.4, 0.25, 0.1);
        float fw = 0.8;
        float dx = bx - ax, dz = bz - az;
        float slen = sqrt(dx * dx + dz * dz);
        if (slen < 0.001)
            continue;
        glNormal3f(0, 1, 0);
        glBegin(GL_QUADS);
        glVertex3f(ax + (-gauge - 1) * sx[i] - (dx / slen) * fw, ay + 0.1, az + (-gauge - 1) * sz[i] - (dz / slen) * fw);
        glVertex3f(ax + (gauge + 1) * sx[i] - (dx / slen) * fw, ay + 0.1, az + (gauge + 1) * sz[i] - (dz / slen) * fw);
        glVertex3f(ax + (gauge + 1) * sx[i] + (dx / slen) * fw, ay + 0.1, az + (gauge + 1) * sz[i] + (dz / slen) * fw);
        glVertex3f(ax + (-gauge - 1) * sx[i] + (dx / slen) * fw, ay + 0.1, az + (-gauge - 1) * sz[i] + (dz / slen) * fw);
        glEnd();
    }
    glEnable(GL_TEXTURE_2D);
}

void drawStars()
{
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glPointSize(2.0);
    glColor3f(1, 1, 1);
    glBegin(GL_POINTS);
    for (int i = 0; i < 200; i++)
    {
        glVertex3f(starX[i], starY[i], starZ[i]);
    }
    glEnd();
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
}

void drawFloor()
{
    glBindTexture(GL_TEXTURE_2D, txId[0]);
    glColor3f(1, 1, 1);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex3f(-500, 0, -500);
    glTexCoord2f(10, 0);
    glVertex3f(500, 0, -500);
    glTexCoord2f(10, 10);
    glVertex3f(500, 0, 500);
    glTexCoord2f(0, 10);
    glVertex3f(-500, 0, 500);
    glEnd();
}

void drawPlanets()
{
    // Earth
    glBindTexture(GL_TEXTURE_2D, txId[1]);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glPushMatrix();
    glTranslatef(-200, 150, -400);
    glRotatef(-90, 1, 0, 0);
    gluSphere(q, 40, 36, 17);
    glPopMatrix();

    // Sun
    glBindTexture(GL_TEXTURE_2D, txId[2]);
    glPushMatrix();
    glTranslatef(300, 200, -500);
    glRotatef(-90, 1, 0, 0);
    gluSphere(q, 80, 36, 17);
    glPopMatrix();
}

void drawCartAt(float t)
{
    float tt = fmod(t + NPTS, NPTS);
    int segment = (int)tt;
    int next = (segment + 1) % NPTS;
    float frac = tt - segment;

    float x = trackX[segment] * (1-frac) + trackX[next] * frac;
    float y = trackY[segment] * (1-frac) + trackY[next] * frac;
    float z = trackZ[segment] * (1-frac) + trackZ[next] * frac;

    float dx = trackX[next] - trackX[segment];
    float dy = trackY[next] - trackY[segment];
    float dz = trackZ[next] - trackZ[segment];
    float cartAngle = atan2(dx, dz) * 180.0 / M_PI;
    float dxz = sqrt(dx*dx + dz*dz);
    float pitchAngle = atan2(dy, dxz) * 180.0 / M_PI;

    glDisable(GL_TEXTURE_2D);
    glPushMatrix();
        glTranslatef(x, y + 1, z);
        glRotatef(cartAngle, 0, 1, 0);
        glRotatef(-pitchAngle, 1, 0, 0);

        glColor3f(0.3, 0.3, 0.3);
        // floor
        glPushMatrix();
            glScalef(6, 0.5, 7);
            glutSolidCube(1);
        glPopMatrix();
        // left wall
        glPushMatrix();
            glTranslatef(-3, 2, 0);
            glScalef(0.5, 4, 7);
            glutSolidCube(1);
        glPopMatrix();
        // right wall
        glPushMatrix();
            glTranslatef(3, 2, 0);
            glScalef(0.5, 4, 7);
            glutSolidCube(1);
        glPopMatrix();
        // back wall
        glPushMatrix();
            glTranslatef(0, 2, 3.5);
            glScalef(6, 4, 0.5);
            glutSolidCube(1);
        glPopMatrix();
        // front wall
        glPushMatrix();
            glTranslatef(0, 2, -3.5);
            glScalef(6, 4, 0.5);
            glutSolidCube(1);
        glPopMatrix();

    glPopMatrix();
    glEnable(GL_TEXTURE_2D);
}

void drawCart()
{
    for (int i = 0; i < 5; i++)
        drawCartAt(cartT + i * 0.4f);
}

void display()
{

    if (wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_LIGHTING);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_LIGHTING);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(eye_x, eye_y, eye_z, look_x, eye_y, look_z, 0., 1., 0.);

    float lightPos[] = {0., 50., 0., 1.};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);


    float floatY = sin(frame * 0.05) * 0.5;

    drawAlien(0, 3 + floatY, 110, alienColourIdx);

    drawStars();
    drawPlanets();
    drawFloor();
    drawTracks();
    drawCart();
    glutSwapBuffers();
}

void special(int key, int x, int y)
{
    if (key == GLUT_KEY_LEFT)
        turnDir = -1.0f;
    else if (key == GLUT_KEY_RIGHT)
        turnDir = 1.0f;
    else if (key == GLUT_KEY_UP)
        moveDir = 1.0f;
    else if (key == GLUT_KEY_DOWN)
        moveDir = -1.0f;
}

void specialUp(int key, int x, int y)
{
    if (key == GLUT_KEY_LEFT || key == GLUT_KEY_RIGHT)
        turnDir = 0.0f;
    else if (key == GLUT_KEY_UP || key == GLUT_KEY_DOWN)
        moveDir = 0.0f;
}

void keyboard(unsigned char key, int x, int y)
{
    if (key == 'q' || key == 'Q')
        wireframe = !wireframe;
}

void myTimer(int value)
{
    // get current segment length for normalised speed
    int seg = (int)cartT;
    int segNext = (seg + 1) % NPTS;
    float sdx = trackX[segNext] - trackX[seg];
    float sdy = trackY[segNext] - trackY[seg];
    float sdz = trackZ[segNext] - trackZ[seg];
    float segLen = sqrt(sdx*sdx + sdy*sdy + sdz*sdz);
    if (segLen < 0.001f) segLen = 0.001f;

    // get current and next height for physics
    float frac = cartT - seg;
    float currentHeight = trackY[seg] * (1-frac) + trackY[segNext] * frac;
    float nextT = cartT - (cartSpeed / segLen);
    if (nextT < 0) nextT += NPTS;
    int nextSeg = (int)nextT;
    float nextFrac = nextT - nextSeg;
    float nextHeight = trackY[nextSeg] * (1-nextFrac) + trackY[(nextSeg+1)%NPTS] * nextFrac;
    float dy = nextHeight - currentHeight;

    float baseSpeed = 1.5f;
    float minSpeed  = 0.3f;
    float maxSpeed  = 6.0f;

    if (dy > 0.05f) {
        cartSpeed -= 0.8f;
        if (cartSpeed < minSpeed) cartSpeed = minSpeed;
    } else if (dy < -0.05f) {
        cartSpeed += 0.10f;
        if (cartSpeed > maxSpeed) cartSpeed = maxSpeed;
    } else {
        if (cartSpeed > baseSpeed) cartSpeed -= 0.05f;
        else if (cartSpeed < baseSpeed) cartSpeed += 0.02f;
    }

    cartT -= cartSpeed / segLen;
    if (cartT < 0) cartT += NPTS;
    angle += turnDir * 0.5f;

    look_x = eye_x + sin(angle * M_PI / 180.0f);
    look_z = eye_z - cos(angle * M_PI / 180.0f);

    eye_x += moveDir * 0.5f * sin(angle * M_PI / 180.0f);
    eye_z -= moveDir * 0.5f * cos(angle * M_PI / 180.0f);

    frame++;
    glutPostRedisplay();
    glutTimerFunc(16, myTimer, 0);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(800, 800);
    glutCreateWindow("Alien Theme Park");
    initialize();
    glutTimerFunc(50, myTimer, 0);
    glutSpecialFunc(special);
    glutSpecialUpFunc(specialUp);
    glutKeyboardFunc(keyboard);
    glutDisplayFunc(display);
    glutMainLoop();
}