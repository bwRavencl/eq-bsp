/*
 * TextureLoader.cpp
 *
 *  Created on: 24.07.2012
 *      Author: matteo
 */

#include "TextureLoader.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <iostream>

GLuint loadTexture(std::string filePath) {
	SDL_Surface* surface = IMG_Load(filePath.c_str());

	if (!surface) {
		std::cout << "WARNING: Couldn't load texture: " << filePath
				<< std::endl;
		return NULL;
	}

	GLuint texture;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	SDL_PixelFormat *format = surface->format;
	if (format->Amask) {
		gluBuild2DMipmaps(GL_TEXTURE_2D, 4, surface->w, surface->h, GL_RGBA,
				GL_UNSIGNED_BYTE, surface->pixels);
	} else {
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, surface->w, surface->h, GL_RGB,
				GL_UNSIGNED_BYTE, surface->pixels);
	}
	SDL_FreeSurface(surface);
	return texture;
}
