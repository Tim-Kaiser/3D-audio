#include "Buffer.h"
#include "Shader.h"
#include <iostream>

Buffer::Buffer()
{
	m_vertexArrayObj = 0;
	m_vertexVBO = 0;
	m_colorVBO = 0;
	m_textureVBO = 0;
	m_vertexCount = 0;
}

void Buffer::CreateBuffer(GLuint vertexCount)
{
	glGenBuffers(1, &m_vertexVBO);
	glGenBuffers(1, &m_colorVBO);
	glGenBuffers(1, &m_textureVBO);
	glGenVertexArrays(1, &m_vertexArrayObj);

	m_vertexCount = vertexCount;
}

void Buffer::FillVBO(VBOType vboType, GLfloat* data, GLsizeiptr bufferSize, FillType fillType)
{

	glBindVertexArray(m_vertexArrayObj);
		if (vboType == VERTEX_BUFFER) {
			glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
		}
		else if(vboType == COLOR_BUFFER){
			glBindBuffer(GL_ARRAY_BUFFER, m_colorVBO);
		}
		else {
			glBindBuffer(GL_ARRAY_BUFFER, m_textureVBO);
		}

		glBufferData(GL_ARRAY_BUFFER, bufferSize, data, fillType);
	glBindVertexArray(0);
}

void Buffer::FillEBO(GLuint* data, GLsizeiptr bufferSize, FillType fillType)
{
}

void Buffer::LinkBuffer(const std::string& attribute, VBOType vboType, ComponentType componentType, DataType dataType)
{
	GLuint shaderProgramID = Shader::Instance()->GetShaderProgramID();

	GLint id = glGetAttribLocation(shaderProgramID, attribute.c_str());
	if (id != -1) {
		glBindVertexArray(m_vertexArrayObj);

			if (vboType == VERTEX_BUFFER) {
				glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
			}
			else if (vboType == COLOR_BUFFER) {
				glBindBuffer(GL_ARRAY_BUFFER, m_colorVBO);
			}
			else {
				glBindBuffer(GL_ARRAY_BUFFER, m_textureVBO);
			}
			glVertexAttribPointer(id, componentType, GL_FLOAT, GL_FALSE, 0, nullptr);
			glEnableVertexAttribArray(id);
		glBindVertexArray(0);
	}
	else {
		std::cout << "Shader variable " << attribute << " does not exist or is not used." << std::endl;
	}

}

void Buffer::Draw(DrawType drawType)
{
	glBindVertexArray(m_vertexArrayObj);
	glDrawArrays(drawType, 0, m_vertexCount);
	glBindVertexArray(0);
}

void Buffer::DestroyBuffer()
{
	glDeleteBuffers(1, &m_vertexVBO);
	glDeleteBuffers(1, &m_colorVBO);
	glDeleteBuffers(1, &m_textureVBO);
	glDeleteVertexArrays(1, &m_vertexArrayObj);
}
