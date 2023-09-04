#include "Input.h"
#include <iostream>
Input::Input()
{
	m_isXClicked = false;
	m_isKeyPressed = false;
	
	m_keyUp = 0;
	m_keyDown = 0;
	
	m_isLeftMouseClicked = false;
	m_isMiddleMouseClicked = false;
	m_isRightMouseClicked = false;
	
	m_mouseX = 0;
	m_mouseY = 0;
	
	m_mouseMoveX = 0;
	m_mouseMoveY = 0;
}

void Input::Reset()
{
	m_isXClicked = false;
	m_isKeyPressed = false;

	m_keyUp = 0;
	m_keyDown = 0;

	m_isLeftMouseClicked = false;
	m_isMiddleMouseClicked = false;
	m_isRightMouseClicked = false;

	m_mouseX = 0;
	m_mouseY = 0;

	m_mouseMoveX = 0;
	m_mouseMoveY = 0;
}



Input* Input::Instance()
{
	static Input* instance = new Input();
	return instance;
}


bool Input::isXClicked()
{
	return m_isXClicked;
}

bool Input::isKeyPressed()
{
	return m_isKeyPressed;
}

char Input::getKeyUp()
{
	return m_keyUp;
}

char Input::getKeyDown()
{
	return m_keyDown;
}

bool Input::isLeftMouseClicked()
{
	return m_isLeftMouseClicked;
}

bool Input::isMiddleMouseClicked()
{
	return m_isMiddleMouseClicked;
}

bool Input::isRightMouseClicked()
{
	return m_isRightMouseClicked;
}

int Input::getMouseX()
{
	return m_mouseX;
}

int Input::getMouseY()
{
	return m_mouseY;
}

int Input::getMouseMoveX()
{
	return m_mouseMoveX;
}

int Input::getMouseMoveY()
{
	return m_mouseMoveY;
}

void Input::Update()
{

	SDL_Event event;

	Reset();

	while (SDL_PollEvent(&event)) {

		switch (event.type) {
			case SDL_KEYDOWN:
			{
				m_isKeyPressed = true;
				m_keyDown = event.key.keysym.sym;
				if (m_keyDown == SDLK_ESCAPE) {
					m_isXClicked = true;
				}
				break;
			}
			case SDL_KEYUP:
			{
				m_isKeyPressed = false;
				m_keyUp = event.key.keysym.sym;
				break;
			}
			case SDL_QUIT:
			{
				m_isXClicked = true;
				break;
			}
			case SDL_MOUSEBUTTONDOWN:
			{
				switch (event.button.button) {
					case SDL_BUTTON_LEFT:
					{
						m_isLeftMouseClicked = true;
						break;
					}
					case SDL_BUTTON_MIDDLE:
					{
						m_isMiddleMouseClicked = true;
						break;
					}
					case SDL_BUTTON_RIGHT:
					{
						m_isRightMouseClicked = true;
						break;
					}
				}
				break;
			}
			case SDL_MOUSEBUTTONUP:
			{
				switch (event.button.button) {
					case SDL_BUTTON_LEFT:
					{
						m_isLeftMouseClicked = false;
						break;
					}
					case SDL_BUTTON_MIDDLE:
					{
						m_isMiddleMouseClicked = false;
						break;
					}
					case SDL_BUTTON_RIGHT:
					{
						m_isRightMouseClicked = false;
						break;
					}
				}
				break;
			}
			case SDL_MOUSEMOTION:
			{
				m_mouseMoveX = event.motion.xrel;
				m_mouseMoveY = event.motion.yrel;

				m_mouseX = event.motion.x;
				m_mouseY = event.motion.y;

			}
		}
	}
}