# This file should be in .gitignore but is important to run the project.

cmake_minimum_required(VERSION 3.10)
project( moon_rover )

# Includes
include_directories(include)

# Executables
add_executable(Main src/main.cpp)
add_executable(ShapeTest src/ShapeTest.cpp )

# Libraries
add_library(shape src/shapes/Shape.cpp)
add_library(rover src/shapes/Rover.cpp)
add_library(utils src/utils.cpp)
add_library(transforms src/transforms.cpp)
add_library(mat4 src/mat4.cpp)
add_library(perlin src/perlin.cpp)
add_library(sphere src/sphere.cpp)
add_library(terrain src/terrain.cpp)
add_library(player src/player.cpp)
add_library(camera src/camera.cpp)

# Graphics libraries differ per platform: Windows uses the DLLs vendored in lib/,
# while other platforms resolve them through the system package manager.
if(WIN32)
	set(GRAPHICS_LIBS glew32 freeglut opengl32)
else()
	find_package(OpenGL REQUIRED)
	find_package(GLUT REQUIRED)
	find_package(GLEW REQUIRED)
	set(GRAPHICS_LIBS GLEW::GLEW GLUT::GLUT OpenGL::GL)
endif()

# Link Libs
target_link_libraries(Main rover shape utils mat4 transforms terrain perlin sphere camera player ${GRAPHICS_LIBS})
target_link_libraries(ShapeTest rover shape utils mat4 transforms ${GRAPHICS_LIBS})

# After build
file(GLOB LIBRARIES "lib/*.dll")
file(COPY ${LIBRARIES} DESTINATION .)
