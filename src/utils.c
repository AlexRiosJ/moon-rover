#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

const char* loadShader(const char* filename) {
	FILE* shaderFile = fopen(filename, "r");
	const int BUFFER_SIZE = 256;
	char buffer[BUFFER_SIZE];
//	Count number of characters in source file
	int charCount = 0;
	int i = 0;
	while(!feof(shaderFile)) {
		if(fgets(buffer, BUFFER_SIZE, shaderFile) == NULL) continue;
		i = 0;
		while(buffer[i ++] != '\0') charCount ++;
	}
	charCount ++; // consider end-of-string character

//	Copy characters to new char array
	rewind(shaderFile);
	char* shaderData = (char*) malloc(sizeof(char) * charCount);
	int charIndex = 0;
	while(!feof(shaderFile)) {
		if(fgets(buffer, BUFFER_SIZE, shaderFile) == NULL) continue;
		i = 0;
		while(buffer[i] != '\0') shaderData[charIndex ++] = buffer[i ++];
	}
	shaderData[charIndex] = '\0';

	fclose(shaderFile);
	return shaderData;
}

GLuint compileShader(const char* filename, GLuint shaderType) {
//	char const* source = "void main() { ... ";
	char const* source = loadShader(filename);
	GLuint shaderId = glCreateShader(shaderType);
	glShaderSource(shaderId, 1, &source, NULL);
	glCompileShader(shaderId);
	return shaderId;
}

bool shaderCompiled(GLuint shaderId) {
	GLint params;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &params);
	if(params == 1) return true;

	GLint maxLength = 0;
	glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength);
	GLchar* errors = (GLchar*) malloc(sizeof(int) * maxLength);
	glGetShaderInfoLog(shaderId, maxLength, &maxLength, errors);
	puts(errors);
	return false;
}

bool loadBMP(const char* filename, unsigned char **pdata, unsigned int *width, unsigned int *height) {
    unsigned char header[54];
    FILE * file = fopen(filename, "rb");
    fread(header, 1, 54, file);
    if(header[0] != 'B' || header[1] != 'M' ) {
        printf("Not a correct BMP file\n");
        return false;
    }
    unsigned char *bytePointerW      = &(header[0x12]);
    unsigned int  *fourBytesPointerW = (unsigned int*) bytePointerW;
    *width = *fourBytesPointerW;
    unsigned char *bytePointerH      = &(header[0x16]);
    unsigned int  *fourBytesPointerH = (unsigned int*) bytePointerH;
    *height = *fourBytesPointerH;
    *pdata = (unsigned char*) malloc(sizeof(unsigned char) * (*width) * (*height) * 3);
    fread(*pdata, 1, (*width) * (*height) * 3, file);
    fclose(file);
    return true;
}

void processArrayBuffer(GLuint bufferId, void* array, int arraySize, GLuint loc, int size, int type)
{
    glBindBuffer(GL_ARRAY_BUFFER, bufferId);
    glBufferData(GL_ARRAY_BUFFER, arraySize, array, GL_STATIC_DRAW);
	glVertexAttribPointer(loc, size, type, 0, 0, 0);
	glEnableVertexAttribArray(loc);
}

void processIndexBuffer(GLuint bufferId, void* array, int arraySize, int restartIndex)
{
    glBindBuffer(GL_ARRAY_BUFFER, bufferId);
    glBufferData(GL_ARRAY_BUFFER, arraySize, array, GL_STATIC_DRAW);
	glPrimitiveRestartIndex(restartIndex);
	glEnable(GL_PRIMITIVE_RESTART);
}
