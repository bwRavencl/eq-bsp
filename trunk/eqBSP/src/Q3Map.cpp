/*
 * Q3Map.cpp
 *
 *  Created on: 16.07.2012
 *      Author: matteo
 */

#include "Q3Map.h"
#include "Utilities/MathUtils.c"

Q3Map::Q3Map(const std::string& filepath) {
	_patches = NULL;

	readMap(filepath, _map);
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
			it <= _map.mLeaves.end(); ++it) {

		// Handle only leaves that are potentially visible
		if (isClusterVisible((*it).mCluster)) {

			// Iterate over all faces that are contained by the leaf
			for (int i = 0; i < (*it).mNbLeafFaces; ++i) {
				const int faceIndex = i + (*it).mLeafFace; // The index of the face we are checking next

				// Add face index to the return vector, if it is not already determined visible
				if (alreadyVisibleFaces.find(faceIndex)
						!= alreadyVisibleFaces.end()) {
					alreadyVisibleFaces.insert(faceIndex);
					visibleFaces.push_back(faceIndex);
				}
			}
		}
	}

	return visibleFaces;
}
