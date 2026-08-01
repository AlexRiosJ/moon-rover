#include "camera.h"
#include <stdlib.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static inline float toRadians(float deg) { return deg * M_PI / 180.0; }

Camera createCamera(Player player, Vertex position, float pitch, float distanceFromPlayer, float angleAroundPlayer)
{
	Camera camera = (Camera)malloc(sizeof(CameraStr));
	if (camera == NULL)
		return NULL;

	camera->player = player;

	camera->position.x = position.x;
	camera->position.y = position.y;
	camera->position.z = position.z;

	camera->pitch = pitch;
	camera->distanceFromPlayer = distanceFromPlayer;
	camera->angleAroundPlayer = angleAroundPlayer;
	cameraFollowPlayerYaw(camera);

	return camera;
}

void destroyCamera(Camera camera)
{
	free(camera);
}

void calculateCameraPosition(Camera camera)
{
	float horizontalDistance = camera->distanceFromPlayer * cos(toRadians(camera->pitch));
	float verticalDistance = camera->distanceFromPlayer * sin(toRadians(camera->pitch));

	float theta = camera->player->yaw + camera->angleAroundPlayer;
	float offsetX = horizontalDistance * sin(toRadians(theta));
	float offsetZ = horizontalDistance * cos(toRadians(theta));
	camera->position.x = camera->player->position.x - offsetX;
	camera->position.z = camera->player->position.z - offsetZ;
	camera->position.y = camera->player->position.y + verticalDistance;
}

void cameraFollowPlayerYaw(Camera camera)
{
	camera->yaw = camera->player->yaw + camera->angleAroundPlayer;
}

void cameraOrbit(Camera camera, float degrees)
{
	camera->angleAroundPlayer = normalizeAngle(camera->angleAroundPlayer + degrees);
}

void cameraPitchBy(Camera camera, float degrees)
{
	float pitch = camera->pitch + degrees;
	if (pitch <= CAMERA_PITCH_LOWER_LIMIT || pitch > CAMERA_PITCH_UPPER_LIMIT)
		return;
	camera->pitch = pitch;
}

void cameraZoom(Camera camera, float distanceDelta)
{
	camera->distanceFromPlayer += distanceDelta;
	if (camera->distanceFromPlayer > CAMERA_DISTANCE_NEAR)
		camera->distanceFromPlayer = CAMERA_DISTANCE_NEAR;
	else if (camera->distanceFromPlayer < CAMERA_DISTANCE_FAR)
		camera->distanceFromPlayer = CAMERA_DISTANCE_FAR;
}