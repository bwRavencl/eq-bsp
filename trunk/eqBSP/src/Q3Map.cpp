/*
 * Q3Map.cpp
 *
 *  Created on: 16.07.2012
 *      Author: matteo
 */

#include "Q3Map.h"
#include "Utilities/MathUtils.h"
#include "Matrices/Matrices.h"
#include <valarray>
#include <GL/gl.h>
#include <boost/bind.hpp>
#include <algorithm>
#include <set>

Q3Map::Q3Map(const std::string& filepath, Camera &camera) {
	_camera = camera;
	readMap(filepath, _map);

	// We need to have the vertices in a simple array to draw them
	_vertices = new TVertex[_map.mVertices.size()];
	copy(_map.mVertices.begin(), _map.mVertices.end(), _vertices);

	_facesColors = new Colors[_map.mFaces.size()];
	for (unsigned int i = 0; i < _map.mFaces.size(); i++) {
		struct Colors c;
		c.r = 1 - (float) rand() / RAND_MAX;
		c.g = 1 - (float) rand() / RAND_MAX;
		c.b = 1 - (float) rand() / RAND_MAX;
		c.a = 1 - (float) rand() / RAND_MAX;
		_facesColors[i] = c;
	}
}

Q3Map::~Q3Map() {
	freeMap(_map);
}

int Q3Map::findLeaf() const {
	int index = 0;

	while (index >= 0) {
		const TNode& node = _map.mNodes[index];
		const TPlane& plane = _map.mPlanes[node.mPlane];

		// Distance from point to a plane
		const double distance = dotProduct(plane.mNormal, _camera.getPosition())
				- plane.mDistance;

		if (distance >= 0) {
			index = node.mChildren[0]; // Index of Child in front
		} else {
			index = node.mChildren[1]; // Index of Child in back
		}
	}

	return -index - 1;
}

bool Q3Map::isClusterVisible(int testCluster) {
	int visCluster = findCameraCluster();

	if ((_map.mVisData.mBuffer == NULL) || (visCluster < 0)) {
		return true;
	}

	int i = (visCluster * _map.mVisData.mBytesPerCluster) + (testCluster >> 3); // (testCluster >> 3) computes (testCluster / 8), i.e. the byte within visData that contains information about the given cluster
	char visSet = _map.mVisData.mBuffer[i];

	return (visSet & (1 << (testCluster & 7))) != 0; // (1 << (testCluster & 7)) creates a bit mask that selects bit (testCluster mod 8) within that byte
}

std::vector<int> Q3Map::findVisibleFaces() {
	std::vector<int> visibleFaces;
	std::set<int> alreadyVisibleFaces;

	// Iterate through entire leaf vector
	for (std::vector<TLeaf>::iterator it = _map.mLeaves.begin();
			it != _map.mLeaves.end(); ++it) {

		// Handle only leaves that are potentially visible
		if (isClusterVisible((*it).mCluster)) {

			// Iterate over all faces that are contained by the leaf
			for (int i = 0; i < (*it).mNbLeafFaces; ++i) {
				const int faceIndex =
						_map.mLeafFaces.at((*it).mLeafFace + i).mFaceIndex; // The index of the face we are checking next

				// Add face index to the return vector, if it is not already determined visible
				if (alreadyVisibleFaces.find(faceIndex)
						== alreadyVisibleFaces.end()) {
					alreadyVisibleFaces.insert(faceIndex);
					visibleFaces.push_back(faceIndex);
				}
			}
		}
	}

	return visibleFaces;
}

bool Q3Map::isInFrontOf(const int faceIndexA, const int faceIndexB) {
	const TFace faceA = _map.mFaces.at(faceIndexA);
	const TFace faceB = _map.mFaces.at(faceIndexB);

	const TVertex firstVertexA = _map.mVertices.at(faceA.mVertex);
	const TVertex firstVertexB = _map.mVertices.at(faceB.mVertex);

	float translationVectorA[3] = { firstVertexA.mPosition[0],
			firstVertexA.mPosition[1], firstVertexA.mPosition[2] };
	float translationVectorB[3] = { firstVertexB.mPosition[0],
			firstVertexB.mPosition[1], firstVertexB.mPosition[2] };

	const Matrix4 modelMatrixA = Matrix4(1.0f, 0.0f, 0.0f,
			translationVectorA[0], 0.0f, 1.0f, 0.0f, translationVectorA[1],
			0.0f, 0.0f, 1.0f, translationVectorA[2], 0.0f, 0.0f, 0.0f, 1.0f);
	const Matrix4 modelMatrixB = Matrix4(1.0f, 0.0f, 0.0f,
			translationVectorB[0], 0.0f, 1.0f, 0.0f, translationVectorB[1],
			0.0f, 0.0f, 1.0f, translationVectorB[2], 0.0f, 0.0f, 0.0f, 1.0f);

	const Matrix4 cameraMatrix = _camera.getViewMatrix();

	const Matrix4 modelViewMatrixA = worldToCameraSpace(modelMatrixA,
			cameraMatrix);
	const Matrix4 modelViewMatrixB = worldToCameraSpace(modelMatrixB,
			cameraMatrix);

	const float zA = modelViewMatrixA[12];
	const float zB = modelViewMatrixA[12];

	if (zA < zB)
		return true;
	else
		return false;
}

void Q3Map::sortFaces(std::vector<int> faces, bool backToFront) {
	// We need to use a specialized boost functor, to access the non-static predicate function
	if (backToFront)
		std::sort(faces.begin(), faces.end(),
				!boost::bind(&Q3Map::isInFrontOf, this, _1, _2)); // Use the predicate the inverse way
	else
		std::sort(faces.begin(), faces.end(),
				boost::bind(&Q3Map::isInFrontOf, this, _1, _2));
}

void Q3Map::renderPolygon(TFace face) {
	static const int stride = sizeof(TVertex); // BSP Vertex, not float[3]

	const int offset = face.mVertex; // Index of first vertex
	//const TVertex firstVertex = arr[0/*offset*/]; //_map.mVertices.at(offset); // First vertex

	//set array pointers
	glVertexPointer(3, GL_FLOAT, stride, &(_vertices[0].mPosition)); //&(firstVertex.mPosition));

	//Draw face
	glDrawArrays(GL_TRIANGLE_FAN, offset, face.mNbVertices);
}

void Q3Map::renderMesh(TFace face) {
	static const int stride = sizeof(TVertex); // BSP Vertex, not float[3]

	const TVertex firstVertex = _vertices[0/*offset*/]; //_map.mVertices.at(offset); // First vertex

//	std::cout << "Rendering Mesh Vertex at: " << firstVertex.mPosition[0] << " " << firstVertex.mPosition[1] << " " << firstVertex.mPosition[2] << std::endl;

	glVertexPointer(3, GL_FLOAT, stride, &(_vertices[0].mPosition)); //&(firstVertex.mPosition));

	glDrawElements(GL_TRIANGLES, face.mNbMeshVertices, GL_UNSIGNED_INT,
			&_map.mMeshVertices.at(face.mMeshVertex));
}

void Q3Map::renderPatch(TFace face) {
	// TODO Implement Rendering code
}

void Q3Map::render() {
	std::vector<int> visibleFaces = findVisibleFaces();
	std::vector<int> opaqueFaces;
	std::vector<int> transparentFaces;
	std::vector<int>::iterator it;
	TFace curFace;

	// Partition visible faces in opaque and transparent lists
	for (it = visibleFaces.begin(); it != visibleFaces.end(); ++it) {
		//curFace = _map.mFaces.at((*it));
		// TODO do actual check for opacity or transparency
		opaqueFaces.push_back(*it);
	}

//	std::cout << "Global Faces:\t\t" << _map.mFaces.size() << std::endl << "Opaque Faces:\t\t" << opaqueFaces.size() << std::endl << "Transparent Faces:\t" << transparentFaces.size() << std::endl;
//	for (it = opaqueFaces.begin(); it != opaqueFaces.end(); ++it) {
//		std::cout << "FaceIndex: " << (*it) << std::endl;
//	}

	// Sort opaque faces front to back
	sortFaces(opaqueFaces);

	// Sort transparent faces back to front
	sortFaces(transparentFaces, true);

	// Turn whole map, so that it is positioned in the right way
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

	glFrontFace(GL_CW);

	// Enable vertex arrays
	glEnableClientState(GL_VERTEX_ARRAY);
//	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

//	glClientActiveTextureARB(GL_TEXTURE1_ARB);
//	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
//	glClientActiveTextureARB(GL_TEXTURE0_ARB);

	// Render opaque faces

	unsigned int colorI = 0;

	for (it = opaqueFaces.begin(); it != opaqueFaces.end(); ++it) {
		curFace = _map.mFaces.at((*it));

		// TODO Render Textures
		// const TTexture texture = _map.mTextures.at(curFace.mTextureIndex);
		// glBindTexture()

		glColor4f(_facesColors[colorI].r, _facesColors[colorI].g, _facesColors[colorI].b, _facesColors[colorI].a);

		switch (curFace.mType) {
		case 1:
			renderPolygon(curFace);
		case 3:
			renderMesh(curFace);
		case 4:
			renderPatch(curFace);
		}

		colorI++;
	}

	// TODO Render transparent faces

	// Disable vertex arrays
//	glClientActiveTextureARB(GL_TEXTURE1_ARB);
//	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//	glClientActiveTextureARB(GL_TEXTURE0_ARB);
//
//	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glFrontFace(GL_CCW);
}
