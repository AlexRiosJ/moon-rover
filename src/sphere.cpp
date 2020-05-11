#include "sphere.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define RESET 0xFFFFFFFF

struct strSphere
{
	GLuint sphereVA;
	GLuint sphereBuffer[5];
	float radius;
	int parallels;
	int meridians;
	Vertex sphereColor;
	Vertex *vertices;
	Vertex *colors;
	Texcoord *texcoords;
	Vertex *normals;
	GLuint *indexBuffer;
};

Sphere sphere_create(float radius, int parallels, int meridians, Vertex sphereColor)
{
	srand(time(NULL));
	int totalPositionVertices = parallels * (meridians + 1) * 2;
	int totalColors = parallels * (meridians + 1) * 2;
	int totalTexcoords = parallels * (meridians + 1) * 2;
	int totalNormals = parallels * (meridians + 1) * 2;
	int totalIndexes = parallels * ((meridians + 1) * 2 + 1);
	Sphere sphere = (Sphere)malloc(sizeof(struct strSphere));
	sphere->radius = radius;
	sphere->parallels = parallels;
	sphere->meridians = meridians;
	sphere->sphereColor = sphereColor;
	sphere->vertices = (Vertex *)malloc(totalPositionVertices * sizeof(Vertex));
	sphere->colors = (Vertex *)malloc(totalColors * sizeof(Vertex));
	sphere->texcoords = (Texcoord *)malloc(totalTexcoords * sizeof(Texcoord));
	sphere->normals = (Vertex *)malloc(totalNormals * sizeof(Vertex));
	sphere->indexBuffer = (GLuint *)malloc(totalIndexes * sizeof(GLuint));
	float phi = 0;
	float theta = 0;
	float dPhi = M_PI / parallels;
	float dTheta = 2 * M_PI / meridians;
	int counter = 0;
	int indexCounter = 0;

	for (int i = 0; i < parallels; i++)
	{
		for (int j = 0; j < (meridians + 1) * 2; j += 2)
		{
			float x = sin(phi) * cos(theta) * radius;
			float y = sin(phi) * sin(theta) * radius;
			float z = cos(phi) * radius;

			sphere->normals[counter].x = sphere->vertices[counter].x = x;
			sphere->normals[counter].y = sphere->vertices[counter].y = y;
			sphere->normals[counter].z = sphere->vertices[counter].z = z;
			// printf("%d: %.2f, %.2f, %.2f\t", counter, sphere->vertices[counter].x, sphere->vertices[counter].y, sphere->vertices[counter].z);

			sphere->colors[counter].x = sphereColor.x;
			sphere->colors[counter].y = sphereColor.y;
			sphere->colors[counter].z = sphereColor.z;

			sphere->texcoords[counter].u = theta / (2 * M_PI);
			sphere->texcoords[counter].v = phi / M_PI;

			sphere->indexBuffer[indexCounter] = counter;

			// printf("%d: %.2f, %.2f, %.2f\t%d\n", counter, sphere->colors[counter].x, sphere->colors[counter].y, sphere->colors[counter].z, sphere->indexBuffer[indexCounter]);

			counter++;
			indexCounter++;

			x = sin(phi + dPhi) * cos(theta) * radius;
			y = sin(phi + dPhi) * sin(theta) * radius;
			z = cos(phi + dPhi) * radius;

			sphere->normals[counter].x = sphere->vertices[counter].x = x;
			sphere->normals[counter].y = sphere->vertices[counter].y = y;
			sphere->normals[counter].z = sphere->vertices[counter].z = z;
			// printf("%d: %.2f, %.2f, %.2f\t", counter, sphere->vertices[counter].x, sphere->vertices[counter].y, sphere->vertices[counter].z);

			sphere->colors[counter].x = sphereColor.x;
			sphere->colors[counter].y = sphereColor.y;
			sphere->colors[counter].z = sphereColor.z;

			sphere->texcoords[counter].u = theta / (2.0 * M_PI);
			sphere->texcoords[counter].v = (phi + dPhi) / M_PI;

			sphere->indexBuffer[indexCounter] = counter;

			// printf("%d: %.2f, %.2f, %.2f\t%d\n", counter, sphere->colors[counter].x, sphere->colors[counter].y, sphere->colors[counter].z, sphere->indexBuffer[indexCounter]);

			counter++;
			indexCounter++;

			theta += dTheta;
		}

		theta = 0;
		phi += dPhi;
		sphere->indexBuffer[indexCounter] = RESET;

		// printf("%X\n", sphere->indexBuffer[indexCounter]);
		indexCounter++;
	}

	return sphere;
}

void sphere_bind(Sphere sphere, GLuint vertexPosLoc, GLuint vertexColLoc, GLuint vertexTexcoordLoc, GLuint vertexNormalLoc)
{
	int totalPositionVertices = sphere->parallels * (sphere->meridians + 1) * 2;
	int totalColors = sphere->parallels * (sphere->meridians + 1) * 2;
	int totalTexcoords = sphere->parallels * (sphere->meridians + 1) * 2;
	int totalNormals = sphere->parallels * (sphere->meridians + 1) * 2;
	int totalIndexes = sphere->parallels * ((sphere->meridians + 1) * 2 + 1);

	glGenVertexArrays(1, &sphere->sphereVA);
	glBindVertexArray(sphere->sphereVA);
	glGenBuffers(5, sphere->sphereBuffer);

	glBindBuffer(GL_ARRAY_BUFFER, sphere->sphereBuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, totalPositionVertices * sizeof(Vertex), sphere->vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexPosLoc);
	glVertexAttribPointer(vertexPosLoc, 3, GL_FLOAT, 0, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, sphere->sphereBuffer[1]);
	glBufferData(GL_ARRAY_BUFFER, totalColors * sizeof(Vertex), sphere->colors, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexColLoc);
	glVertexAttribPointer(vertexColLoc, 3, GL_FLOAT, 0, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, sphere->sphereBuffer[2]);
	glBufferData(GL_ARRAY_BUFFER, totalTexcoords * sizeof(Texcoord), sphere->texcoords, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexTexcoordLoc);
	glVertexAttribPointer(vertexTexcoordLoc, 2, GL_FLOAT, 0, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, sphere->sphereBuffer[3]);
	glBufferData(GL_ARRAY_BUFFER, totalNormals * sizeof(Vertex), sphere->normals, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexNormalLoc);
	glVertexAttribPointer(vertexNormalLoc, 3, GL_FLOAT, 0, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, sphere->sphereBuffer[4]);
	glBufferData(GL_ARRAY_BUFFER, totalIndexes * sizeof(GLuint), sphere->indexBuffer, GL_STATIC_DRAW);
	glPrimitiveRestartIndex(RESET);
	glEnable(GL_PRIMITIVE_RESTART);
}

void sphere_draw(Sphere sphere)
{
	glBindVertexArray(sphere->sphereVA);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere->sphereBuffer[4]);
	int totalIndexes = sphere->parallels * ((sphere->meridians + 1) * 2 + 1);
	glDrawElements(GL_TRIANGLE_STRIP, totalIndexes * sizeof(GLuint), GL_UNSIGNED_INT, 0);
}