#include "terrain.h"
#include "perlin.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define RESET 0xFFFFFFFF

Vertex hueToRgb(float H);
static int clampInt(int value, int minValue, int maxValue);
static float wrapToTile(float value, float sideLength);
static void sampleGrid(Terrain terrain, float x, float z, int *x0, int *x1, int *z0, int *z1, float *tx, float *tz);
static Vertex safeNormalize(Vertex v);

struct strTerrain
{
	GLuint terrainVA;
	GLuint terrainBuffer[5];
	int numVertexX;
	int numVertexZ;
	int sideLengthX;
	int sideLengthZ;
	Vertex terrainColor;
	Vertex *vertices;
	Vertex *colors;
	Texcoord *texcoords;
	Vertex *normals;
	GLuint *indexBuffer;
};

Terrain terrain_create(int numVertexX, int numVertexZ, int sideLengthX, int sideLengthZ, Vertex terrainColor)
{
	int totalPositionVertices = numVertexX * numVertexZ;
	int totalColors = numVertexX * numVertexZ;
	int totalTexcoords = numVertexX * numVertexZ;
	int totalNormals = numVertexX * numVertexZ;
	int totalIndexes = (numVertexX - 1) * (numVertexZ * 2 + 1);
	Terrain terrain = (Terrain)malloc(sizeof(struct strTerrain));
	terrain->numVertexX = numVertexX;
	terrain->numVertexZ = numVertexZ;
	terrain->sideLengthX = sideLengthX;
	terrain->sideLengthZ = sideLengthZ;
	terrain->vertices = (Vertex *)malloc(totalPositionVertices * sizeof(Vertex));
	terrain->colors = (Vertex *)malloc(totalColors * sizeof(Vertex));
	terrain->texcoords = (Texcoord *)malloc(totalTexcoords * sizeof(Texcoord));
	terrain->normals = (Vertex *)malloc(totalNormals * sizeof(Vertex));
	terrain->indexBuffer = (GLuint *)malloc(totalIndexes * sizeof(GLuint));

	// printf("%d\n", (numVertexX - 1) * (numVertexZ * 2 + 1));
	float x = -sideLengthX / 2.0;
	float z = -sideLengthZ / 2.0;
	float dx = (float)sideLengthX / (float)(numVertexX - 1);
	float dz = (float)sideLengthZ / (float)(numVertexZ - 1);
	srand(time(NULL));
	// printf("%.2f, %.2f, %.2f, %.2f\n", x, z, dx, dz);
	for (int i = 0; i < numVertexZ; i++)
	{
		for (int j = 0; j < numVertexX; j++)
		{
			terrain->vertices[i * numVertexX + j].x = x;
			terrain->vertices[i * numVertexX + j].y = Perlin_Get2d(x, z, 0.1, 20) * 
													  ((-cos(j * M_PI * 2 / numVertexX) + 1) + 
													  ((-cos(j * M_PI * 2 / numVertexX * 5) + 1) / 5.0) + 
													  ((-cos(j * M_PI * 2 / numVertexX * 7) + 1) / 7.0)) * 
													  
													  ((-cos(i * M_PI * 2 / numVertexZ) + 1) + 
													  ((-cos(i * M_PI * 2 / numVertexZ * 5) + 1) / 5.0) + 
													  ((-cos(i * M_PI * 2 / numVertexZ * 7) + 1) / 7.0)) * 0.25;
			terrain->vertices[i * numVertexX + j].z = z;

			// Test height map with HUE value.
			// Vertex rgb = hueToRgb(terrain->vertices[i * numVertexX + j].y * 240);
			// terrain->colors[i * numVertexX + j].x = rgb.x / 255.0;
			// terrain->colors[i * numVertexX + j].y = rgb.y / 255.0;
			// terrain->colors[i * numVertexX + j].z = rgb.z / 255.0;

			terrain->colors[i * numVertexX + j].x = 1;
			terrain->colors[i * numVertexX + j].y = 1;
			terrain->colors[i * numVertexX + j].z = 1;

			terrain->texcoords[i * numVertexX + j].u = ((x / (float)sideLengthX) + 0.5) * 40;
			terrain->texcoords[i * numVertexX + j].v = ((z / (float)sideLengthZ) + 0.5) * 40;

			x += dx;
		}
		x = -sideLengthX / 2.0;
		z += dz;
	}

	// Generate index buffer
	for (int i = 0; i < numVertexX - 1; i++)
	{
		for (int j = 0; j < numVertexZ * 2 + 1; j++)
		{
			int index = i * (numVertexZ * 2 + 1) + j;
			if (j == numVertexZ * 2)
			{
				// printf("%d, %x\n", index, RESET);
				terrain->indexBuffer[index] = RESET;
			}
			else
			{
				int num = i * numVertexZ + (j / 2);
				terrain->indexBuffer[index] = j % 2 == 0 ? num : num + numVertexZ;
				// printf("%d, %d\n", index, j % 2 == 0 ? num : num + numVertexZ);
			}
		}
	}

	for (int i = 0; i < numVertexX * numVertexZ; i++)
	{
		terrain->normals[i] = {0, 0, 0};
	}

	int index = 0;
	for (int i = 0; i < (numVertexZ - 1); i++)
	{
		for (int j = 0; j < (numVertexX - 1) * 2; j++)
		{
			Vertex A = subtractVertex(terrain->vertices[terrain->indexBuffer[index + j]], terrain->vertices[terrain->indexBuffer[index + j + 1]]);
			Vertex B = subtractVertex(terrain->vertices[terrain->indexBuffer[index + j]], terrain->vertices[terrain->indexBuffer[index + j + 2]]);
			Vertex C = crossProduct(A, B);
			if (j % 2 != 0)
			{
				C.x *= -1;
				C.y *= -1;
				C.z *= -1;
			}

			terrain->normals[terrain->indexBuffer[index + j]].x += C.x;
			terrain->normals[terrain->indexBuffer[index + j]].y += C.y;
			terrain->normals[terrain->indexBuffer[index + j]].z += C.z;

			terrain->normals[terrain->indexBuffer[index + j + 1]].x += C.x;
			terrain->normals[terrain->indexBuffer[index + j + 1]].y += C.y;
			terrain->normals[terrain->indexBuffer[index + j + 1]].z += C.z;

			terrain->normals[terrain->indexBuffer[index + j + 2]].x += C.x;
			terrain->normals[terrain->indexBuffer[index + j + 2]].y += C.y;
			terrain->normals[terrain->indexBuffer[index + j + 2]].z += C.z;
		}
		index += (numVertexX * 2 + 1);
	}

	return terrain;
}

void terrain_bind(Terrain terrain, GLuint vertexPosLoc, GLuint vertexColLoc, GLuint vertexTexcoordLoc, GLuint vertexNormalLoc)
{
	int totalPositionVertices = terrain->numVertexX * terrain->numVertexZ;
	int totalColors = terrain->numVertexX * terrain->numVertexZ;
	int totalTexcoords = terrain->numVertexX * terrain->numVertexZ;
	int totalNormals = terrain->numVertexX * terrain->numVertexZ;
	int totalIndexes = (terrain->numVertexX - 1) * (terrain->numVertexZ * 2 + 1);

	glGenVertexArrays(1, &terrain->terrainVA);
	glBindVertexArray(terrain->terrainVA);
	glGenBuffers(5, terrain->terrainBuffer);

	glBindBuffer(GL_ARRAY_BUFFER, terrain->terrainBuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, totalPositionVertices * sizeof(Vertex), terrain->vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexPosLoc);
	glVertexAttribPointer(vertexPosLoc, 3, GL_FLOAT, 0, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, terrain->terrainBuffer[1]);
	glBufferData(GL_ARRAY_BUFFER, totalColors * sizeof(Vertex), terrain->colors, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexColLoc);
	glVertexAttribPointer(vertexColLoc, 3, GL_FLOAT, 0, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, terrain->terrainBuffer[2]);
	glBufferData(GL_ARRAY_BUFFER, totalTexcoords * sizeof(Texcoord), terrain->texcoords, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexTexcoordLoc);
	glVertexAttribPointer(vertexTexcoordLoc, 2, GL_FLOAT, 0, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, terrain->terrainBuffer[3]);
	glBufferData(GL_ARRAY_BUFFER, totalNormals * sizeof(Vertex), terrain->normals, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexNormalLoc);
	glVertexAttribPointer(vertexNormalLoc, 3, GL_FLOAT, 0, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrain->terrainBuffer[4]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalIndexes * sizeof(GLuint), terrain->indexBuffer, GL_STATIC_DRAW);
	glPrimitiveRestartIndex(RESET);
	glEnable(GL_PRIMITIVE_RESTART);
}

void terrain_draw(Terrain terrain)
{
	glBindVertexArray(terrain->terrainVA);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrain->terrainBuffer[4]);
	int totalIndexes = (terrain->numVertexX - 1) * (terrain->numVertexZ * 2 + 1);
	glDrawElements(GL_TRIANGLE_STRIP, totalIndexes, GL_UNSIGNED_INT, 0);
}

Vertex hueToRgb(float H)
{
	Vertex rgb = {255, 0, 0};

	if (H <= 60)
	{
		rgb.y = (H / 60.0) * 255;
	}
	else if (H <= 120)
	{
		rgb.x = 255 - (((H - 60.0) / 60.0) * 255);
		rgb.y = 255;
	}
	else if (H <= 180)
	{
		rgb.x = 0;
		rgb.y = 255;
		rgb.z = ((H - 120.0) / 60.0) * 255;
	}
	else if (H <= 240)
	{
		rgb.x = 0;
		rgb.y = 255 - (((H - 180.0) / 60.0) * 255);
		rgb.z = 255;
	}
	else if (H <= 300)
	{
		rgb.x = ((H - 240.0) / 60.0) * 255;
		rgb.y = 0;
		rgb.z = 255;
	}
	else if (H <= 360)
	{
		rgb.x = 255;
		rgb.y = 0;
		rgb.z = 255 - (((H - 300.0) / 60.0) * 255);
	}

	return rgb;
}

static int clampInt(int value, int minValue, int maxValue)
{
	if (value < minValue)
	{
		return minValue;
	}
	if (value > maxValue)
	{
		return maxValue;
	}
	return value;
}

static float wrapToTile(float value, float sideLength)
{
	float wrapped = fmodf(value, sideLength);
	if (wrapped < 0.0f)
	{
		wrapped += sideLength;
	}
	return wrapped;
}

static void sampleGrid(Terrain terrain, float x, float z, int *x0, int *x1, int *z0, int *z1, float *tx, float *tz)
{
	float dx = (float)terrain->sideLengthX / (float)(terrain->numVertexX - 1);
	float dz = (float)terrain->sideLengthZ / (float)(terrain->numVertexZ - 1);
	float localX = wrapToTile(x + terrain->sideLengthX / 2.0f, (float)terrain->sideLengthX);
	float localZ = wrapToTile(z + terrain->sideLengthZ / 2.0f, (float)terrain->sideLengthZ);

	float gridX = localX / dx;
	float gridZ = localZ / dz;

	int baseX = clampInt((int)floorf(gridX), 0, terrain->numVertexX - 1);
	int baseZ = clampInt((int)floorf(gridZ), 0, terrain->numVertexZ - 1);
	int nextX = clampInt(baseX + 1, 0, terrain->numVertexX - 1);
	int nextZ = clampInt(baseZ + 1, 0, terrain->numVertexZ - 1);

	*x0 = baseX;
	*x1 = nextX;
	*z0 = baseZ;
	*z1 = nextZ;
	*tx = fminf(fmaxf(gridX - (float)baseX, 0.0f), 1.0f);
	*tz = fminf(fmaxf(gridZ - (float)baseZ, 0.0f), 1.0f);
}

static Vertex safeNormalize(Vertex v)
{
	float magnitude = sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
	if (!isfinite(magnitude) || magnitude <= 0.000001f)
	{
		return {0.0f, 1.0f, 0.0f};
	}

	Vertex normalized = {v.x / magnitude, v.y / magnitude, v.z / magnitude};
	if (!isfinite(normalized.x) || !isfinite(normalized.y) || !isfinite(normalized.z))
	{
		return {0.0f, 1.0f, 0.0f};
	}

	return normalized;
}

Vertex vertexFromXZPosition(Terrain terrain, float x, float z)
{
	Vertex vertex = {x, 0, z};
	int x0, x1, z0, z1;
	float tx, tz;

	sampleGrid(terrain, x, z, &x0, &x1, &z0, &z1, &tx, &tz);

	int index00 = z0 * terrain->numVertexX + x0;
	int index10 = z0 * terrain->numVertexX + x1;
	int index01 = z1 * terrain->numVertexX + x0;
	int index11 = z1 * terrain->numVertexX + x1;

	float y0 = terrain->vertices[index00].y + (terrain->vertices[index10].y - terrain->vertices[index00].y) * tx;
	float y1 = terrain->vertices[index01].y + (terrain->vertices[index11].y - terrain->vertices[index01].y) * tx;
	vertex.y = y0 + (y1 - y0) * tz;

	return vertex;
}

Vertex normalFromXZPosition(Terrain terrain, float x, float z)
{
	int x0, x1, z0, z1;
	float tx, tz;

	sampleGrid(terrain, x, z, &x0, &x1, &z0, &z1, &tx, &tz);

	int index00 = z0 * terrain->numVertexX + x0;
	int index10 = z0 * terrain->numVertexX + x1;
	int index01 = z1 * terrain->numVertexX + x0;
	int index11 = z1 * terrain->numVertexX + x1;

	Vertex n0 = {
		terrain->normals[index00].x + (terrain->normals[index10].x - terrain->normals[index00].x) * tx,
		terrain->normals[index00].y + (terrain->normals[index10].y - terrain->normals[index00].y) * tx,
		terrain->normals[index00].z + (terrain->normals[index10].z - terrain->normals[index00].z) * tx
	};
	Vertex n1 = {
		terrain->normals[index01].x + (terrain->normals[index11].x - terrain->normals[index01].x) * tx,
		terrain->normals[index01].y + (terrain->normals[index11].y - terrain->normals[index01].y) * tx,
		terrain->normals[index01].z + (terrain->normals[index11].z - terrain->normals[index01].z) * tx
	};
	Vertex n = {
		n0.x + (n1.x - n0.x) * tz,
		n0.y + (n1.y - n0.y) * tz,
		n0.z + (n1.z - n0.z) * tz
	};

	return safeNormalize(n);
}