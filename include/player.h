#ifndef PLAYER_H
#define PLAYER_H

#include "utils.h"
#include <math.h>

typedef struct {
	Vertex position;
	float pitch;
	float yaw;
	float roll;
	float speed;
}PlayerStr;

typedef PlayerStr* Player;

Player createPlayer(Vertex position, float pitch, float yaw, float roll, float speed);
void printPlayer(Player player);

#endif  // PERLIN_H