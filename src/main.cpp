#include <GL/glew.h>
#include <GL/freeglut.h>
#include "utils.h"
#include "transforms.h"
#include "sphere.h"
#include "terrain.h"
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
Terrain terrain;
Sphere skybox;

unsigned char keys[256];

static GLuint programId1, vertexPosLoc1, vertexColLoc1, vertexTexcoordLoc1, vertexNormalLoc1, modelMatrixLoc1, viewMatrixLoc1, projMatrixLoc1;
static GLuint programId2, vertexPosLoc2, vertexColLoc2, vertexTexcoordLoc2, vertexNormalLoc2, modelMatrixLoc2, viewMatrixLoc2, projMatrixLoc2;
static GLuint programId3, vertexPosLoc3, vertexColLoc3, vertexTexcoordLoc3, vertexNormalLoc3, modelMatrixLoc3, viewMatrixLoc3, projMatrixLoc3;
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

static GLuint textures[4];

static void initTexture(const char *filename, GLuint textureId)
{
	unsigned char *data;
	unsigned int width, height;
	glBindTexture(GL_TEXTURE_2D, textureId);
	loadBMP(filename, &data, &width, &height);
	printf("%d, %d\n", width, height);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 8);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

static void initTextures()
{
	glGenTextures(4, textures);
	initTexture("textures/moon-sand.bmp", textures[0]);
	initTexture("textures/earth.bmp", textures[1]);
	initTexture("textures/earth-clouds.bmp", textures[2]);
	initTexture("textures/skybox.bmp", textures[3]);
}

static void initShaders()
{
	GLuint vShader = compileShader("shaders/projection.vsh", GL_VERTEX_SHADER);
	if (!shaderCompiled(vShader))
		return;
	GLuint fShader = compileShader("shaders/color.fsh", GL_FRAGMENT_SHADER);
	if (!shaderCompiled(fShader))
		return;

	programId1 = glCreateProgram();
	glAttachShader(programId1, vShader);
	glAttachShader(programId1, fShader);
	glLinkProgram(programId1);
	glUseProgram(programId1);

	vertexPosLoc1 = glGetAttribLocation(programId1, "vertexPosition");
	vertexColLoc1 = glGetAttribLocation(programId1, "vertexColor");
	vertexTexcoordLoc1 = glGetAttribLocation(programId1, "vertexTexcoord");
	vertexNormalLoc1 = glGetAttribLocation(programId1, "vertexNormal");
	modelMatrixLoc1 = glGetUniformLocation(programId1, "modelMatrix");
	viewMatrixLoc1 = glGetUniformLocation(programId1, "viewMatrix");
	projMatrixLoc1 = glGetUniformLocation(programId1, "projectionMatrix");

	ambientLightLoc = glGetUniformLocation(programId1, "ambientLight");
	diffuseLightLoc = glGetUniformLocation(programId1, "diffuseLight");
	lightPositionLoc = glGetUniformLocation(programId1, "lightPosition");
	materialALoc = glGetUniformLocation(programId1, "materialA");
	materialDLoc = glGetUniformLocation(programId1, "materialD");
	materialSLoc = glGetUniformLocation(programId1, "materialS");
	exponentLoc = glGetUniformLocation(programId1, "exponent");
	cameraLoc = glGetUniformLocation(programId1, "camera");

	glUniform3fv(ambientLightLoc, 1, ambientLight);
	glUniform3fv(diffuseLightLoc, 1, diffuseLight);
	glUniform3fv(lightPositionLoc, 1, lightPosition);
	glUniform3fv(materialALoc, 1, materialA);
	glUniform3fv(materialDLoc, 1, materialD);
	glUniform3fv(materialSLoc, 1, materialS);
	glUniform1f(exponentLoc, exponent);

	GLuint vShader2 = compileShader("shaders/earth.vsh", GL_VERTEX_SHADER);
	if (!shaderCompiled(vShader2))
		return;
	GLuint fShader2 = compileShader("shaders/earth.fsh", GL_FRAGMENT_SHADER);
	if (!shaderCompiled(fShader2))
		return;
	programId2 = glCreateProgram();
	glAttachShader(programId2, vShader2);
	glAttachShader(programId2, fShader2);
	glLinkProgram(programId2);
	glUseProgram(programId2);

	vertexPosLoc2 = glGetAttribLocation(programId2, "vertexPosition");
	vertexColLoc2 = glGetAttribLocation(programId2, "vertexColor");
	vertexTexcoordLoc2 = glGetAttribLocation(programId2, "vertexTexcoord");
	vertexNormalLoc2 = glGetAttribLocation(programId2, "vertexNormal");
	modelMatrixLoc2 = glGetUniformLocation(programId2, "modelMatrix");
	viewMatrixLoc2 = glGetUniformLocation(programId2, "viewMatrix");
	projMatrixLoc2 = glGetUniformLocation(programId2, "projectionMatrix");

	ambientLightLoc = glGetUniformLocation(programId2, "ambientLight");
	diffuseLightLoc = glGetUniformLocation(programId2, "diffuseLight");
	lightPositionLoc = glGetUniformLocation(programId2, "lightPosition");
	materialALoc = glGetUniformLocation(programId2, "materialA");
	materialDLoc = glGetUniformLocation(programId2, "materialD");
	materialSLoc = glGetUniformLocation(programId2, "materialS");
	exponentLoc = glGetUniformLocation(programId2, "exponent");
	cameraLoc = glGetUniformLocation(programId2, "camera");

	glUniform3fv(ambientLightLoc, 1, ambientLight);
	glUniform3fv(diffuseLightLoc, 1, diffuseLight);
	glUniform3fv(lightPositionLoc, 1, lightPosition);
	glUniform3fv(materialALoc, 1, materialA);
	glUniform3fv(materialDLoc, 1, materialD);
	glUniform3fv(materialSLoc, 1, materialS);
	glUniform1f(exponentLoc, exponent);

	GLuint vShader3 = compileShader("shaders/skybox.vsh", GL_VERTEX_SHADER);
	if (!shaderCompiled(vShader3))
		return;
	GLuint fShader3 = compileShader("shaders/skybox.fsh", GL_FRAGMENT_SHADER);
	if (!shaderCompiled(fShader3))
		return;
	programId3 = glCreateProgram();
	glAttachShader(programId3, vShader3);
	glAttachShader(programId3, fShader3);
	glLinkProgram(programId3);
	glUseProgram(programId3);

	vertexPosLoc3 = glGetAttribLocation(programId3, "vertexPosition");
	vertexColLoc3 = glGetAttribLocation(programId3, "vertexColor");
	vertexTexcoordLoc3 = glGetAttribLocation(programId3, "vertexTexcoord");
	vertexNormalLoc3 = glGetAttribLocation(programId3, "vertexNormal");
	modelMatrixLoc3 = glGetUniformLocation(programId3, "modelMatrix");
	viewMatrixLoc3 = glGetUniformLocation(programId3, "viewMatrix");
	projMatrixLoc3 = glGetUniformLocation(programId3, "projectionMatrix");

	glEnable(GL_DEPTH_TEST);
	//	glEnable(GL_CULL_FACE);
	//	glFrontFace(GL_CW);
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
			glUniformMatrix4fv(modelMatrixLoc1, 1, GL_TRUE, modelMatrix.values);
			terrain_draw(terrain);
		}
	}
}

static void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	move();

	// MVP al shader 1
	glUseProgram(programId1);
	glUniformMatrix4fv(projMatrixLoc1, 1, GL_TRUE, projectionMatrix.values);
	mIdentity(&viewMatrix);
	glUniform3f(cameraLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
	rotateX(&viewMatrix, movey * 0.08);
	rotateY(&viewMatrix, movex * 0.08);
	translate(&viewMatrix, -cameraPosition.x, -cameraPosition.y, -cameraPosition.z);
	glUniformMatrix4fv(viewMatrixLoc1, 1, GL_TRUE, viewMatrix.values);

	// Dibujar terreno
	mIdentity(&modelMatrix);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(programId1, "texture0"), 0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	drawTerrain(cameraPosition.x / SIDE_LENGTH_X, cameraPosition.z / SIDE_LENGTH_Z);

	// MVP al shader 2 (earth)
	glUseProgram(programId2);
	glUniformMatrix4fv(projMatrixLoc2, 1, GL_TRUE, projectionMatrix.values);
	mIdentity(&viewMatrix);
	glUniform3f(cameraLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
	rotateX(&viewMatrix, movey * 0.08);
	rotateY(&viewMatrix, movex * 0.08);
	translate(&viewMatrix, -cameraPosition.x, -cameraPosition.y, -cameraPosition.z);
	glUniformMatrix4fv(viewMatrixLoc2, 1, GL_TRUE, viewMatrix.values);

	// Dibujar Earth
	mIdentity(&modelMatrix);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glActiveTexture(GL_TEXTURE1);
	glUniform1i(glGetUniformLocation(programId2, "texture1"), 1);
	glBindTexture(GL_TEXTURE_2D, textures[1]);

	glEnable(GL_BLEND);
	glActiveTexture(GL_TEXTURE2);
	glUniform1i(glGetUniformLocation(programId2, "texture2"), 2);
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	glDisable(GL_BLEND);

	static float angleEarth = -45;
	static float angleSkybox = -45;

	translate(&modelMatrix, cameraPosition.x + 50, 20, cameraPosition.z + 50);
	rotateX(&modelMatrix, 23.5);
	rotateZ(&modelMatrix, -angleEarth);
	glUniformMatrix4fv(modelMatrixLoc2, 1, GL_TRUE, modelMatrix.values);
	sphere_draw(earth);

	// MVP al shader 3 (skybox)
	glUseProgram(programId3);
	glUniformMatrix4fv(projMatrixLoc3, 1, GL_TRUE, projectionMatrix.values);
	mIdentity(&viewMatrix);
	glUniform3f(cameraLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
	rotateX(&viewMatrix, movey * 0.08);
	rotateY(&viewMatrix, movex * 0.08);
	translate(&viewMatrix, -cameraPosition.x, -cameraPosition.y, -cameraPosition.z);
	glUniformMatrix4fv(viewMatrixLoc3, 1, GL_TRUE, viewMatrix.values);

	mIdentity(&modelMatrix);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(programId3, "texture3"), 3);
	glBindTexture(GL_TEXTURE_2D, textures[3]);
	translate(&modelMatrix, cameraPosition.x, 0, cameraPosition.z);
	rotateX(&modelMatrix, 180);
	rotateZ(&modelMatrix, -angleSkybox);
	glUniformMatrix4fv(modelMatrixLoc3, 1, GL_TRUE, modelMatrix.values);
	sphere_draw(skybox);

	angleEarth += 0.08;
	if (angleEarth >= 360.0)
		angleEarth -= 360.0;

	angleSkybox += 0.008;
	if (angleSkybox >= 360.0)
		angleSkybox -= 360.0;

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
	glUniformMatrix4fv(projMatrixLoc1, 1, GL_TRUE, projectionMatrix.values);
	glUniformMatrix4fv(projMatrixLoc2, 1, GL_TRUE, projectionMatrix.values);
}

static void exitFunc(unsigned char key, int x, int y)
{
	if (key == 27)
	{
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
	// Activate MSAA (Multisample Antialiasing)
	// SRC: http://bankslab.berkeley.edu/members/chris/AntiAliasing/AntiAliasingInOpenGL.html
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glEnable(GL_MULTISAMPLE);
	glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
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
	initTextures();
	initShaders();

	terrain = terrain_create(NUM_VERTEX_X, NUM_VERTEX_Z, SIDE_LENGTH_X, SIDE_LENGTH_Z, {1, 1, 1});
	terrain_bind(terrain, vertexPosLoc1, vertexColLoc1, vertexTexcoordLoc1, vertexNormalLoc1);

	earth = sphere_create(7, 40, 40, {1, 1, 1});
	sphere_bind(earth, vertexPosLoc2, vertexColLoc2, vertexTexcoordLoc2, vertexNormalLoc2);

	skybox = sphere_create(1500, 40, 40, {1.2, 1.2, 1.2});
	sphere_bind(skybox, vertexPosLoc3, vertexColLoc3, vertexTexcoordLoc3, vertexNormalLoc3);

	glClearColor(0, 0, 0, 1.0);
	glutMainLoop();
	return 0;
}
