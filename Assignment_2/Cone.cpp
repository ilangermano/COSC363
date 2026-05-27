#include "Cone.h"
#include <cmath>

float Cone::intersect(glm::vec3 p0, glm::vec3 dir)
{
    float k = (radius/height) * (radius/height);
    float dx = p0.x - center.x;
    float dz = p0.z - center.z;
    float dy = p0.y - (center.y + height);  // from apex

    float a = dir.x*dir.x + dir.z*dir.z - k*dir.y*dir.y;
    float b = 2*(dx*dir.x + dz*dir.z - k*dy*dir.y);
    float c = dx*dx + dz*dz - k*dy*dy;

    float disc = b*b - 4*a*c;
    if (disc < 0) return -1;

    float t1 = (-b - sqrt(disc)) / (2*a);
    float t2 = (-b + sqrt(disc)) / (2*a);

    float t = -1;
    float y1 = p0.y + t1*dir.y;
    if (t1 > 0 && y1 >= center.y && y1 <= center.y + height) t = t1;

    float y2 = p0.y + t2*dir.y;
    if (t2 > 0 && y2 >= center.y && y2 <= center.y + height)
        if (t < 0 || t2 < t) t = t2;

    return t;
}

glm::vec3 Cone::normal(glm::vec3 p)
{
    float r = sqrt((p.x-center.x)*(p.x-center.x) + (p.z-center.z)*(p.z-center.z));
    glm::vec3 n(p.x-center.x, r*(radius/height), p.z-center.z);
    return glm::normalize(n);
}