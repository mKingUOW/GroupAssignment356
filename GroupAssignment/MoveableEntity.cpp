#include "stdafx.h"
#include "MoveableEntity.h"
#include "TankApp.h"

//static variables
int MoveableEntity::numSelected = 0;
int MoveableEntity::tanksPerTeam = 0;
int MoveableEntity::totalBBCreated = 0;

const float FULL_HEALTH = 1.0f;

//--------------------------------------------------------MOVEABLE ENTITY---------------------------------------------------//
MoveableEntity::MoveableEntity()
{
	selected = false;
	tankHealth = 0;
	tankNode = NULL;
	healthName = "health";
	circleName = "circle";
	mMove = 80;
	mChase = 40;
	mRotate = 250;
	rotationSoFar = 0;
	isRotating = false;
	isMoving = false;
	isChasing = false;
	nodeToChase = NULL;
	moveVector = Ogre::Vector3(80, 0, 0);
	chaseVector = Ogre::Vector3(40, 0, 0);
}

//sets the scenenode for the entity
void MoveableEntity::setSceneNode(Ogre::SceneNode* node)
{
	tankNode = node;
}

//returns the scenenode for the entity
Ogre::SceneNode* MoveableEntity::getSceneNode()
{
	return tankNode;
}

//sets the health for the entity
void MoveableEntity::setHealth(float health)
{
	tankHealth = health;
}

//gets the health for the entity
float MoveableEntity::getHealth()
{
	return tankHealth;
}

//sets whether or not the entity is selected
void MoveableEntity::toggleSelection()
{
	//if already selected detach billboard
	if (isSelected())
		detach();
	else
		attach();
}

//returns whether or not the entity is currently selected
bool MoveableEntity::isSelected()
{
	return selected;
}

//sets the unique name for the health bar
void MoveableEntity::setHealthName(const Ogre::String num)
{
	healthName.append(num);
}

//sets the unique name for the selection circle
void MoveableEntity::setCircleName(const Ogre::String num)
{
	circleName.append(num);
}

//sets up billboards for health bar and selection circle for the entity
void MoveableEntity::setupBillboards(Ogre::SceneManager* mSceneMgr)
{
	// Create a BillboardSet to represent a health bar and set its properties
	mHealthBar = mSceneMgr->createBillboardSet(healthName);
	mHealthBar->setCastShadows(false);
	mHealthBar->setDefaultDimensions(45, 3);
	mHealthBar->setMaterialName("myMaterial/HealthBar");

	// Create a billboard for the health bar BillboardSet
	mHealthBarBB = mHealthBar->createBillboard(Ogre::Vector3(0, 100, 0));

	// Set it to always draw on top of other objects
	mHealthBar->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY);

	// Create a BillboardSet for a selection circle and set its properties
	mSelectionCircle = mSceneMgr->createBillboardSet(circleName);
	mSelectionCircle->setCastShadows(false);
	mSelectionCircle->setDefaultDimensions(150, 150);
	if (team == TankApp::BLUE)
		mSelectionCircle->setMaterialName("myMaterial/SelectionCircleBlue");
	else
		mSelectionCircle->setMaterialName("myMaterial/SelectionCircleRed");
	mSelectionCircle->setBillboardType(Ogre::BillboardType::BBT_PERPENDICULAR_COMMON);
	mSelectionCircle->setCommonDirection(Ogre::Vector3(0, 1, 0));
	mSelectionCircle->setCommonUpVector(Ogre::Vector3(0, 0, -1));

	// Create a billboard for the selection circle BillboardSet
	mSelectionCircleBB = mSelectionCircle->createBillboard(Ogre::Vector3(0, -17, 0));
	mSelectionCircleBB->setTexcoordRect(0.0, 0.0, 1.0, 1.0);
}

void MoveableEntity::setupTank(Ogre::SceneManager* mSceneMgr, RandomPosition* posMgr, Ogre::Entity** allParts, int aTeam)
{
	team = aTeam;
	attachParts(mSceneMgr, allParts);
	setHealth(FULL_HEALTH);
	tankNode->translate(posMgr->getRandPosition(aTeam));

	std::ostringstream number;
	number << totalBBCreated++;

	setHealthName(number.str());
	setCircleName(number.str());

	setupBillboards(mSceneMgr);
}

//attaches entities to nodes and positions at random location based on team
void MoveableEntity::attachParts(Ogre::SceneManager* mSceneMgr, Ogre::Entity** allParts)
{
	for (int i = 0; i < 3; i++)
		parts[i] = allParts[i];
	tankNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	turretNode = tankNode->createChildSceneNode();
	barrelNode = turretNode->createChildSceneNode();

	tankNode->attachObject(parts[0]);

	turretNode->attachObject(parts[1]);
	turretNode->translate(0, 3, 0);

	barrelNode->attachObject(parts[2]);
	barrelNode->translate(-30, 10, 0);
}

int MoveableEntity::getTeam()
{
	return team;
}

void MoveableEntity::attach()
{
	attachHealthBar();
	attachCircle();
	selected = true;
	numSelected++;	//increments the number of entities selected
	TankApp::setCurrentSelection(this);
}

void MoveableEntity::detach()
{
	detachHealthBar();
	detachCircle();
	selected = false;
	numSelected--;	//decrements the number of entities selected
}
//adjusts the health before attaching the health bar to the entity
void MoveableEntity::attachHealthBar()
{
	adjustHealth();
	tankNode->attachObject(mHealthBar);
}

//detaches health bar from entity
void MoveableEntity::detachHealthBar()
{
	tankNode->detachObject(mHealthBar);
}

//attaches selection circle to entity
void MoveableEntity::attachCircle()
{
	tankNode->attachObject(mSelectionCircle);
}

//detaches selection circle from entity
void MoveableEntity::detachCircle()
{
	tankNode->detachObject(mSelectionCircle);
}

//adjust health bar to represent health of entity
void MoveableEntity::adjustHealth()
{
	// Calculate the health bar adjustments
	float healthBarAdjuster = (1.0 - tankHealth)/2;	// This must range from 0.0 to 0.5
	// Set the health bar to the appropriate level
	mHealthBarBB->setTexcoordRect(0.0 + healthBarAdjuster, 0.0, 0.5 + healthBarAdjuster, 1.0);
}

//turns the entity a fraction of the total angle to ensure a smooth rotation
//this will continue to be called until the totalAngle is reached
void MoveableEntity::turnEntity(Ogre::Real time)
{
	if (isPositive)	//positive rotation angle
	{
		if (rotationSoFar < rotateAngle)
		{
			rotAmount = (mRotate * time);
			tankNode->rotate(Ogre::Vector3::UNIT_Y, Ogre::Degree(rotAmount));
			rotationSoFar += Ogre::Degree(rotAmount);
			return;
		}
	}
	else	//negative rotation angle
	{
		if (rotationSoFar > rotateAngle)
		{
			rotAmount = (mRotate * time);
			tankNode->rotate(Ogre::Vector3::UNIT_Y, Ogre::Degree(rotAmount));
			rotationSoFar -= Ogre::Degree(rotAmount);
			return;
		}
	}
	//when total angle reached
	tankNode->lookAt(destination, Ogre::Node::TS_WORLD, Ogre::Vector3::UNIT_X);
	moveStart();
	rotationComplete();
	rotationSoFar = 0;
	distToDest = destination.distance(tankNode->_getDerivedPosition()); //sets the distance to the destination
	distTravelled = 0;
}

//moves the entity a fraction of the total distance to destination
//this will continue to be called until the destination is reached
void MoveableEntity::moveEntity(Ogre::Real time)
{
	//checks if the distance travelled is less than the total distance from the initial position to the destination
	if (distTravelled < distToDest)
	{
		tankNode->translate(time * moveVector, Ogre::Node::TS_LOCAL);
		distTravelled += time * mMove;
	}
	else //sets the final position of the scene node and completes the move
	{
		tankNode->setPosition(destination);
		moveComplete();
		if (TankApp::isPFEnabled() && nextNode != lastNode)
			startMove(TankApp::getGraphRef()->getPosition(path[nextNode++]));	//next node in path list
	}
}

//method used to chase another agent
void MoveableEntity::chaseEntity(Ogre::Real time)
{
	tankNode->lookAt(nodeToChase->getPosition(), Ogre::Node::TS_WORLD, Ogre::Vector3::UNIT_X);
	tankNode->translate(time * chaseVector, Ogre::Node::TS_LOCAL);
}

void MoveableEntity::startMove(Ogre::Vector3 dest)
{
	rotationStart();

	destination = dest;
	//get the angle between entity and destination
	rotateAngle = tankNode->_getDerivedPosition().angleBetween(destination);
	if (rotateAngle > Ogre::Degree(0))
		isPositive = true;
	else
		isPositive = false;
}

void MoveableEntity::update(Ogre::Real time)
{
	if (isRotating)
		turnEntity(time);
	else if (isChasing)
		chaseEntity(time);
	else if (isMoving)
		moveEntity(time);
	else {} //do nothing
}

void MoveableEntity::rotationStart()
{
	isRotating = true;
}

void MoveableEntity::rotationComplete()
{
	isRotating = false;
}

void MoveableEntity::moveStart()
{
	isMoving = true;
}

void MoveableEntity::moveComplete()
{
	isMoving = false;
}

void MoveableEntity::setPath(std::vector<int> &thePath)
{
	path.assign(thePath.begin(), thePath.end());
	nextNode = 1;
	lastNode = path.size();
}

int MoveableEntity::getFirstNode()
{
	return path[nextNode];
}

void MoveableEntity::chase(Ogre::SceneNode *node)
{
	nodeToChase = node;
	rotationStart();
}

bool MoveableEntity::getChaseStatus()
{
	return isChasing;
}
void MoveableEntity::setChaseStatus(bool chase)
{
	isChasing = chase;
	if (!chase)
			moveComplete();
}

//-------------------------------------------STATIC METHODS---------------------------------------------//

//returns the number of entites currently selected
int MoveableEntity::getNumSelected()
{
	return numSelected;
}

void MoveableEntity::setStartNumEnts(int startNum)
{
	tanksPerTeam = startNum;
}