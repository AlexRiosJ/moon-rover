
#ifndef MAT4_H_
#define MAT4_H_

typedef union {
	float at[4][4];
	float values[16];
} Mat4;

void mIdentity(Mat4 *);

void mMult(Mat4 *, Mat4);

void mPrint(Mat4);

#endif /* MAT4_H_ */
