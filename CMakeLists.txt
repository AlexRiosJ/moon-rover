cmake_minimum_required(VERSION 2.8)
project( moon_rover )
find_package(GLUT REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

# Includes
include_directories(GLUT_INCLUDE_DIRS})
include_directories(OPENGL_INCLUDE_DIR})
include_directories(GLEW_INCLUDE_DIRS})
include_directories(include)

# Executables
add_executable(Main src/main.cpp )
add_executable(ShapeTest src/ShapeTest.cpp )
# Libraries
<<<<<<< HEAD
add_library(EGL SHARED IMPORTED)
add_library(Utils src/utils.cpp)
add_library(Mat4 src/mat4.cpp)
add_library(Transforms src/transforms.cpp)
add_library(Shape src/shapes/Shape.cpp)
add_library(Rover src/shapes/Rover.cpp)
# Link dirs

# Link Libs
target_link_libraries(Shape ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})
target_link_libraries(Transforms Mat4 m)
target_link_libraries(Rover Shape Transforms)
target_link_libraries(Main ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} Utils Transforms m)
target_link_libraries(ShapeTest ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} Rover Utils Transforms m)
=======
add_library(utils src/utils.cpp)
add_library(transforms src/transforms.cpp)
add_library(mat4 src/mat4.cpp)
add_library(perlin src/perlin.cpp)
add_library(sphere src/sphere.cpp)
add_library(terrain src/terrain.cpp)

# Link Libs
target_link_libraries(main utils mat4 transforms terrain perlin sphere glew32 freeglut opengl32)

>>>>>>> master
# After build
# add_custom_command()
