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

// Returns a heap-allocated copy of the shader source, or NULL if the file cannot be read.
// The caller owns the returned buffer.
const char *loadShader(const char *filename);
// Returns 0 when the shader source could not be loaded.
GLuint compileShader(const char *filename, GLuint shaderType);
bool shaderCompiled(GLuint shaderId);
// On success *pdata owns a heap-allocated pixel buffer; on failure *pdata is NULL.
bool loadBMP(const char *filename, unsigned char **pdata, unsigned int *width, unsigned int *height);
void processArrayBuffer(GLuint bufferId, void *array, int arraySize, GLuint loc, int size, int type);
void processIndexBuffer(GLuint bufferId, void *array, int arraySize, int restartIndex);

Vertex subtractVertex(Vertex origin, Vertex dest);
Vertex crossProduct(Vertex A, Vertex B);
Vertex normalize(Vertex v);

#endif /* UTILS_H_ */
