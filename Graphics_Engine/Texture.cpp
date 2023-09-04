#include "Texture.h"
#include <iostream>
#include <SDL_image.h>
#include <vector>

Texture::Texture()
{
	m_ID = 0;
	m_width = 0;
	m_height = 0;
}

void Texture::Bind()
{
	glBindTexture(GL_TEXTURE_2D, m_ID);

}

bool Texture::Load(const std::string& filename)
{
	SDL_Surface* textureData = IMG_Load(filename.c_str());
	if (!textureData) {
		std::cout << "Error loading texture from file "  << filename << std::endl;
		return false;
	}

	GLsizei width = textureData->w;
	GLsizei height = textureData->h;
	Uint8* pixels = (Uint8*)textureData->pixels;
	Uint8 depth = textureData->format->BytesPerPixel;
	GLint format = ((depth == 4) ? GL_RGBA : GL_RGB);

	glGenTextures(1, &m_ID);
	glBindTexture(GL_TEXTURE_2D, m_ID);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);

	glBindTexture(GL_TEXTURE_2D, 0);

	SDL_FreeSurface(textureData);


	return true;
}

bool Texture::Load(GLsizei width, GLsizei height, GLint format)
{
	m_width = width;
	m_height = height;

	Uint8* pixels = static_cast<Uint8*>(malloc(width*height*4));

	glGenTextures(1, &m_ID);
	glBindTexture(GL_TEXTURE_2D, m_ID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);

	glBindTexture(GL_TEXTURE_2D, 0);


	return true;
}

void Texture::saveTex() {
	//RGBA
	std::vector<GLbyte> pixels(m_width * m_height * 4);
	glReadPixels(0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
	std::cout << "TEXTURE DATA: " << pixels.data() << std::endl;
}

void Texture::Unbind()
{
	glBindTexture(GL_TEXTURE_2D, 0);

}

void Texture::Unload()
{
	glDeleteTextures(1, &m_ID);
}
