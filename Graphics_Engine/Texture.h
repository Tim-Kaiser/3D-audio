#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include "glad.h"

class Texture {

public:
	Texture();

	void Bind();
	bool Load(const std::string& filename);
	bool Load(GLsizei width, GLsizei height, GLint format);

	void saveTex();
	void Unbind();
	void Unload();

private:
	GLuint m_ID;
	int m_width;
	int m_height;

};
#endif
