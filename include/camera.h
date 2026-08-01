#ifndef CAMERA_H
#define CAMERA_H

#include "utils.h"
#include "player.h"
#include <math.h>

// distanceFromPlayer is negative, so the near limit is the greater value.
#define CAMERA_DISTANCE_NEAR -0.5
#define CAMERA_DISTANCE_FAR -10.0

// The camera always looks down at the player, so the pitch stays inside these limits.
#define CAMERA_PITCH_LOWER_LIMIT -90
#define CAMERA_PITCH_UPPER_LIMIT -5

typedef struct
{
	Player player;
	Vertex position;
	float pitch;
	float yaw;
	float distanceFromPlayer;
	float angleAroundPlayer;
} CameraStr;

typedef CameraStr *Camera;

Camera createCamera(Player player, Vertex position, float pitch, float distanceFromPlayer, float angleAroundPlayer);
void destroyCamera(Camera camera);

void calculateCameraPosition(Camera camera);
// Keeps the view direction locked behind the player as it turns.
void cameraFollowPlayerYaw(Camera camera);
void cameraOrbit(Camera camera, float degrees);
void cameraPitchBy(Camera camera, float degrees);
void cameraZoom(Camera camera, float distanceDelta);

#endif // CAMERA_H