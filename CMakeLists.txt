# This file should be in .gitignore but is important to run the project.

cmake_minimum_required(VERSION 3.0)
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

# Link Libs
target_link_libraries(Main rover shape utils mat4 transforms terrain perlin sphere glew32 freeglut opengl32)
target_link_libraries(ShapeTest rover shape utils mat4 transforms glew32 freeglut opengl32)

# After build
file(GLOB LIBRARIES "lib/*.dll")
file(COPY ${LIBRARIES} DESTINATION .)
