#ifndef CAMERA_H
#define CAMERA_H

#include "utils.h"
#include "player.h"
#include <math.h>

typedef struct {
	Player player;
	Vertex position;
	float pitch;
	float yaw;
	float roll;
	float speed;
	float sensibility;
	float distanceFromPlayer;
	float angleAroundPlayer;
}CameraStr;

typedef CameraStr* Camera;

Camera createCamera(Player player, Vertex position, float pitch, float yaw, float roll, float speed, 
                                                   float sensiblity, float distanceFromPlayer, float angleAroundPlayer);
void calculateCameraPosition(Camera);
void printCamera(Camera);

#endif  // PERLIN_H