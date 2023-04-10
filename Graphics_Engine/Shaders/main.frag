#version 460

in vec3 col;
in vec2 uv;
layout(location = 0) out vec4 fragCol;

uniform sampler2D textureImg;

void main(){
	
	fragCol = texture(textureImg, uv);
}