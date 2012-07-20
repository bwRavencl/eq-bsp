/*
 * Camera.cpp
 *
 *  Created on: 16.07.2012
 *      Author: matteo
 */

#include "Camera.h"
#include <GL/gl.h>
#include <math.h>

Camera::Camera() {
	_position[0] = 0.0f;
	_position[1] = 0.0f;
	_position[2] = 0.0f;

	_rotation[0] = 0.0f;
	_rotation[1] = 0.0f;
	_rotation[2] = 0.0f;
}

Camera::~Camera() {
	// TODO Auto-generated destructor stub
}

bool Camera::isVisible(float min[3], float max[3]) {
	// TODO Has to be implemented, camera need access to the view frustum (member?) to do the necessary calculations
	return true;
}

void Camera::render() {
	glRotatef(_rotation[0], 1.0f, 0.0f, 0.0f);
	glRotatef(_rotation[1], 0.0f, 1.0f, 0.0f);
	glRotatef(_rotation[2], 0.0f, 0.0f, 1.0f);

	glTranslatef(_position[0], _position[1], _position[2]);
}

void Camera::walkForeward(float distance) {
	_position[0] -= (float) sin(_rotation[1] * M_PI / 180) * distance;
	_position[2] += (float) cos(_rotation[1] * M_PI / 180) * distance;
}

void Camera::walkBackward(float distance) {
	_position[0] += (float) sin(_rotation[1] * M_PI / 180) * distance;
	_position[2] -= (float) cos(_rotation[1] * M_PI / 180) * distance;

}

void Camera::strafeLeft(float distance) {
	_position[0] += (float) cos(_rotation[1] * M_PI / 180) * distance;
	_position[2] += (float) sin(_rotation[1] * M_PI / 180) * distance;
}

void Camera::strafeRight(float distance) {
	_position[0] -= (float) cos(_rotation[1] * M_PI / 180) * distance;
	_position[2] -= (float) sin(_rotation[1] * M_PI / 180) * distance;
}

void Camera::moveUp(float distance) {
	_position[1] -= distance;
}

void Camera::moveDown(float distance) {
	_position[1] += distance;
}

void Camera::turnLeft(float angle) {
	_rotation[1] -= angle;
}

void Camera::turnRight(float angle) {
	_rotation[1] += angle;
}

void Camera::lookUp(float angle) {
	_rotation[0] -= angle;
}

void Camera::lookDown(float angle) {
	_rotation[0] += angle;
}
