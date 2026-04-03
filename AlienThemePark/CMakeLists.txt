cmake_minimum_required(VERSION 2.8...3.5)

project(AlienThemePark)

add_executable(AlienThemePark.out main.cpp)

set(OpenGL_GL_PREFERENCE LEGACY)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )

target_link_libraries( AlienThemePark.out ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} )

file(GLOB IMAGE_FILES "${CMAKE_SOURCE_DIR}/*.tga" "${CMAKE_SOURCE_DIR}/*.bmp")
foreach(IMAGE_FILE ${IMAGE_FILES})
    get_filename_component(IMAGE_NAME ${IMAGE_FILE} NAME)
    configure_file(${IMAGE_FILE} ${CMAKE_BINARY_DIR}/${IMAGE_NAME} COPYONLY)
endforeach()