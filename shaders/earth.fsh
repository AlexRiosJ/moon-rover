#version 400

in mat4 modelMatrixToFS;
in vec4 vertexWorldPosition;
in vec3 vertexNormalToFS;
in vec3 vertexColorToFS;
in vec2 vertexTexcoordToFS1;
in vec2 vertexTexcoordToFS2;
in vec2 vertexTexcoordToFS3;

uniform sampler2D earthTexture;
uniform sampler2D earthCloudsTexture;
uniform sampler2D earthSpecularTexture;

uniform vec3 ambientLight;
uniform vec3 materialA;

uniform vec3 diffuseLight;
uniform vec3 lightPosition;
uniform vec3 materialD;

uniform vec3 materialS; 
uniform float exponent;
uniform vec3 camera;

out vec4 pixelColor; 

void main() {
	vec3 vertexToLight = normalize(lightPosition - vertexWorldPosition.xyz);

    mat4 G = transpose(inverse(modelMatrixToFS));
    vec4 worldNormal = G * vec4(vertexNormalToFS, 0);
    vec3 unitaryNormal = normalize(worldNormal.xyz);
    float factorD = clamp(dot(vertexToLight, unitaryNormal), 0.0, 1.0);

    vec3 vertexToCamera = normalize(camera - vertexWorldPosition.xyz);
    vec3 reflected = normalize(2 * unitaryNormal * dot(unitaryNormal, vertexToLight) - vertexToLight);
    float factorS = clamp(pow(dot(reflected, vertexToCamera), exponent), 0.0, 1.0);

	vec3 tempColor;

    tempColor = ambientLight * materialA + diffuseLight * (materialD * factorD + materialS * factorS * texture(earthSpecularTexture, vertexTexcoordToFS3).xyz * 10);
    tempColor = clamp(tempColor * vertexColorToFS, 0.0, 1.0);

    pixelColor = vec4(tempColor * vec3(3, 3, 3), 1) * mix(texture(earthTexture, vertexTexcoordToFS1), texture(earthCloudsTexture, vertexTexcoordToFS2), 0.5);
}