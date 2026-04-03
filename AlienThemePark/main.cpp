#include <iostream>
#include <cmath>
#include "loadTGA.h"
#include "loadBMP.h"
#include <GL/freeglut.h>
#include <cstring>

float pitch = 0.0f;
bool keys[256] = {false};
int lastMouseX = 400, lastMouseY = 400;
bool mouseCaptured = false;
int frame = 0;
GLUquadric *q;
float starX[200], starY[200], starZ[200];
float turnDir = 0.0f;
float moveDir = 0.0f;
float cartT = 0.0f;                                                            // Current position along track (0 to SPTS)
float cartSpeed = 0.15f;                                                       // Current speed
float angle = 90.0f, look_x = 0, look_y = 20.0f, look_z = -1., eye_x = -110.0f, eye_y = 20.0f, eye_z = 0.0f;
bool wireframe = false;
GLuint txId[4];
GLuint skyId[6];
const int NPTS = 20;
const int SPTS = 500;
float arcLength[SPTS];
float totalLen;
float splineX[SPTS], splineY[SPTS], splineZ[SPTS];
float trackX[] = {0, 26, 47, 50, 50, 50, 50, 50, 50, 47, 26, 0, -26, -47, -50, -50, -50, -50, -47, -26};
float trackY[] = {0, 0, 5, 15, 35, 55, 75, 90, 90, 70, 30, 0, 0, 0, 0, 0, 0, 0, 0, 0};
float trackZ[] = {100, 96, 83, 65, 40, 10, -20, -45, -65, -83, -96, -100, -96, -83, -65, -20, 20, 65, 83, 96};
float alienColours[3][3] = {
    {0.78, 0.19, 0.84},
    {0.21, 0.54, 0.54},
    {0.05, 0.63, 0.16}
};
int alienColourIdx = rand() % 3;
const int COL_N = 10;
float colVX[] = {3.5, 2.0, 1.5, 1.5, 1.8, 1.5, 1.5, 2.0, 3.0, 4.0};
float colVY[] = {0.0, 1.0, 3.0, 8.0, 12.0, 16.0, 20.0, 23.0, 25.0, 26.0};
bool inShadow = false;
bool rideRunning = false;
float boardingTimer = 0.0f;
const float BOARDING_DURATION = 200.0f;  
int   stationStopTimer = 0;
bool  stationTriggered = false;
const int STATION_STOP_DURATION = 312;// change this number for longer/shorter stop


float catmullRom(float p0, float p1, float p2, float p3, float t)
{
    return 0.5f * ( (2.0f * p1)
                  + (-p0 + p2) * t
                  + (2.0f*p0 - 5.0f*p1 + 4.0f*p2 - p3) * (t*t)
                  + (-p0 + 3.0f*p1 - 3.0f*p2 + p3) * (t*t*t) );
}

void buildSpline()
{
    int subdiv = SPTS / NPTS;
    for (int i = 0; i < NPTS; i++) {
        int i0 = (i - 1 + NPTS) % NPTS;
        int i1 = i;
        int i2 = (i + 1) % NPTS;
        int i3 = (i + 2) % NPTS;
        for (int j = 0; j < subdiv; j++) {
            float t = (float)j / subdiv;
            int idx = i * subdiv + j;
            splineX[idx] = catmullRom(trackX[i0], trackX[i1], trackX[i2], trackX[i3], t);
            splineY[idx] = catmullRom(trackY[i0], trackY[i1], trackY[i2], trackY[i3], t);
            splineZ[idx] = catmullRom(trackZ[i0], trackZ[i1], trackZ[i2], trackZ[i3], t);
            if (splineY[idx] < 1.0f) splineY[idx] = 1.0f;
        }
    }

    // Compute arc Lenghts for normalised speed
    arcLength[0] = 0;
    for (int i = 1; i < SPTS; i++) {
        float dx = splineX[i] - splineX[i-1];
        float dy = splineY[i] - splineY[i-1];
        float dz = splineZ[i] - splineZ[i-1];
        arcLength[i] = arcLength[i-1] + sqrt(dx*dx + dy*dy + dz*dz);
    }
    totalLen = arcLength[SPTS-1];
}

int findIndexAtDist(float targetDist)
{
    // Linear search - could be optimised with binary search if needed
    // Wrap targetDist around the track
    targetDist = fmod(targetDist + totalLen, totalLen);

    for (int i = 0; i < SPTS; i++) {
        if (arcLength[i] >= targetDist) {
            return i;
        }
    }
    return 0; 
}

void drawSkybox()
{
    float s = 600.0f;
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);
    // Front
    glBindTexture(GL_TEXTURE_2D, skyId[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex3f(-s,-s,-s);
    glTexCoord2f(1,0); glVertex3f( s,-s,-s);
    glTexCoord2f(1,1); glVertex3f( s, s,-s);
    glTexCoord2f(0,1); glVertex3f(-s, s,-s);
    glEnd();
    // Back
    glBindTexture(GL_TEXTURE_2D, skyId[1]);
    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex3f( s,-s, s);
    glTexCoord2f(1,0); glVertex3f(-s,-s, s);
    glTexCoord2f(1,1); glVertex3f(-s, s, s);
    glTexCoord2f(0,1); glVertex3f( s, s, s);
    glEnd();
    // Left
    glBindTexture(GL_TEXTURE_2D, skyId[2]);
    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex3f(-s,-s, s);
    glTexCoord2f(1,0); glVertex3f(-s,-s,-s);
    glTexCoord2f(1,1); glVertex3f(-s, s,-s);
    glTexCoord2f(0,1); glVertex3f(-s, s, s);
    glEnd();
    // Right
    glBindTexture(GL_TEXTURE_2D, skyId[3]);
    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex3f( s,-s,-s);
    glTexCoord2f(1,0); glVertex3f( s,-s, s);
    glTexCoord2f(1,1); glVertex3f( s, s, s);
    glTexCoord2f(0,1); glVertex3f( s, s,-s);
    glEnd();
    // Up
    glBindTexture(GL_TEXTURE_2D, skyId[4]);
    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex3f(-s, s,-s);
    glTexCoord2f(1,0); glVertex3f( s, s,-s);
    glTexCoord2f(1,1); glVertex3f( s, s, s);
    glTexCoord2f(0,1); glVertex3f(-s, s, s);
    glEnd();
    // Down
    glBindTexture(GL_TEXTURE_2D, skyId[5]);
    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex3f(-s,-s, s);
    glTexCoord2f(1,0); glVertex3f( s,-s, s);
    glTexCoord2f(1,1); glVertex3f( s,-s,-s);
    glTexCoord2f(0,1); glVertex3f(-s,-s,-s);
    glEnd();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void initialize()
{

    float white[] = {1., 1., 1., 1.};
    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, white);
    glLightfv(GL_LIGHT1, GL_SPECULAR, white);
    glLightfv(GL_LIGHT1, GL_SPECULAR, white);
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 25.0f); // Cone angle
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 2.0f); // Focus

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

    glBindTexture(GL_TEXTURE_2D, txId[3]);
    loadTGA("Stone_Texture.tga"); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    for (int i = 0; i < 200; i++)
    {
        starX[i] = (rand() % 1000) - 500;
        starY[i] = (rand() % 1000);
        starZ[i] = (rand() % 1000) - 500;
    }

    cartT = 399.0f;   // spline index near the station (X=-50 flat section)


    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60., 1., 1., 1000);

    const char* skyFiles[6] = {"space_ft.tga","space_bk.tga","space_lf.tga","space_rt.tga","space_up.tga","space_dn.tga"};
    glGenTextures(6, skyId);
    for (int i = 0; i < 6; i++) {
        glBindTexture(GL_TEXTURE_2D, skyId[i]);
        loadTGA(skyFiles[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    buildSpline();
}

void drawColumn()
{
    int nSlices = 24;
    float angleStepRad = (360.0f / nSlices) * M_PI / 180.0f;

    float vx[COL_N], vy[COL_N], vz[COL_N];
    float wx[COL_N], wy[COL_N], wz[COL_N];
    float nx[COL_N], ny[COL_N], nz[COL_N];
    float mx[COL_N], my[COL_N], mz[COL_N];

    for (int i = 0; i < COL_N; i++) {
        vx[i] = colVX[i];
        vy[i] = colVY[i];
        vz[i] = 0;

        float tx, ty;
        if (i < COL_N - 1) {
            tx = colVX[i+1] - colVX[i];
            ty = colVY[i+1] - colVY[i];
        } else {
            tx = colVX[i] - colVX[i-1];
            ty = colVY[i] - colVY[i-1];
        }
        float len = sqrt(tx*tx + ty*ty);
        nx[i] =  ty / len;
        ny[i] = -tx / len;
        nz[i] = 0;
    }

    glDisable(GL_TEXTURE_2D);
    glColor3f(0.4, 0.8, 0.5);

    for (int j = 0; j < nSlices; j++) {
        for (int i = 0; i < COL_N; i++) {
            wx[i] =  cos(angleStepRad) * vx[i] + sin(angleStepRad) * vz[i];
            wy[i] =  vy[i];
            wz[i] = -sin(angleStepRad) * vx[i] + cos(angleStepRad) * vz[i];
            mx[i] =  cos(angleStepRad) * nx[i] + sin(angleStepRad) * nz[i];
            my[i] =  ny[i];
            mz[i] = -sin(angleStepRad) * nx[i] + cos(angleStepRad) * nz[i];
        }

        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i < COL_N; i++) {
            glNormal3f(nx[i], ny[i], nz[i]);
            glVertex3f(vx[i], vy[i], vz[i]);
            glNormal3f(mx[i], my[i], mz[i]);
            glVertex3f(wx[i], wy[i], wz[i]);
        }
        glEnd();

        for (int i = 0; i < COL_N; i++) {
            vx[i] = wx[i]; vy[i] = wy[i]; vz[i] = wz[i];
            nx[i] = mx[i]; ny[i] = my[i]; nz[i] = mz[i];
        }
    }
    glEnable(GL_TEXTURE_2D);
}

void drawAlien(float x, float y, float z, int colourIndex)
{
    float r = alienColours[colourIndex][0];
    float g = alienColours[colourIndex][1];
    float b = alienColours[colourIndex][2];

    glDisable(GL_TEXTURE_2D);
    glPushMatrix();
    glTranslatef(x, y, z);

        if (!inShadow) glColor3f(r, g, b);
            glPushMatrix();
            glScalef(1.0, 0.75, 1.0);
            glutSolidSphere(3, 16, 16); // Head
        glPopMatrix();

        // Eye
        if (!inShadow) glColor3f(1, 1, 1);
        glPushMatrix();
            glTranslatef(0, 1, -2.5);
            glutSolidSphere(1.2, 12, 12);
        glPopMatrix();

        // Cyan Pupil
        if (!inShadow)glColor3f(0, 1, 1);
        glPushMatrix();
            glTranslatef(0, 1, -3.2);
            glutSolidSphere(0.6, 10, 10);
        glPopMatrix();

        // Mouth
        if (!inShadow) glColor3f(0, 0, 0);
        glPushMatrix();
            glTranslatef(0, -1, -2.8);
            glScalef(1.5, 0.3, 0.3);
            glutSolidCube(1);
        glPopMatrix();
    glPopMatrix();
    glEnable(GL_TEXTURE_2D);
}

void drawWaitingAliens()
{
    float cartDist = arcLength[(int)cartT];
    int half = STATION_STOP_DURATION / 2;

    // ---- INITIAL BOARDING (before ride starts) ----
    if (!rideRunning) {
        float t = fminf(boardingTimer / BOARDING_DURATION, 1.0f);
        for (int i = 0; i < 5; i++) {
            float dist = cartDist - i * 8.0f;
            if (dist < 0) dist += totalLen;
            int ci = findIndexAtDist(dist);
            float startX = splineX[ci] - 25.0f;
            float px = startX + (splineX[ci] - startX) * t;
            glPushMatrix();
                glTranslatef(px, splineY[ci] + 3.0f, splineZ[ci]);
                glRotatef(-90, 0, 1, 0);
                drawAlien(0, 0, 0, i % 3);
            glPopMatrix();
        }
        return;
    }

    // ---- ALIGHTING: old aliens sink below floor (first half of stop) ----
    if (stationStopTimer > half) {
        float t = 1.0f - (float)(stationStopTimer - half) / half;
        for (int i = 0; i < 5; i++) {
            float dist = cartDist - i * 8.0f;
            if (dist < 0) dist += totalLen;
            int ci   = findIndexAtDist(dist);
            int next = (ci + 1) % SPTS;
            float x = splineX[ci], y = splineY[ci], z = splineZ[ci];
            float dx = splineX[next]-x, dy = splineY[next]-y, dz = splineZ[next]-z;
            float ca = atan2(dx, dz) * 180.0f / M_PI;
            float pa = atan2(dy, sqrt(dx*dx+dz*dz)) * 180.0f / M_PI;
            float py = (y + 4.5f) + (-15.0f - (y + 4.5f)) * t;  // sink to Y=-15
            glPushMatrix();
                glTranslatef(x, py, z);
                glRotatef(ca, 0, 1, 0);
                glRotatef(-pa, 1, 0, 0);
                drawAlien(0, 0, 0, i % 3);
            glPopMatrix();
        }

    // ---- NEW BOARDING: fresh aliens walk in (second half of stop) ----
    } else if (stationStopTimer > 0) {
        float t = 1.0f - (float)stationStopTimer / half;
        for (int i = 0; i < 5; i++) {
            float dist = cartDist - i * 8.0f;
            if (dist < 0) dist += totalLen;
            int ci = findIndexAtDist(dist);
            float startX = splineX[ci] - 25.0f;
            float px = startX + (splineX[ci] - startX) * t;
            glPushMatrix();
                glTranslatef(px, splineY[ci] + 3.0f, splineZ[ci]);
                glRotatef(-90, 0, 1, 0);
                drawAlien(0, 0, 0, (i + 1) % 3);  // different colours = new aliens
            glPopMatrix();
        }

    // ---- RIDING: aliens inside moving cart ----
    } else {
        for (int i = 0; i < 5; i++) {
            float dist = cartDist - i * 8.0f;
            if (dist < 0) dist += totalLen;
            int ci   = findIndexAtDist(dist);
            int next = (ci + 1) % SPTS;
            float x = splineX[ci], y = splineY[ci], z = splineZ[ci];
            float dx = splineX[next]-x, dy = splineY[next]-y, dz = splineZ[next]-z;
            float ca = atan2(dx, dz) * 180.0f / M_PI;
            float pa = atan2(dy, sqrt(dx*dx+dz*dz)) * 180.0f / M_PI;
            glPushMatrix();
                glTranslatef(x, y + 1.5f, z);
                glRotatef(ca, 0, 1, 0);
                glRotatef(-pa, 1, 0, 0);
                glTranslatef(0, 3.0f, 0);
                drawAlien(0, 0, 0, i % 3);
            glPopMatrix();
        }
    }
}

void drawGate()
{
    // Left column
    glPushMatrix();
        glTranslatef(-15, 0, 0);
        drawColumn();
    glPopMatrix();
    // Right column
    glPushMatrix();
        glTranslatef(15, 0, 0);
        drawColumn();
    glPopMatrix();
    // Connecting beam
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.4, 0.8, 0.5);
    glPushMatrix();
        glTranslatef(0, 27, 0);
        glScalef(30, 2, 2);
        glutSolidCube(1);
    glPopMatrix();
    glEnable(GL_TEXTURE_2D);
}

void drawStation()
{
    const int N  = 20;
    float cx     = -50.0f;
    float rInner = 13.0f;
    float rOuter = 16.0f;
    float zS     = -50.0f;
    float zE     =  40.0f;
    float len    = zE - zS;
    float maxAngle = M_PI;   // right wall + ceiling, left side open

    glBindTexture(GL_TEXTURE_2D, txId[3]);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glColor3f(1, 1, 1);

    // Inner surface
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= N; i++) {
        float a  = maxAngle * i / N;
        float px = cx + rInner * cos(a);
        float py = rInner * sin(a);
        glNormal3f(-cos(a), -sin(a), 0);
        glTexCoord2f((float)i/N, 0      ); glVertex3f(px, py, zS);
        glTexCoord2f((float)i/N, len/15 ); glVertex3f(px, py, zE);
    }
    glEnd();

    // Outer surface
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= N; i++) {
        float a  = maxAngle * i / N;
        float px = cx + rOuter * cos(a);
        float py = rOuter * sin(a);
        glNormal3f(cos(a), sin(a), 0);
        glTexCoord2f((float)i/N, 0      ); glVertex3f(px, py, zE);
        glTexCoord2f((float)i/N, len/15 ); glVertex3f(px, py, zS);
    }
    glEnd();

    // End caps at entrance and exit
    for (int end = 0; end < 2; end++) {
        float z  = (end == 0) ? zS : zE;
        float nz = (end == 0) ? -1.0f : 1.0f;
        glNormal3f(0, 0, nz);
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= N; i++) {
            float a = maxAngle * i / N;
            if (end == 0) {
                glTexCoord2f((float)i/N, 0); glVertex3f(cx + rOuter*cos(a), rOuter*sin(a), z);
                glTexCoord2f((float)i/N, 1); glVertex3f(cx + rInner*cos(a), rInner*sin(a), z);
            } else {
                glTexCoord2f((float)i/N, 0); glVertex3f(cx + rInner*cos(a), rInner*sin(a), z);
                glTexCoord2f((float)i/N, 1); glVertex3f(cx + rOuter*cos(a), rOuter*sin(a), z);
            }
        }
        glEnd();
    }

    // Right base slab only (no left wall so no left base)
    glNormal3f(0, 1, 0);
    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex3f(cx+rInner, 0, zS);
    glTexCoord2f(1,0); glVertex3f(cx+rOuter, 0, zS);
    glTexCoord2f(1,1); glVertex3f(cx+rOuter, 0, zE);
    glTexCoord2f(0,1); glVertex3f(cx+rInner, 0, zE);
    glEnd();
}

void drawTracks()
{
    glDisable(GL_TEXTURE_2D);
    float gauge = 3.0;
    float rw = 0.4;
    float rh = 0.5;

    // Pre-compute averaged sideways vector at each waypoint
    float sx[SPTS], sz[SPTS];
    for (int i = 0; i < SPTS; i++)
    {
        int prev = (i - 1 + SPTS) % SPTS;
        int next = (i + 1) % SPTS;

        float dx_in = splineX[i] - splineX[prev];
        float dz_in = splineZ[i] - splineZ[prev];
        float dx_out = splineX[next] - splineX[i];
        float dz_out = splineZ[next] - splineZ[i];

        float dx_avg = dx_in + dx_out;
        float dz_avg = dz_in + dz_out;
        float len = sqrt(dx_avg * dx_avg + dz_avg * dz_avg);
        if (len < 0.001)
            len = 1.0;

        sx[i] = dz_avg / len;
        sz[i] = -dx_avg / len;
    }

    for (int i = 0; i < SPTS; i++)
    {
        int j = (i + 1) % SPTS;
        float ax = splineX[i], ay = splineY[i], az = splineZ[i];
        float bx = splineX[j], by = splineY[j], bz = splineZ[j];

        glColor3f(0.6, 0.6, 0.6);

        // Left rail - top
        glNormal3f(0, 1, 0);
        glBegin(GL_QUADS);
        glVertex3f(ax + (-gauge - rw) * sx[i], ay + rh, az + (-gauge - rw) * sz[i]);
        glVertex3f(ax + (-gauge + rw) * sx[i], ay + rh, az + (-gauge + rw) * sz[i]);
        glVertex3f(bx + (-gauge + rw) * sx[j], by + rh, bz + (-gauge + rw) * sz[j]);
        glVertex3f(bx + (-gauge - rw) * sx[j], by + rh, bz + (-gauge - rw) * sz[j]);
        glEnd();
        // Left rail - inner side
        glBegin(GL_QUADS);
        glNormal3f(sx[i], 0, sz[i]);
        glVertex3f(ax + (-gauge + rw) * sx[i], ay,      az + (-gauge + rw) * sz[i]);
        glVertex3f(ax + (-gauge + rw) * sx[i], ay + rh, az + (-gauge + rw) * sz[i]);
        glVertex3f(bx + (-gauge + rw) * sx[j], by + rh, bz + (-gauge + rw) * sz[j]);
        glVertex3f(bx + (-gauge + rw) * sx[j], by,      bz + (-gauge + rw) * sz[j]);
        glEnd();
        // Left rail - outer side
        glBegin(GL_QUADS);
        glNormal3f(-sx[i], 0, -sz[i]);
        glVertex3f(ax + (-gauge - rw) * sx[i], ay + rh, az + (-gauge - rw) * sz[i]);
        glVertex3f(ax + (-gauge - rw) * sx[i], ay,      az + (-gauge - rw) * sz[i]);
        glVertex3f(bx + (-gauge - rw) * sx[j], by,      bz + (-gauge - rw) * sz[j]);
        glVertex3f(bx + (-gauge - rw) * sx[j], by + rh, bz + (-gauge - rw) * sz[j]);
        glEnd();

        // Right rail - top
        glNormal3f(0, 1, 0);
        glBegin(GL_QUADS);
        glVertex3f(ax + (gauge - rw) * sx[i], ay + rh, az + (gauge - rw) * sz[i]);
        glVertex3f(ax + (gauge + rw) * sx[i], ay + rh, az + (gauge + rw) * sz[i]);
        glVertex3f(bx + (gauge + rw) * sx[j], by + rh, bz + (gauge + rw) * sz[j]);
        glVertex3f(bx + (gauge - rw) * sx[j], by + rh, bz + (gauge - rw) * sz[j]);
        glEnd();
        // Right rail - inner side
        glBegin(GL_QUADS);
        glNormal3f(-sx[i], 0, -sz[i]);
        glVertex3f(ax + (gauge - rw) * sx[i], ay + rh, az + (gauge - rw) * sz[i]);
        glVertex3f(ax + (gauge - rw) * sx[i], ay,      az + (gauge - rw) * sz[i]);
        glVertex3f(bx + (gauge - rw) * sx[j], by,      bz + (gauge - rw) * sz[j]);
        glVertex3f(bx + (gauge - rw) * sx[j], by + rh, bz + (gauge - rw) * sz[j]);
        glEnd();
        // Right rail - outer side
        glBegin(GL_QUADS);
        glNormal3f(sx[i], 0, sz[i]);
        glVertex3f(ax + (gauge + rw) * sx[i], ay,      az + (gauge + rw) * sz[i]);
        glVertex3f(ax + (gauge + rw) * sx[i], ay + rh, az + (gauge + rw) * sz[i]);
        glVertex3f(bx + (gauge + rw) * sx[j], by + rh, bz + (gauge + rw) * sz[j]);
        glVertex3f(bx + (gauge + rw) * sx[j], by,      bz + (gauge + rw) * sz[j]);
        glEnd();

        // Sleeper
        if (i % 2 == 0) {
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
        

        // Support pillar every 25 points where track is elevated
        if (i % 25 == 0 && splineY[i] > 3.0f) {
            glColor3f(0.5, 0.5, 0.55);
            glPushMatrix();
                glTranslatef(splineX[i], 0, splineZ[i]);
                glRotatef(-90, 1, 0, 0);
                gluCylinder(q, 1.0, 1.0, splineY[i], 8, 1);
            glPopMatrix();
        }
    }
    glEnable(GL_TEXTURE_2D);
}

void drawFloor()
{
    glBindTexture(GL_TEXTURE_2D, txId[0]);
    glColor3f(1, 1, 1);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex3f(-500, -0.01, -500);
    glTexCoord2f(10, 0);
    glVertex3f(500, -0.01, -500);
    glTexCoord2f(10, 10);
    glVertex3f(500, -0.01, 500);
    glTexCoord2f(0, 10);
    glVertex3f(-500, -0.01, 500);
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
    int segment = (int)fmod(t + SPTS, SPTS);
    int next = (segment + 1) % SPTS;
    float frac = t - floor(t);

    float x = splineX[segment] * (1-frac) + splineX[next] * frac;
    float y = splineY[segment] * (1-frac) + splineY[next] * frac;
    float z = splineZ[segment] * (1-frac) + splineZ[next] * frac;

    float dx = splineX[next] - splineX[segment];
    float dy = splineY[next] - splineY[segment];
    float dz = splineZ[next] - splineZ[segment];
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
    int index;
    float cartDist = arcLength[(int)cartT]; // Look ahead distance for next cart
    for (int i = 0; i < 5; i++)
    {
        index = findIndexAtDist(cartDist - i * 8);
        drawCartAt(index);
    }
}

void updateCartLight()
{
    int seg = (int)fmod(cartT, SPTS);
    int nxt = (seg - 1 + SPTS) % SPTS;

    float lx = splineX[seg];
    float ly = splineY[seg] + 3.0f;
    float lz = splineZ[seg];

    float dx = splineX[nxt] - splineX[seg];
    float dy = splineY[nxt] - splineY[seg];
    float dz = splineZ[nxt] - splineZ[seg];

    float light1_pos[] = {lx, ly, lz, 1.0f};
    float light1_dir[] = {dx, dy, dz};
    glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, light1_dir);
}

void drawShadow()
{
    float lx = 0, ly = 50, lz = 0;
    float shadowMat[16] = {
        ly, 0, 0, 0,
        -lx, 0, -lz, -1,
        0, 0, ly, 0,
        0, 0, 0, ly
    };

    inShadow = true;
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColor3f(0, 0, 0);

    glPushMatrix();
        glMultMatrixf(shadowMat);
        drawWaitingAliens();   // aliens only — no cart (too elevated, shadow is huge)
    glPopMatrix();

    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    inShadow = false;
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
    gluLookAt(eye_x, eye_y, eye_z, look_x, look_y, look_z, 0., 1., 0.);

    float lightPos[] = {0., 50., 0., 1.};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);


    float floatY = sin(frame * 0.05) * 0.5;

    // drawAlien(0, 3 + floatY, 110, alienColourIdx);

    drawSkybox();
    drawPlanets();
    drawFloor();
    drawShadow();
    drawTracks();
    drawCart();
    drawStation();
    drawWaitingAliens();
    updateCartLight();
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
    keys[key] = true;
    if (key == 'q' || key == 'Q') {
        wireframe = !wireframe;
    }

    if (key == 27) {
        exit(0);
    }

    if (key == 'g' || key == 'G') {
        rideRunning = true;
    }
}

void keyboardUp(unsigned char key, int x, int y)
{
    keys[key] = false;
}

void mouseMove(int x, int y)
{
    int dx = x - lastMouseX;
    int dy = y - lastMouseY;
    lastMouseX = x;
    lastMouseY = y;

    if (abs(dx) > 50 || abs(dy) > 50) return;  // ignore big jumps on focus/warp

    angle += dx * 0.3f;
    pitch -= dy * 0.3f;
    if (pitch >  89.0f) pitch =  89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    // look_x/y/z recomputed every tick in myTimer
}

// Clear keys when window is not visible to prevent stuck keys when alt-tabbing out
// Helped by Claude
void onFocusChange(int state)
{
    if (state == GLUT_NOT_VISIBLE) {
        memset(keys, 0, sizeof(keys)); // Clear all keys when window is not visible
    }
}

// Clear keys when mouse leaves the window to prevent stuck keys when mouse leaves the window
// Helped by Claude
void mouseEntry(int state)
{
    if (state == GLUT_LEFT) {
        memset(keys, 0, sizeof(keys)); // Clear all keys when mouse leaves window
    }
}

void myTimer(int value)
{
    // get current segment length for normalised speed
    int seg = (int)cartT;
    int segNext = (seg + 1) % SPTS;
    float sdx = splineX[segNext] - splineX[seg];
    float sdy = splineY[segNext] - splineY[seg];
    float sdz = splineZ[segNext] - splineZ[seg];
    float segLen = sqrt(sdx*sdx + sdy*sdy + sdz*sdz);
    if (segLen < 0.001f) segLen = 0.001f;

    // look 40 units ahead to anticipate hills early
    float lookAheadDist = arcLength[(int)cartT] - 40.0f;
    if (lookAheadDist < 0) lookAheadDist += totalLen;
    int nextSeg = findIndexAtDist(lookAheadDist);
    float currentHeight = splineY[(int)cartT];
    float nextHeight = splineY[nextSeg];
    float dy = nextHeight - currentHeight;

    float baseSpeed = 1.5f;
    float minSpeed  = 0.3f;
    float maxSpeed  = 4.0f;

    if (dy > 0.05f) {
        cartSpeed -= 0.12f;
        if (cartSpeed < minSpeed) cartSpeed = minSpeed;
    } else if (dy < -0.05f) {
        cartSpeed += 0.10f;
        if (cartSpeed > maxSpeed) cartSpeed = maxSpeed;
    } else {
        if (cartSpeed > baseSpeed) cartSpeed -= 0.05f;
        else if (cartSpeed < baseSpeed) cartSpeed += 0.02f;
    }

    if (!rideRunning) {
        boardingTimer += 1.0f;
        if (boardingTimer >= BOARDING_DURATION) rideRunning = true;
    } else {
        float cx = splineX[(int)cartT];
        float cy = splineY[(int)cartT];

        if (stationTriggered && fabs(cx - (-50.0f)) > 20.0f)
            stationTriggered = false;

        if (!stationTriggered && fabs(cx - (-50.0f)) < 0.00001f && cy < 3.0f) {
 
            stationStopTimer = STATION_STOP_DURATION;
            stationTriggered = true;
        }

        if (stationStopTimer > 0) {
            stationStopTimer--;
        } else {
            cartT -= cartSpeed / segLen;
            if (cartT < 0) cartT += SPTS;
        }
    }




    float speed = 0.5f;
    float rad = angle * M_PI / 180.0f;
    if (keys['w'] || keys['W']) { eye_x += speed * sin(rad); eye_z -= speed * cos(rad); }
    if (keys['s'] || keys['S']) { eye_x -= speed * sin(rad); eye_z += speed * cos(rad); }
    if (keys['a'] || keys['A']) { eye_x -= speed * cos(rad); eye_z -= speed * sin(rad); }
    if (keys['d'] || keys['D']) { eye_x += speed * cos(rad); eye_z += speed * sin(rad); }
    if (keys[' '])              { eye_y += speed; } // Fly up
    if (keys['f'] || keys['F']) { eye_y -= speed; } // Fly down (SHIFT was impossible to figure out so f will do it for now)

    // update look target
    look_x = eye_x + cos(pitch * M_PI/180.0f) * sin(rad);
    look_y = eye_y + sin(pitch * M_PI/180.0f);
    look_z = eye_z - cos(pitch * M_PI/180.0f) * cos(rad);


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
    glutVisibilityFunc(onFocusChange);
    glutEntryFunc(mouseEntry);
    glutTimerFunc(50, myTimer, 0);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutPassiveMotionFunc(mouseMove);
    glutSetCursor(GLUT_CURSOR_NONE);
    glutDisplayFunc(display);
    glutMainLoop();
}