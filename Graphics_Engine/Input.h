#ifndef INPUT_H
#define INPUT_H

#include <SDL.h>

class Input {
private:
	Input();
	Input(const Input&);
	Input& operator=(const Input&);

	void Reset();

	bool m_isXClicked;
	bool m_isKeyPressed;

	char m_keyUp;
	char m_keyDown;

	bool m_isLeftMouseClicked;
	bool m_isMiddleMouseClicked;
	bool m_isRightMouseClicked;

	int m_mouseX;
	int m_mouseY;

	int m_mouseMoveX;
	int m_mouseMoveY;


public:
	static Input* Instance();
	void Update();

	bool isXClicked();
	bool isKeyPressed();

	char getKeyUp();
	char getKeyDown();

	bool isLeftMouseClicked();
	bool isMiddleMouseClicked();
	bool isRightMouseClicked();

	int getMouseX();
	int getMouseY();

	int getMouseMoveX();
	int getMouseMoveY();
};


#endif