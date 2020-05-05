#ifndef SPHERE_H_
#define SPHERE_H_

#include "utils.h"

typedef struct strSphere *Sphere;

Sphere sphere_create(float radius, int parallels, int meridians, Vertex sphereColor);
void sphere_bind(Sphere sphere, GLuint vertexPosLoc, GLuint vertexColLoc, GLuint vertexNormalLoc);
void sphere_draw(Sphere sphere);

#endif /* SPHERE_H_ */
