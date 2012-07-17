/*
 * Camera.h
 *
 *  Created on: 16.07.2012
 *      Author: matteo
 */

#ifndef CAMERA_H_
#define CAMERA_H_

class Camera {
private:
	float _position[3];

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

	// Returns true if the world space bounding box with the specified corners has non-zero intersection with the camera's view frustum
	bool isVisible(float min[3], float max[3]);
};

#endif /* CAMERA_H_ */
