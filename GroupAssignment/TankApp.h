/*
-----------------------------------------------------------------------------
Filename:    TankApp.h
Modified By: Matthew King
-----------------------------------------------------------------------------
 
This file was created using the Orge tutorial source code as a template and
as a result may contain some original code from the Ogre tutorial.

-----------------------------------------------------------------------------
*/

#ifndef __TankApp_h_
#define __TankApp_h_
 
#include "stdafx.h"
#include "BaseApplication.h"
#include "PathFinding.h"

#include <fstream>
#include <string>

class TankApp : public BaseApplication
{
public:
    TankApp(void);
    virtual ~TankApp(void);

	MoveableEntity* getCurrentSelection();
	MoveableEntity* getPrevSelection();
	void toggleChase();

	static void setCurrentSelection(MoveableEntity *current);
	static bool isPFEnabled();
	static Graph* getGraphRef();

	RandomPosition *getPosMgr();

	//path finding related
	void enablePFNetwork();

	//constants
	static const int GRID_SEGMENTS = 28;
	static const int NUM_TEAMS = 2;
	static const int NUM_STARTING_ENTS = 2;
	static const int MAX_NUM_TEAM_ENTITIES = 4;	//max per team
	static const int MAX_ENTITIES = 8;
	static const int HALF_MAP_SIZE = 750;
	static const int RED = 0;
	static const int BLUE = 2;
 
protected:
	virtual bool setup();
    virtual void createScene(void);

	virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
    virtual bool keyPressed( const OIS::KeyEvent &arg );
    virtual bool keyReleased( const OIS::KeyEvent &arg );
	virtual bool mouseMoved( const OIS::MouseEvent &arg );
    virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
	virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

private:
	//methods
	void updatePositions(Ogre::Real time);
	void updateSelectionBox();
	void checkBoxSelection();
	void checkClickSelection();
	Ogre::Plane createPlane(const Ogre::Vector3 &v1, const Ogre::Vector3 &v2, const Ogre::Vector3 &v3);
	void detachAll();

	//scene creation
	void addTank(MoveableEntity& tank, int count, int team);
	void createBoundaryWalls();
	void createWall(std::string &name, Ogre::Vector3 postition, Ogre::Real scaleFactor, Ogre::Real scaleFactorZ);

	//path finding
	void checkState();
	void toggleGrid();
	void createGrid();

	int mCurrentState;
	int startNode;
	int goalNode;
	static Graph* pathFindingGraph;
	Ogre::ManualObject* path;
	static bool pathFindingEnabled;
	PathFinding mPathFinder;

	RandomPosition *randPosMgr;		//used for managing random position generation

	Ogre::ManualObject* grid[GRID_SEGMENTS];
	bool isGridVisible;

	//static variables
	static MoveableEntity *currentSelection;
	static MoveableEntity *previousSelection;
	static int currentNumTanks;
	static int currentTanksPerTeam;

	//variables
	Ogre::Entity* tankEnt[MAX_ENTITIES];
	MoveableEntity blueTeam[MAX_NUM_TEAM_ENTITIES];
	MoveableEntity redTeam[MAX_NUM_TEAM_ENTITIES];
	int mSelectedRobots[MAX_NUM_TEAM_ENTITIES];

	float mMove;
	float mCameraMove;
	Ogre::Vector3 moveVector;
	Ogre::SceneNode *cameraNode;
	Ogre::SceneNode *boxNode;

	// Creates a selection box
	Ogre::ManualObject* createSelectionBox(Ogre::String name = "SelectionBox");

	// Converts a 3D world position to a 2D relative screen coordinate (i.e. between 0 and 1 for x and y respectively)
	Ogre::Vector2 worldToScreenPosition(const Ogre::Vector3& position);

	// Selection box object
	Ogre::ManualObject* mSelectionBox;

	void createPath(Ogre::ManualObject* line, float height, std::vector<int>& path, Ogre::ColourValue& colour);

	// Mouse positions
	float normMouseX;
	float normMouseY;
	Ogre::Vector2 pos1;
	Ogre::Vector2 pos2;

	bool mSelecting;
	bool drawingBox;
	bool alterSelection;
	bool isBoxAttached;

	//debug variables
	bool inside;
	Ogre::String name;
};
 
#endif // #ifndef __TankApp_h_