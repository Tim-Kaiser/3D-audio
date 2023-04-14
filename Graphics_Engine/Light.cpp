#include "Light.h"
#include "Shader.h"
#include <chrono>
#include "Input.h"

Light::Light()
{
	m_modelMatrix = glm::mat4();

	m_position = glm::vec3(0.0f, 1.0f, 0.0f);
	m_ambient = glm::vec3(1.0f, 1.0f, 1.0f);
	m_diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
	m_specular = glm::vec3(1.0f, 1.0f, 1.0f);

	GLfloat vertices[] = { 0.0f, 0.0f, 0.0f};
	GLfloat colors[] = { 1.0f, 1.0f, 1.0f };


	m_buffer.CreateBuffer(1);

	m_buffer.FillVBO(Buffer::VERTEX_BUFFER, vertices, sizeof(vertices), Buffer::SINGLE);
	m_buffer.FillVBO(Buffer::COLOR_BUFFER, colors, sizeof(colors), Buffer::SINGLE);

	m_buffer.LinkBuffer("vertex", Buffer::VERTEX_BUFFER, Buffer::XYZ, Buffer::FLOAT);
	m_buffer.LinkBuffer("color", Buffer::COLOR_BUFFER, Buffer::RGB, Buffer::FLOAT);

}

Light::~Light()
{
	m_buffer.DestroyBuffer();
}

void Light::Update()
{
	char key = Input::Instance()->getKeyDown();

	if (key == 'i') {
		m_position.z -= 0.01f;
	}
	if (key == 'k') {
		m_position.z += 0.01f;
	}
	if (key == 'j') {
		m_position.x -= 0.01f;
	}
	if (key == 'l') {
		m_position.x += 0.01f;
	}
	if (key == 'u') {
		m_position.y += 0.01f;
	}	
	if (key == 'o') {
		m_position.y -= 0.01f;
	}

	m_modelMatrix = glm::mat4(1.0f);
	m_modelMatrix = glm::translate(m_modelMatrix, m_position);

}

void Light::Render()
{
	Shader::Instance()->SendUniformData("modelMatrix", m_modelMatrix);
	Shader::Instance()->SendUniformData("isLit", 0);
	Shader::Instance()->SendUniformData("isTextured", 0);

	glPointSize(20.0f);
	m_buffer.Draw(Buffer::POINTS);
}

void Light::SendToShader()
{
	Shader::Instance()->SendUniformData("light.ambient", m_ambient.r, m_ambient.g, m_ambient.b);
	Shader::Instance()->SendUniformData("light.specular", m_specular.r, m_specular.g, m_specular.b);
	Shader::Instance()->SendUniformData("light.diffuse", m_diffuse.r, m_diffuse.g, m_diffuse.b);
	Shader::Instance()->SendUniformData("light.position", m_position.x, m_position.y, m_position.b);

}
