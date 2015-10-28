/*
-----------------------------------------------------------------------------
Filename:    TankApp.cpp
Modified By: Matthew King
-----------------------------------------------------------------------------
 
This file was created using the Orge tutorial source code as a template and
as a result may contain some original code from the Ogre tutorial.

-----------------------------------------------------------------------------
*/

#include "stdafx.h"
#include "MoveableEntity.h"
#include "TankApp.h"
 
MoveableEntity* TankApp::currentSelection = NULL;
MoveableEntity* TankApp::previousSelection = NULL;
Graph* TankApp::pathFindingGraph = NULL;
bool TankApp::pathFindingEnabled = false;
int TankApp::currentTanksPerTeam = 2;	//default number of starting tanks per team
int TankApp::currentNumTanks = 4;		//default number of starting tanks in game

//-------------------------------------------------------------------------------------
TankApp::TankApp(void)
{
	isGridVisible = false;
	mCurrentState = 0;
	randPosMgr = new RandomPosition;
}
//-------------------------------------------------------------------------------------
TankApp::~TankApp(void)
{
	if (pathFindingGraph != NULL)
		delete pathFindingGraph;
	delete randPosMgr;
}

bool TankApp::setup(void)
{
	BaseApplication::setup();

	// Set the camera's position and where it is looking
	mCamera->setPosition(0, 600, 600);
	mCamera->lookAt(0, 0, 0);

	// Display the mouse cursor
	mTrayMgr->showCursor();
	mTrayMgr->hideLogo();

	// Create a selection box object
	mSelectionBox = createSelectionBox();

	alterSelection = false;
	mSelecting = false;
	drawingBox = false;
	isBoxAttached = false;
	mMove = 0.2;
	mCameraMove = 10;
	moveVector = Ogre::Vector3::ZERO;

	return true;
};

//-------------------------------------------------------------------------------------
void TankApp::createScene(void)
{
	// Set ambient lighting
	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));
 
    // Create a light
    Ogre::Light *light = mSceneMgr->createLight("Light1");
    light->setType(Ogre::Light::LT_POINT);
    light->setPosition(Ogre::Vector3(250, 150, 250));
    light->setDiffuseColour(Ogre::ColourValue::White);
    light->setSpecularColour(Ogre::ColourValue::White);

	// Use a plane to represent the ground
	Ogre::Plane plane(Ogre::Vector3::UNIT_Y, 0);
    Ogre::MeshManager::getSingleton().createPlane("ground", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        plane, 3400, 1600, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Z);

	Ogre::Entity* entGround = mSceneMgr->createEntity("GroundEntity", "ground");
	entGround->setMaterialName("myMaterial/GrassFloor");
	entGround->setCastShadows(false);
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(entGround);

	//create and add boundary walls to the map
	createBoundaryWalls();

	//create the camera node and attach a camera
	cameraNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	cameraNode->attachObject(mCamera);

	//create tank entities
	int i;
	MoveableEntity::setStartNumEnts(NUM_STARTING_ENTS);
	for (i = 0; i < NUM_STARTING_ENTS; i++)
		addTank(allTanks[i], i, BLUE);

	for (i; i < NUM_STARTING_ENTS*2; i++)
		addTank(allTanks[i], i, RED);

	// Set background colour to dark grey so that you can see the health bar
	mWindow->getViewport(0)->setBackgroundColour(Ogre::ColourValue(0.2f, 0.2f, 0.2f));

	//create the lines for the grid
	createGrid();
}

bool TankApp::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    if(mWindow->isClosed())
        return false;
 
	if(mShutDown)
		return false;
 
	//Need to capture/update each device
	mKeyboard->capture();
	mMouse->capture();

	//Move and rotate the player node
	cameraNode->translate(evt.timeSinceLastFrame * moveVector, Ogre::Node::TS_LOCAL);

	mTrayMgr->frameRenderingQueued(evt);

	if (!mTrayMgr->isDialogVisible())
	{
		if (mDetailsPanel->isVisible())   // if details panel is visible, then update its contents
		{
			mDetailsPanel->setParamValue(0, Ogre::StringConverter::toString(mCamera->getDerivedPosition().x));
			mDetailsPanel->setParamValue(1, Ogre::StringConverter::toString(mCamera->getDerivedPosition().y));
			mDetailsPanel->setParamValue(2, Ogre::StringConverter::toString(mCamera->getDerivedPosition().z));
			mDetailsPanel->setParamValue(4, name);
			mDetailsPanel->setParamValue(5, Ogre::StringConverter::toString(MoveableEntity::getNumSelected()));
			//mDetailsPanel->setParamValue(6, Ogre::StringConverter::toString();
			//mDetailsPanel->setParamValue(7, Ogre::StringConverter::toString(robot[0].distTravelled));
		}
	}
	//updates all entity positions
	updatePositions(evt.timeSinceLastFrame);

	return true;
}

// OIS::KeyListener
bool TankApp::keyPressed( const OIS::KeyEvent &arg )
{
	BaseApplication::keyPressed(arg);

    switch (arg.key)
	{
		case OIS::KC_ESCAPE: 
			mShutDown = true;
			break;

		case OIS::KC_LCONTROL:
			alterSelection = true;
			break;

		case OIS::KC_C:
			toggleChase();
			break;

		case OIS::KC_P:
			pathFindingEnabled = true;
			enablePFNetwork();
			break;

		case OIS::KC_1:
			toggleGrid();
			break;

		default:
			break;
	}

	return true;
}

bool TankApp::keyReleased(const OIS::KeyEvent &arg)
{
	BaseApplication::keyReleased(arg);

	switch (arg.key)
	{
		case OIS::KC_LCONTROL:
			alterSelection = false;
			break;

		default:
			break;
	}

	return true;
}

bool TankApp::mouseMoved( const OIS::MouseEvent &arg )
{
	if (mTrayMgr->injectMouseMove(arg)) return true;

	// only rotate camera if the right mouse button is held
	if (arg.state.buttonDown(OIS::MB_Right))
	{
		mCamera->yaw(Ogre::Degree(-0.1 * arg.state.X.rel));
		return true;
	}

	//camera zoom
	Ogre::Vector3 zVector(0, 0, (-arg.state.Z.rel * 0.1));
	mCamera->moveRelative(zVector);

	// If left mouse button is down and a starting position is already stored
	if(mMouse->getMouseState().buttonDown(OIS::MouseButtonID::MB_Left) && mSelecting)
	{
		drawingBox = true;
		updateSelectionBox();
	}

	normMouseX = static_cast<float>(mMouse->getMouseState().X.abs)/mMouse->getMouseState().width;
	normMouseY = static_cast<float>(mMouse->getMouseState().Y.abs)/mMouse->getMouseState().height;
	
	if (normMouseX < 0.1)
		moveVector.x -= mCameraMove;
	else if (normMouseX > 0.9)
		moveVector.x += mCameraMove;
	else
		moveVector.x = 0;

	if (normMouseY < 0.1)
		moveVector.z -= mCameraMove;
	else if (normMouseY > 0.9)
		moveVector.z += mCameraMove;
	else
		moveVector.z = 0;
	return true;
}

bool TankApp::mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{
	if (mTrayMgr->injectMouseDown(arg, id)) return true;

	switch (id)
	{
		case OIS::MB_Left:
			{
				// Store starting position
				pos1.x = mMouse->getMouseState().X.abs;
				pos1.y = mMouse->getMouseState().Y.abs;

				// Start position stored, move to next state
				mSelecting = true;
				
			}
			break;

		case OIS::MB_Right:
			mTrayMgr->hideCursor();
			break;

		default:
			break;
	}
	return true;
}

bool TankApp::mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{
	if (mTrayMgr->injectMouseUp(arg, id)) return true;

	switch (id)
	{
		case OIS::MB_Left:
			{
				// If visible set to invisible
				if(mSelectionBox->isVisible() && drawingBox)
				{
					mSelectionBox->setVisible(false);
					mSelecting = false;
					drawingBox = false;
					checkBoxSelection();
				}
				else
					checkClickSelection();
			}
			break;

		case OIS::MB_Right:
			mTrayMgr->showCursor();
			break;

		default:
			break;
	}

	return true;
}

MoveableEntity* TankApp::getCurrentSelection()
{
	return currentSelection;
}

void TankApp::toggleChase()
{
	//check there is only one entity selected
	if (MoveableEntity::getNumSelected() == 1)
	{
		//check toggle status of currently selected
		if (currentSelection->getChaseStatus())
			currentSelection->setChaseStatus(false);
		else
		{
			currentSelection->setChaseStatus(true);
			if (currentSelection != previousSelection && previousSelection != NULL)
				currentSelection->chase(previousSelection->getSceneNode());
		}

	}
}

bool TankApp::isPFEnabled()
{
	return pathFindingEnabled;
}

Graph* TankApp::getGraphRef()
{
	return pathFindingGraph;
}

RandomPosition* TankApp::getPosMgr()
{
	return randPosMgr;
}

//------------------------------------STATIC METHODS------------------------------------------//
void TankApp::setCurrentSelection(MoveableEntity* current)
{
	previousSelection = currentSelection;
	currentSelection = current;
}

//--------------------------------------------------------------------------------------------//

//enables path finding mode
void TankApp::enablePFNetwork()
{
	pathFindingGraph = new Graph;

	//add obstacles to the ground
	for(int nodeNumber=0; nodeNumber<TOTAL_NODES; nodeNumber++)
	{
		int contents = pathFindingGraph->getContent(nodeNumber);

		if(contents)
		{
			// Create unique name
			std::ostringstream oss;
			oss << nodeNumber;
			std::string entityName = "Cube" + oss.str();

			// Create entity
			Ogre::Entity* cube = mSceneMgr->createEntity(entityName, "cube.mesh");
			cube->setMaterialName("Examples/BumpyMetal");

			// Attach entity to scene node
			Ogre::SceneNode* myNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
			myNode->attachObject(cube);
			myNode->scale(1, 0.1, 1);
			
			// Place object at appropriate position
			Ogre::Vector3 position = pathFindingGraph->getPosition(nodeNumber);
			position.y = 0.5;
			myNode->translate(position);
		}
	}
	path = mSceneMgr->createManualObject("AStarPath");
	path->clear();
	mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(path);
}

void TankApp::updatePositions(Ogre::Real time)
{
	for (int i = 0; i < currentNumTanks; i++)
		allTanks[i].update(time);
}

void TankApp::updateSelectionBox()
{
	//if selection box is not attached to root scene node attach it
	if (!isBoxAttached)
	{
		mSceneMgr->getRootSceneNode()->attachObject(mSelectionBox);
		isBoxAttached = true;
	}

	float left, right, top, bottom;
	// Get screen width and height
	int screenWidth = mWindow->getWidth();
	int screenHeight = mWindow->getHeight();

	// Get end position
	pos2.x = mMouse->getMouseState().X.abs;
	pos2.y = mMouse->getMouseState().Y.abs;
				
	// Find left and right boundaries
	if(pos1.x < pos2.x)
	{
		left = static_cast<float>(pos1.x) / screenWidth;
		right = static_cast<float>(pos2.x) / screenWidth;
	}
	else
	{
		left = static_cast<float>(pos2.x) / screenWidth;
		right = static_cast<float>(pos1.x) / screenWidth;
	}

	// Find top and bottom boundaries
	if(pos1.y < pos2.y)
	{
		top = static_cast<float>(pos1.y) / screenHeight;
		bottom = static_cast<float>(pos2.y) / screenHeight;
	}
	else
	{
		top = static_cast<float>(pos2.y) / screenHeight;
		bottom = static_cast<float>(pos1.y) / screenHeight;
	}

	// Find selection box corners in 3D coordinates in front of the camera
	Ogre::Ray topLeft = mCamera->getCameraToViewportRay(left, top); 
	Ogre::Ray topRight = mCamera->getCameraToViewportRay(right, top); 
	Ogre::Ray bottomRight = mCamera->getCameraToViewportRay(right, bottom); 
	Ogre::Ray bottomLeft = mCamera->getCameraToViewportRay(left, bottom); 

	// Update the vertices of the selection box
	mSelectionBox->beginUpdate(0);
	mSelectionBox->position(topLeft.getPoint(1));
	mSelectionBox->position(topRight.getPoint(1));
	mSelectionBox->position(bottomRight.getPoint(1));
	mSelectionBox->position(bottomLeft.getPoint(1));
	mSelectionBox->position(topLeft.getPoint(1));

	// Finished defining the 2D line strip
	mSelectionBox->end();

	// If not visible set to visible
	if(!mSelectionBox->isVisible())
		mSelectionBox->setVisible(true);
			
}

void TankApp::checkBoxSelection()
{
	float left, right, top, bottom;
	// Get screen width and height
	int screenWidth = mWindow->getWidth();
	int screenHeight = mWindow->getHeight();

	// Get end position
	pos2.x = mMouse->getMouseState().X.abs;
	pos2.y = mMouse->getMouseState().Y.abs;
				
	// Find left and right boundaries
	if(pos1.x < pos2.x)
	{
		left = static_cast<float>(pos1.x) / screenWidth;
		right = static_cast<float>(pos2.x) / screenWidth;
	}
	else
	{
		left = static_cast<float>(pos2.x) / screenWidth;
		right = static_cast<float>(pos1.x) / screenWidth;
	}

	// Find top and bottom boundaries
	if(pos1.y < pos2.y)
	{
		top = static_cast<float>(pos1.y) / screenHeight;
		bottom = static_cast<float>(pos2.y) / screenHeight;
	}
	else
	{
		top = static_cast<float>(pos2.y) / screenHeight;
		bottom = static_cast<float>(pos1.y) / screenHeight;
	}

	// Find selection box corners in 3D coordinates in front of the camera
	Ogre::Ray topLeft = mCamera->getCameraToViewportRay(left, top); 
	Ogre::Ray topRight = mCamera->getCameraToViewportRay(right, top); 
	Ogre::Ray bottomRight = mCamera->getCameraToViewportRay(right, bottom); 
	Ogre::Ray bottomLeft = mCamera->getCameraToViewportRay(left, bottom); 

	//create the planes for the volume
	Ogre::PlaneBoundedVolume vol;
	vol.planes.push_back(createPlane(topLeft.getOrigin(), topRight.getOrigin(), bottomRight.getOrigin()));
	vol.planes.push_back(createPlane(topLeft.getOrigin(), topLeft.getPoint(10), topRight.getPoint(10)));
	vol.planes.push_back(createPlane(topLeft.getOrigin(), bottomLeft.getPoint(10), topLeft.getPoint(10)));
	vol.planes.push_back(createPlane(bottomLeft.getOrigin(), bottomRight.getPoint(10), bottomLeft.getPoint(10)));
	vol.planes.push_back(createPlane(topRight.getOrigin(), topRight.getPoint(10), bottomRight.getPoint(10)));

	Ogre::PlaneBoundedVolumeList volList;
	volList.push_back(vol);
	Ogre::PlaneBoundedVolumeListSceneQuery *volumeQuery = mSceneMgr->createPlaneBoundedVolumeQuery(Ogre::PlaneBoundedVolumeList());
	volumeQuery->setVolumes(volList);
	Ogre::SceneQueryResult &result = volumeQuery->execute();

	//detach all currently selected entities
	detachAll();

	Ogre::SceneQueryResultMovableList::iterator it;
	//increments the character value of i each iteration so we can use a character to search the entity name
	for (it = result.movables.begin(); it != result.movables.end(); ++it)
	{
		Ogre::String name = (*it)->getName();

		int index = getIndexFromString(name);
		
		//make sure index is within correct range, more thatn 0 and less than the current number of tanks in the game
		if(index >= 0 && index < currentNumTanks)
			allTanks[index].toggleSelection();
	}
	mSceneMgr->destroyQuery(volumeQuery);
	//need to detach the manual object to avoid inability to select entities underneath it
	mSceneMgr->getRootSceneNode()->detachObject(mSelectionBox);
	isBoxAttached = false;
}

void TankApp::checkClickSelection()
{
	// Get the mouse ray, i.e. ray from the mouse cursor 'into' the screen 
	Ogre::Ray mouseRay = mCamera->getCameraToViewportRay(
		static_cast<float>(mMouse->getMouseState().X.abs)/mMouse->getMouseState().width, 
		static_cast<float>(mMouse->getMouseState().Y.abs)/mMouse->getMouseState().height);

	Ogre::RaySceneQuery * mRaySceneQuery = mSceneMgr->createRayQuery(mouseRay);

	// Set type of objects to query
	mRaySceneQuery->setQueryTypeMask(Ogre::SceneManager::ENTITY_TYPE_MASK);

	mRaySceneQuery->setSortByDistance(true);

	// Ray-cast and get first hit
	Ogre::RaySceneQueryResult &result = mRaySceneQuery->execute();
	Ogre::RaySceneQueryResult::iterator itr = result.begin();

	inside = false;
	// If hit a movable object
	if(itr != result.end() && itr->movable)
	{
		// Get name of movable object that was hit
		name = itr->movable->getName();
		
		int index = getIndexFromString(name);
		
		//make sure index is within correct range, more thatn 0 and less than the current number of tanks in the game
		if(index >= 0 && index < currentNumTanks)
		{
			if (!alterSelection && !allTanks[index].isSelected())
				detachAll();
			allTanks[index].toggleSelection();
		}
		//can freely move around the map unless path finding is enabled
		else if (name == "GroundEntity" && !pathFindingEnabled)
		{
			// if there is exactly one entity selected
			if(MoveableEntity::getNumSelected() == 1)
			{
				// Get hit location
				Ogre::Vector3 location = mouseRay.getPoint(itr->distance);
				getCurrentSelection()->startMove(location);
			}
		}

		else {} //do nothing	
	}
	if (pathFindingEnabled)
		checkState();
}

//creates and returns an Ogre plane object
Ogre::Plane TankApp::createPlane(const Ogre::Vector3 &v1, const Ogre::Vector3 &v2, const Ogre::Vector3 &v3)
{
	return Ogre::Plane(v1, v2, v3);
}

//detaches all previously selected entities health bars and selection circles
void TankApp::detachAll()
{
	for (int i = 0; i < currentNumTanks; i++)
	{
		if (allTanks[i].isSelected())
			allTanks[i].detach();
	}
}

//character digit minus character baseChar will give us the correct index for an entity
//ASCII value of '0' is 30 therefore, 30 - 30 = 0, ASCII value of '1' is 31 therefore 31 - 30 = 1 etc
//THIS METHOD SHOULD ONLY BE USED FOR VALUES INCLUDING 0-9 (10 VALUES) 
int TankApp::getIndexFromString(const std::string& name)
{
	char baseChar = '0';

	//get the last character of the entities name
	char digit = name[name.length()-1];

	return digit - baseChar;
}


//--------------------------------------------------SCENE CREATION METHODS-------------------------------------------------//

//
void TankApp::addTank(MoveableEntity& tank, int count, int team)
{
	std::ostringstream oss;
	oss << count;
	std::string entBody = "body" + oss.str();
	std::string entTurret = "turret" + oss.str();
	std::string entBarrel = "barrel" + oss.str();

	Ogre::Entity* parts[3];
	parts[0] = mSceneMgr->createEntity(entBody, "chbody.mesh");
	parts[0]->setCastShadows(true);
	parts[1] = mSceneMgr->createEntity(entTurret, "chturret.mesh");
	parts[1]->setCastShadows(true);
	parts[2] = mSceneMgr->createEntity(entBarrel, "chbarrel.mesh");
	parts[2]->setCastShadows(true);

	tank.setupTank(mSceneMgr, randPosMgr, parts, team);

	return;
}

void TankApp::createBoundaryWalls()
{
	std::string entityName = "northWall";
	createWall(entityName, Ogre::Vector3(0,25,-812.5), 34.5f, 0.25f);
	entityName = "westWall";
	createWall(entityName, Ogre::Vector3(-1712.5,25,0), 0.25f, 16.0f);
	entityName = "southWall";
	createWall(entityName, Ogre::Vector3(0,25,812.5), 34.5f, 0.25f);
	entityName = "eastWall";
	createWall(entityName, Ogre::Vector3(1712.5,25,0), 0.25f, 16.0f);
	
}

void TankApp::createWall(std::string &name, Ogre::Vector3 position, Ogre::Real scaleFactorX, Ogre::Real scaleFactorZ)
{
	// Create entity
	Ogre::Entity* cube = mSceneMgr->createEntity(name, "cube.mesh");
	cube->setMaterialName("myMaterial/Rockwall");

	// Attach entity to scene node
	Ogre::SceneNode* myNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	myNode->attachObject(cube);
	myNode->scale(scaleFactorX, 0.5, scaleFactorZ);

	// Place object at appropriate position
	myNode->translate(position);

	return;
}

//add one additional tank to each team
void TankApp::addAdditionalTanks()
{

}


void TankApp::checkState()
{
	//// if path already exists
	//if (mCurrentState > 1)
	//{
	//	// reset
	//	mCurrentState = 0;
	//	path->clear();
	//}
	//// if no path yet
	//else
	//{
	//	// Create RaySceneQuery
	//	Ogre::Ray mouseRay = mCamera->getCameraToViewportRay(
	//		static_cast<float>(mMouse->getMouseState().X.abs)/mMouse->getMouseState().width, 
	//		static_cast<float>(mMouse->getMouseState().Y.abs)/mMouse->getMouseState().height);

	//	Ogre::RaySceneQuery * mRaySceneQuery = mSceneMgr->createRayQuery(Ogre::Ray());

	//	// Set ray
	//	mRaySceneQuery->setRay(mouseRay);

	//	// Ray-cast and get first hit
	//	Ogre::RaySceneQueryResult &result = mRaySceneQuery->execute();
	//	Ogre::RaySceneQueryResult::iterator itr = result.begin();

	//	// if hit an object
	//	if (itr != result.end())
	//	{
	//		// Get hit location
	//		Ogre::Vector3 location = mouseRay.getPoint(itr->distance);

	//		// if hit the floor
	//		if (location.y < 0.001)
	//		{
	//			// if no start node yet
	//			if (mCurrentState == 0)
	//			{
	//				// set start node
	//				startNode = pathFindingGraph->getNode(location);
	//				// set state to goal node state
	//				mCurrentState++;
	//			}
	//			// if start node already assigned
	//			else if (mCurrentState == 1)
	//			{
	//				// set goal node
	//				goalNode = pathFindingGraph->getNode(location);

	//				if (MoveableEntity::getNumSelected() == 1)
	//				{
	//					startNode = pathFindingGraph->getNode(robot[0].getSceneNode()->getPosition());
	//					path->clear();
	//				}

	//				// check that goal node is not the same as start node
	//				if (goalNode != startNode)
	//				{
	//					// try to find path from start to goal node
	//					std::vector<int> mainPath;

	//					// if path exists
	//					if (mPathFinder.AStar(startNode, goalNode, *pathFindingGraph, mainPath))
	//					{
	//						// draw path
	//						createPath(path, 0.5, mainPath, Ogre::ColourValue(1, 0, 0));

	//						//if an entity is selected we should set the path for it to follow
	//						if (MoveableEntity::getNumSelected() == 1)
	//						{
	//							robot[0].setPath(mainPath);
	//							robot[0].startMove(pathFindingGraph->getPosition(mainPath[1]));//********************************************************
	//						}

	//						// set state to path found
	//						mCurrentState++;
	//					}
	//					else
	//					{
	//						// no path so set state to no start node
	//						mCurrentState = 0;
	//					}
	//				}
	//			}
	//		}
	//	}
	//}
}

void TankApp::toggleGrid()
{
	for (int i = 0; i < GRID_SEGMENTS; i++)
		grid[i]->setVisible(!isGridVisible);
	isGridVisible = !isGridVisible;
}

void TankApp::createGrid()
{
	Ogre::Real startX = HALF_MAP_SIZE - SQUARE_SIZE;	//POSITIVE X
	Ogre::Real startZ = -HALF_MAP_SIZE;					//NEGATIVE Z
	Ogre::Real endX = startX;							//POSITIVE X
	Ogre::Real endZ = HALF_MAP_SIZE;					//POSITIVE Z
	int cols = GRID_SEGMENTS / 2;

	//draw column lines
	for (int i = 0; i < cols; i++)
	{
		// Create unique name
		std::ostringstream oss;
		oss << i;
		std::string lineName = "line" + oss.str();
		grid[i] = mSceneMgr->createManualObject(lineName);

		grid[i]->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_STRIP);
		grid[i]->position(Ogre::Vector3(startX, 1, startZ));
		grid[i]->position(Ogre::Vector3(endX, 1, endZ));
		grid[i]->colour(1, 1, 1);
		grid[i]->end();
		grid[i]->setVisible(false);
		
		startX -= SQUARE_SIZE;
		endX = startX;
		mSceneMgr->getRootSceneNode()->attachObject(grid[i]);
	}
	startX = -HALF_MAP_SIZE;
	startZ = -HALF_MAP_SIZE + SQUARE_SIZE;
	endX = HALF_MAP_SIZE;
	endZ = startZ;

	//draw row lines
	for (int i = cols; i < GRID_SEGMENTS; i++)
	{
		// Create unique name
		std::ostringstream oss;
		oss << i;
		std::string lineName = "line" + oss.str();
		grid[i] = mSceneMgr->createManualObject(lineName);

		grid[i]->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_STRIP);
		grid[i]->position(Ogre::Vector3(startX, 1, startZ));
		grid[i]->position(Ogre::Vector3(endX, 1, endZ));
		grid[i]->colour(1, 1, 1);
		grid[i]->end();
		grid[i]->setVisible(false);

		startZ += SQUARE_SIZE;
		endZ = startZ;
		mSceneMgr->getRootSceneNode()->attachObject(grid[i]);
	}
}

Ogre::ManualObject* TankApp::createSelectionBox(Ogre::String name)
{
	// Create manual object
	Ogre::ManualObject* box2D = mSceneMgr->createManualObject(name);

	// Indicate that object is likely to be updated
	box2D->setDynamic(true);
	box2D->estimateVertexCount(5);

	// Set in render queue without depth checking
	box2D->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY);

	// Specify the material and rendering type
	box2D->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_STRIP);
	
	// Points of the line strip
	box2D->position(0, 0, 0);
	box2D->position(0, 0, 0);
	box2D->position(0, 0, 0);
	box2D->position(0, 0, 0);
	box2D->position(0, 0, 0);

	// Finished defining the 2D line strip
	box2D->end();

	return box2D;
}

Ogre::Vector2 TankApp::worldToScreenPosition(const Ogre::Vector3& position)
{
	// The origin of the screen position here is at the centre of screen
	Ogre::Vector3 screenPosition = mCamera->getProjectionMatrix() * mCamera->getViewMatrix() * position;

	// Compute the 2D screen position with the origin at the top left of screen
	return Ogre::Vector2(0.5 + 0.5 * screenPosition.x, 0.5 - 0.5 *screenPosition.y);
}

void TankApp::createPath(Ogre::ManualObject* line, float height, std::vector<int>& path, Ogre::ColourValue& colour)
{
	line->clear();

	// Specify the material and rendering type
	line->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_STRIP);

	// Specify the vertices and vertex colour for the line
	Ogre::Vector3 position;

	for(std::vector<int>::iterator it=path.begin(); it!=path.end(); it++)
	{
		position = pathFindingGraph->getPosition(*it);
		line->position(Ogre::Vector3(position.x, height, position.z));
		line->colour(colour);
	}

	// Finished defining line
	line->end();
}


#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif
 
#ifdef __cplusplus
extern "C" {
#endif
 
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
    int main(int argc, char *argv[])
#endif
    {
        // Create application object
        TankApp app;
 
        try {
            app.go();
        } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
            std::cerr << "An exception has occured: " <<
                e.getFullDescription().c_str() << std::endl;
#endif
        }
 
        return 0;
    }
 
#ifdef __cplusplus
}
#endif