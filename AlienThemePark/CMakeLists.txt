cmake_minimum_required(VERSION 2.8...3.5)

project(AlienThemePark)

add_executable(AlienThemePark.out main.cpp)

set(OpenGL_GL_PREFERENCE LEGACY)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )

target_link_libraries( AlienThemePark.out ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} )