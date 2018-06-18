#include "Agent.h"
#include <OgreMath.h>
#include <iostream>
//#include "Grid.h"

//////////////////
///////////////////////////CONSTRUCTORS AND SETUP////////////////////////////////////////////
/////////////////
Agent::Agent(Ogre::SceneManager* SceneManager, std::string name, std::string filename, float height, float scale)
{
	using namespace Ogre;

	mSceneMgr = SceneManager; // keep a pointer to where this agent will be

	if (mSceneMgr == NULL)
	{
		std::cout << "ERROR: No valid scene manager in Agent constructor" << std::endl;
		return;
	}

	this->height = height;
	this->scale = scale;
	moving = false;

	agentGrid = NULL;
	startNodeID = -1;

	mBodyNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(); // create a new scene node
	mBodyEntity = mSceneMgr->createEntity(name, filename); // load the model
	mBodyNode->attachObject(mBodyEntity);	// attach the model to the scene node

	mBodyNode->translate(0,height,0); // make the Ogre stand on the plane (almost)
	mBodyNode->scale(scale,scale,scale); // Scale the figure

	// configure walking parameters
	velocity = 10.0f;	
	mDirection = Ogre::Vector3::ZERO;
}

Agent::~Agent(){
	// mSceneMgr->destroySceneNode(mBodyNode); // Note that OGRE does not recommend doing this. It prefers to use clear scene
	// mSceneMgr->destroyEntity(mBodyEntity);
//	if(agentGrid!=NULL)
//		delete agentGrid;
	agentGrid = NULL;
	mBodyNode = NULL;
	mBodyEntity = NULL;
	if(mSceneMgr!=NULL)
	{
		mSceneMgr->clearScene();
		delete mSceneMgr;
		mSceneMgr = NULL;
	}
}

void Agent::setPosition(float x, float y, float z)
{   
	this->mBodyNode->setPosition(x, y + height, z);
	initialPosition = Ogre::Vector3(x, y+height, z);
}

Ogre::AxisAlignedBox Agent::getAgentBoundingBox()
{
	return mBodyEntity->getWorldBoundingBox();
}

Ogre::Vector3 Agent::getOrientation()
{   return mBodyNode->getOrientation()*Ogre::Vector3::UNIT_Z;}

Ogre::Vector3 Agent::getPosition()
{   return this->mBodyNode->getPosition();}

// update is called at every frame from GameApplication::addTime
void Agent::update(Ogre::Real deltaTime) 
{   this->updateLocomote(deltaTime);}

bool Agent::nextLocation()
{
	//if there are no more destinations to walk to, return false
	if (mWalkList.empty())
		return false;

	//otherwise, set up for the next destination
	mDestination = mWalkList.front(); //set the next destination
	mDestination.y = height;  //make sure the avatar doesn't sink or hover
	mWalkList.pop_front(); //remove the destination from the deque
	mDirection = mDestination - mBodyNode->getPosition(); //define which direction the char will face

	Ogre::Vector3 src = mBodyNode->getOrientation() * Ogre::Vector3::UNIT_Z;
	Ogre::Quaternion quat = src.getRotationTo(mDirection);
	mBodyNode->rotate(quat);

	mDistance = mDirection.normalise(); //normalize the Direction vector
	return true;
}

void Agent::updateLocomote(Ogre::Real deltaTime)
{
	//update the timer
	timer += deltaTime;

	//did we make it in the barrel?
	if(!moving)
		return;

	if(agentCompass==1) //N, fall S
	{
		//zPosition++;
		float zNew = initialPosition.z - (((-50)*timer) + (0.5f*(-50)*timer*timer));
		mBodyNode->setPosition(Ogre::Vector3(initialPosition.x, initialPosition.y, zNew));
	}
	else if(agentCompass==2) //E, fall W
	{
		//xPosition--;
		float xNew = initialPosition.x + (((-50)*timer) + (0.5f*(-50)*timer*timer));
		mBodyNode->setPosition(Ogre::Vector3(xNew, initialPosition.y, initialPosition.z));
	}
	else if(agentCompass==3) //S, fall N
	{
		//zPosition--;
		float zNew = initialPosition.z + (((-50)*timer) + (0.5f*(-50)*timer*timer));
		mBodyNode->setPosition(Ogre::Vector3(initialPosition.x, initialPosition.y, zNew));
	}
	else //W, fall E
	{
		//xPositon++;
		float xNew = initialPosition.x - (((-50)*timer) + (0.5f*(-50)*timer*timer));
		mBodyNode->setPosition(Ogre::Vector3(xNew, initialPosition.y, initialPosition.z));
	}
}

void Agent::setDirection(const OIS::MouseEvent& evt)
{
	if(moving==false)
	{
		//mBodyNode->yaw(Ogre::Degree(-evt.state.X.rel * 0.15f));
		mBodyNode->roll(Ogre::Degree(-evt.state.Y.rel * 0.15f));
	}
}

//////////////////
////////////////////////////////////DEFINE TRANSLATION////////////////////////////////////////////
/////////////////
void Agent::blastOff(Grid* g, int comp)
{
	//start the timer
	timer = 0.0f;
	//start moving
	if(moving!=true)
	   moving = true;
	agentGrid = g;
	agentCompass = comp;
}


void Agent::setVelocity(Ogre::Real v)
{   velocity = v;}
bool Agent::isMoving()
{   return moving;}
void Agent::reset(GridNode *newPosNode)
{
	//we are no longer moving
	moving = false;

	//adjust the agent's position to the nearest node
	Ogre::Vector3 newPosition = newPosNode->getPosition(agentGrid->getRows(), agentGrid->getCols());
	this->mBodyNode->setPosition(newPosition.x, newPosition.y+height, newPosition.z);
	initialPosition = Ogre::Vector3(newPosition.x, newPosition.y+height, newPosition.z);
}
Ogre::Entity* Agent::getEntity()
{   return mBodyEntity;}



