/*
 * Q3Map.cpp
 *
 *  Created on: 16.07.2012
 *      Author: matteo
 */

#define SKYBOX_SIZE 4096.0f

#define	SURF_SKY 0x4

#define POLYGON 1
#define PATCH 2
#define MESH 3
#define BILLBOARD 4

#define TESSELATION_LEVEL 10

#include "Q3Map.h"
#include "Utilities/MathUtils.h"
#include "Utilities/OpenGLExtensions.h"
#include "Utilities/TextureLoader.h"
#include "Matrices/Matrices.h"
#include <valarray>
#include <GL/gl.h>
#include <boost/bind.hpp>
#include <algorithm>
#include <set>

static const std::string basePath = "/home/matteo/tmp/baseq3a/";
static const unsigned char gray[1][1][3] = { { { 128, 128, 128 } } };

Q3Map::Q3Map(const std::string& filepath, Camera *camera) {
	unsigned int i;
	_camera = camera;

	// Read the map
	readMap(filepath, _map);

	// We need to have the vertices in a simple array to draw them
	_vertices = new TVertex[_map.mVertices.size()];
	copy(_map.mVertices.begin(), _map.mVertices.end(), _vertices);

	// We need to have the mesh vertices in a simple array to draw them
	_meshVertices = new int[_map.mMeshVertices.size()];

	for (i = 0; i < _map.mMeshVertices.size(); i++) {
		_meshVertices[i] = _map.mMeshVertices.at(i).mMeshVert;
	}

	// Load textures
	_textures = new GLuint[_map.mTextures.size()];
	for (i = 0; i < _map.mTextures.size(); i++) {
		_textures[i] = loadTexture(
				basePath + _map.mTextures.at(i).mName + ".jpg");
		if (!_textures[i])
			loadTexture(basePath + _map.mTextures.at(i).mName + ".tga");
	}

	// Load lightmaps
	int numberOfLightmaps = _map.mLightMaps.size();
	_lightmaps = new GLuint[numberOfLightmaps];

	// Generate the lightmap identifiers
	glGenTextures(numberOfLightmaps, _lightmaps);

	for (int i = 0; i < numberOfLightmaps; ++i) {
		glBindTexture(GL_TEXTURE_2D, _lightmaps[i]);

		//Create texture
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA8, 128, 128, GL_RGB,
				GL_UNSIGNED_BYTE, _map.mLightMaps.at(i).mMapData);

		//Set Parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	// Create white texture for if no lightmap specified
	glGenTextures(1, &_whiteTexture);
	glBindTexture(GL_TEXTURE_2D, _whiteTexture);
	//Create texture
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA8, 1, 1, GL_RGB, GL_UNSIGNED_BYTE,
			&gray);
	//Set Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	std::string skyTextureName = "";
	for (std::vector<TTexture>::iterator it = _map.mTextures.begin();
			it != _map.mTextures.end(); ++it) {
		if ((*it).mFlags & 0x4) {
			skyTextureName = (*it).mName;
			break;
		}
	}
	_skyTexture = loadTexture(basePath + "textures/skies/pjbasesky.jpg");
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
		const double distance = dotProduct(plane.mNormal,
				_camera->getPosition()) - plane.mDistance;

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
//		if (isClusterVisible((*it).mCluster)) {

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
//		}
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

	const Matrix4 cameraMatrix = _camera->getViewMatrix();

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
	std::sort(faces.begin(), faces.end(),
			boost::bind(&Q3Map::isInFrontOf, this, _1, _2));

	// Reverse order if requested
	if (backToFront) {
		faces.reserve(faces.size());
	}
}

void Q3Map::renderPolygon(TFace face) {
	if (_textures[face.mTextureIndex] == false)
		return;

	static const int stride = sizeof(TVertex); // BSP Vertex, not float[3]
	const int offset = face.mVertex; // Index of first vertex

	// Set array pointers
	// Vertices
	glVertexPointer(3, GL_FLOAT, stride, &(_vertices[0].mPosition));

	// Texture
	glTexCoordPointer(2, GL_FLOAT, stride, &_vertices[0].mTexCoord[0]);

	// Lightmap
	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glTexCoordPointer(2, GL_FLOAT, stride, &_vertices[0].mTexCoord[1]);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);

	// Bind textures
	// Texture
	if (_textures[face.mTextureIndex])
		glBindTexture(GL_TEXTURE_2D, _textures[face.mTextureIndex]);

	// Lightmap
	glActiveTextureARB(GL_TEXTURE1_ARB);
	if (face.mLightmapIndex >= 0)	//only bind a lightmap if one exists
		glBindTexture(GL_TEXTURE_2D, _lightmaps[face.mLightmapIndex]);
	else
		glBindTexture(GL_TEXTURE_2D, _whiteTexture);

	glActiveTextureARB(GL_TEXTURE0_ARB);

	// Draw face
	glDrawArrays(GL_TRIANGLE_FAN, offset, face.mNbVertices);
}

void Q3Map::renderMesh(TFace face) {
	if (_textures[face.mTextureIndex] == false)
		return;

	static const int stride = sizeof(TVertex); // BSP Vertex, not float[3]

	// Set array pointers
	// Vertices
	glVertexPointer(3, GL_FLOAT, stride, &(_vertices[face.mVertex].mPosition));

	// Texture
	glTexCoordPointer(2, GL_FLOAT, stride,
			&_vertices[face.mVertex].mTexCoord[0]);

	// Lightmap
	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glTexCoordPointer(2, GL_FLOAT, stride,
			&_vertices[face.mVertex].mTexCoord[1]);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);

	// Bind textures
	// Texture
	if (_textures[face.mTextureIndex])
		glBindTexture(GL_TEXTURE_2D, _textures[face.mTextureIndex]);

	// Lightmap
	glActiveTextureARB(GL_TEXTURE1_ARB);
	if (face.mLightmapIndex >= 0)	//only bind a lightmap if one exists
		glBindTexture(GL_TEXTURE_2D, _lightmaps[face.mLightmapIndex]);
	else
		glBindTexture(GL_TEXTURE_2D, _whiteTexture);

	glActiveTextureARB(GL_TEXTURE0_ARB);

//	glDrawElements(GL_TRIANGLES, face.mNbMeshVertices, GL_UNSIGNED_INT,
//			&_meshVertices[face.mMeshVertex]);

	glDrawRangeElementsEXT(GL_TRIANGLES, 0, face.mNbVertices,
			face.mNbMeshVertices, GL_UNSIGNED_INT,
			(const void**) (&_meshVertices[face.mMeshVertex]));
}

void Q3Map::renderPatch(TFace face, const int tesselationLevel) {
	if (_textures[face.mTextureIndex] == false)
		return;

	static const int stride = sizeof(TVertex); // BSP Vertex, not float[3]

	// Tesselate

	// Tesselation-Level
	const int level = tesselationLevel;

	float px, py;

	// The number of vertices along a side is 1 + num edges
	const int level1 = level + 1;

	TVertex *vertices = new TVertex[level1 * level1];

	TVertex controls[9];

	int i;

	// Gather controls
	for (i = 0; i < face.mNbVertices; ++i) {
		controls[i] = _vertices[face.mVertex + i];
	}

	for (int v = 0; v <= level; ++v) {
		px = (float) v / level;

		vertices[v] = vertexAddition(
				vertexAddition(
						vertexMultiplication(controls[0],
								((1.0f - px) * (1.0f - px))),
						vertexMultiplication(controls[3],
								((1.0f - px) * px * 2))),
				vertexMultiplication(controls[6], (px * px)));
	}

	// Compute the vertices
	for (i = 0; i <= level; ++i) {
		px = (float) i / level;
		py = 1.0f - px;

		vertices[i] = vertexAddition(
				vertexAddition(vertexMultiplication(controls[0], (py * py)),
						vertexMultiplication(controls[3], (2 * py * px))),
				vertexMultiplication(controls[6], (px * px)));
	}

	for (i = 1; i <= level; ++i) {
		px = (float) i / level;
		py = 1.0f - px;

		TVertex temp[3];

		int j;
		for (j = 0; j < 3; ++j) {
			int k = 3 * j;
			temp[j] = vertexAddition(
					vertexAddition(
							vertexMultiplication(controls[k + 0], (py * py)),
							vertexMultiplication(controls[k + 1],
									(2 * py * px))),
					vertexMultiplication(controls[k + 2], (px * px)));
		}

		for (j = 0; j <= level; ++j) {
			px = (float) j / level;
			py = 1.0f - px;

			vertices[i * level1 + j] = vertexAddition(
					vertexAddition(vertexMultiplication(temp[0], (py * py)),
							vertexMultiplication(temp[1], (2 * py * px))),
					vertexMultiplication(temp[2], (px * px)));
		}
	}

	// Compute the indices
	int row;
	int indices[(level * (level + 1) * 2)];

	for (row = 0; row < level; ++row) {
		for (int col = 0; col <= level; ++col) {
			indices[(row * (level + 1) + col) * 2 + 1] = row * level1 + col;
			indices[(row * (level + 1) + col) * 2] = (row + 1) * level1 + col;
		}
	}

	int trianglesPerRow[level];
	int *rowIndices[level];

	for (row = 0; row < level; ++row) {
		trianglesPerRow[row] = 2 * level1;
		rowIndices[row] = &indices[row * 2 * level1];
	}

	glVertexPointer(3, GL_FLOAT, stride, &vertices[0].mPosition);

	glTexCoordPointer(2, GL_FLOAT, stride, &vertices[0].mTexCoord[0]); // Texture

	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glTexCoordPointer(2, GL_FLOAT, stride, &vertices[0].mTexCoord[1]); // Lightmap
	glClientActiveTextureARB(GL_TEXTURE0_ARB);

	// Bind textures
	// Texture
	if (_textures[face.mTextureIndex])
		glBindTexture(GL_TEXTURE_2D, _textures[face.mTextureIndex]);

	// Lightmap
	glActiveTextureARB(GL_TEXTURE1_ARB);
	if (face.mLightmapIndex >= 0)	//only bind a lightmap if one exists
		glBindTexture(GL_TEXTURE_2D, _lightmaps[face.mLightmapIndex]);
	else
		glBindTexture(GL_TEXTURE_2D, _whiteTexture);

	glActiveTextureARB(GL_TEXTURE0_ARB);

//	for(int row=0; row<level; ++row)
//	{
//		glDrawElements(	GL_TRIANGLE_STRIP, 2*(level+1), GL_UNSIGNED_INT,
//						&indices[row*2*(level+1)]);
//	}

	glMultiDrawElementsEXT(GL_TRIANGLE_STRIP, trianglesPerRow, GL_UNSIGNED_INT,
			(const void **) (rowIndices), level);
}

void Q3Map::renderFaces(std::vector<int> faces) {
	std::vector<int>::iterator it;
	TFace curFace;
	unsigned int colorI = 0;

	for (it = faces.begin(); it != faces.end(); ++it) {
		curFace = _map.mFaces.at((*it));

		switch (curFace.mType) {
		case POLYGON:
			renderPolygon(curFace);
			break;
		case MESH:
			renderMesh(curFace);
			break;
		case PATCH:
			renderPatch(curFace, TESSELATION_LEVEL);
			break;
		case BILLBOARD:
			break;
		default:
			break;
		}

		colorI++;
	}
}

void Q3Map::renderSkybox() {
	if (_skyTexture == false)
		return;

	glPushMatrix();
	glLoadIdentity();

	glRotatef(_camera->getRotation()[0], 1.0f, 0.0f, 0.0f);
	glRotatef(_camera->getRotation()[1], 0.0f, 1.0f, 0.0f);
	glRotatef(_camera->getRotation()[2], 0.0f, 0.0f, 1.0f);

	// Enable/Disable features
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	// Just in case we set all vertices to white.
	glColor4f(1, 1, 1, 1);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, _skyTexture);

	// Render the front quad
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);
	glTexCoord2f(1, 0);
	glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);
	glTexCoord2f(1, 1);
	glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
	glTexCoord2f(0, 1);
	glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
	glEnd();

	// Render the left quad
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
	glTexCoord2f(1, 0);
	glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);
	glTexCoord2f(1, 1);
	glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
	glTexCoord2f(0, 1);
	glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
	glEnd();

	// Render the back quad
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
	glTexCoord2f(1, 0);
	glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
	glTexCoord2f(1, 1);
	glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
	glTexCoord2f(0, 1);
	glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);

	glEnd();

	// Render the right quad
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);
	glTexCoord2f(1, 0);
	glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
	glTexCoord2f(1, 1);
	glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
	glTexCoord2f(0, 1);
	glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
	glEnd();

	// Render the top quad
	glBegin(GL_QUADS);
	glTexCoord2f(0, 1);
	glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
	glTexCoord2f(0, 0);
	glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
	glTexCoord2f(1, 0);
	glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
	glTexCoord2f(1, 1);
	glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
	glEnd();

	// Render the bottom quad
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);
	glTexCoord2f(0, 1);
	glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
	glTexCoord2f(1, 1);
	glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
	glTexCoord2f(1, 0);
	glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);
	glEnd();

	// Restore enable bits and matrix
	glPopAttrib();
	glPopMatrix();
}

void Q3Map::render() {
	renderSkybox();

	std::vector<int> visibleFaces = findVisibleFaces();
	std::vector<int> opaqueFaces;
	std::vector<int> transparentFaces;
	std::vector<int>::iterator it;

	// Partition visible faces in opaque and transparent lists
	for (it = visibleFaces.begin(); it != visibleFaces.end(); ++it) {
		//curFace = _map.mFaces.at((*it));
		// TODO do actual check for opacity or transparency
		opaqueFaces.push_back(*it);
	}

	// Sort opaque faces front to back
	sortFaces(opaqueFaces, false);

	// Sort transparent faces back to front
	sortFaces(transparentFaces, true);

	// Turn whole map, so that it is positioned in the right way
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

	glFrontFace(GL_CW);

	// Enable vertex arrays
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);

	// Render opaque faces
	renderFaces(opaqueFaces);

	// Render transparent faces
	renderFaces(transparentFaces);

	// Disable vertex arrays
	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glFrontFace(GL_CCW);
}
