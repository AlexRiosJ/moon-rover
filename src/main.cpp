#include <GL/glew.h>
#include <GL/freeglut.h>
#include "utils.h"
#include "transforms.h"
#include "perlin.h"
#include <stdio.h>
#include <math.h>
#include <time.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define RESET 0xFFFFFFFF
#define NUM_VERTEX_X 512
#define NUM_VERTEX_Z 512
#define SIDE_LENGTH_X 40
#define SIDE_LENGTH_Z 40

static GLuint programId, va[1], bufferId[2], vertexPosLoc, vertexColLoc, modelMatrixLoc, viewMatrixLoc, projMatrixLoc;
static Mat4 projMatrix;
static GLboolean usePerspective = GL_TRUE;
static float angleY = 0, angleZ = 0;
typedef enum
{
	NONE,
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
} Motion;
Motion motion = NONE;
static float cameraX = 0;
static float cameraZ = 0;
static float cameraAngle = 0;
static float cameraSpeed = 0.05;
static float rotationSpeed = 2;

static void initShaders()
{
	GLuint vShader = compileShader("shaders/projection.vsh", GL_VERTEX_SHADER);
	if (!shaderCompiled(vShader))
		return;
	GLuint fShader = compileShader("shaders/color.fsh", GL_FRAGMENT_SHADER);
	if (!shaderCompiled(fShader))
		return;

	programId = glCreateProgram();
	glAttachShader(programId, vShader);
	glAttachShader(programId, fShader);
	glLinkProgram(programId);
	vertexPosLoc = glGetAttribLocation(programId, "vertexPosition");
	vertexColLoc = glGetAttribLocation(programId, "vertexColor");
	modelMatrixLoc = glGetUniformLocation(programId, "modelMatrix");
	viewMatrixLoc = glGetUniformLocation(programId, "viewMatrix");
	projMatrixLoc = glGetUniformLocation(programId, "projMatrix");

	glUseProgram(programId);
	glEnable(GL_DEPTH_TEST);
	//	glEnable(GL_CULL_FACE);
	//	glFrontFace(GL_CW);
}

static void generateTerrain()
{
	Vertex *vertexes = new Vertex[NUM_VERTEX_X * NUM_VERTEX_Z];
	GLuint *indexBuffer = new GLuint[(NUM_VERTEX_X - 1) * (NUM_VERTEX_Z * 2 + 1)];
	// printf("%d\n", (NUM_VERTEX_X - 1) * (NUM_VERTEX_Z * 2 + 1));
	float x = -SIDE_LENGTH_X / 2.0;
	float z = -SIDE_LENGTH_Z / 2.0;
	float dx = (float)SIDE_LENGTH_X / (float)(NUM_VERTEX_X - 1);
	float dz = (float)SIDE_LENGTH_Z / (float)(NUM_VERTEX_Z - 1);
	srand(time(NULL));
	// printf("%.2f, %.2f, %.2f, %.2f\n", x, z, dx, dz);
	for (int i = 0; i < NUM_VERTEX_Z; i++)
	{
		for (int j = 0; j < NUM_VERTEX_X; j++)
		{
			vertexes[i * NUM_VERTEX_X + j].x = x;
			vertexes[i * NUM_VERTEX_X + j].y = Perlin_Get2d(x, z, 0.25, 15);
			vertexes[i * NUM_VERTEX_X + j].z = z;
			x += dx;
		}
		x = -SIDE_LENGTH_X / 2.0;
		z += dz;
	}

	// Generate index buffer
	for (int i = 0; i < NUM_VERTEX_X - 1; i++)
	{
		for (int j = 0; j < NUM_VERTEX_Z * 2 + 1; j++)
		{
			int index = i * (NUM_VERTEX_Z * 2 + 1) + j;
			if (j == NUM_VERTEX_Z * 2)
			{
				// printf("%d, %x\n", index, RESET);
				indexBuffer[index] = RESET;
			}
			else
			{
				int num = i * NUM_VERTEX_Z + (j / 2);
				indexBuffer[index] = j % 2 == 0 ? num : num + NUM_VERTEX_Z;
				// printf("%d, %d\n", index, j % 2 == 0 ? num : num + NUM_VERTEX_Z);
			}
		}
	}

	glGenVertexArrays(1, va);
	glBindVertexArray(va[0]);
	glGenBuffers(2, bufferId);

	glBindBuffer(GL_ARRAY_BUFFER, bufferId[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * NUM_VERTEX_X * NUM_VERTEX_Z, vertexes, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexPosLoc);
	glVertexAttribPointer(vertexPosLoc, 3, GL_FLOAT, 0, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, bufferId[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * (NUM_VERTEX_X - 1) * (NUM_VERTEX_Z * 2 + 1), indexBuffer, GL_STATIC_DRAW);
	glPrimitiveRestartIndex(RESET);
	glEnable(GL_PRIMITIVE_RESTART);
}

static void moveForward()
{
	float radians = M_PI * cameraAngle / 180;
	cameraX -= cameraSpeed * sin(radians);
	cameraZ -= cameraSpeed * cos(radians);
}

static void moveBackward()
{
	float radians = M_PI * cameraAngle / 180;
	cameraX += cameraSpeed * sin(radians);
	cameraZ += cameraSpeed * cos(radians);
}

static void rotateLeft()
{
	cameraAngle += rotationSpeed;
}

static void rotateRight()
{
	cameraAngle -= rotationSpeed;
}

static void display()
{
	Mat4 modelMat;
	Mat4 viewMat;
	mIdentity(&modelMat);
	mIdentity(&viewMat);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(programId);

	switch (motion)
	{
	case FORWARD:
		moveForward();
		break;
	case BACKWARD:
		moveBackward();
		break;
	case LEFT:
		rotateLeft();
		break;
	case RIGHT:
		rotateRight();
	}
	rotateY(&viewMat, -cameraAngle);
	translate(&viewMat, -cameraX, -1, -cameraZ);

	glUniformMatrix4fv(viewMatrixLoc, 1, GL_TRUE, viewMat.values);
	glUniformMatrix4fv(modelMatrixLoc, 1, GL_TRUE, modelMat.values);

	glBindVertexArray(va[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId[1]);
	glDrawElements(GL_TRIANGLE_STRIP, (NUM_VERTEX_X - 1) * (NUM_VERTEX_Z * 2 + 1), GL_UNSIGNED_INT, 0);
	glutSwapBuffers();
}

static void timerFunc(int id)
{
	glutTimerFunc(20, timerFunc, id);
	glutPostRedisplay();
}

static void reshapeFunc(int w, int h)
{
	glViewport(0, 0, w, h);
	float aspect = (float)w / h;
	if (usePerspective)
	{
		setPerspective(&projMatrix, 80, aspect, -0.1, -2000);
	}
	else
	{
		if (aspect >= 1.0)
			setOrtho(&projMatrix, -6 * aspect, 6 * aspect, -6, 6, -6, 6);
		else
			setOrtho(&projMatrix, -6, 6, -6 / aspect, 6 / aspect, -6, 6);
	}
	glUniformMatrix4fv(projMatrixLoc, 1, GL_TRUE, projMatrix.values);
}

static void exitFunc(unsigned char key, int x, int y)
{
	if (key == 27)
	{
		glDeleteVertexArrays(1, va);
		exit(0);
	}
	if (key == 13)
	{
		usePerspective = !usePerspective;
		int w = glutGet(GLUT_WINDOW_WIDTH);
		int h = glutGet(GLUT_WINDOW_HEIGHT);
		reshapeFunc(w, h);
	}
}

static void specialKeyPressed(int code, int x, int y)
{
	switch (code)
	{
	case 101:
		motion = FORWARD;
		break;
	case 103:
		motion = BACKWARD;
		break;
	case 100:
		motion = LEFT;
		break;
	case 102:
		motion = RIGHT;
	}
}

static void specialKeyReleased(int code, int x, int y)
{
	motion = NONE;
}

int main(int argc, char **argv)
{
	setbuf(stdout, NULL);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE);
	glutInitWindowSize(1280, 720);
	glutInitWindowPosition(100, 100);
	glutTimerFunc(50, timerFunc, 1);

	glutCreateWindow("Moon Rover");
	glutDisplayFunc(display);
	glutKeyboardFunc(exitFunc);
	glutSpecialFunc(specialKeyPressed);
	glutSpecialUpFunc(specialKeyReleased);
	glutReshapeFunc(reshapeFunc);
	glewInit();
	initShaders();
	generateTerrain();
	glClearColor(0.05, 0.05, 0.10, 1.0);
	glutMainLoop();
	return 0;
}
