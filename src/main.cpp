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
#include <stdlib.h>
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

unsigned char keys[256];
bool mouseButtonsClicked[5] = {};
float lastClickedCoord[2] = {};

static ShaderProgram terrainProgram, earthProgram, skyboxProgram;
static Mat4 modelMatrix, viewMatrix, projectionMatrix;

static GLuint texturesLocs[5];

static Player player;
static Camera camera;

static float ambientLight[] = {0, 0, 0};
static float materialA[] = {0.5, 0.5, 0.5};
static float diffuseLight[] = {1.0, 1.0, 1.0};
static float lightPosition[] = {0, 60, 0};
static float materialD[] = {0.7, 0.7, 0.7};
static float materialS[] = {0.7, 0.7, 0.7};
static float exponent = 32;

static GLuint textures[5];

static bool initTexture(const char *filename, GLuint textureId)
{
	unsigned char *data;
	unsigned int width, height;
	if (!loadBMP(filename, &data, &width, &height))
		return false;

	glBindTexture(GL_TEXTURE_2D, textureId);
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

	free(data);
	return true;
}

static bool initTextures()
{
	glGenTextures(5, textures);
	bool loaded = true;
	loaded &= initTexture("textures/moon-sand.bmp", textures[0]);
	loaded &= initTexture("textures/earth.bmp", textures[1]);
	loaded &= initTexture("textures/earth-clouds.bmp", textures[2]);
	loaded &= initTexture("textures/earth-specular.bmp", textures[3]);
	loaded &= initTexture("textures/skybox.bmp", textures[4]);
	return loaded;
}

static void setupTextures()
{
	texturesLocs[0] = glGetUniformLocation(terrainProgram.id, "moonTexture");
	texturesLocs[1] = glGetUniformLocation(earthProgram.id, "earthTexture");
	texturesLocs[2] = glGetUniformLocation(earthProgram.id, "earthCloudsTexture");
	texturesLocs[3] = glGetUniformLocation(earthProgram.id, "earthSpecularTexture");
	texturesLocs[4] = glGetUniformLocation(skyboxProgram.id, "skyboxTexture");

	glUseProgram(terrainProgram.id);
	glUniform1i(texturesLocs[0], textures[0]);
	glUseProgram(earthProgram.id);
	glUniform1i(texturesLocs[1], textures[1]);
	glUniform1i(texturesLocs[2], textures[2]);
	glUniform1i(texturesLocs[3], textures[3]);
	glUseProgram(skyboxProgram.id);
	glUniform1i(texturesLocs[4], textures[4]);
}

static void setupLighting(const ShaderProgram *program)
{
	glUseProgram(program->id);
	glUniform3fv(requiredUniformLocation(program->id, "ambientLight"), 1, ambientLight);
	glUniform3fv(requiredUniformLocation(program->id, "diffuseLight"), 1, diffuseLight);
	glUniform3fv(requiredUniformLocation(program->id, "lightPosition"), 1, lightPosition);
	glUniform3fv(requiredUniformLocation(program->id, "materialA"), 1, materialA);
	glUniform3fv(requiredUniformLocation(program->id, "materialD"), 1, materialD);
	glUniform3fv(requiredUniformLocation(program->id, "materialS"), 1, materialS);
	glUniform1f(requiredUniformLocation(program->id, "exponent"), exponent);
}

static bool initShaders()
{
	if (!createShaderProgram(&terrainProgram, "shaders/projection.vsh", "shaders/color.fsh"))
		return false;
	setupLighting(&terrainProgram);

	if (!createShaderProgram(&earthProgram, "shaders/earth.vsh", "shaders/earth.fsh"))
		return false;
	setupLighting(&earthProgram);

	if (!createShaderProgram(&skyboxProgram, "shaders/skybox.vsh", "shaders/skybox.fsh"))
		return false;

	// Setup textures
	setupTextures();

	glEnable(GL_DEPTH_TEST);
	//	glEnable(GL_CULL_FACE);
	//	glFrontFace(GL_CW);
	return true;
}

static void move()
{
	Vertex forwardStep = playerForwardStep(player);

	if (keys['w'])
	{
		float turn = 0;
		if (keys['a'])
		{
			turn += 2;
		}
		if (keys['d'])
		{
			turn -= 2;
		}
		playerTurn(player, turn);
		rover.rotateWheels(1);
		rover.setYawRotation(player->yaw);
		playerMoveForward(player, forwardStep);
	}

	if (keys['s'])
	{
		float turn = 0;
		if (keys['a'])
		{
			turn -= 2;
		}
		if (keys['d'])
		{
			turn += 2;
		}
		playerTurn(player, turn);
		rover.rotateWheels(0);
		rover.setYawRotation(player->yaw);
		playerMoveBackward(player, forwardStep);
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

	cameraFollowPlayerYaw(camera);
}

static void drawTerrain(int offsetX, int offsetZ)
{
	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			mIdentity(&modelMatrix);
			translate(&modelMatrix, (i + offsetX) * SIDE_LENGTH_X, 0, (j + offsetZ) * SIDE_LENGTH_Z);
			glUniformMatrix4fv(terrainProgram.modelMatrixLoc, 1, GL_TRUE, modelMatrix.values);
			terrain_draw(terrain);
		}
	}
}

static void useProgramWithCameraMatrices(const ShaderProgram *program)
{
	glUseProgram(program->id);
	glUniformMatrix4fv(program->projMatrixLoc, 1, GL_TRUE, projectionMatrix.values);
	mIdentity(&viewMatrix);
	glUniform3f(program->cameraLoc, camera->position.x, camera->position.y, camera->position.z);
	rotateX(&viewMatrix, -camera->pitch);
	rotateY(&viewMatrix, -camera->yaw);
	translate(&viewMatrix, -camera->position.x, -camera->position.y, -camera->position.z);
	glUniformMatrix4fv(program->viewMatrixLoc, 1, GL_TRUE, viewMatrix.values);
}

static void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	calculateCameraPosition(camera);
	move();

	// ----------------- MVP to the terrain program
	useProgramWithCameraMatrices(&terrainProgram);

	// Draw an object to build third person view from it
	mIdentity(&modelMatrix);

	player->position.y = vertexFromXZPosition(terrain, player->position.x, player->position.z).y;
	translate(&modelMatrix, player->position.x, player->position.y, player->position.z);

	Vertex normalInXZ = normalFromXZPosition(terrain, player->position.x, player->position.z);
	glUniformMatrix4fv(terrainProgram.modelMatrixLoc, 1, GL_TRUE, modelMatrix.values);

	rover.setPosition(player->position.x, player->position.y + 0.164, player->position.z);

	float roverPitch = toDeg(atan(normalInXZ.z / -normalInXZ.y));
	float roverRoll = toDeg(atan(normalInXZ.x / -normalInXZ.y));

	float finalRoverPitch = -sin(toRadians(player->yaw)) * roverRoll - cos(toRadians(player->yaw)) * roverPitch;
	float finalRoverRoll = cos(toRadians(player->yaw)) * roverRoll - sin(toRadians(player->yaw)) * roverPitch;

	rover.rotateRoverPitch(finalRoverPitch);
	rover.rotateRoverRoll(finalRoverRoll);
	rover.draw(terrainProgram.modelMatrixLoc);

	// Draw Terrain
	mIdentity(&modelMatrix);
	glActiveTexture(GL_TEXTURE0 + 0);
	glUniform1i(texturesLocs[0], 0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	drawTerrain(player->position.x / SIDE_LENGTH_X, player->position.z / SIDE_LENGTH_Z);

	// ---------------- MVP to the earth program
	useProgramWithCameraMatrices(&earthProgram);

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

	translate(&modelMatrix, camera->position.x + 50, 20, camera->position.z + 50);
	rotateX(&modelMatrix, 23.5); // 23Â° It's the approximate inclination of the Earth
	rotateZ(&modelMatrix, -angleEarth);
	glUniformMatrix4fv(earthProgram.modelMatrixLoc, 1, GL_TRUE, modelMatrix.values);
	sphere_draw(earth);

	// ------------ MVP to the skybox program
	useProgramWithCameraMatrices(&skyboxProgram);

	mIdentity(&modelMatrix);
	glActiveTexture(GL_TEXTURE0 + 4);
	glUniform1i(texturesLocs[4], 4);
	glBindTexture(GL_TEXTURE_2D, textures[4]);
	translate(&modelMatrix, player->position.x, 0, player->position.z);
	rotateX(&modelMatrix, 180);
	rotateZ(&modelMatrix, -angleSkybox);
	glUniformMatrix4fv(skyboxProgram.modelMatrixLoc, 1, GL_TRUE, modelMatrix.values);
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

	GLint previousProgram = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &previousProgram);

	const ShaderProgram *programs[] = {&terrainProgram, &earthProgram, &skyboxProgram};
	for (int i = 0; i < 3; i++)
	{
		glUseProgram(programs[i]->id);
		glUniformMatrix4fv(programs[i]->projMatrixLoc, 1, GL_TRUE, projectionMatrix.values);
	}

	glUseProgram(previousProgram);
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
		cameraOrbit(camera, -angleAroundPlayerChange);

		float cameraPitchChange = (ny - lastClickedCoord[1]) * 100;
		// printf("Camera pitch decrement: %f\n", cameraPitchChange);
		cameraPitchBy(camera, cameraPitchChange);

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
		cameraZoom(camera, 0.1);
		break;

	case BACK_WHEEL:
		cameraZoom(camera, -0.1);
		break;

	default:
		break;
	}
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
	if (!initTextures())
		fprintf(stderr, "Warning: some textures were not loaded, the scene will be rendered without them\n");
	if (!initShaders())
	{
		fprintf(stderr, "Error: shaders could not be initialized. Launch the executable from the repository root so that asset paths resolve.\n");
		return EXIT_FAILURE;
	}

	// Init scene set up
	Vertex playerStartPosition = {0, 1, 0};
	Vertex cameraStartPosition = {0, 1.5, 1.0};
	player = createPlayer(playerStartPosition, 0.0, 0.02);
	camera = player == NULL ? NULL : createCamera(player, cameraStartPosition, -30, -1.5, 180);
	if (camera == NULL)
	{
		fprintf(stderr, "Error: could not allocate the player and camera state\n");
		destroyPlayer(player);
		return EXIT_FAILURE;
	}

	terrain = terrain_create(NUM_VERTEX_X, NUM_VERTEX_Z, SIDE_LENGTH_X, SIDE_LENGTH_Z, {1, 1, 1});
	terrain_bind(terrain, terrainProgram.vertexPosLoc, terrainProgram.vertexColLoc, terrainProgram.vertexTexcoordLoc, terrainProgram.vertexNormalLoc);

	earth = sphere_create(7, 40, 40, {1, 1, 1});
	sphere_bind(earth, earthProgram.vertexPosLoc, earthProgram.vertexColLoc, earthProgram.vertexTexcoordLoc, earthProgram.vertexNormalLoc);

	sphereRover = sphere_create(0.3, 40, 40, {1, 1, 1});
	sphere_bind(sphereRover, earthProgram.vertexPosLoc, earthProgram.vertexColLoc, earthProgram.vertexTexcoordLoc, earthProgram.vertexNormalLoc);

	skybox = sphere_create(1500, 40, 40, {1.2, 1.2, 1.2});
	sphere_bind(skybox, skyboxProgram.vertexPosLoc, skyboxProgram.vertexColLoc, skyboxProgram.vertexTexcoordLoc, skyboxProgram.vertexNormalLoc);

	rover.load();
	rover.bind(terrainProgram.id, terrainProgram.vertexPosLoc, terrainProgram.vertexNormalLoc, terrainProgram.vertexColLoc);

	glClearColor(0, 0, 0, 1.0);
	glutMainLoop();

	destroyCamera(camera);
	destroyPlayer(player);
	return 0;
}
