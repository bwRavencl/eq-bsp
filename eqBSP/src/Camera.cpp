/*
 * Camera.cpp
 *
 *  Created on: 16.07.2012
 *      Author: matteo
 */

#include "Camera.h"

Camera::Camera() {
	_position[0] = 0.0f;
	_position[1] = 0.0f;
	_position[2] = 0.0f;
}

Camera::~Camera() {
	// TODO Auto-generated destructor stub
}

bool Camera::isVisible(float min[3], float max[3]) {
	// TODO Has to be implemented, camera need access to the view frustum (member?) to do the necessary calculations
	return true;
}

