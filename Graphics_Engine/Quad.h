#ifndef QUAD_H
#define QUAD_H

#include "glad.h"
#include "Buffer.h"
#include <glm.hpp>

class Quad {

public:
	Quad();
	~Quad();

	void Update();
	void Render();


private:
	Buffer m_buffer;
	glm::vec3 m_position;
};

#endif