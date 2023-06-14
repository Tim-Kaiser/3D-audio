#define SAF_USE_INTEL_MKL_LP64

#include <iostream>
#include <SDL.h>
#include "Screen.h"
#include "Input.h"
#include "glad.h"
#include "Shader.h"
#include "Quad.h"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include "Camera.h"
#include "Light.h"



bool isRunning = true;
int width = 800;
int height = 400;

int main(int argc, char* argv[]) {

	Screen::Instance()->Init();

	if (!Shader::Instance()->CreateProgram()) {
		return 0;
	}

	if (!Shader::Instance()->CreateShaders()) {
		return 0;
	}

	if (!Shader::Instance()->CompileShaders("Shaders/main.vert", Shader::ShaderType::VERTEX_SHADER)) {
		return 0;
	}
	if (!Shader::Instance()->CompileShaders("Shaders/main.frag", Shader::ShaderType::FRAGMENT_SHADER)) {
		return 0;
	}

	Shader::Instance()->AttachShaders();
	
	if (!Shader::Instance()->LinkProgram()) {
		return 0;
	}



	//================================================================
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	//modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, -1.0f));
	//modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	//modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f, 0.5f, 0.0f));

	Quad quad(modelMatrix);
	Camera camera;
	camera.SetProjection();

	Light light;

	//================================================================

	while (isRunning) {
		Screen::Instance()->ClearScreen();
		Input::Instance()->Update();

		if (Input::Instance()->isXClicked()) {
			isRunning = false;
		}
			
		camera.Update();
		light.Update();

		light.SendToShader();

		light.Render();
		quad.Render();

		Screen::Instance()->SwapBuffer();
	}

	Shader::Instance()->DetachShaders();
	Shader::Instance()->DestroyShaders();
	Shader::Instance()->DestroyProgram();

	Screen::Instance()->Close();
	return 0;
}