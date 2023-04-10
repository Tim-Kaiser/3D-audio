#ifndef SCREEN_H
#define SCREEN_H

#include <SDL.h>

class Screen {

private:
	SDL_Window* window;
	SDL_GLContext context;

// constructors
private:
	Screen();
	Screen(const Screen&);
	Screen& operator=(const Screen&);

public:
	static Screen* Instance();
	bool Init();
	void ClearScreen();
	void SwapBuffer();
	void Close();
};
#endif // !SCREEN_H
