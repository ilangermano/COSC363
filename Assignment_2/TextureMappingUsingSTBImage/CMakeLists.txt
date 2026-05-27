cmake_minimum_required(VERSION 2.8...3.5)

project(stb_texture)

add_executable(RayTracer_stb_texture.out RayTracer_stb_texture.cpp Ray.cpp SceneObject.cpp Sphere.cpp Plane.cpp)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(glm REQUIRED)

include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} ${GLM_INCLUDE_DIR} )

target_link_libraries( RayTracer_stb_texture.out ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} ${GLM_LIBRARY} )

