#include <gtc/matrix_transform.hpp>
#include "Camera.h"
#include "Input.h"
#include "Shader.h"
#include <iostream>


Camera::Camera(glm::vec3 position, glm::vec3 lookAt, float fov)
{
	m_position = position;
	m_up = glm::vec3(0, 1, 0);
	m_globalUp = glm::vec3(0, 1, 0);

	m_direction = glm::normalize(lookAt - position);
	m_pitch = glm::degrees(asin(m_direction.y));
	m_yaw = glm::degrees(atan2(m_direction.z, m_direction.x));

	m_fov = glm::radians(fov);
	m_focalDistance = 0.1f;
	m_aperture = 0.0;
	Update();
}

void Camera::offsetPosition(glm::vec3 offset)
{
	m_position += offset;
	Update();
}

void Camera::offsetLookAt(float offsetX, float offsetY)
{
	m_pitch -= offsetY;
	m_yaw += offsetX;
	Update();
}



void Camera::Update()
{	
	glm::vec3 tmp;
	tmp.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	tmp.y = sin(glm::radians(m_pitch));
	tmp.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

	m_fwd = glm::normalize(tmp);
	m_right = glm::normalize(glm::cross(m_fwd, m_globalUp));
	m_up = glm::normalize(glm::cross(m_right, m_fwd));

	SendToShader();
}

void Camera::SendToShader()
{

	Shader::Instance()->SendUniformData("camera.up", m_up);
	Shader::Instance()->SendUniformData("camera.right", m_right);
	Shader::Instance()->SendUniformData("camera.forward", m_fwd);
	Shader::Instance()->SendUniformData("camera.position", m_position);
	Shader::Instance()->SendUniformData("camera.fov", m_fov);
	Shader::Instance()->SendUniformData("camera.focalDist", m_focalDistance);
	Shader::Instance()->SendUniformData("camera.aperture", m_aperture);


}