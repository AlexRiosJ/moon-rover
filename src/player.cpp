#include "player.h"
#include <stdlib.h>

Player createPlayer(Vertex position, float pitch, float yaw, float roll, float speed)
{
	Player newPlayer = (Player) malloc(sizeof(PlayerStr));
    newPlayer->position.x = position.x;
    newPlayer->position.y = position.y;
    newPlayer->position.z = position.z;

    newPlayer->pitch = pitch;
    newPlayer->yaw = yaw;
    newPlayer->roll = roll;
    newPlayer->speed = speed;

	return newPlayer;
}

void printPlayer(Player player)
{
    
}