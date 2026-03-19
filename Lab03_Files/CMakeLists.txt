cmake_minimum_required(VERSION 2.8...3.5)

project(lab3)

add_executable(ToyTrain.out I_ToyTrain/ToyTrain.cpp I_ToyTrain/RailModels.cpp)
add_executable(Yard.out II_Yard/Yard.cpp)
add_executable(Earth.out III_Earth/Earth.cpp)

set(OpenGL_GL_PREFERENCE LEGACY)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )

target_link_libraries( ToyTrain.out ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} )
target_link_libraries( Yard.out ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} )
target_link_libraries( Earth.out ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} )
