#include "utils.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const char *loadShader(const char *filename)
{
	FILE *shaderFile = fopen(filename, "r");
	if (shaderFile == NULL)
	{
		fprintf(stderr, "Error: could not open shader file \"%s\". Run the executable from the repository root.\n", filename);
		return NULL;
	}
	const int BUFFER_SIZE = 256;
	char buffer[BUFFER_SIZE];
	//	Count number of characters in source file
	int charCount = 0;
	int i = 0;
	while (!feof(shaderFile))
	{
		if (fgets(buffer, BUFFER_SIZE, shaderFile) == NULL)
			continue;
		i = 0;
		while (buffer[i++] != '\0')
			charCount++;
	}
	charCount++; // consider end-of-string character

	//	Copy characters to new char array
	rewind(shaderFile);
	char *shaderData = (char *)malloc(sizeof(char) * charCount);
	if (shaderData == NULL)
	{
		fprintf(stderr, "Error: out of memory while loading shader file \"%s\"\n", filename);
		fclose(shaderFile);
		return NULL;
	}
	int charIndex = 0;
	while (!feof(shaderFile))
	{
		if (fgets(buffer, BUFFER_SIZE, shaderFile) == NULL)
			continue;
		i = 0;
		while (buffer[i] != '\0')
			shaderData[charIndex++] = buffer[i++];
	}
	shaderData[charIndex] = '\0';

	fclose(shaderFile);
	return shaderData;
}

GLuint compileShader(const char *filename, GLuint shaderType)
{
	//	char const* source = "void main() { ... ";
	char const *source = loadShader(filename);
	if (source == NULL)
	{
		fprintf(stderr, "Error: shader \"%s\" was not compiled because its source could not be loaded\n", filename);
		return 0;
	}
	GLuint shaderId = glCreateShader(shaderType);
	glShaderSource(shaderId, 1, &source, NULL);
	glCompileShader(shaderId);
	free((void *)source);
	return shaderId;
}

bool shaderCompiled(GLuint shaderId)
{
	if (shaderId == 0)
		return false;

	GLint params;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &params);
	if (params == GL_TRUE)
		return true;

	GLint maxLength = 0;
	glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength);
	if (maxLength > 0)
	{
		GLchar *errors = (GLchar *)malloc(sizeof(GLchar) * maxLength);
		if (errors != NULL)
		{
			glGetShaderInfoLog(shaderId, maxLength, &maxLength, errors);
			puts(errors);
			free(errors);
		}
	}
	return false;
}

bool loadBMP(const char *filename, unsigned char **pdata, unsigned int *width, unsigned int *height)
{
	unsigned char header[54];
	*pdata = NULL;
	*width = 0;
	*height = 0;

	FILE *file = fopen(filename, "rb");
	if (file == NULL)
	{
		fprintf(stderr, "Error: could not open texture file \"%s\". Run the executable from the repository root.\n", filename);
		return false;
	}
	if (fread(header, 1, 54, file) != 54)
	{
		fprintf(stderr, "Error: texture file \"%s\" is too short to contain a BMP header\n", filename);
		fclose(file);
		return false;
	}
	if (header[0] != 'B' || header[1] != 'M')
	{
		fprintf(stderr, "Error: texture file \"%s\" is not a correct BMP file\n", filename);
		fclose(file);
		return false;
	}
	unsigned char *bytePointerW = &(header[0x12]);
	unsigned int *fourBytesPointerW = (unsigned int *)bytePointerW;
	unsigned int bmpWidth = *fourBytesPointerW;
	unsigned char *bytePointerH = &(header[0x16]);
	unsigned int *fourBytesPointerH = (unsigned int *)bytePointerH;
	unsigned int bmpHeight = *fourBytesPointerH;
	if (bmpWidth == 0 || bmpHeight == 0)
	{
		fprintf(stderr, "Error: texture file \"%s\" declares an empty image (%u x %u)\n", filename, bmpWidth, bmpHeight);
		fclose(file);
		return false;
	}

	size_t dataSize = (size_t)bmpWidth * (size_t)bmpHeight * 3;
	unsigned char *data = (unsigned char *)malloc(sizeof(unsigned char) * dataSize);
	if (data == NULL)
	{
		fprintf(stderr, "Error: out of memory while loading texture file \"%s\"\n", filename);
		fclose(file);
		return false;
	}
	if (fread(data, 1, dataSize, file) != dataSize)
	{
		fprintf(stderr, "Error: texture file \"%s\" holds less pixel data than its header declares\n", filename);
		free(data);
		fclose(file);
		return false;
	}
	fclose(file);

	*pdata = data;
	*width = bmpWidth;
	*height = bmpHeight;
	return true;
}

void processArrayBuffer(GLuint bufferId, void *array, int arraySize, GLuint loc, int size, int type)
{
	glBindBuffer(GL_ARRAY_BUFFER, bufferId);
	glBufferData(GL_ARRAY_BUFFER, arraySize, array, GL_STATIC_DRAW);
	glVertexAttribPointer(loc, size, type, 0, 0, 0);
	glEnableVertexAttribArray(loc);
}

void processIndexBuffer(GLuint bufferId, void *array, int arraySize, int restartIndex)
{
	glBindBuffer(GL_ARRAY_BUFFER, bufferId);
	glBufferData(GL_ARRAY_BUFFER, arraySize, array, GL_STATIC_DRAW);
	glPrimitiveRestartIndex(restartIndex);
	glEnable(GL_PRIMITIVE_RESTART);
}

Vertex subtractVertex(Vertex origin, Vertex dest)
{
	return {dest.x - origin.x, dest.y - origin.y, dest.z - origin.z};
}

Vertex crossProduct(Vertex A, Vertex B)
{
	return {A.y * B.z - A.z * B.y, -(A.x * B.z - A.z * B.x), A.x * B.y - A.y * B.x};
}

Vertex normalize(Vertex v)
{
	float mag = sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
	return {v.x / mag, v.y / mag, v.z / mag};
}