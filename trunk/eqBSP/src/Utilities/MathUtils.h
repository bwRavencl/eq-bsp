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

// Vertex addition
TVertex vertexAddition(TVertex a, TVertex b) {
	TVertex result;

	int i;

	for (i = 0; i < 3; i++) {
		result.mPosition[i] = a.mPosition[i] + b.mPosition[i];
		result.mNormal[i] = a.mNormal[i] + b.mNormal[i];
	}

	for (int j = 0; j < 2; j++) {
		for (i = 0; i < 2; i++) {
			result.mTexCoord[i][j] = a.mTexCoord[i][j] + b.mTexCoord[i][j];
		}
	}

	return result;
}

// Vertex multiplication
TVertex vertexMultiplication(TVertex v, float f) {
	TVertex result;

	int i;

	for (i = 0; i < 3; i++) {
		result.mPosition[i] = v.mPosition[i] * f;
		result.mNormal[i] = v.mNormal[i] * f;
	}

	for (int j = 0; j < 2; j++) {
		for (i = 0; i < 2; i++) {
			result.mTexCoord[i][j] = v.mTexCoord[i][j] * f;
		}
	}

	return result;
}
