#include <shapes/Rover.hpp>
#include <GL/glew.h>
#include <transforms.h>
#include <mat4.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Rover::Rover()
{

	this->x = 0.0;
	this->y = 0.0;
	this->z = 0.0;
	this->radY = 0.0;

	this->wheelAngle = 0.0;
	this->currentTurnAnimation = 0.0;

	// Wheels X offsets
	this->wheelLeftMidOffsetX = 0.3005;
	this->wheelRightMidOffsetX = -0.3005;
	this->wheelLeftBackOffsetX = 0.27;
	this->wheelRightBackOffsetX = -0.27;
	this->wheelLeftFrontOffsetX = 0.27;
	this->wheelRightFrontOffsetX = -0.27;

	// Bogie animations
	this->leftBogieDeg = 0.0;
	this->rightBogieDeg = 0.0;

	// Wheel Z offsets
	this->wheelsBackOffsetZ = -0.253;
	this->wheelsMidOffsetZ = 0.002;
	this->wheelsFrontOffsetZ = 0.285;
}

Rover::~Rover()
{
}

void Rover::load()
{
	this->chasis.Load("./meshes/body.obj");
	this->rockerLeft.Load("./meshes/rocker.obj");
	this->bogieLeft.Load("./meshes/bogie.obj");
	this->driveLeftFront.Load("./meshes/drive.obj");
	this->driveLeftBack.Load("./meshes/drive.obj");
	this->wheelLeftFront.Load("./meshes/wheel.obj");
	this->wheelLeftMid.Load("./meshes/wheel.obj");
	this->wheelLeftBack.Load("./meshes/wheel.obj");
	this->rockerRight.Load("./meshes/rocker_m.obj");
	this->bogieRight.Load("./meshes/bogie_m.obj");
	this->driveRightFront.Load("./meshes/drive_m.obj");
	this->driveRightBack.Load("./meshes/drive_m.obj");
	this->wheelRightFront.Load("./meshes/wheel_m.obj");
	this->wheelRightMid.Load("./meshes/wheel_m.obj");
	this->wheelRightBack.Load("./meshes/wheel_m.obj");
}

void Rover::bind(GLuint programId, GLuint vertexLocation, GLuint normalLocation, GLuint colorLocation)
{
	this->chasis.Bind(programId, vertexLocation, normalLocation, colorLocation);
	this->rockerLeft.Bind(programId, vertexLocation, normalLocation, colorLocation);
	this->bogieLeft.Bind(programId, vertexLocation, normalLocation, colorLocation);
	this->driveLeftFront.Bind(programId, vertexLocation, normalLocation, colorLocation);
	this->driveLeftBack.Bind(programId, vertexLocation, normalLocation, colorLocation);
	this->wheelLeftFront.Bind(programId, vertexLocation, normalLocation, colorLocation);
	this->wheelLeftMid.Bind(programId, vertexLocation, normalLocation, colorLocation);
	this->wheelLeftBack.Bind(programId, vertexLocation, normalLocation, colorLocation);
	this->rockerRight.Bind(programId, vertexLocation, normalLocation, colorLocation);
	this->bogieRight.Bind(programId, vertexLocation, normalLocation, colorLocation);
	this->driveRightFront.Bind(programId, vertexLocation, normalLocation, colorLocation);
	this->driveRightBack.Bind(programId, vertexLocation, normalLocation, colorLocation);
	this->wheelRightFront.Bind(programId, vertexLocation, normalLocation, colorLocation);
	this->wheelRightMid.Bind(programId, vertexLocation, normalLocation, colorLocation);
	this->wheelRightBack.Bind(programId, vertexLocation, normalLocation, colorLocation);
}

void Rover::draw(GLuint modelLoc)
{
	Mat4 modelMatrix;

	mIdentity(&modelMatrix);
	translate(&modelMatrix, this->x, this->y, this->z);

	rotateY(&modelMatrix, this->angleY);
	rotateX(&modelMatrix, this->angleX);
	rotateZ(&modelMatrix, this->angleZ);

	pushMatrix(&modelMatrix);

	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	chasis.Draw();

	translate(&modelMatrix, 0.15620, 0.05507, 0.07490);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	rockerLeft.Draw();

	pushMatrix(&modelMatrix);

	translate(&modelMatrix, 0.0247, -0.06163, -0.18007);
	rotateX(&modelMatrix, this->leftBogieDeg);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	bogieLeft.Draw();

	pushMatrix(&modelMatrix);
	translate(&modelMatrix, 0.1196, -0.09744, 0.10717);
	rotateX(&modelMatrix, this->wheelAngle);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	wheelLeftMid.Draw();

	popMatrix(&modelMatrix);
	translate(&modelMatrix, 0.0791, -0.09844, -0.14783);
	rotateY(&modelMatrix, -currentTurnAnimation);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	driveLeftBack.Draw();

	translate(&modelMatrix, 0.01, 0.0, 0.0);
	rotateX(&modelMatrix, this->wheelAngle);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	wheelLeftBack.Draw();

	popMatrix(&modelMatrix);
	translate(&modelMatrix, 0.1038, -0.16007, 0.2101);
	rotateY(&modelMatrix, currentTurnAnimation);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	driveLeftFront.Draw();

	translate(&modelMatrix, 0.01, 0.0, 0.0);
	rotateX(&modelMatrix, this->wheelAngle);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	wheelLeftFront.Draw();

	popMatrix(&modelMatrix);
	translate(&modelMatrix, -0.15620, 0.05507, 0.07490);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	rockerRight.Draw();

	pushMatrix(&modelMatrix);

	translate(&modelMatrix, -0.0247, -0.06163, -0.18007);
	rotateX(&modelMatrix, this->rightBogieDeg);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	bogieRight.Draw();

	pushMatrix(&modelMatrix);
	translate(&modelMatrix, -0.1196, -0.09744, 0.10717);
	rotateX(&modelMatrix, this->wheelAngle);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	wheelRightMid.Draw();

	popMatrix(&modelMatrix);
	translate(&modelMatrix, -0.0791, -0.09844, -0.14783);
	rotateY(&modelMatrix, -currentTurnAnimation);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	driveRightBack.Draw();

	translate(&modelMatrix, -0.01, 0.0, 0.0);
	rotateX(&modelMatrix, this->wheelAngle);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	wheelRightBack.Draw();

	popMatrix(&modelMatrix);
	translate(&modelMatrix, -0.1038, -0.16007, 0.2101);
	rotateY(&modelMatrix, currentTurnAnimation);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	driveRightFront.Draw();

	translate(&modelMatrix, -0.01, 0.0, 0.0);
	rotateX(&modelMatrix, this->wheelAngle);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	wheelRightFront.Draw();
}

void Rover::setPosition(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

void Rover::getWheelLeftBackXZPosition(float *coord)
{
	coord[0] = this->x + cosf(this->radY) * this->wheelLeftBackOffsetX;
	coord[1] = this->z + sinf(this->radY) * this->wheelsBackOffsetZ;
}

void Rover::getWheelLeftMidXZPosition(float *coord)
{
	coord[0] = this->x + cosf(this->radY) * this->wheelLeftMidOffsetX;
	coord[1] = this->z + sinf(this->radY) * this->wheelsBackOffsetZ;
}

void Rover::getWheelLeftFrontXZPosition(float *coord)
{
	coord[0] = this->x + cosf(this->radY) * this->wheelLeftFrontOffsetX;
	coord[1] = this->z + sinf(this->radY) * this->wheelsBackOffsetZ;
}

void Rover::getWheelRightBackXZPosition(float *coord)
{
	coord[0] = this->x + cosf(this->radY) * this->wheelRightBackOffsetX;
	coord[1] = this->z + sinf(this->radY) * this->wheelsBackOffsetZ;
}

void Rover::getWheelRightMidXZPosition(float *coord)
{
	coord[0] = this->x + cosf(this->radY) * this->wheelRightMidOffsetX;
	coord[1] = this->z + sinf(this->radY) * this->wheelsBackOffsetZ;
}

void Rover::getWheelRightFrontXZPosition(float *coord)
{
	coord[0] = this->x + cosf(this->radY) * this->wheelRightFrontOffsetX;
	coord[1] = this->z + sinf(this->radY) * this->wheelsBackOffsetZ;
}

void Rover::rotateWheels(int forward)
{
	float direction = forward ? 1.0 : -1.0;
	this->wheelAngle += 3.5 * direction;
	if (this->wheelAngle >= 360.0)
	{
		this->wheelAngle = 0.0;
	}
}

void Rover::turnWheels(float angle)
{
	int direction = angle >= 0.0 ? 1 : 0;
	if (direction && this->currentTurnAnimation <= angle)
	{
		this->currentTurnAnimation += 2;
		return;
	}

	if (!direction && this->currentTurnAnimation >= angle)
	{
		this->currentTurnAnimation -= 2;
		return;
	}
}

void Rover::resetTurnWheels()
{
	int direction = this->currentTurnAnimation > 0.0 ? 1 : 0;

	if (direction && this->currentTurnAnimation != 0)
	{
		this->currentTurnAnimation -= 2;
		return;
	}

	if (!direction && this->currentTurnAnimation != 0)
	{
		this->currentTurnAnimation += 2;
		return;
	}
}

void Rover::rotateRover(int clock)
{
	float direction = clock ? 1.0 : -1.0;
	this->angleY += 0.25 * direction;
	if(this->angleY >= 360.0)
	{
		this->angleY = 0.0;
	}
	this->radY = this->angleY * M_PI / 180;
}

void Rover::setWheelsY(float lf, float lm, float lb, float rf, float rm, float rb)
{
}

void Rover::rotateRoverPitch(float deg)
{
	this->angleX = deg;
}

void Rover::rotateRoverRoll(float deg)
{
	this->angleZ = deg;
}

void Rover::setYawRotation(float angle)
{
	this->angleY = angle;
}