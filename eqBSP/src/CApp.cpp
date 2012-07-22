/*
 * CApp.cpp
 *
 *  Created on: 19.07.2012
 *      Author: matteo
 */

#define WINDOW_WIDTH 1680
#define WINDOW_HEIGHT 1050

#define HEAD_SENSITIVITY 3.0f
#define MOVEMENT_SPEED 4.0f

#include "CApp.h"
#include "Utilities/OpenGLExtensions.h"

CApp::CApp() {
	_running = true;

	Surf_Display = NULL;

	_map = NULL;
	_camera = NULL;

	_lookUp = false;
	_lookDown = false;
	_turnLeft = false;
	_turnRight = false;

	_walkForeward = false;
	_walkBackward = false;
	_strafeLeft = false;
	_strafeRight = false;
	_moveUp = false;
	_moveDown = false;

	_wireframeEnabled = false;
}

bool CApp::OnInit() {
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		return false;
	}

	if ((Surf_Display = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 32,
			SDL_HWSURFACE | SDL_OPENGL)) == NULL) {
		return false;
	}

	if(!prepareOpenGLExtensions()) {
		std::cout << "Error: Required OpenGL extensions not supported!" << std::endl;
		return false;
	}

	glClearColor(0, 0, 0, 0);

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(75.0f, (GLfloat) WINDOW_WIDTH / (GLfloat) WINDOW_HEIGHT, 0.1f, 10000.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glShadeModel(GL_SMOOTH);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	//depth
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	//hints
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glEnable(GL_CULL_FACE);

	_camera = new Camera();
	_map = new Q3Map("/home/matteo/tmp/baseq3a/maps/q3dm17.bsp", *_camera);

	return true;
}

void CApp::OnEvent(SDL_Event* event) {

	if (event->type == SDL_KEYDOWN) {
		// Handle beginning head movement
		if (event->key.keysym.sym == SDLK_UP)
			_lookUp = true;
		if (event->key.keysym.sym == SDLK_DOWN)
			_lookDown = true;
		if (event->key.keysym.sym == SDLK_LEFT)
			_turnLeft = true;
		if (event->key.keysym.sym == SDLK_RIGHT)
			_turnRight = true;

		// Handle beginning of walking
		if (event->key.keysym.sym == SDLK_w)
			_walkForeward = true;
		if (event->key.keysym.sym == SDLK_s)
			_walkBackward = true;
		if (event->key.keysym.sym == SDLK_a)
			_strafeLeft = true;
		if (event->key.keysym.sym == SDLK_d)
			_strafeRight = true;
		if (event->key.keysym.sym == SDLK_PAGEUP)
			_moveUp = true;
		if (event->key.keysym.sym == SDLK_PAGEDOWN)
			_moveDown = true;

		// Turn wireframe on and off
		if (event->key.keysym.sym == SDLK_m) {
			_wireframeEnabled = !_wireframeEnabled;

			if (_wireframeEnabled)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		if (event->key.keysym.sym == SDLK_ESCAPE) {
			_running = false;
		}
	}

	if (event->type == SDL_KEYUP) {
		// Handle end head movement
		if (event->key.keysym.sym == SDLK_UP)
			_lookUp = false;
		if (event->key.keysym.sym == SDLK_DOWN)
			_lookDown = false;
		if (event->key.keysym.sym == SDLK_LEFT)
			_turnLeft = false;
		if (event->key.keysym.sym == SDLK_RIGHT)
			_turnRight = false;

		// Handle end of walking
		if (event->key.keysym.sym == SDLK_w)
			_walkForeward = false;
		if (event->key.keysym.sym == SDLK_s)
			_walkBackward = false;
		if (event->key.keysym.sym == SDLK_a)
			_strafeLeft = false;
		if (event->key.keysym.sym == SDLK_d)
			_strafeRight = false;
		if (event->key.keysym.sym == SDLK_PAGEUP)
			_moveUp = false;
		if (event->key.keysym.sym == SDLK_PAGEDOWN)
			_moveDown = false;
	}

	if (event->type == SDL_QUIT) {
		_running = false;
	}
}

void CApp::OnLoop() {
	if (_lookUp)
		_camera->lookUp(HEAD_SENSITIVITY);
	if (_lookDown)
		_camera->lookDown(HEAD_SENSITIVITY);
	if (_turnLeft)
		_camera->turnLeft(HEAD_SENSITIVITY);
	if (_turnRight)
		_camera->turnRight(HEAD_SENSITIVITY);

	if (_walkForeward)
		_camera->walkForeward(MOVEMENT_SPEED);
	if (_walkBackward)
		_camera->walkBackward(MOVEMENT_SPEED);
	if (_strafeLeft)
		_camera->strafeLeft(MOVEMENT_SPEED);
	if (_strafeRight)
		_camera->strafeRight(MOVEMENT_SPEED);
	if (_moveUp)
		_camera->moveUp(MOVEMENT_SPEED);
	if (_moveDown)
		_camera->moveDown(MOVEMENT_SPEED);
}

void CApp::OnRender() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	_camera->render();

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	//Unit 0 - replace with decal textures
	glEnable(GL_TEXTURE_2D);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	//Unit 1
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glEnable(GL_TEXTURE_2D);

//	if(renderMethod==MODULATE_TEXTURES)	//Then modulate by lightmap, then double
//	{
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);

		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);

		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);

		glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 2.0f);
//	}
//
//	if(renderMethod==SHOW_TEXTURES)	//Then replace with previous
//	{
//		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
//
//		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
//		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
//
//		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
//	}
//
//	if(renderMethod==SHOW_LIGHTMAPS)//Then replace with lightmaps
//	{
//		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
//
//		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE);
//		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
//
//		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
//	}

	glActiveTextureARB(GL_TEXTURE0_ARB);

	_map->render();

	glPopAttrib();

	SDL_GL_SwapBuffers();
}

void CApp::OnCleanup() {
	SDL_Quit();
}

int CApp::OnExecute() {
	if (OnInit() == false) {
		return -1;
	}

	SDL_Event Event;

	while (_running) {
		while (SDL_PollEvent(&Event)) {
			OnEvent(&Event);
		}

		OnLoop();
		OnRender();
	}

	OnCleanup();

	return 0;
}

int main(int argc, char* argv[]) {
	CApp theApp;

	return theApp.OnExecute();
}
