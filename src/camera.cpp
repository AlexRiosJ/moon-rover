#include "camera.h"
#include <stdlib.h>
#include <stdio.h>

static inline float toRadians(float deg) {return deg *M_PI / 180.0 ;}


Camera createCamera(Player player, Vertex position, float pitch, float yaw, float roll, float speed, 
                                                   float sensiblity, float distanceFromPlayer, float angleAroundPlayer)
{

	Camera camera = (Camera) malloc(sizeof(CameraStr));

	camera->player = player;

	camera->position.x = position.x;
	camera->position.y = position.y;
	camera->position.z = position.z;

	camera->pitch = pitch / sensiblity;
	camera->yaw   = yaw / sensiblity;
	camera->roll  = roll / sensiblity;
	camera->speed = speed;
	camera->sensibility = sensiblity;
	camera->distanceFromPlayer = distanceFromPlayer;
	camera->angleAroundPlayer = angleAroundPlayer;


	return camera;
}

void calculateCameraPosition(Camera camera) 
{
	float horizontalDistance = camera->distanceFromPlayer * cos(toRadians(camera->pitch));
	float verticalDistance   = camera->distanceFromPlayer * sin(toRadians(camera->pitch)); 

	float theta =  camera->player->yaw + camera->angleAroundPlayer;
	float offsetX = horizontalDistance * sin(toRadians(theta));
	float offsetZ = horizontalDistance * cos(toRadians(theta));
	camera->position.x = camera->player->position.x - offsetX;
	camera->position.z = camera->player->position.z - offsetZ;
	camera->position.y = camera->player->position.y + verticalDistance;

}

void printCamera(Camera camera) 
{
	printf("Position: {x: %f, y: %f, z: %f}\n", camera->position.x, camera->position.y, camera->position.z);
}