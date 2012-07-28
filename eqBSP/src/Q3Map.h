/*
 * Q3Map.h
 *
 *  Created on: 16.07.2012
 *      Author: matteo
 */

#ifndef Q3MAP_H_
#define Q3MAP_H_

#include "SimpleQ3Loader/Q3Loader.h"
#include "Camera.h"
#include <GL/gl.h>

class Q3Map {
private:
	TMapQ3 _map;
	Camera *_camera;

	TVertex *_vertices;
	int *_meshVertices;

	GLuint *_textures;
	GLuint *_lightmaps;
	GLuint _whiteTexture; //used if no lightmap specified

	GLuint _skyTexture;


	// Walks the BSP tree until a leaf is found and returns the leaf index
	int findLeaf() const;

	// Returns the index of the cluster containing the camera
	int findCameraCluster() {
		return _map.mLeaves.at(findLeaf()).mCluster;
	}

	// Returns true if the testCluster is potentially visible
	bool isClusterVisible(int testCluster);

	// Returns a vector with the indices of all potentially visible leaves
	std::vector<int> findVisibleFaces();

	// Predicate that sorts faces from front to back
	bool isInFrontOf(const int faceIndexA, const int faceIndexB);

	// Quicksorts a vector containing face-indices from front to back or back to front
	void sortFaces(std::vector<int> faces, bool backToFront = false);

	// Renders a polygon
	void renderPolygon(TFace face);

	// Renders a mesh
	void renderMesh(TFace face);

	// Renders a Patch
	void renderPatch(TFace face, const int tesselationLevel);

	void renderFaces(std::vector<int> faces);

	// Renders the Skybox
	void renderSkybox();

public:
	Q3Map(const std::string& filepath, Camera *camera);
	virtual ~Q3Map();

	// Render the map
	void render();
};

#endif /* Q3MAP_H_ */
