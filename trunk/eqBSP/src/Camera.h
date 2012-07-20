/*
 * Camera.h
 *
 *  Created on: 16.07.2012
 *      Author: matteo
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#include "Matrices/Matrices.h"

class Camera {
private:
	float _position[3];
	float _rotation[3];

public:
	Camera();
	virtual ~Camera();

	const float* getPosition() const {
		return _position;
	}

	void setPosition(float x, float y, float z) {
		_position[0] = x;
		_position[1] = y;
		_position[2] = z;
	}

	const float* getRotation() const {
		return _rotation;
	}

	void setRotation(float x, float y, float z) {
		_rotation[0] = x;
		_rotation[1] = y;
		_rotation[2] = z;
	}

	// Returns true if the world space bounding box with the specified corners has non-zero intersection with the camera's view frustum
	bool isVisible(float min[3], float max[3]);

	// Returns the Model-View-Matrix
	Matrix4 getViewMatrix() {
		return Matrix4(_rotation[0],	0.0f,			0.0f,			_position[0],
						0.0f,			_rotation[1],	0.0f,			_position[1],
						0.0f,			0.0f,			_rotation[2],	_position[2],
						0.0f,			0.0f,			0.0f,			1.0f);
	}

	void render();

	void walkForeward(float distance);
	void walkBackward(float distance);
	void strafeLeft(float distance);
	void strafeRight(float distance);
	void moveUp(float distance);
	void moveDown(float distance);

	void turnLeft(float angle);
	void turnRight(float angle);
	void lookUp(float angle);
	void lookDown(float angle);
};

#endif /* CAMERA_H_ */
