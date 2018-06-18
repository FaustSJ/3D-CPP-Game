#ifndef AGENT_H
#define AGENT_H
#include "BaseApplication.h"
#include <deque>
#include "Grid.h"
//class Grid;
//class GridNode;
//class GridRows;

class Agent
{
private:
	Ogre::SceneManager* mSceneMgr;		// pointer to scene graph
	Ogre::SceneNode* mBodyNode;			
	Ogre::Entity* mBodyEntity;
	float height;						// height the character should be moved up
	float scale;						// scale of character from original model
	Grid* agentGrid;	//for pathfinding
	int agentCompass;
	int startNodeID; //for pathfinding
	bool moving; //the user can't interact with the agent once it's moving
	

	// all of the animations our character has, and a null ID
	// some of these affect separate body parts and will be blended together

	Ogre::Real mTimer;						// general timer to see how long animations have been playing

	// for locomotion
	Ogre::Real mDistance;					// The distance the agent has left to travel
	Ogre::Vector3 mDirection;				// The direction the object is moving
	Ogre::Vector3 mDestination;				// The destination the object is moving towards
	std::deque<Ogre::Vector3> mWalkList;	// The list of points we are walking to
	Ogre::Real velocity;					// The speed at which the object is moving

	bool nextLocation();					// Is there another destination?
	void updateLocomote(Ogre::Real deltaTime);			// update the character's walking

	//for physics
	Ogre::Vector3 initialPosition;
	Ogre::Real timer;

public:
	Agent(Ogre::SceneManager* SceneManager, std::string name, std::string filename, float height, float scale);
	~Agent();
	void setPosition(float x, float y, float z);
	Ogre::Vector3 getPosition();
	Ogre::Vector3 getOrientation();

	void update(Ogre::Real deltaTime);		// update the agent
	void blastOff(Grid* g, int comp);

	Ogre::AxisAlignedBox getAgentBoundingBox();

	//for physics
	void setDirection(const OIS::MouseEvent& evt);
	void setVelocity(Ogre::Real v);
	bool isMoving();
	void reset(GridNode *newPosNode);
	Ogre::Entity* getEntity(); //for collision



};
#endif
