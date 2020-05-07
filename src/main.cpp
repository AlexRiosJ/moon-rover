#include <GL/glew.h>
#include <GL/freeglut.h>
#include "utils.h"
#include "transforms.h"
#include "perlin.h"
#include "sphere.h"
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
#define SIDE_LENGTH_X 20
#define SIDE_LENGTH_Z 20

Sphere earth;

unsigned char keys[256];

static GLuint programId, va[1], bufferId[4], vertexPosLoc, vertexColLoc, vertexNormalLoc, modelMatrixLoc, viewMatrixLoc, projMatrixLoc;
static Mat4 modelMatrix, viewMatrix, projectionMatrix;

static float movex = 0, movey = 0;
Vertex cameraPosition = {0, 1.5, 0};
float cameraSpeed = 0.05;

static GLuint ambientLightLoc, diffuseLightLoc, lightPositionLoc, materialALoc, materialDLoc, materialSLoc, exponentLoc, cameraLoc;

static float ambientLight[] = {0, 0, 0};
static float materialA[] = {0.5, 0.5, 0.5};
static float diffuseLight[] = {1.0, 1.0, 1.0};
static float lightPosition[] = {0, 60, 0};
static float materialD[] = {0.7, 0.7, 0.7};
static float materialS[] = {0.7, 0.7, 0.7};
static float exponent = 32;

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
	glUseProgram(programId);

	vertexPosLoc = glGetAttribLocation(programId, "vertexPosition");
	vertexColLoc = glGetAttribLocation(programId, "vertexColor");
	vertexNormalLoc = glGetAttribLocation(programId, "vertexNormal");
	modelMatrixLoc = glGetUniformLocation(programId, "modelMatrix");
	viewMatrixLoc = glGetUniformLocation(programId, "viewMatrix");
	projMatrixLoc = glGetUniformLocation(programId, "projectionMatrix");

	ambientLightLoc = glGetUniformLocation(programId, "ambientLight");
	diffuseLightLoc = glGetUniformLocation(programId, "diffuseLight");
	lightPositionLoc = glGetUniformLocation(programId, "lightPosition");
	materialALoc = glGetUniformLocation(programId, "materialA");
	materialDLoc = glGetUniformLocation(programId, "materialD");
	materialSLoc = glGetUniformLocation(programId, "materialS");
	exponentLoc = glGetUniformLocation(programId, "exponent");
	cameraLoc = glGetUniformLocation(programId, "camera");

	glUniform3fv(ambientLightLoc, 1, ambientLight);
	glUniform3fv(diffuseLightLoc, 1, diffuseLight);
	glUniform3fv(lightPositionLoc, 1, lightPosition);
	glUniform3fv(materialALoc, 1, materialA);
	glUniform3fv(materialDLoc, 1, materialD);
	glUniform3fv(materialSLoc, 1, materialS);
	glUniform1f(exponentLoc, exponent);

	glEnable(GL_DEPTH_TEST);
	//	glEnable(GL_CULL_FACE);
	//	glFrontFace(GL_CW);
}

static void generateTerrain()
{
	Vertex *vertexes = new Vertex[NUM_VERTEX_X * NUM_VERTEX_Z];
	Vertex *colors = new Vertex[NUM_VERTEX_X * NUM_VERTEX_Z];
	Vertex *normals = new Vertex[NUM_VERTEX_X * NUM_VERTEX_Z];
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
			vertexes[i * NUM_VERTEX_X + j].y = Perlin_Get2d(x, z, 0.4, 15) * ((-cos(j * M_PI * 2 / NUM_VERTEX_X) + 1) + ((-cos(j * M_PI * 2 / NUM_VERTEX_X * 5) + 1) / 5.0) + ((-cos(j * M_PI * 2 / NUM_VERTEX_X * 7) + 1) / 7.0)) * ((-cos(i * M_PI * 2 / NUM_VERTEX_Z) + 1) + ((-cos(i * M_PI * 2 / NUM_VERTEX_Z * 5) + 1) / 5.0) + ((-cos(i * M_PI * 2 / NUM_VERTEX_Z * 7) + 1) / 7.0)) * 0.25;
			vertexes[i * NUM_VERTEX_X + j].z = z;

			colors[i * NUM_VERTEX_X + j].x = 1;
			colors[i * NUM_VERTEX_X + j].y = 1;
			colors[i * NUM_VERTEX_X + j].z = 1;

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

	for (int i = 0; i < NUM_VERTEX_X * NUM_VERTEX_Z; i++)
	{
		normals[i] = {0, 0, 0};
	}

	int index = 0;
	for (int i = 0; i < (NUM_VERTEX_Z - 1); i++)
	{
		for (int j = 0; j < (NUM_VERTEX_X - 1) * 2; j++)
		{
			Vertex A = subtractVertex(vertexes[indexBuffer[index + j]], vertexes[indexBuffer[index + j + 1]]);
			Vertex B = subtractVertex(vertexes[indexBuffer[index + j]], vertexes[indexBuffer[index + j + 2]]);
			Vertex C = crossProduct(A, B);
			if (j % 2 != 0)
			{
				C.x *= -1;
				C.y *= -1;
				C.z *= -1;
			}

			normals[indexBuffer[index + j]].x += C.x;
			normals[indexBuffer[index + j]].y += C.y;
			normals[indexBuffer[index + j]].z += C.z;

			normals[indexBuffer[index + j + 1]].x += C.x;
			normals[indexBuffer[index + j + 1]].y += C.y;
			normals[indexBuffer[index + j + 1]].z += C.z;

			normals[indexBuffer[index + j + 2]].x += C.x;
			normals[indexBuffer[index + j + 2]].y += C.y;
			normals[indexBuffer[index + j + 2]].z += C.z;
		}
		index += (NUM_VERTEX_X * 2 + 1);
	}

	glGenVertexArrays(1, va);
	glBindVertexArray(va[0]);
	glGenBuffers(4, bufferId);

	glBindBuffer(GL_ARRAY_BUFFER, bufferId[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * NUM_VERTEX_X * NUM_VERTEX_Z, vertexes, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexPosLoc);
	glVertexAttribPointer(vertexPosLoc, 3, GL_FLOAT, 0, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, bufferId[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * NUM_VERTEX_X * NUM_VERTEX_Z, colors, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexColLoc);
	glVertexAttribPointer(vertexColLoc, 3, GL_FLOAT, 0, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, bufferId[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * NUM_VERTEX_X * NUM_VERTEX_Z, normals, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexNormalLoc);
	glVertexAttribPointer(vertexNormalLoc, 3, GL_FLOAT, 0, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, bufferId[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * (NUM_VERTEX_X - 1) * (NUM_VERTEX_Z * 2 + 1), indexBuffer, GL_STATIC_DRAW);
	glPrimitiveRestartIndex(RESET);
	glEnable(GL_PRIMITIVE_RESTART);
}

static void move()
{
	cameraSpeed = keys[32] ? 0.1 : 0.05; // If space bar is pressed duplicate speed

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

static void drawTerrain(int offsetX, int offsetZ)
{
	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			mIdentity(&modelMatrix);
			translate(&modelMatrix, (i + offsetX) * SIDE_LENGTH_X, 0, (j + offsetZ) * SIDE_LENGTH_Z);
			glUniformMatrix4fv(modelMatrixLoc, 1, GL_TRUE, modelMatrix.values);
			glBindVertexArray(va[0]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId[3]);
			glDrawElements(GL_TRIANGLE_STRIP, (NUM_VERTEX_X - 1) * (NUM_VERTEX_Z * 2 + 1), GL_UNSIGNED_INT, 0);
		}
	}
}

static void display()
{
	mIdentity(&modelMatrix);
	mIdentity(&viewMatrix);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(programId);

	move();
	glUniform3f(cameraLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

	rotateX(&viewMatrix, movey * 0.08);
	rotateY(&viewMatrix, movex * 0.08);
	translate(&viewMatrix, -cameraPosition.x, -cameraPosition.y, -cameraPosition.z);
	glUniformMatrix4fv(viewMatrixLoc, 1, GL_TRUE, viewMatrix.values);

	static float angleEarth = -45;

	translate(&modelMatrix, cameraPosition.x + 2, 2, cameraPosition.z + 2);
	glUniformMatrix4fv(modelMatrixLoc, 1, GL_TRUE, modelMatrix.values);
	sphere_draw(earth);

	drawTerrain(cameraPosition.x / SIDE_LENGTH_X, cameraPosition.z / SIDE_LENGTH_Z);

	angleEarth += 0.5;
	if (angleEarth >= 360.0)
		angleEarth -= 360.0;

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
	setPerspective(&projectionMatrix, 70, aspect, -0.05, -2000);
	glUniformMatrix4fv(projMatrixLoc, 1, GL_TRUE, projectionMatrix.values);
}

static void exitFunc(unsigned char key, int x, int y)
{
	if (key == 27)
	{
		glDeleteVertexArrays(1, va);
		exit(0);
	}
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
	glutKeyboardFunc(keyPressed);
	glutKeyboardUpFunc(keyReleased);
	glutReshapeFunc(reshapeFunc);
	glewInit();
	initShaders();

	earth = sphere_create(0.25, 40, 40, {1.2, 1.2, 2});
	sphere_bind(earth, vertexPosLoc, vertexColLoc, vertexNormalLoc);

	generateTerrain();
	glClearColor(0, 0, 0, 1.0);
	glutMainLoop();
	return 0;
}
