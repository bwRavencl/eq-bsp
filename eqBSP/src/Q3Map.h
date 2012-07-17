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
#include <set>

class Q3Map {
private:
	TMapQ3 _map;
	Camera _camera;

	TFace *_patches;

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

public:
	Q3Map(const std::string& filepath);
	virtual ~Q3Map();
};

#endif /* Q3MAP_H_ */
