#ifndef PLAYER_H
#define PLAYER_H

#include "utils.h"
#include <math.h>

// Shared by the player and the camera to keep accumulating angles inside [0, 360).
static inline float normalizeAngle(float deg)
{
	deg = fmodf(deg, 360.0);
	if (deg < 0.0)
		deg += 360.0;
	return deg;
}

typedef struct
{
	Vertex position;
	float yaw;
	float speed;
} PlayerStr;

typedef PlayerStr *Player;

Player createPlayer(Vertex position, float yaw, float speed);
void destroyPlayer(Player player);

// Displacement of a single step at the current yaw. Read it before turning, so a turn
// taken in the same frame does not affect the step already committed to that frame.
Vertex playerForwardStep(Player player);
void playerTurn(Player player, float degrees);
void playerMoveForward(Player player, Vertex step);
void playerMoveBackward(Player player, Vertex step);

#endif // PLAYER_H