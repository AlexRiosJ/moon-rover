#include <vector>
#include <iostream>
#include <fstream>
#include <GL/glew.h>

class Shape
{
private:
    struct Vec3
    {
        double x;
        double y;
        double z;
    };

    float * vertexArray;
    float * normalArray;
    float * colorArray;
    int vertexCount;

    GLuint buffers[4];
    GLuint vertexArrayId;
    GLuint vertexPosLoc;
    GLuint vertexColorLoc;
    GLuint vertexNormalLoc;
    GLuint programId;

    float * VectorToBuffer(std::vector<Vec3>);

public:
    Shape();
    ~Shape();
    void Load(const char *);
    void Bind(GLuint, GLuint, GLuint, GLuint);
    void Draw();
    void PrintVertex();
};
