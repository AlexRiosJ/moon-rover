cmake_minimum_required(VERSION 3.0)
project( moon_rover )

# Includes
include_directories(include)

# Executables
add_executable(main src/main.c)

# Libraries
add_library(utils src/utils.c)
add_library(transforms src/transforms.c)
add_library(mat4 src/mat4.c)

# Link Libs
target_link_libraries(main utils mat4 transforms glew32 freeglut opengl32)

# After build
file(GLOB LIBRARIES "lib/*.dll")
file(COPY ${LIBRARIES} DESTINATION .)
