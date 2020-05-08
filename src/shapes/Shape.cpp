#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <shapes/Shape.hpp>
#include <GL/glew.h>

#define BIND_BUFFER_SIZE 4
#define BIND_BUFFER_VERTEX_ARRAY 0
#define BIND_BUFFER_COLOR_ARRAY 1
#define BIND_BUFFER_NORMAL_ARRAY 2
#define BIND_BUFFER_INDEX_ARRAY 3

std::vector<std::string> split(const std::string& s, char delimiter)
{
   std::vector<std::string> tokens;
   std::string token;
   std::istringstream tokenStream(s);
   while (std::getline(tokenStream, token, delimiter))
   {
      tokens.push_back(token);
   }
   return tokens;
}

Shape::Shape()
{
}

Shape::~Shape()
{
}

void Shape::Load(const char *path)
{
    std::string s;
    std::ifstream file(path);

    if (!file)
    {
        std::cout << "Impossible to open the file!\n";
        return;
    }

    while (file >> s)
    {
        // If line header is a vertex
        if (s.compare("v") == 0)
        {
            Vec3 vertex;
            file >> vertex.x >> vertex.y >> vertex.z;
            this->rawVetexes.push_back(vertex);
        }
        // If line header is a normal
        else if (s.compare("vn") == 0)
        {
            Vec3 normal;
            file >> normal.x >> normal.y >> normal.z;
            this->rawNormals.push_back(normal);
        }
        // If line header is a face
        else if (s.compare("f") == 0)
        {
            std::string faceVertex;
            for(int i = 0; i < 3; i++)
            {
                file >> faceVertex;
                std::vector<std::string> tokens = split(faceVertex, '/');
                unsigned int v = stoi(tokens[0]) - 1;
                unsigned int vn = stoi(tokens[2]) - 1;
                Vec3 vertex = this->rawVetexes[v];
                Vec3 normal = this->rawNormals[vn];
                Vec3 color;
                color.x = 1.0;
                color.y = 1.0;
                color.z = 1.0;

                this->vertexArray.push_back(vertex);
                this->normalArray.push_back(normal);
                this->colorArray.push_back(color);
            }
        }
    }
}

void Shape::Bind(GLuint programId, GLuint vertexArrayLoc, GLuint normalArrayLoc, GLuint colorArrayLoc)
{
    this->programId = programId;
    glGenVertexArrays(1, &this->vertexArrayId);
    glBindVertexArray(this->vertexArrayId);
    glGenBuffers(BIND_BUFFER_SIZE, this->buffers);

    this->vertexPosLoc = vertexArrayLoc;
    this->vertexColorLoc = colorArrayLoc;
    this->vertexNormalLoc = normalArrayLoc;

    int arraySize = this->vertexArray.size() * 3 * sizeof(float);

    glBindBuffer(GL_ARRAY_BUFFER, this->buffers[BIND_BUFFER_VERTEX_ARRAY]);
    glBufferData(GL_ARRAY_BUFFER, arraySize, this->vertexArray.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(this->vertexPosLoc);
    glVertexAttribPointer(this->vertexPosLoc, 3, GL_FLOAT, 0, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, this->buffers[BIND_BUFFER_COLOR_ARRAY]);
    glBufferData(GL_ARRAY_BUFFER, arraySize, this->colorArray.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(this->vertexColorLoc);
    glVertexAttribPointer(this->vertexColorLoc, 3, GL_FLOAT, 0, 0, 0);
}

void Draw()
{
    int count = this->vertexArray.size();
    glDrawArrays(GL_TRIANGLES, 0, count);
}
