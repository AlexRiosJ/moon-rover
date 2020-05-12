#include <shapes/Rover.hpp>
#include <GL/glew.h>
#include <transforms.h>
#include <mat4.h>

Rover::Rover()
{
    this->angleX = 0;
    this->angleY = 0;
    this->angleZ = 0;
}

Rover::~Rover()
{

}

void Rover::Load(){
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

void Rover::Bind(GLuint programId, GLuint vertexLocation, GLuint normalLocation, GLuint colorLocation)
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

void Rover::Draw(GLuint modelLoc)
{
    Mat4 modelMatrix;

    mIdentity(&modelMatrix);
	translate(&modelMatrix, 0, 0, -0.8);
	
	rotateX(&modelMatrix, this->angleX -= 0.2);
	rotateY(&modelMatrix, this->angleY -= 0.2);
	rotateZ(&modelMatrix, this->angleZ -= 0.2);
	
	pushMatrix(&modelMatrix);

	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	chasis.Draw();

	translate(&modelMatrix, 0.15620, 0.05507, 0.07490);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	rockerLeft.Draw();

	pushMatrix(&modelMatrix);
	
	translate(&modelMatrix, 0.0247, -0.06163, -0.18007);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	bogieLeft.Draw();

	pushMatrix(&modelMatrix);
	translate(&modelMatrix, 0.1196, -0.09744, 0.10717);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	wheelLeftMid.Draw();

	popMatrix(&modelMatrix);
	translate(&modelMatrix, 0.0791, -0.09844, -0.14783);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	driveLeftBack.Draw();

	translate(&modelMatrix, 0.01, 0.0, 0.0);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	wheelLeftBack.Draw();

	popMatrix(&modelMatrix);
	translate(&modelMatrix, 0.1038, -0.16007, 0.2101);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	driveLeftFront.Draw();

	translate(&modelMatrix, 0.01, 0.0, 0.0);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	wheelLeftFront.Draw();

	popMatrix(&modelMatrix);
	translate(&modelMatrix, -0.15620, 0.05507, 0.07490);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	rockerRight.Draw();

	pushMatrix(&modelMatrix);

	translate(&modelMatrix, -0.0247, -0.06163, -0.18007);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	bogieRight.Draw();

	pushMatrix(&modelMatrix);
	translate(&modelMatrix, -0.1196, -0.09744, 0.10717);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	wheelRightMid.Draw();

	popMatrix(&modelMatrix);
	translate(&modelMatrix, -0.0791, -0.09844, -0.14783);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	driveRightBack.Draw();

	translate(&modelMatrix, -0.01, 0.0, 0.0);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	wheelRightBack.Draw();

	popMatrix(&modelMatrix);
	translate(&modelMatrix, -0.1038, -0.16007, 0.2101);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	driveRightFront.Draw();

	translate(&modelMatrix, -0.01, 0.0, 0.0);
	glUniformMatrix4fv(modelLoc, 1, 1, modelMatrix.values);
	wheelRightFront.Draw();
}
