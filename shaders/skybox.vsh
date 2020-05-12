#version 400

in vec3 vertexPosition;
in vec3 vertexColor;
in vec2 vertexTexcoord;
in vec3 vertexNormal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out mat4 modelMatrixToFS;
out vec4 vertexWorldPosition;
out vec3 vertexNormalToFS;
out vec3 vertexColorToFS;
out vec2 vertexTexcoordToFS;

void main() {
    vec4 worldPosition = modelMatrix * vec4(vertexPosition, 1);
    gl_Position = projectionMatrix * viewMatrix * worldPosition;

    modelMatrixToFS = modelMatrix;
    vertexWorldPosition = worldPosition;
    vertexNormalToFS = vertexNormal;
    vertexColorToFS = vertexColor;
    vertexTexcoordToFS = vertexTexcoord;
}