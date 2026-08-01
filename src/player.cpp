#include "player.h"
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static inline float toRadians(float deg) { return deg * M_PI / 180.0; }

Player createPlayer(Vertex position, float yaw, float speed)
{
	Player newPlayer = (Player)malloc(sizeof(PlayerStr));
	if (newPlayer == NULL)
		return NULL;

	newPlayer->position.x = position.x;
	newPlayer->position.y = position.y;
	newPlayer->position.z = position.z;

	newPlayer->yaw = yaw;
	newPlayer->speed = speed;

	return newPlayer;
}

void destroyPlayer(Player player)
{
	free(player);
}

Vertex playerForwardStep(Player player)
{
	float radians = toRadians(player->yaw);
	Vertex step;
	step.x = player->speed * -sin(radians);
	step.y = 0;
	step.z = player->speed * -cos(radians);
	return step;
}

void playerTurn(Player player, float degrees)
{
	player->yaw = normalizeAngle(player->yaw + degrees);
}

void playerMoveForward(Player player, Vertex step)
{
	player->position.x -= step.x;
	player->position.z -= step.z;
}

void playerMoveBackward(Player player, Vertex step)
{
	player->position.x += step.x;
	player->position.z += step.z;
}