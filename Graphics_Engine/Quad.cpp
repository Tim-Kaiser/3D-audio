#include "Quad.h"
#include "Shader.h"
#include <gtc/matrix_transform.hpp>

Quad::Quad(glm::mat4 modelMatrix)
{
	m_modelMatrix = modelMatrix;

	GLfloat vertices[] = {
		-1.0f,  1.0f, 0.0f, // triangle 1
		 1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,

		-1.0f, -1.0f, 0.0f, // triangle 2
		 1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f
	};

	GLfloat colors[] = {
		1.0f, 0.0f, 0.0f, // triangle 1
		0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 1.0f,

		0.0f, 1.0f, 1.0f, // triangle 2
		0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f
	};

	GLfloat uvs[] = {
		0.0f, 1.0f, // triangle 1
		1.0f, 1.0f,
		0.0f, 0.0f,

		0.0f, 0.0f, // triangle 2
		1.0f, 1.0f,
		1.0f, 0.0f
	};
	
	m_buffer.CreateBuffer(6);
	m_buffer.FillVBO(Buffer::VERTEX_BUFFER, vertices, sizeof(vertices), Buffer::SINGLE);
	m_buffer.FillVBO(Buffer::COLOR_BUFFER, colors, sizeof(colors), Buffer::SINGLE);
	m_buffer.FillVBO(Buffer::TEXTURE_BUFFER, uvs, sizeof(uvs), Buffer::SINGLE);


	m_buffer.LinkBuffer("vertex", Buffer::VERTEX_BUFFER, Buffer::XYZ, Buffer::FLOAT);
	m_buffer.LinkBuffer("color", Buffer::COLOR_BUFFER, Buffer::RGB, Buffer::FLOAT);
	m_buffer.LinkBuffer("textureCoord", Buffer::TEXTURE_BUFFER, Buffer::UV, Buffer::FLOAT);

	m_tex.Load("Textures/wood.jpg");

	m_shininess = 50.0f;
	m_position = glm::vec3(0.0f);
	m_ambient = glm::vec3(0.4f, 0.4f, 0.4f);
	m_diffuse = glm::vec3(0.1f, 0.7f, 0.2f);
	m_specular = glm::vec3(0.8f, 0.8f, 0.8f);;

}

Quad::~Quad()
{
	// remove this if the buffers are going to be reused between objects
	m_buffer.DestroyBuffer();
}

void Quad::Update()
{
}

void Quad::Render()
{
	Shader::Instance()->SendUniformData("modelMatrix", m_modelMatrix);
	Shader::Instance()->SendUniformData("isLit", 1);
	Shader::Instance()->SendUniformData("isTextured", 1);

	Shader::Instance()->SendUniformData("material.shininess", m_shininess);
	Shader::Instance()->SendUniformData("material.ambient", m_ambient.r, m_ambient.g, m_ambient.b);
	Shader::Instance()->SendUniformData("material.diffuse", m_diffuse.b, m_diffuse.g, m_diffuse.b);
	Shader::Instance()->SendUniformData("material.specular", m_specular.r, m_specular.g, m_specular.b);


	m_tex.Bind();
	m_buffer.Draw(Buffer::TRIANGLES);
	m_tex.Unbind();
}

void Quad::setModelMatrix(glm::mat4 modelMatrix)
{
	m_modelMatrix = modelMatrix;
}

glm::mat4 Quad::getModelMatrix()
{
	return m_modelMatrix;
}
