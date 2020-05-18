#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <shapes/Shape.hpp>
#include <GL/glew.h>
#include <stdlib.h>

#define BIND_BUFFER_SIZE 3
#define BIND_BUFFER_VERTEX_ARRAY 0
#define BIND_BUFFER_COLOR_ARRAY 1
#define BIND_BUFFER_NORMAL_ARRAY 2

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

float * Shape::VectorToBuffer(std::vector<Vec3> vector) 
{
    float * buffer = new float[vector.size() * 3]();
    for (int i = 0; i < vector.size(); i++)
    {
        Vec3 vertex = vector[i];
        int offset = i * 3;
        buffer[offset + 0] = vertex.x;
        buffer[offset + 1] = vertex.y;
        buffer[offset + 2] = vertex.z;
    }

    return buffer;
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

    std::vector<Vec3> rawVetexes;
    std::vector<Vec3> rawNormals;

    std::vector<Vec3> tmpVertexArray;
    std::vector<Vec3> tmpNormalArray;
    std::vector<Vec3> tmpColorArray;

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
            rawVetexes.push_back(vertex);
        }
        // If line header is a normal
        else if (s.compare("vn") == 0)
        {
            Vec3 normal;
            file >> normal.x >> normal.y >> normal.z;
            rawNormals.push_back(normal);
        }
        // If line header is a face
        else if (s.compare("f") == 0)
        {
            std::string faceVertex;
            for(int i = 0; i < 3; i++)
            {
                file >> faceVertex;
                std::vector<std::string> tokens = split(faceVertex, '/');
                
                unsigned int v = atoi(tokens[0].c_str()) - 1;
                unsigned int vn = atoi(tokens[2].c_str()) - 1;
                Vec3 vertex = rawVetexes[v];
                Vec3 normal = rawNormals[vn];
                Vec3 color;
                color.x = 0.8980;
                color.y = 0.3098;
                color.z = 0.2705;
                
                tmpVertexArray.push_back(vertex);
                tmpNormalArray.push_back(normal);
                tmpColorArray.push_back(color);
            }
            
        }
    }

    this->vertexArray = this->VectorToBuffer(tmpVertexArray);
    this->colorArray = this->VectorToBuffer(tmpColorArray);
    this->normalArray = this->VectorToBuffer(tmpNormalArray);

    this->vertexCount = tmpVertexArray.size();
}

void Shape::Bind(GLuint programId, GLuint vertexLocation, GLuint normalLocation, GLuint colorLocation)
{
    this->programId = programId;
    this->vertexPosLoc = vertexLocation;
    this->vertexColorLoc = colorLocation;
    this->vertexNormalLoc = normalLocation;
    
    glGenVertexArrays(1, &this->vertexArrayId);
    glBindVertexArray(this->vertexArrayId);
    glGenBuffers(BIND_BUFFER_SIZE, this->buffers);

    int arraySize = this->vertexCount * sizeof(float) * 3;

    glBindBuffer(GL_ARRAY_BUFFER, this->buffers[BIND_BUFFER_VERTEX_ARRAY]);
    glBufferData(GL_ARRAY_BUFFER, arraySize, this->vertexArray, GL_STATIC_DRAW);
    glEnableVertexAttribArray(this->vertexPosLoc);
    glVertexAttribPointer(this->vertexPosLoc, 3, GL_FLOAT, 0, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, this->buffers[BIND_BUFFER_COLOR_ARRAY]);
    glBufferData(GL_ARRAY_BUFFER, arraySize, this->colorArray, GL_STATIC_DRAW);
    glEnableVertexAttribArray(this->vertexColorLoc);
    glVertexAttribPointer(this->vertexColorLoc, 3, GL_FLOAT, 0, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, this->buffers[BIND_BUFFER_NORMAL_ARRAY]);
    glBufferData(GL_ARRAY_BUFFER, arraySize, this->normalArray, GL_STATIC_DRAW);
    glEnableVertexAttribArray(this->vertexNormalLoc);
    glVertexAttribPointer(this->vertexNormalLoc, 3, GL_FLOAT, 0, 0, 0);
}

void Shape::Draw()
{
    int triangleCount = this->vertexCount;
    glBindVertexArray(this->vertexArrayId);
    glDrawArrays(GL_TRIANGLES, 0, triangleCount);
}

void Shape::PrintVertex()
{
    for(int i = 0; i < this->vertexCount; i++)
    {
        int offset = i * 3;
        std::cout << i + 1 << " <" << this->vertexArray[offset + 0] << "," << this->vertexArray[offset + 1] << "," << this->vertexArray[offset + 2] << ">\n";
    }
}
