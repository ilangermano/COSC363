#ifndef H_CYLINDER
#define H_CYLINDER
#include <glm/glm.hpp>
#include "SceneObject.h"
#include <glm/gtc/matrix_transform.hpp>

class Cylinder : public SceneObject {
private:
    glm::vec3 center;
    float radius;
    float height;
    glm::mat4 invTransform = glm::mat4(1.0f);  // identity by default
public:
    Cylinder() {}
    Cylinder(glm::vec3 c, float r, float h) : center(c), radius(r), height(h) {}
    void setTransform(glm::mat4 transform) { invTransform = glm::inverse(transform); }
    float intersect(glm::vec3 p0, glm::vec3 dir);
    glm::vec3 normal(glm::vec3 p);
};


#endif
