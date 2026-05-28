#include "Torus.h"
#include <cmath>

float Torus::intersect(glm::vec3 p0, glm::vec3 dir)
{
    p0  = glm::vec3(invTransform * glm::vec4(p0, 1.0));
    dir = glm::vec3(invTransform * glm::vec4(dir, 0.0));

    float R = majorRadius, r = minorRadius;
    glm::vec3 o = p0 - center;

    auto f = [&](float t) {
        float x = o.x + t*dir.x;
        float y = o.y + t*dir.y;
        float z = o.z + t*dir.z;
        float dxz = sqrtf(x*x + z*z);
        return (dxz - R)*(dxz - R) + y*y - r*r;
    };

    float tBest = -1;
    int   N     = 200;
    float tmin  = 0.001f, tmax = 200.0f;
    float step  = (tmax - tmin) / N;

    float prev = f(tmin);
    for (int i = 1; i <= N; i++)
    {
        float tNext = tmin + i * step;
        float curr  = f(tNext);
        if (prev * curr < 0)
        {
            float lo = tNext - step, hi = tNext;
            for (int j = 0; j < 50; j++)
            {
                float mid = (lo + hi) * 0.5f;
                if (f(lo) * f(mid) < 0) hi = mid;
                else                    lo = mid;
            }
            float root = (lo + hi) * 0.5f;
            if (tBest < 0 || root < tBest) tBest = root;
        }
        prev = curr;
    }
    return tBest;
}

glm::vec3 Torus::normal(glm::vec3 p)
{
    p = glm::vec3(invTransform * glm::vec4(p, 1.0));
    glm::vec3 o = p - center;
    glm::vec3 q = majorRadius * glm::normalize(glm::vec3(o.x, 0, o.z));
    glm::vec3 n = glm::normalize(o - q);
    return glm::normalize(glm::vec3(glm::transpose(invTransform) * glm::vec4(n, 0.0)));
}
