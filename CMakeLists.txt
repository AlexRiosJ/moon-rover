cmake_minimum_required(VERSION 3.0)
project( moon_rover )

# Includes
include_directories(include)

# Executables
add_executable(main src/main.cpp)

# Libraries
add_library(utils src/utils.cpp)
add_library(transforms src/transforms.cpp)
add_library(mat4 src/mat4.cpp)

# Link Libs
target_link_libraries(main utils mat4 transforms glew32 freeglut opengl32)

# After build
file(GLOB LIBRARIES "lib/*.dll")
file(COPY ${LIBRARIES} DESTINATION .)
