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
#define toRadians(deg) deg *M_PI / 180.0

#define RESET 0xFFFFFFFF
#define NUM_VERTEX_X 512
#define NUM_VERTEX_Z 512
#define SIDE_LENGTH_X 40
#define SIDE_LENGTH_Z 40

unsigned char keys[256];
static float movex = 0, movey = 0;

static GLuint programId, va[1], bufferId[2], vertexPosLoc, vertexColLoc, modelMatrixLoc, viewMatrixLoc, projMatrixLoc;
static Mat4 projMatrix;
static GLboolean usePerspective = GL_TRUE;
static float angleY = 0, angleZ = 0;

Vertex cameraPosition = {0, 5, 0};
float cameraSpeed = 0.05;

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

static void move()
{
	cameraSpeed = keys[112] ? 0.1 : 0.05;

	float nextForwardXPosition = cameraSpeed * -sin(toRadians(movex * 0.08));
	float nextForwardYPosition = cameraSpeed * sin(toRadians(movey * 0.08));
	float nextForwardZPosition = cameraSpeed * cos(toRadians(movex * 0.08));

	float nextSideXPosition = cameraSpeed * -cos(toRadians(movex * 0.08));
	float nextSideZPosition = cameraSpeed * -sin(toRadians(movex * 0.08));

	if (keys['w'])
	{
		cameraPosition.x -= nextForwardXPosition;
		cameraPosition.y -= nextForwardYPosition;
		cameraPosition.z -= nextForwardZPosition;
	}
	if (keys['s'])
	{
		cameraPosition.x += nextForwardXPosition;
		cameraPosition.y += nextForwardYPosition;
		cameraPosition.z += nextForwardZPosition;
	}
	if (keys['a'])
	{
		cameraPosition.x += nextSideXPosition;
		cameraPosition.z += nextSideZPosition;
	}
	if (keys['d'])
	{
		cameraPosition.x -= nextSideXPosition;
		cameraPosition.z -= nextSideZPosition;
	}
}

static void display()
{
	Mat4 modelMat;
	Mat4 viewMat;
	mIdentity(&modelMat);
	mIdentity(&viewMat);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(programId);

	move();

	rotateX(&viewMat, movey * 0.08);
	rotateY(&viewMat, movex * 0.08);
	translate(&viewMat, -cameraPosition.x, -cameraPosition.y, -cameraPosition.z);

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

static void specialKeyPressed(int key, int x, int y)
{
	keys[key] = 1;
}

static void specialKeyReleased(int key, int x, int y)
{
	keys[key] = 0;
}

static void keyPressed(unsigned char key, int x, int y)
{
	if (key == 27)
		exit(0);
	else
		keys[key] = 1;
}

static void keyReleased(unsigned char key, int x, int y)
{
	keys[key] = 0;
}

void rotateCamera(int x, int y)
{
	movex += (float)(x - glutGet(GLUT_WINDOW_WIDTH) / 2);
	movey += (float)(y - glutGet(GLUT_WINDOW_HEIGHT) / 2);
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
}

static void mouseMove(int x, int y)
{
	rotateCamera(x, y);
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
	glutFullScreen();
	glutSetCursor(GLUT_CURSOR_CROSSHAIR);
	glutPassiveMotionFunc(mouseMove);
	glutMotionFunc(mouseMove);
	glutDisplayFunc(display);
	glutSpecialFunc(specialKeyPressed);
	glutSpecialUpFunc(specialKeyReleased);
	glutKeyboardFunc(keyPressed);
	glutKeyboardUpFunc(keyReleased);
	glutReshapeFunc(reshapeFunc);
	glewInit();
	initShaders();
	generateTerrain();
	glClearColor(0.05, 0.05, 0.10, 1.0);
	glutMainLoop();
	return 0;
}
