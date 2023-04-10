#ifndef CAMERA_H
#define CAMERA_H

#include <glm.hpp>
#include "glad.h"

class Camera {

public:
	Camera();
	void Update();
	//void SetProjection(GLfloat width, GLfloat height, GLfloat fov = 45.0f, GLfloat nearClip = 0.001f, GLfloat farClip = 1000.0f);
	void SetProjection();

private:
	glm::mat4 m_viewMatrix;
	glm::mat4 m_projectionMatrix;

	glm::vec3 m_position;
	glm::vec3 m_direction;
	glm::vec3 m_up;
};

#endif // !CAMERA_H
