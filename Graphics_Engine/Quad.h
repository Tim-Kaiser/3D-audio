#ifndef H_QUAD
#define H_QUAD

#include "glad.h"
#include "Buffer.h"
#include <glm.hpp>
#include "Texture.h"

class Quad {

public:
	Quad(glm::mat4 modelMatrix);
	~Quad();

	void Update();
	void Render();

	void setModelMatrix(glm::mat4 modelMatrix);
	glm::mat4 getModelMatrix();


private:
	Buffer m_buffer;
	Texture m_tex;
	glm::mat4 m_modelMatrix;
	glm::vec3 m_position;
};

#endif