#include <GL/glew.h>
#include <GL/freeglut.h>
#include "utils.h"
#include "transforms.h"
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <shapes/Rover.hpp>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static Rover r;

static Mat4 modelMatrix, projectionMatrix, viewMatrix;
static GLuint programId1;
static GLuint vertexPositionLoc, vertexNormalLoc, modelMatrixLoc, viewMatrixLoc, projectionMatrixLoc, vertexColorLoc;
static GLuint ambientLightLoc, diffuseLightLoc, lightPositionLoc, materialALoc, materialDLoc, materialSLoc, exponentLoc, cameraLoc;

static float ambientLight[] = {1.0, 1.0, 1.0};
static float materialA[] = {0.5, 0.5, 0.5};
static float diffuseLight[] = {1.0, 1.0, 1.0};
static float lightPosition[] = {0.0, 0.0, -1.0};
static float materialD[] = {0.5, 0.5, 0.5};
static float materialS[] = {0.5, 0.5, 0.5};
static float exponent = 16;

static float rz = -1.0;

static void initShaders()
{
	GLuint vShader = compileShader("shaders/Gouraud.vsh", GL_VERTEX_SHADER);
	if (!shaderCompiled(vShader))
		return;
	GLuint fShader = compileShader("shaders/modelColor.fsh", GL_FRAGMENT_SHADER);
	if (!shaderCompiled(fShader))
		return;
	programId1 = glCreateProgram();
	printf("Program %d created\n", programId1);
	glAttachShader(programId1, vShader);
	glAttachShader(programId1, fShader);
	glLinkProgram(programId1);

	glUseProgram(programId1);
	// MODELS
	modelMatrixLoc = glGetUniformLocation(programId1, "modelMatrix");
	viewMatrixLoc = glGetUniformLocation(programId1, "viewMatrix");
	projectionMatrixLoc = glGetUniformLocation(programId1, "projectionMatrix");

	vertexPositionLoc = glGetAttribLocation(programId1, "vertexPosition");
	vertexNormalLoc = glGetAttribLocation(programId1, "vertexNormal");
	vertexColorLoc = glGetAttribLocation(programId1, "vertexColor");

	// LIGHSTS
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
}

static void reshapeFunc(int w, int h)
{
	if (h == 0)
		h = 1;
	glViewport(0, 0, w, h);
	float aspect = (float)w / h;
	setPerspective(&projectionMatrix, 65, aspect, -0.1, -500);
}

static void timerFunc(int id)
{
	glutTimerFunc(10, timerFunc, id);
	glutPostRedisplay();
}

static void displayFunc()
{
	Mat4 modelMatrix;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programId1);

	glUniform3f(cameraLoc, 0, 0, 0);

	glUniformMatrix4fv(projectionMatrixLoc, 1, 1, projectionMatrix.values);
	mIdentity(&viewMatrix);
	glUniformMatrix4fv(viewMatrixLoc, 1, 1, viewMatrix.values);

	r.rotateWheels(1);
	// r.rotateRover(1);
	r.rotateRoverRoll(45);
	r.turnWheels(40);
	r.setPosition(0, 0, rz);
	r.draw(modelMatrixLoc);

	glUniformMatrix4fv(viewMatrixLoc, 1, 1, viewMatrix.values);
	// emptyStack();
	glutSwapBuffers();
}

int main(int argc, char **argv)
{
    setbuf(stdout, NULL);
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("TDA");

	glutDisplayFunc(displayFunc);
	glutReshapeFunc(reshapeFunc);
	glutTimerFunc(10, timerFunc, 1);

	glewInit();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	initShaders();

	r.load();
	r.bind(programId1, vertexPositionLoc, vertexNormalLoc, vertexColorLoc);

	glClearColor(0.3, 0.3, 0.3, 1.0);
	glutMainLoop();
	return 0;
}
