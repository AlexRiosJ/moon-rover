#include <GL/glew.h>
#include <GL/freeglut.h>
#include "shapes/Rover.hpp"
#include "perlin.h"
#include "utils.h"
#include "transforms.h"
#include "sphere.h"
#include "terrain.h"
#include "camera.h"
#include "player.h"
#include <stdio.h>
#include <math.h>
#include <time.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static inline float toRadians(float deg) { return deg * M_PI / 180.0; }
static inline float toDeg(float rad) { return rad * 180.0 / M_PI; }

#define RESET 0xFFFFFFFF
#define NUM_VERTEX_X 512
#define NUM_VERTEX_Z 512
#define SIDE_LENGTH_X 20
#define SIDE_LENGTH_Z 20

Sphere earth, sphereRover;
Terrain terrain;
Sphere skybox;
Rover rover;

typedef enum
{
	LEFT,
	MIDDLE,
	RIGHT,
	FRONT_WHEEL,
	BACK_WHEEL
} MouseButton;

typedef struct
{
	GLuint id;
	GLuint vertexPosLoc;
	GLuint vertexColLoc;
	GLuint vertexTexCoord;
	GLuint vertexNormal;
	GLuint modelMatrix;
	GLuint viewMatrix;
	GLuint projectionMatrix;

} ShaderProgramLocationVariables;

unsigned char keys[256];
bool mouseButtonsClicked[5] = {};
float lastClickedCoord[2] = {};

static GLuint programId1, vertexPosLoc1, vertexColLoc1, vertexTexcoordLoc1, vertexNormalLoc1, modelMatrixLoc1, viewMatrixLoc1, projMatrixLoc1;
static GLuint programId2, vertexPosLoc2, vertexColLoc2, vertexTexcoordLoc2, vertexNormalLoc2, modelMatrixLoc2, viewMatrixLoc2, projMatrixLoc2;
static GLuint programId3, vertexPosLoc3, vertexColLoc3, vertexTexcoordLoc3, vertexNormalLoc3, modelMatrixLoc3, viewMatrixLoc3, projMatrixLoc3;
static Mat4 modelMatrix, viewMatrix, projectionMatrix;

static GLuint texturesLocs[5];

Vertex cameraPosition = {0, 1.5, 1.0};
float cameraPitch = -30;
float cameraYaw = 0.0;
float cameraRoll;
float cameraSpeed = 0.05;
float distanceFromPlayer = -1.5;
float angleAroundPlayer = 180;

Vertex thirdPersonObj = {0, 1, 0};
float objectYaw = 0.0; // object yaw
float objectPitch = 0.0;
float objectSpeed = 0.02;

// Player player = createPlayer(thirdPersonObj, objectPitch, objectYaw, 0.0, objectSpeed);
// Camera camera = createCamera(player, cameraPosition, cameraPitch, cameraYaw, cameraRoll, cameraSpeed, 0.08, -2.5, angleAroundPlayer);

static GLuint ambientLightLoc, diffuseLightLoc, lightPositionLoc, materialALoc, materialDLoc, materialSLoc, exponentLoc, cameraLoc;

static float ambientLight[] = {0, 0, 0};
static float materialA[] = {0.5, 0.5, 0.5};
static float diffuseLight[] = {1.0, 1.0, 1.0};
static float lightPosition[] = {0, 60, 0};
static float materialD[] = {0.7, 0.7, 0.7};
static float materialS[] = {0.7, 0.7, 0.7};
static float exponent = 32;

static GLuint textures[5];

void calculateCameraPosition();

static void initTexture(const char *filename, GLuint textureId)
{
	unsigned char *data;
	unsigned int width, height;
	glBindTexture(GL_TEXTURE_2D, textureId);
	loadBMP(filename, &data, &width, &height);
	// printf("%d, %d\n", width, height);
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
	glGenTextures(5, textures);
	initTexture("textures/moon-sand.bmp", textures[0]);
	initTexture("textures/earth.bmp", textures[1]);
	initTexture("textures/earth-clouds.bmp", textures[2]);
	initTexture("textures/earth-specular.bmp", textures[3]);
	initTexture("textures/skybox.bmp", textures[4]);
}

static void setupTextures()
{
	texturesLocs[0] = glGetUniformLocation(programId1, "moonTexture");
	texturesLocs[1] = glGetUniformLocation(programId2, "earthTexture");
	texturesLocs[2] = glGetUniformLocation(programId2, "earthCloudsTexture");
	texturesLocs[3] = glGetUniformLocation(programId2, "earthSpecularTexture");
	texturesLocs[4] = glGetUniformLocation(programId3, "skyboxTexture");

	glUseProgram(programId1);
	glUniform1i(texturesLocs[0], textures[0]);
	glUseProgram(programId2);
	glUniform1i(texturesLocs[1], textures[1]);
	glUniform1i(texturesLocs[2], textures[2]);
	glUniform1i(texturesLocs[3], textures[3]);
	glUseProgram(programId3);
	glUniform1i(texturesLocs[4], textures[4]);
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

	// Setup textures
	setupTextures();

	glEnable(GL_DEPTH_TEST);
	//	glEnable(GL_CULL_FACE);
	//	glFrontFace(GL_CW);
}

static void move()
{
	float nextForwardXPosition = objectSpeed * -sin(toRadians(objectYaw));
	float nextForwardZPosition = objectSpeed * -cos(toRadians(objectYaw));

	if (keys['w'])
	{
		if (keys['a'])
		{
			objectYaw += 2;
		}
		if (keys['d'])
		{
			objectYaw -= 2;
		}
		rover.rotateWheels(1);
		rover.setYawRotation(objectYaw);
		thirdPersonObj.x -= nextForwardXPosition;
		thirdPersonObj.z -= nextForwardZPosition;
	}

	if (keys['s'])
	{
		if (keys['a'])
		{
			objectYaw -= 2;
		}
		if (keys['d'])
		{
			objectYaw += 2;
		}
		rover.rotateWheels(0);
		rover.setYawRotation(objectYaw);
		thirdPersonObj.x += nextForwardXPosition;
		thirdPersonObj.z += nextForwardZPosition;
	}

	if (keys['a'])
	{
		rover.turnWheels(30);
	}
	else if (keys['d'])
	{
		rover.turnWheels(-30);
	}
	else
	{
		rover.resetTurnWheels();
	}

	cameraYaw = (objectYaw + angleAroundPlayer);
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

	calculateCameraPosition();
	move();

	// ----------------- MVP to shader 1
	glUseProgram(programId1);
	glUniformMatrix4fv(projMatrixLoc1, 1, GL_TRUE, projectionMatrix.values);
	mIdentity(&viewMatrix);
	glUniform3f(cameraLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
	rotateX(&viewMatrix, -cameraPitch);
	rotateY(&viewMatrix, -cameraYaw);
	translate(&viewMatrix, -cameraPosition.x, -cameraPosition.y, -cameraPosition.z);
	glUniformMatrix4fv(viewMatrixLoc1, 1, GL_TRUE, viewMatrix.values);

	// Draw an object to build third person view from it
	mIdentity(&modelMatrix);

	thirdPersonObj.y = vertexFromXZPosition(terrain, thirdPersonObj.x, thirdPersonObj.z).y;
	translate(&modelMatrix, thirdPersonObj.x, thirdPersonObj.y, thirdPersonObj.z);

	Vertex normalInXZ = normalFromXZPosition(terrain, thirdPersonObj.x, thirdPersonObj.z);
	glUniformMatrix4fv(modelMatrixLoc1, 1, GL_TRUE, modelMatrix.values);

	rover.setPosition(thirdPersonObj.x, thirdPersonObj.y + 0.164, thirdPersonObj.z);

	float roverPitch = toDeg(atan(normalInXZ.z / -normalInXZ.y));
	float roverRoll = toDeg(atan(normalInXZ.x / -normalInXZ.y));

	float finalRoverPitch = -sin(toRadians(objectYaw)) * roverRoll - cos(toRadians(objectYaw)) * roverPitch;
	float finalRoverRoll = cos(toRadians(objectYaw)) * roverRoll - sin(toRadians(objectYaw)) * roverPitch;

	rover.rotateRoverPitch(finalRoverPitch);
	rover.rotateRoverRoll(finalRoverRoll);
	rover.draw(modelMatrixLoc1);

	// Draw Terrain
	mIdentity(&modelMatrix);
	glActiveTexture(GL_TEXTURE0 + 0);
	glUniform1i(texturesLocs[0], 0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	drawTerrain(thirdPersonObj.x / SIDE_LENGTH_X, thirdPersonObj.z / SIDE_LENGTH_Z);

	// ---------------- MVP to shader 2 (earth)
	glUseProgram(programId2);
	glUniformMatrix4fv(projMatrixLoc2, 1, GL_TRUE, projectionMatrix.values);
	mIdentity(&viewMatrix);
	glUniform3f(cameraLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
	rotateX(&viewMatrix, -cameraPitch);
	rotateY(&viewMatrix, -cameraYaw);
	translate(&viewMatrix, -cameraPosition.x, -cameraPosition.y, -cameraPosition.z);
	glUniformMatrix4fv(viewMatrixLoc2, 1, GL_TRUE, viewMatrix.values);

	// Draw Earth
	mIdentity(&modelMatrix);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glActiveTexture(GL_TEXTURE0 + 1);
	glUniform1i(texturesLocs[1], 1);
	glBindTexture(GL_TEXTURE_2D, textures[1]);

	// Apply clouds texture
	glEnable(GL_BLEND);
	glActiveTexture(GL_TEXTURE0 + 3);
	glUniform1i(texturesLocs[3], 3);
	glBindTexture(GL_TEXTURE_2D, textures[3]);
	glActiveTexture(GL_TEXTURE0 + 2);
	glUniform1i(texturesLocs[2], 2);
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	glDisable(GL_BLEND);

	static float angleEarth = -45;
	static float angleSkybox = -45;

	translate(&modelMatrix, cameraPosition.x + 50, 20, cameraPosition.z + 50);
	rotateX(&modelMatrix, 23.5); // 23° It's the approximate inclination of the Earth
	rotateZ(&modelMatrix, -angleEarth);
	glUniformMatrix4fv(modelMatrixLoc2, 1, GL_TRUE, modelMatrix.values);
	sphere_draw(earth);

	// ------------ MVP to shader 3 (skybox)
	glUseProgram(programId3);
	glUniformMatrix4fv(projMatrixLoc3, 1, GL_TRUE, projectionMatrix.values);
	mIdentity(&viewMatrix);
	glUniform3f(cameraLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);
	rotateX(&viewMatrix, -cameraPitch);
	rotateY(&viewMatrix, -cameraYaw);
	translate(&viewMatrix, -cameraPosition.x, -cameraPosition.y, -cameraPosition.z);
	glUniformMatrix4fv(viewMatrixLoc3, 1, GL_TRUE, viewMatrix.values);

	mIdentity(&modelMatrix);
	glActiveTexture(GL_TEXTURE0 + 4);
	glUniform1i(texturesLocs[4], 4);
	glBindTexture(GL_TEXTURE_2D, textures[4]);
	translate(&modelMatrix, thirdPersonObj.x, 0, thirdPersonObj.z);
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
	{
		keys[key] = 1;
	}
}

static void keyReleased(unsigned char key, int x, int y)
{
	keys[key] = 0;
}

static void mouseMove(int x, int y)
{
	float nx = 2.0 * x / glutGet(GLUT_WINDOW_WIDTH) - 1;
	float ny = -1 * (2.0 * y / glutGet(GLUT_WINDOW_HEIGHT) - 1);
	if (mouseButtonsClicked[LEFT] || mouseButtonsClicked[RIGHT])
	{
		float angleAroundPlayerChange = (nx - lastClickedCoord[0]) * 500;
		// printf("Angle around  player decrement: %f\n", angleAroundPlayerChange);
		angleAroundPlayer -= angleAroundPlayerChange;

		float cameraPitchChange = (ny - lastClickedCoord[1]) * 100;
		// printf("Camera pitch decrement: %f\n", cameraPitchChange);
		cameraPitch += cameraPitchChange;
		if (cameraPitch <= -90 || cameraPitch > -5)
		{
			cameraPitch -= cameraPitchChange;
		}

		lastClickedCoord[0] = nx;
		lastClickedCoord[1] = ny;
		// printf("Moving the mouse! x: %f, y: %f\n\n", nx, ny);
	}
	glutPostRedisplay();
}

void mouseFunction(int button, int state, int mx, int my)
{
	// Obtener coordenadas de dispositivo normalizado
	float nx = 2.0 * mx / glutGet(GLUT_WINDOW_WIDTH) - 1;
	float ny = -1 * (2.0 * my / glutGet(GLUT_WINDOW_HEIGHT) - 1);
	// printf("Clicking the mouse!\nstate: %d, button: %d, x: %f, y: %f\n\n", state, button, nx, ny);

	switch (button)
	{
	case LEFT:
		mouseButtonsClicked[LEFT] = !state;
		if (!state)
		{
			lastClickedCoord[0] = nx;
			lastClickedCoord[1] = ny;
		}
		break;
	case RIGHT:
		mouseButtonsClicked[RIGHT] = !state;
		if (!state)
		{
			lastClickedCoord[0] = nx;
			lastClickedCoord[1] = ny;
		}
		break;
	case MIDDLE:

		break;

	case FRONT_WHEEL:
		distanceFromPlayer += 0.1;
		break;

	case BACK_WHEEL:
		distanceFromPlayer -= 0.1;
		break;

	default:
		break;
	}
}

void calculateCameraPosition()
{
	float horizontalDistance = distanceFromPlayer * cos(toRadians(cameraPitch));
	float verticalDistance = distanceFromPlayer * sin(toRadians(cameraPitch));

	float theta = objectYaw + angleAroundPlayer;
	float offsetX = horizontalDistance * sin(toRadians(theta));
	float offsetZ = horizontalDistance * cos(toRadians(theta));
	cameraPosition.x = thirdPersonObj.x - offsetX;
	cameraPosition.z = thirdPersonObj.z - offsetZ;
	cameraPosition.y = thirdPersonObj.y + verticalDistance;
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
	// glutFullScreen();
	// glutSetCursor(GLUT_CURSOR_CROSSHAIR);

	// Supported functions
	// glutPassiveMotionFunc(mouseMove);
	glutMotionFunc(mouseMove);
	glutMouseFunc(mouseFunction);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyPressed);
	glutKeyboardUpFunc(keyReleased);
	glutReshapeFunc(reshapeFunc);

	// Init
	glewInit();
	initTextures();
	initShaders();

	// Init scene set up
	terrain = terrain_create(NUM_VERTEX_X, NUM_VERTEX_Z, SIDE_LENGTH_X, SIDE_LENGTH_Z, {1, 1, 1});
	terrain_bind(terrain, vertexPosLoc1, vertexColLoc1, vertexTexcoordLoc1, vertexNormalLoc1);

	earth = sphere_create(7, 40, 40, {1, 1, 1});
	sphere_bind(earth, vertexPosLoc2, vertexColLoc2, vertexTexcoordLoc2, vertexNormalLoc2);

	sphereRover = sphere_create(0.3, 40, 40, {1, 1, 1});
	sphere_bind(sphereRover, vertexPosLoc2, vertexColLoc2, vertexTexcoordLoc2, vertexNormalLoc2);

	skybox = sphere_create(1500, 40, 40, {1.2, 1.2, 1.2});
	sphere_bind(skybox, vertexPosLoc3, vertexColLoc3, vertexTexcoordLoc3, vertexNormalLoc3);

	rover.load();
	rover.bind(programId1, vertexPosLoc1, vertexNormalLoc1, vertexColLoc1);

	glClearColor(0, 0, 0, 1.0);
	glutMainLoop();
	return 0;
}
