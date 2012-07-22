/*
 * OpenGLExtensions.cpp
 *
 *  Created on: 22.07.2012
 *      Author: matteo
 */

#include "OpenGLExtensions.h"

glDrawRangeElementsEXT_func glDrawRangeElementsEXT;
glMultiDrawElementsEXT_func glMultiDrawElementsEXT;

bool prepareOpenGLExtensions() {
	glDrawRangeElementsEXT = NULL;
	glMultiDrawElementsEXT = NULL;

	glDrawRangeElementsEXT =
			(glDrawRangeElementsEXT_func) SDL_GL_GetProcAddress(
					"glDrawRangeElementsEXT");

	glMultiDrawElementsEXT =
			(glMultiDrawElementsEXT_func) SDL_GL_GetProcAddress(
					"glMultiDrawElementsEXT");

	if (glDrawRangeElementsEXT || glMultiDrawElementsEXT)
		return true;
	else
		return false;
}

