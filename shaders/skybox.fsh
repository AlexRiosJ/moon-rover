#version 400

in mat4 modelMatrixToFS;
in vec4 vertexWorldPosition;
in vec3 vertexNormalToFS;
in vec3 vertexColorToFS;
in vec2 vertexTexcoordToFS;

uniform sampler2D skyboxTexture;

out vec4 pixelColor;

void main() {
    pixelColor = vec4(vertexColorToFS, 1) * texture(skyboxTexture, vertexTexcoordToFS);
}