#ifndef H_TORUS
#define H_TORUS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "SceneObject.h"

class Torus : public SceneObject {
private:
    glm::vec3 center;
    float majorRadius;
    float minorRadius;
    glm::mat4 invTransform = glm::mat4(1.0f);

public:
    Torus() {}
    Torus(glm::vec3 c, float R, float r) : center(c), majorRadius(R), minorRadius(r) {}
    void setTransform(glm::mat4 transform) { invTransform = glm::inverse(transform); }
    float intersect(glm::vec3 p0, glm::vec3 dir);
    glm::vec3 normal(glm::vec3 p);
};

#endif
