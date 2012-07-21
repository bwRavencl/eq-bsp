/*
 * CApp.h
 *
 *  Created on: 19.07.2012
 *      Author: matteo
 */

#ifndef CAPP_H_
#define CAPP_H_

#include "Q3Map.h"
#include "Camera.h"
#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>

class CApp {
private:
	bool _running;

	SDL_Surface* Surf_Display;

	Camera* _camera;
	Q3Map* _map;

	bool _lookUp;
	bool _lookDown;
	bool _turnLeft;
	bool _turnRight;

	bool _walkForeward;
	bool _walkBackward;
	bool _strafeLeft;
	bool _strafeRight;
	bool _moveUp;
	bool _moveDown;

	bool _wireframeEnabled;

public:
	CApp();

	int OnExecute();

public:

	bool OnInit();

	void OnEvent(SDL_Event* event);

	void OnLoop();

	void OnRender();

	void OnCleanup();

};

#endif /* CAPP_H_ */
