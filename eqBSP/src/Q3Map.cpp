/*
 * Q3Map.cpp
 *
 *  Created on: 16.07.2012
 *      Author: matteo
 */

#include "Q3Map.h"
#include "Utilities/MathUtils.c"

Q3Map::Q3Map(const std::string& filepath) {
	_visibleFaces = NULL;
	_patches = NULL;

	readMap(filepath, _map);
}

Q3Map::~Q3Map() {
	freeMap(_map);
}

// Walks the BSP tree until a leaf is found and returns the leaf index
int Q3Map::findLeaf(const float& camPos) const {

	int index = 0;

	while (index >= 0) {
		const TNode& node = _map.mNodes[index];
		const TPlane& plane = _map.mPlanes[node.mPlane];

		// Distance from point to a plane
		const double distance = dotProduct(plane.mNormal, &camPos)
				- plane.mDistance;

		if (distance >= 0) {
			index = node.mChildren[0]; // Index of Child in front
		} else {
			index = node.mChildren[1]; // Index of Child in back
		}
	}

	return -index - 1;
}

// Returns true if the testCluster is potentially visible
bool Q3Map::isClusterVisible(int visCluster, int testCluster) const {

	if ((_map.mVisData.mBuffer == NULL) || (visCluster < 0)) {
		return true;
	}

	int i = (visCluster * _map.mVisData.mBytesPerCluster) + (testCluster >> 3); // (testCluster >> 3) computes (testCluster / 8), i.e. the byte within visData that contains information about the given cluster
	char visSet = _map.mVisData.mBuffer[i];

	return (visSet & (1 << (testCluster & 7))) != 0; // (1 << (testCluster & 7)) creates a bit mask that selects bit (testCluster mod 8) within that byte
}

