#ifndef TRANSFORMS_H_
#define TRANSFORMS_H_

#include "mat4.h"

void translate(Mat4 *, float, float, float);
void rotateX(Mat4 *, float);
void rotateY(Mat4 *, float);
void rotateZ(Mat4 *, float);
void scale(Mat4 *, float, float, float);
void loadIdentity(Mat4 *);
void pushMatrix(Mat4 *);
void popMatrix(Mat4 *);
void setOrtho(Mat4 *, float, float, float, float, float, float);
void setPerspective(Mat4 *m, float fovy, float aspect, float nearZ, float farZ);

#endif /* TRANSFORMS_H_ */
