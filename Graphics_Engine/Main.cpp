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
#include <random>
#include <chrono>
#include <SDL_mixer.h>

bool isRunning = true;
const char AUDIO_FILE[] = ".Audio/Heavy.wav";
const int SAMPLE_RATE = 44100;
const int NUM_SAMPLES_PER_FILL = 512;
const int SAMPLE_SIZE = sizeof(float);


//AUDIO



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

	Quad quad(modelMatrix);
	Camera camera(glm::vec3(0,0,1), glm::vec3(0,0,0), 45.0f);

	Light light;
	SDL_DisplayMode DM;
	SDL_GetCurrentDisplayMode(0, &DM);
	auto Width = DM.w;
	auto Height = DM.h;

	
	//================================================================
	//AUDIO
	int initMix = Mix_Init(0);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);

	Mix_Music* music = Mix_LoadMUS("Audio/Heavy.wav");
	//Mix_PlayMusic(music, -1);



	int audio_rate = MIX_DEFAULT_FREQUENCY;
	int audio_channels = MIX_DEFAULT_CHANNELS;
	int audio_buffers = 4096;
	
	//================================================================
	// HRTF




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
		Input::Instance()->Update();
		Shader::Instance()->SendUniformData("time", t);
		t += 0.00001;


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