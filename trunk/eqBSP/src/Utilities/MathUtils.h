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

	for(int i = 0; i < 2; i++)
		result.mPosition[i] = a.mPosition[i] + b.mPosition[i];

	// TODO Do missing additions:
//	result.position=position+rhs.position;
//	result.decalS=decalS+rhs.decalS;
//	result.decalT=decalT+rhs.decalT;
//	result.lightmapS=lightmapS+rhs.lightmapS;
//	result.lightmapT=lightmapT+rhs.lightmapT;

	return result;
}

// Vertex multiplication
TVertex vertexMultiplication(TVertex v, float f) {
	TVertex result;

	for(int i = 0; i < 2; i++)
		result.mPosition[i] = v.mPosition[i] * f;

	// TODO Do missing multiplications:
//	result.position=position*rhs;
//	result.decalS=decalS*rhs;
//	result.decalT=decalT*rhs;
//	result.lightmapS=lightmapS*rhs;
//	result.lightmapT=lightmapT*rhs;

	return result;
}
