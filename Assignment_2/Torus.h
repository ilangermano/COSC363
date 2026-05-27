#ifndef H_TORUS
#define H_TORUS
#include <glm/glm.hpp>
#include "SceneObject.h"

class Torus : public SceneObject {
private:
    glm::vec3 center;
    float majorRadius;  // distance from center to tube center
    float minorRadius;  // radius of the tube

public:
    Torus() {}
    Torus(glm::vec3 c, float R, float r) : center(c), majorRadius(R), minorRadius(r) {}

    float intersect(glm::vec3 p0, glm::vec3 dir);
    glm::vec3 normal(glm::vec3 p);
};

#endif
