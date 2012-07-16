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

	std::set<int> _alreadyVisibleFaces;
	int *_visibleFaces;
	TFace *_patches;

public:
	Q3Map(const std::string& filepath);
	virtual ~Q3Map();

	int findLeaf(const float& camPos) const;
	bool isClusterVisible(int visCluster, int testCluster) const;
};

#endif /* Q3MAP_H_ */
