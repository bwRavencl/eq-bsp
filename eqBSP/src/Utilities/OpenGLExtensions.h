/*
 * OpenGLExtensions.h
 *
 *  Created on: 22.07.2012
 *      Author: matteo
 */

#ifndef OPENGLEXTENSIONS_H_
#define OPENGLEXTENSIONS_H_

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

typedef void (APIENTRY *glDrawRangeElementsEXT_func)(GLenum mode, GLuint start,
		GLuint end, GLsizei count, GLenum type, const GLvoid** indices);

typedef void (APIENTRY *glMultiDrawElementsEXT_func)(GLenum mode,
		GLsizei *count, GLenum type, const GLvoid** indices, GLsizei primcount);

extern glDrawRangeElementsEXT_func glDrawRangeElementsEXT;
extern glMultiDrawElementsEXT_func glMultiDrawElementsEXT;

extern bool prepareOpenGLExtensions();

#endif /* OPENGLEXTENSIONS_H_ */
