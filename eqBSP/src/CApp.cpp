/*
 * CApp.cpp
 *
 *  Created on: 19.07.2012
 *      Author: matteo
 */

#include "CApp.h"

#define windowWidth 1680
#define windowHeight 1050

CApp::CApp() {
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

	Running = true;
}

bool CApp::OnInit() {
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		return false;
	}

	if ((Surf_Display = SDL_SetVideoMode(windowWidth, windowHeight, 32,
			SDL_HWSURFACE | SDL_OPENGL)) == NULL) {
		return false;
	}

	glClearColor(0, 0, 0, 0);

	glViewport(0, 0, windowWidth, windowHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(75.0f, (GLfloat) windowWidth / (GLfloat) windowHeight, 0.1f, 1000.0f);

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
	_map = new Q3Map("examples/maps/final.bsp", *_camera);

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
		Running = false;
	}
}

void CApp::OnLoop() {
	if (_lookUp)
		_camera->lookUp(1.0f);
	if (_lookDown)
		_camera->lookDown(1.0f);
	if (_turnLeft)
		_camera->turnLeft(1.0f);
	if (_turnRight)
		_camera->turnRight(1.0f);

	if (_walkForeward)
		_camera->walkForeward(1.0f);
	if (_walkBackward)
		_camera->walkBackward(1.0f);
	if (_strafeLeft)
		_camera->strafeLeft(1.0f);
	if (_strafeRight)
		_camera->strafeRight(1.0f);
	if (_moveUp)
		_camera->moveUp(1.0f);
	if (_moveDown)
		_camera->moveDown(1.0f);
}

void CApp::OnRender() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	_camera->render();

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	//glScalef(0.1f, 0.1f, 0.1f);

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

	while (Running) {
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
