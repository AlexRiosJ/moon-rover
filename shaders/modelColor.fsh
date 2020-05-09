#version 400

in vec3 vertexColorToFS; 
out vec4 pixelColor; 

void main() { 
  pixelColor = vec4(vertexColorToFS, 1.0); 
} 
