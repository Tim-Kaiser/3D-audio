#version 460

in vec3 vertex;
in vec2 textureCoord;

out vec3 col;
out vec2 uv;
out vec3 vertexOut;

void main(){
	gl_Position = vec4(vertex, 1.0);

	uv = textureCoord;
	vertexOut = vec3(textureCoord, 0.);
}