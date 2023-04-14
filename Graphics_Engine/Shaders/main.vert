#version 460

in vec3 vertex;
in vec3 color;
in vec2 textureCoord;

out vec3 col;
out vec2 uv;
out vec3 vertexOut;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main(){
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertex, 1.0);

	uv = textureCoord;
	col = color;
	vertexOut = (modelMatrix * vec4(vertex, 1.0)).xyz;
}