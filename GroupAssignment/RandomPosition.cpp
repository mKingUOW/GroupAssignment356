#include "stdafx.h"
#include "RandomPosition.h"

RandomPosition::RandomPosition()
{
	srand(time(0));
	tankOnGround = 18;
	int i = 0;
	minMaxXZ[red][minX] = -1650;
	minMaxXZ[red][maxX] = -1250;
	minMaxXZ[red][minZ] = -750;
	minMaxXZ[red][maxZ] = 750;

	minMaxXZ[battle][minX] = -350;
	minMaxXZ[battle][maxX] = 350;
	minMaxXZ[battle][minZ] = -750;
	minMaxXZ[battle][maxZ] = 750;

	minMaxXZ[blue][minX] = 1250;
	minMaxXZ[blue][maxX] = 1650;
	minMaxXZ[blue][minZ] = -750;
	minMaxXZ[blue][maxZ] = 750;
}

Ogre::Vector3 RandomPosition::getRandPosition(int area)
{
	Ogre::Vector3 randPos(0, tankOnGround, 0);
	
	setX(area, randPos);
	setZ(area, randPos);
			
	return randPos;
}

//sets the x value of the vector passed to a random value within and including 
//the min and max values of x set in constructor
void RandomPosition::setX(int area, Ogre::Vector3 &pos)
{
	int range, x;
	
	//if both values are negative need to deduct the min from the max otherwise deduct max from min
	if (minMaxXZ[area][minX] < 0 && minMaxXZ[area][maxX] < 0)
		range = minMaxXZ[area][minX] - minMaxXZ[area][maxX];
	else
		range = minMaxXZ[area][maxX] - minMaxXZ[area][minX];

	//calculate a random value in the range and add it to the min x value
	x = rand() % range;
	pos.x = x + minMaxXZ[area][minX];
}

//sets the z value of the vector passed to a random value within and including 
//the min and max values of z set in constructor
void RandomPosition::setZ(int area, Ogre::Vector3 &pos)
{
	int range, z;
	range = minMaxXZ[area][maxZ] - minMaxXZ[area][minZ];

	//calculate a random value in the range and add it to the min z value
	z = rand() % range;
	pos.z = z + minMaxXZ[area][minZ];
}