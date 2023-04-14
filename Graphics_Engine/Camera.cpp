#include <gtc/matrix_transform.hpp>
#include "Camera.h"
#include "Input.h"
#include "Shader.h"
#include <iostream>


Camera::Camera()
{
	m_viewMatrix = glm::mat4(1.0f);
	m_projectionMatrix = glm::mat4(1.0f);

	m_position = glm::vec3(0.0f, 0.0f, 1.0f);
	m_direction = glm::vec3(0.0f, 0.0f, -1.0f);
	m_up = glm::vec3(0.0f, 1.0f, 0.0f);
}

//void Camera::SetProjection(GLfloat width, GLfloat height, GLfloat fov, GLfloat nearClip, GLfloat farClip)
//{
//	GLfloat aspectRatio = 1280.0f / 700.0f;
//	GLfloat FOV = 45.0f;
//	//m_projectionMatrix = glm::perspective(fov, aspectRatio, nearClip, farClip);
//	m_projectionMatrix = glm::perspective(FOV, aspectRatio, 0.001f, 1000.0f);
//	std::cout << "test" << std::endl;
//
//	Shader::Instance()->SendUniformData("projectionMatrix", m_projectionMatrix);
//}

void Camera::SetProjection() {

		GLfloat aspectRatio = 1280.0f / 700.0f;
		GLfloat FOV = 45.0f;
		GLfloat near = 0.001f;
		GLfloat far = 1000.0f;

		m_projectionMatrix = glm::perspective(FOV, aspectRatio, near, far);
	
		Shader::Instance()->SendUniformData("projectionMatrix", m_projectionMatrix);
}


void Camera::Update()
{	
	char key = Input::Instance()->getKeyDown();

	if (key == 'w') {
		m_position.z -= 0.01f;
	}
	else if (key == 's') {
		m_position.z += 0.01f;
	}
	else if (key == 'q') {
		m_position.y += 0.01f;

	}
	else if (key == 'e') {
		m_position.y -= 0.01f;

	}
	else if (key == 'a') {
		m_position.x -= 0.01f;

	}
	else if (key == 'd') {
		m_position.x += 0.01f;
	}


	m_viewMatrix = glm::lookAt(m_position, m_position + m_direction, m_up);
	Shader::Instance()->SendUniformData("viewMatrix", m_viewMatrix);
	Shader::Instance()->SendUniformData("cameraPos", m_position);

}

