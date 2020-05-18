#ifndef TERRAIN_H_
#define TERRAIN_H_

#include "utils.h"

typedef struct strTerrain *Terrain;

Terrain terrain_create(int numVertexX, int numVertexZ, int sideLengthX, int sideLengthZ, Vertex terrainColor);
void terrain_bind(Terrain terrain, GLuint vertexPosLoc, GLuint vertexColLoc, GLuint vertexTexcoordLoc, GLuint vertexNormalLoc);
void terrain_draw(Terrain terrain);

Vertex vertexFromXZPosition(Terrain terrain, float x, float z);
Vertex normalFromXZPosition(Terrain terrain, float x, float z);

#endif /* TERRAIN_H_ */
