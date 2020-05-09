#version 400

in vec3 vertexPosition;
in vec3 vertexColor;
in vec3 vertexNormal;

uniform mat4 modelMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

uniform vec3 ambientLight;
uniform vec3 materialA;

uniform vec3 diffuseLight;
uniform vec3 lightPosition;
uniform vec3 materialD;

uniform vec3 materialS; 
uniform float exponent;
uniform vec3 camera;

out vec3 vertexColorToFS;

void main() {
  vec4 worldPosition = modelMatrix * vec4(vertexPosition, 1);
  gl_Position = projectionMatrix * viewMatrix * worldPosition;

  // Modelo de Phong implementado a nivel de vertice (Gouraud)
  vec3 vertexToLight = normalize(lightPosition - worldPosition.xyz);

  mat4 G = transpose(inverse(modelMatrix));
  vec4 worldNormal = G * vec4(vertexNormal, 0);
  vec3 unitaryNormal = normalize(worldNormal.xyz);
  float factorD = clamp(dot(vertexToLight, unitaryNormal), 0.0, 1.0);

  vec3 vertexToCamera = normalize(camera - worldPosition.xyz);
  vec3 reflected = normalize(2 * unitaryNormal * dot(unitaryNormal, vertexToLight) - vertexToLight);
  float factorS = clamp(pow(dot(reflected, vertexToCamera), exponent), 0.0, 1.0);

  vertexColorToFS = ambientLight * materialA + diffuseLight * (materialD * factorD + materialS * factorS);
  vertexColorToFS = clamp(vertexColorToFS * vertexColor, 0.0, 1.0);
  
  // vertexColorToFS = vertexColor;
}
