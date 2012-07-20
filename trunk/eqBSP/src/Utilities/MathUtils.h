/*
 * MathUtils.h
 *
 *  Created on: 16.07.2012
 *      Author: matteo
 */

#include "../Matrices/Matrices.h"

// Returns dot product of two vectors a and b
float dotProduct(const float a[3], const float b[3]) {
	return (a[0] * b[0] + a[1] * b[1] + a[2] * b[2]);
}

// Calculates the Model-View-Matrix
Matrix4 worldToCameraSpace(Matrix4 modelMatrix, Matrix4 viewMatrix) {
	return viewMatrix * modelMatrix;
}
