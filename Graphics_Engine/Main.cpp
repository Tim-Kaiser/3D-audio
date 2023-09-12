#define SAF_USE_INTEL_MKL_LP64

#include "Screen.h"
#include "Input.h"
#include "glad.h"
#include "Shader.h"
#include "Quad.h"
#include "Camera.h"
#include "Sound.h"


#include <random>
#include <chrono>
#include <SDL_mixer.h>
#include <algorithm>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <iostream>
#include <SDL.h>
#include <fftw3.h>

bool isRunning = true;


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

	Quad quad;
	Camera camera(glm::vec3(0,0.5,1), glm::vec3(0,0.5,0), 70.0f);

	SDL_DisplayMode DM;
	SDL_GetCurrentDisplayMode(0, &DM);
	auto Width = DM.w;
	auto Height = DM.h;

	//================================================================
 
	Sound audio;
	/*audio.initAudio();
	audio.Play();*/
	//================================================================
	// SSBO

	Shader::Instance()->SetUpSSBO();

	
	 
	//================================================================
	float aspect_ratio = 21.0 / 9.0;
	float viewport_height = 2.0;
	float viewport_width = aspect_ratio * viewport_height;
	float focal_length = 1.0;

	Shader::Instance()->SendUniformData("vh", viewport_height);
	Shader::Instance()->SendUniformData("vw", viewport_width);
	Shader::Instance()->SendUniformData("fl", focal_length);

	Shader::Instance()->SendUniformData("resolution", Width, Height);
	//================================================================
	static std::uniform_real_distribution<float> dist(0.0, 1.0);
	static std::mt19937 gen;
	float x = dist(gen);
	float y = dist(gen);
	Shader::Instance()->SendUniformData("seedVector", x,y);

	float t = 0.0f;

	//================================================================
	while (isRunning) {
		Screen::Instance()->ClearScreen();
		Shader::Instance()->SendUniformData("time", t);
		t += 0.01;
		Input::Instance()->Update();



		if (Input::Instance()->isXClicked()) {
			isRunning = false;
		}
		if (Input::Instance()->getKeyDown() == 'w') {
			camera.offsetPosition(glm::vec3(0.0, 0.0, -0.1));
		}
		else if (Input::Instance()->getKeyDown() == 's') {
			camera.offsetPosition(glm::vec3(0.0, 0.0, 0.1));
		}
		else if (Input::Instance()->getKeyDown() == 'a') {
			camera.offsetPosition(glm::vec3(-0.1, 0.0, 0.0));
		}
		else if (Input::Instance()->getKeyDown() == 'd') {
			camera.offsetPosition(glm::vec3(0.1, 0.0, 0.0));
		}

		camera.offsetLookAt(Input::Instance()->getMouseMoveX(), Input::Instance()->getMouseMoveY());
			
		camera.Update();
		quad.Render();
		audio.getAngles();

		Shader::Instance()->UpdateSSBO();

		Screen::Instance()->SwapBuffer();
	}

	SDL_CloseAudio();

	audio.WriteToFile();
	//Shader::Instance()->GetSSBOData();
	Shader::Instance()->DetachShaders();
	Shader::Instance()->DestroyShaders();
	Shader::Instance()->DestroyProgram();

	Screen::Instance()->Close();

	return 0;
}