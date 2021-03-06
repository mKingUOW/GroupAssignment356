#ifndef MOVEABLEENTITY_H
#define MOVEABLEENTITY_H

#include "stdafx.h"
#include "BaseApplication.h"
#include "PathFinding.h"
#include "RandomPosition.h"

class MoveableEntity
{
	public:
		MoveableEntity();
		void setSceneNode(Ogre::SceneNode* node);
		Ogre::SceneNode* getSceneNode();
		void setHealth(float health);
		float getHealth();
		void toggleSelection();
		bool isSelected();
		void setHealthName(const Ogre::String num);
		void setCircleName(const Ogre::String num);
		void setupBillboards(Ogre::SceneManager* mSceneMgr);
		void setupTank(Ogre::SceneManager* mSceneMgr, RandomPosition* thePosMgr, Ogre::Entity** allParts, int team);
		void attachParts(Ogre::SceneManager* mSceneMgr, Ogre::Entity** allParts);
		int getTeam();

		void attach();
		void detach();
		void attachHealthBar();
		void detachHealthBar();
		void attachCircle();
		void detachCircle();
		void adjustHealth();

		//state methods
		void travelToBattleground();
		void searchForTarget(Ogre::Real time);
		void chaseTarget(Ogre::Real time);
		bool checkForEnemy();

		//movement methods
		void turnEntity(Ogre::Real time);
		void moveEntity(Ogre::Real time);
		void chaseEntity(Ogre::Real time);
		void startMove(Ogre::Vector3 dest);
		void rotationComplete();
		void rotationStart();
		void moveStart();
		void moveComplete();
		void setPath (std::vector<int> &thePath);
		int getFirstNode();

		void chase(Ogre::SceneNode *node);
		bool getChaseStatus();
		void setChaseStatus(bool chase);

		static int getNumSelected();
		static void setStartNumEnts(int startNum);

		Ogre::Degree rotationSoFar;
		Ogre::Real rotAmount;

		Ogre::Real distToDest;
		Ogre::Real distTravelled;

		// moved from private to be used in TankApp functions
		// {
				Ogre::SceneNode* tankNode;

				int team;		//0 is red, 2 is blue
				Ogre::Real timeSinceLastCheck;

				//states
				bool isSearchingForTarget;
				bool isChasingTarget;
				bool isRotating;
				bool isMoving;
				bool isChasing;
		
				bool isPositive;
		// }

	private:
		static int numSelected;
		static int tanksPerTeam;
		static int totalBBCreated;

		Ogre::SceneNode* turretNode;
		Ogre::SceneNode* barrelNode;
		RandomPosition* posMgr;

		Ogre::Entity* parts[3];
		float tankHealth;
		bool selected;
		Ogre::Vector3 destination;
		Ogre::SceneNode *nodeToChase;

		Ogre::BillboardSet* mHealthBar;
		Ogre::Billboard* mHealthBarBB;
		Ogre::BillboardSet* mSelectionCircle;
		Ogre::Billboard* mSelectionCircleBB;

		Ogre::String circleName;
		Ogre::String healthName;

		//rotation and movement
		Ogre::Real mMove;
		Ogre::Real mChase;
		Ogre::Real mRotate;
		Ogre::Degree rotateAngle;
		Ogre::Vector3 moveVector;
		Ogre::Vector3 chaseVector;
		std::vector<int> path;
		int nextNode;
		int lastNode;
};

#endif
