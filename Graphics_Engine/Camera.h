#ifndef CAMERA_H
#define CAMERA_H

#include <glm.hpp>
#include "glad.h"

class Camera {

public:
	Camera(glm::vec3 position, glm::vec3 lookAt, float fov);
	void offsetPosition(glm::vec3 offset);
	void offsetLookAt(float offsetX, float offsetY);
	void Update();
	void SendToShader();

private:

	glm::vec3 m_position;
	glm::vec3 m_direction;
	glm::vec3 m_up;
	glm::vec3 m_globalUp;

	glm::vec3 m_right;
	glm::vec3 m_fwd;
	float m_fov;
	float m_focalDistance;
	float m_aperture;
	float m_yaw;
	float m_pitch;
};

#endif // !CAMERA_H
