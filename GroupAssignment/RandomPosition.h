#ifndef RandomPostition_h
#define RandomPostition_h

#include "stdafx.h"
#include <cstdlib>

class RandomPosition
{
public:
	RandomPosition();
	Ogre::Vector3 getRandPosition(int area);

private:
	enum spawnArea {red, battle, blue};
	enum XZValues {minX, maxX, minZ, maxZ};
	Ogre::Real tankOnGround;
	int minMaxXZ[3][4];		//min and max X and Y positions for each area

	void setX(int area, Ogre::Vector3 &pos);
	void setZ(int area, Ogre::Vector3 &pos);

};

#endif