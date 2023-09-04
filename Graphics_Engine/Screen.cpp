#include <iostream>
#include "Screen.h"
#include "glad.h"

Screen::Screen() {
	window = nullptr;
	context = nullptr;
}

Screen* Screen::Instance()
{
	static Screen* screen = new Screen;
	return screen;
}

bool Screen::Init()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1) {
		std::cout << "Error initializing SDL" << std::endl;
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	std::cout << "Running OpenGL in CORE mode" << std::endl;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	std::cout << "Running OpenGL with version 4.6" << std::endl;

	SDL_DisplayMode DM;
	SDL_GetCurrentDisplayMode(0, &DM);
	auto Width = DM.w;
	auto Height = DM.h;

	window = SDL_CreateWindow("Bachelor Project Tim Kaiser", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Width, Height, SDL_WINDOW_OPENGL);
	SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

	// hide cursor
	SDL_ShowCursor(SDL_DISABLE);


	if (!window) {
		std::cout << "Error creating window" << std::endl;
		return false;
	}

	context = SDL_GL_CreateContext(window);

	if (!context) {
		std::cout << "Error creating OpenGL context" << std::endl;
		return false;
	}

	if (!gladLoadGL()) {
		std::cout << "Error loading glad extensions" << std::endl;
		return false;
	}	return true;
}

void Screen::ClearScreen()
{
	glClear(GL_COLOR_BUFFER_BIT);

}

void Screen::SwapBuffer()
{
	SDL_GL_SwapWindow(window);

}

void Screen::Close()
{
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
