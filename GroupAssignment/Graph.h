#ifndef __Graph_h_
#define __Graph_h_

#include "stdafx.h"
#include <math.h>

#include "OgreRoot.h"
#include "connection.h"

#define GRID_DIMENSION 15	// 15 x 15 grid
#define TOTAL_NODES 225
#define SQUARE_SIZE 100

class Graph
{
private:
	static int mGrid[TOTAL_NODES];
	float mDiagonalCost;

public:
	Graph();
	~Graph();

	// given a node number, return the contents of the node, i.e. 0 or 1
	int getContent(int nodeNum);
	// given a node number, return the x, y, z position of the node's centre
	Ogre::Vector3 getPosition(int nodeNum);
	// given an x, y, z position, compute which node it belongs to
	int getNode(Ogre::Vector3 position);
	// given a node, return all the outgoing connections
	void getConnections(int fromNode, Connection connections[]);
};

#endif