#include "Quad.h"
#include "Shader.h"
#include <gtc/matrix_transform.hpp>

Quad::Quad(glm::mat4 modelMatrix)
{
	m_modelMatrix = modelMatrix;
	m_position = glm::vec3(0.0f, 0.0f, 0.0f);

	GLfloat vertices[] = {
		-0.5f,  0.5f, 0.0f, // triangle 1
		 0.5f,  0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,

		-0.5f, -0.5f, 0.0f, // triangle 2
		 0.5f,  0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f
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
