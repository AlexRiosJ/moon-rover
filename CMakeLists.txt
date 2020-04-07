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
# Libraries
add_library(EGL SHARED IMPORTED)
add_library(Utils src/utils.cpp)
add_library(Mat4 src/mat4.cpp)
add_library(Transforms src/transforms.cpp)
# Link dirs

# Link Libs
target_link_libraries(Transforms Mat4 m)
target_link_libraries(Main ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} Utils Transforms m)

# After build
# add_custom_command()
