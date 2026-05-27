#include "Cylinder.h"
#include <cmath>

float Cylinder::intersect(glm::vec3 p0, glm::vec3 dir)
{
    p0  = glm::vec3(invTransform * glm::vec4(p0, 1.0));
    dir = glm::vec3(invTransform * glm::vec4(dir, 0.0));

    float dx = p0.x - center.x;
    float dz = p0.z - center.z;

    float a = dir.x*dir.x + dir.z*dir.z;
    float b = 2*(dx*dir.x + dz*dir.z);
    float c = dx*dx + dz*dz - radius*radius;

    float disc = b*b - 4*a*c;
    float t = -1;

    if (disc >= 0)
    {
        float t1 = (-b - sqrt(disc)) / (2*a);
        float t2 = (-b + sqrt(disc)) / (2*a);

        float y1 = p0.y + t1*dir.y;
        if (t1 > 0 && y1 >= center.y && y1 <= center.y + height) t = t1;

        float y2 = p0.y + t2*dir.y;
        if (t2 > 0 && y2 >= center.y && y2 <= center.y + height)
            if (t < 0 || t2 < t) t = t2;
    }

    if (fabs(dir.y) > 1e-4)
    {
        float tBot = (center.y - p0.y) / dir.y;
        float xBot = p0.x + tBot*dir.x - center.x;
        float zBot = p0.z + tBot*dir.z - center.z;
        if (tBot > 0 && xBot*xBot + zBot*zBot <= radius*radius)
            if (t < 0 || tBot < t) t = tBot;

        
        float tTop = (center.y + height - p0.y) / dir.y;
        float xTop = p0.x + tTop*dir.x - center.x;
        float zTop = p0.z + tTop*dir.z - center.z;
        if (tTop > 0 && xTop*xTop + zTop*zTop <= radius*radius)
            if (t < 0 || tTop < t) t = tTop;
    }

    return t;
}

glm::vec3 Cylinder::normal(glm::vec3 p)
{
    p = glm::vec3(invTransform * glm::vec4(p, 1.0));
    glm::vec3 n;
    if (fabs(p.y - (center.y + height)) < 1e-3) n = glm::vec3(0, 1, 0);
    else if (fabs(p.y - center.y) < 1e-3)       n = glm::vec3(0, -1, 0);
    else n = glm::normalize(glm::vec3(p.x-center.x, 0, p.z-center.z));
    // Transform normal back to world space
    return glm::normalize(glm::vec3(glm::transpose(invTransform) * glm::vec4(n, 0.0)));
}
