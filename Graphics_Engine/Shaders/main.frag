#version 460

struct Light{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct Material{
	float shininess;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in vec3 col;
in vec2 uv;
in vec3 vertexOut;
layout(location = 0) out vec4 fragCol;


uniform Light light;
uniform Material material;

uniform vec3 cameraPos;
uniform int isLit;
uniform int isTextured;
uniform sampler2D textureImg;

void main(){

	if(isLit == 1){
		vec3 ambientCol = light.ambient * material.ambient;


		vec3 normal = vec3(0., 1., 0.);
		vec3 lightDir = normalize(light.position - vertexOut);
		float lightIntensity = max(dot(lightDir, normal), 0.0);

		vec3 diffuseCol = light.diffuse * material.diffuse * lightIntensity;

		vec3 viewDir = normalize(cameraPos - vertexOut);
		vec3 reflectionDir = reflect(lightDir, normal);
		float spec = pow(max(dot(viewDir, reflectionDir),0.0), material.shininess);

		vec3 specularCol = light.specular * material.specular * spec;

		vec3 lighColor = ambientCol + diffuseCol + specularCol;

		vec4 color = vec4(lighColor, 1.);


		if(isTextured == 1){
			color *= texture(textureImg, uv);
		}
		fragCol = color;
	}else{
		fragCol = vec4(col, 1.);
	}
}