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
    std::vector<Vec3> rawVetexes;
    std::vector<Vec3> rawNormals;

    std::vector<Vec3> vertexArray;
    std::vector<Vec3> normalArray;
    std::vector<Vec3> colorArray;

    GLuint buffers[4];
    GLuint vertexArrayId;
    GLuint vertexPosLoc;
    GLuint vertexColorLoc;
    GLuint vertexNormalLoc;
    GLuint programId;

public:
    Shape();
    ~Shape();
    void Load(const char *);
    void Bind(GLuint, GLuint, GLuint, GLuint);
    void Draw();
};
