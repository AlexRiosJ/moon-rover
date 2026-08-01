#ifndef UTILS_H_
#define UTILS_H_

#include <GL/glew.h>

typedef struct Vertex
{
    float x;
    float y;
    float z;
} Vertex;

typedef struct Texcoord
{
    float u;
    float v;
} Texcoord;

typedef struct Triangle
{
    Vertex pointA;
    Vertex pointB;
    Vertex pointC;
} Triangle;

// A shader program together with the locations every program of the scene shares.
// cameraLoc is optional and stays -1 for programs that do not shade with the camera position.
typedef struct ShaderProgram
{
    GLuint id;
    GLint vertexPosLoc;
    GLint vertexColLoc;
    GLint vertexTexcoordLoc;
    GLint vertexNormalLoc;
    GLint modelMatrixLoc;
    GLint viewMatrixLoc;
    GLint projMatrixLoc;
    GLint cameraLoc;
} ShaderProgram;

// Returns a heap-allocated copy of the shader source, or NULL if the file cannot be read.
// The caller owns the returned buffer.
const char *loadShader(const char *filename);
// Returns 0 when the shader source could not be loaded.
GLuint compileShader(const char *filename, GLuint shaderType);
bool shaderCompiled(GLuint shaderId);
// Compiles, links and queries the common locations of a program, leaving it in use on success.
// Returns false and reports the reason on stderr when any step fails.
bool createShaderProgram(ShaderProgram *program, const char *vertexShaderFile, const char *fragmentShaderFile);
// Returns the uniform location, reporting on stderr when the uniform is not active.
GLint requiredUniformLocation(GLuint programId, const char *name);
// On success *pdata owns a heap-allocated pixel buffer; on failure *pdata is NULL.
bool loadBMP(const char *filename, unsigned char **pdata, unsigned int *width, unsigned int *height);
void processArrayBuffer(GLuint bufferId, void *array, int arraySize, GLuint loc, int size, int type);
void processIndexBuffer(GLuint bufferId, void *array, int arraySize, int restartIndex);

Vertex subtractVertex(Vertex origin, Vertex dest);
Vertex crossProduct(Vertex A, Vertex B);
Vertex normalize(Vertex v);

#endif /* UTILS_H_ */
