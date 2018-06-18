#include "Enemy.h"
#include <iostream>
//#include "Grid.h"

//////////////////
///////////////////////////CONSTRUCTORS AND SETUP////////////////////////////////////////////
/////////////////
Enemy::Enemy(Ogre::SceneManager* SceneManager, std::string name, std::string filename, float height, float scale)
{
	using namespace Ogre;

	mSceneMgr = SceneManager; // keep a pointer to where this enemy will be

	if (mSceneMgr == NULL)
	{
		std::cout << "ERROR: No valid scene manager in Enemy constructor" << std::endl;
		return;
	}

	this->height = height;
	this->scale = scale;

	enemyGrid = NULL;
	pathNodeFrom = NULL;
	pathNodeTo = NULL;
	startNodeID = -1;

	this->totalDH = -1;
	ready = false;

	mBodyNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(); // create a new scene node
	mBodyEntity = mSceneMgr->createEntity(name, filename); // load the model
	mBodyNode->attachObject(mBodyEntity);	// attach the model to the scene node

	mBodyNode->translate(0,height,0); // make the Ogre stand on the plane (almost)
	mBodyNode->scale(scale,scale,scale); // Scale the figure

	this->setupAnimations();

	// configure walking parameters
	mWalkSpeed = 35.0f;	
	mDirection = Ogre::Vector3::ZERO;
}

Enemy::~Enemy(){
	// mSceneMgr->destroySceneNode(mBodyNode); // Note that OGRE does not recommend doing this. It prefers to use clear scene
	// mSceneMgr->destroyEntity(mBodyEntity);
//	if(enemyGrid!=NULL)
//		delete enemyGrid;
//	if(pathNodeFrom!=NULL)
//		delete pathNodeFrom;
//	if(pathNodeTo!=NULL)
//		delete pathNodeTo;
	enemyGrid = NULL;
	pathNodeFrom = NULL;
	pathNodeTo = NULL;
//	mSceneMgr->clearScene();
	mBodyNode = NULL;
	mBodyEntity = NULL;
//	if(mSceneMgr!=NULL)
//		delete mSceneMgr;
	mSceneMgr = NULL;
}

void Enemy::setPosition(float x, float y, float z)
{
	this->mBodyNode->setPosition(x, y + height, z);
}

Ogre::Vector3 Enemy::getPosition()
{   return this->mBodyNode->getPosition();}

Ogre::AxisAlignedBox Enemy::getEnemyBoundingBox()
{
	return mBodyEntity->getWorldBoundingBox();
}

// update is called at every frame from GameApplication::addTime
void Enemy::update(Ogre::Real deltaTime) 
{
	if(ready)
	{
		this->updateAnimations(deltaTime);	// Update animation playback
		this->updateLocomote(deltaTime);	// Update Locomotion
	}
}

void Enemy::setupAnimations()
{
	this->mTimer = 0;	// Start from the beginning
	this->mVerticalVelocity = 0;	// Not jumping

	// this is very important due to the nature of the exported animations
	mBodyEntity->getSkeleton()->setBlendMode(Ogre::ANIMBLEND_CUMULATIVE);

	// Name of the animations for this character
	Ogre::String animNames[] =
		{"IdleBase", "IdleTop", "RunBase", "RunTop", "HandsClosed", "HandsRelaxed", "DrawSwords",
		"SliceVertical", "SliceHorizontal", "Dance", "JumpStart", "JumpLoop", "JumpEnd"};

	// populate our animation list
	for (int i = 0; i < 13; i++)
	{
		mAnims[i] = mBodyEntity->getAnimationState(animNames[i]);
		mAnims[i]->setLoop(true);
		mFadingIn[i] = false;
		mFadingOut[i] = false;
	}
	// start off in the idle state (top and bottom together)
	setBaseAnimation(ANIM_IDLE_BASE);
	setTopAnimation(ANIM_IDLE_TOP);

	// relax the hands since we're not holding anything
	mAnims[ANIM_HANDS_RELAXED]->setEnabled(true);
}

//////////////////
////////////////////////////////////ANIMATION////////////////////////////////////////////
/////////////////
void Enemy::setBaseAnimation(AnimID id, bool reset)
{
	if (mBaseAnimID >= 0 && mBaseAnimID < 13)
	{
		// if we have an old animation, fade it out
		mFadingIn[mBaseAnimID] = false;
		mFadingOut[mBaseAnimID] = true;
	}

	mBaseAnimID = id; 

	if (id != ANIM_NONE)
	{
		// if we have a new animation, enable it and fade it in
		mAnims[id]->setEnabled(true);
		mAnims[id]->setWeight(0);
		mFadingOut[id] = false;
		mFadingIn[id] = true;
		if (reset) mAnims[id]->setTimePosition(0);
	}
}
	
void Enemy::setTopAnimation(AnimID id, bool reset)
{
	if (mTopAnimID >= 0 && mTopAnimID < 13)
	{
		// if we have an old animation, fade it out
		mFadingIn[mTopAnimID] = false;
		mFadingOut[mTopAnimID] = true;
	}

	mTopAnimID = id;

	if (id != ANIM_NONE)
	{
		// if we have a new animation, enable it and fade it in
		mAnims[id]->setEnabled(true);
		mAnims[id]->setWeight(0);
		mFadingOut[id] = false;
		mFadingIn[id] = true;
		if (reset) mAnims[id]->setTimePosition(0);
	}
}

void Enemy::updateAnimations(Ogre::Real deltaTime)
{
	using namespace Ogre;

	Real baseAnimSpeed = 1;
	Real topAnimSpeed = 1;

	mTimer += deltaTime; // how much time has passed since the last update

	//if (mTopAnimID != ANIM_IDLE_TOP)
	//if (mTopAnimID != ANIM_NONE)
	//if (mTimer >= mAnims[mTopAnimID]->getLength())
	//	{
	//		setTopAnimation(ANIM_IDLE_TOP, true);
	//		setBaseAnimation(ANIM_IDLE_BASE, true);
	//		mTimer = 0;
	//	}
	
	// increment the current base and top animation times
	if (mBaseAnimID != ANIM_NONE) mAnims[mBaseAnimID]->addTime(deltaTime * baseAnimSpeed);
	if (mTopAnimID != ANIM_NONE) mAnims[mTopAnimID]->addTime(deltaTime * topAnimSpeed);

	// apply smooth transitioning between our animations
	fadeAnimations(deltaTime);
}

void Enemy::fadeAnimations(Ogre::Real deltaTime)
{
	using namespace Ogre;

	for (int i = 0; i < 13; i++)
	{
		if (mFadingIn[i])
		{
			// slowly fade this animation in until it has full weight
			Real newWeight = mAnims[i]->getWeight() + deltaTime * 7.5f; //ANIM_FADE_SPEED;
			mAnims[i]->setWeight(Math::Clamp<Real>(newWeight, 0, 1));
			if (newWeight >= 1) mFadingIn[i] = false;
		}
		else if (mFadingOut[i])
		{
			// slowly fade this animation out until it has no weight, and then disable it
			Real newWeight = mAnims[i]->getWeight() - deltaTime * 7.5f; //ANIM_FADE_SPEED;
			mAnims[i]->setWeight(Math::Clamp<Real>(newWeight, 0, 1));
			if (newWeight <= 0)
			{
				mAnims[i]->setEnabled(false);
				mFadingOut[i] = false;
			}
		}
	}
}

bool Enemy::nextLocation()
{
	//if there are no more destinations to walk to, return false
	if (mWalkList.empty())
	{
		this->totalDH = -1;
		if(startNodeID==pathNodeFrom->getID())
		{
			startNodeID = pathNodeTo->getID();
			bool pathFound = findPath(pathNodeTo, pathNodeFrom, 0);
		}
		else
		{
			startNodeID = pathNodeFrom->getID();
			bool pathFound = findPath(pathNodeFrom, pathNodeTo, 0);
		}
		enemyGrid->reset(); //reset the node's fields for the next enemy
	}

	//otherwise, set up for the next destination
	mDestination = mWalkList.front(); //set the next destination
	mDestination.y = height;  //make sure the avatar doesn't sink or hover
	mWalkList.pop_front(); //remove the destination from the deque
	mDirection = mDestination - mBodyNode->getPosition(); //define which direction the char will face

	mDistance = mDirection.normalise(); //normalize the Direction vector

	setDirection();

	return true;
}

void Enemy::updateLocomote(Ogre::Real deltaTime)
{
	
	if (mDirection == Ogre::Vector3::ZERO) 
	{
		if (nextLocation())
		{
			setBaseAnimation(ANIM_RUN_BASE);
			setTopAnimation(ANIM_RUN_TOP);
  		}
	}
	else
	{
		//slow the animation to work better with speed the character is moving
		Ogre::Real move = mWalkSpeed * deltaTime;
		mDistance -= move;

		//did we reach our destination?
		if (mDistance <= 0)
		{
			//make sure the avatar doesn't sink or hover
			mBodyNode->setPosition(mDestination);

			mDirection = Ogre::Vector3::ZERO;

			//rotate the character towards the next destination
			if (nextLocation())
			{   
				setDirection();
			}
			else
			{
				setBaseAnimation(ANIM_IDLE_BASE);
				setTopAnimation(ANIM_IDLE_TOP);
			}
		}
		else
		{//If the destination hasn't been reached, continue moving along the path
			//mDirection is defined in nexxtLocation()
			mBodyNode->translate(move * mDirection);
		}
	}
}

void Enemy::setDirection()
{
	//quaternions represent the rotation of a 3D object
	Ogre::Vector3 src = mBodyNode->getOrientation() * Ogre::Vector3::UNIT_Z;
 
	/*
	//this code could result in a division by zero if we ever need to rotate by exactly 180 degrees
	Ogre::Quaternion quat = src.getRotationTo(mDirection);
	mBodyNode->rotate(quat);
	*/

	//due to the limitations of floating point numbers, they may not
	//actually equal 0, but be very close to it. "close enough."
	//So we check for that instead.
	if ((1.0 + src.dotProduct(mDirection)) < 0.0001) 
	{
		mBodyNode->yaw(Ogre::Degree(180));
	}
	else
	{
		Ogre::Quaternion quat = src.getRotationTo(mDirection);
		mBodyNode->rotate(quat);
	}      
}

//////////////////
////////////////////////////////////DEFINE TRANSLATION////////////////////////////////////////////
/////////////////
void Enemy::walkTo(GridNode* gn, Grid* g)
{
	//save the grid as a field variable for setWayPoints() to access.
	enemyGrid = g;

	//Remember: z is rows, x is columns
	//grab the grid square that the enemy is currently on
	Ogre::Vector3 enemyCoor = enemyGrid->getGridPosition(this->mBodyNode->getPosition().z, this->mBodyNode->getPosition().x);
	//find the node representing the above grid square and call the pathfinding function

	pathNodeFrom = enemyGrid->getNode(enemyCoor.z, enemyCoor.x);
	pathNodeTo = gn;

	startNodeID = pathNodeFrom->getID();
	bool pathFound = findPath(pathNodeFrom, pathNodeTo, 0);
	enemyGrid->reset(); //reset the node's fields for the next enemy
	ready = true;
}

//takes the goalNode and traces all the way back to the startNode and saves the path
void Enemy::setWayPoints(GridNode* gn)
{
	//if any path was stored before, erase it because a more optimal A* path was found.
	if(!mWalkList.empty())
	{   mWalkList.clear();}

	int count = 0;
	//start by adding the goalNode...
	mWalkList.push_front(gn->getPosition(enemyGrid->getRows(), enemyGrid->getCols()));
	//...and work back to startNode from there
	while(gn->getPrevNode() != NULL)
	{
		count++;
		if(count==15)
			break;
		gn = gn->getPrevNode();
		mWalkList.push_front(gn->getPosition(enemyGrid->getRows(), enemyGrid->getCols()));
	}
} 


//This is a RECURSIVE function that implements pathfinding through A*
bool Enemy::findPath(GridNode* currentNode, GridNode* goalNode, int costSoFar)
{
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//SET UP
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	std::vector<GridNode*> adjList; //the list of the current node's neighbors
	std::vector<GridNode*> toTraverse; //the list of nodes to visit next	
	currentNode->setVisited(); //mark the current node as visited
	int minimum = -2;
	int nodeIndex = 0;
	int prevNodeIndex = -2; //-2 because PNI+1 is checked and -1 still isn't a proper index
	GridNode* bestNode = NULL;
	bool toReturn = false;

//	std::cout<< "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" <<std::endl;
//	std::cout<< "currently at node "<< currentNode->getRow() << "," << currentNode->getColumn() <<std::endl;
//	std::cout<<"goalNode is row,col "<<goalNode->getRow()<<", "<<goalNode->getColumn()<<std::endl;
//	std::cout<< "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" <<std::endl;

	//STOP CONDITION 1 : if the goal node is reached
	if(currentNode->getID() == goalNode->getID()) //compare IDs
	{
//		std::cout<< "Path found!" <<std::endl;
		if((this->totalDH > (currentNode->getHeuristic() + costSoFar))||(this->totalDH == -1)) 
		{   
//			std::cout<< "This is currently the shortest path." <<std::endl;
			setWayPoints(currentNode);
			this->totalDH = currentNode->getHeuristic() + costSoFar;
		}
		else
//		{   std::cout<< "...however, it's not the shortest path." <<std::endl;}
		enemyGrid->softReset();
		return true; 
	}
	//STOP CONDITION 2 : if this path's d+h surpasses the totalDH (set by another path)
	//if((this->totalDH < (currentNode->getHeuristic() + costSoFar))&&(this->totalDH != -1))
	//{   
	//	std::cout<< "Path halted, shorter path found." <<std::endl;
	//	std::cout<< "totalDH = "<<totalDH << std::endl;
	//	std::cout<< "this node's cost total: "<<currentNode->getHeuristic()<<" + "<<costSoFar<<std::endl;
	//	enemyGrid->softReset();
	//	return true; 
	//}
	if(this->totalDH!=-1)
	{
		return true;
	}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//start by picking the node with the best cost+heuristic
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	//compiles a list of adjacent, non-Null node. Their costs have been set.
	adjList = enemyGrid->getWalkableAdjacentNodes(currentNode);//grab neighbors

	int counter = -1; //for index tracking
	for(GridNode* adjNode : adjList)
	{   
		counter++; //for index tracking
		if(adjNode != NULL)
		{
			//update each neighbor's g+h
			if(!adjNode->isHeuristicSet())
			{   adjNode->setHeuristic(enemyGrid->getDistance(adjNode, goalNode));}

			//only check the visitable nodes
			if(adjNode->notVisited())//&&(adjNode->isClear()))
			{
				//have the first visitable node's heuristic set the minimum
				if(minimum == -2)
				{   minimum = adjNode->getCost() + adjNode->getHeuristic();}

				//is the node's g+h < the minimum? Is the node also NOT the previous node nor the start node?
				if(((adjNode->getCost() + adjNode->getHeuristic()) <= minimum)&&((currentNode->getPrevNode()==NULL)||(adjNode->getID() != currentNode->getPrevNode()->getID()))&&(adjNode->getID()!=startNodeID))
				{
					minimum = adjNode->getCost() + adjNode->getHeuristic();
//					std::cout<<"minimum is "<<minimum<<std::endl;
//					std::cout<<"node's cost is "<<adjNode->getCost()<<std::endl;
//					std::cout<<"node's heuristic is "<<adjNode->getHeuristic()<<std::endl;
					bestNode = adjNode;
					nodeIndex = counter;
//					std::cout<< "best node has a Row,Col of " << bestNode->getRow() << "," << bestNode->getColumn() <<std::endl;
				}		
			}
			//is the node is visited, check if it's previous and save its index
			else
			{
				if(adjNode->getID()==currentNode->getPrevNode()->getID())
					prevNodeIndex=counter;
			}
		}
//		else{   std::cout<<"null node found."<<std::endl;}
	}
	
	if((bestNode!=NULL)&&(bestNode->isClear()))
	{   
//		std::cout<< "bestNode is walkable." <<std::endl;
		toTraverse.push_back(bestNode);
	}//add to list of nodes to traverse
//	else
//	{   std::cout<<"Best node is a wall."<<std::endl;}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//store the walkable adjacent nodes as well
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	if(bestNode!=NULL)
	{
		//we start by checking the directly-adjacent nodes
		//clockwise node
		if(nodeIndex==7)
		{
			if((adjList[0]!=NULL)&&(adjList[0]->isClear())&&(adjList[0]->notVisited()))
			{   
				//make sure it isn't the previous node, nor the start node, nor the nodes adjacent to the previous node
				if((prevNodeIndex!=0)&&(adjList[0]->getID()!=startNodeID)&&(prevNodeIndex!=7)&&(prevNodeIndex!=1))
				{
//					std::cout<< "directly-adjacent node "<< adjList[0]->getRow() << "," << adjList[0]->getColumn() << " is walkable." <<std::endl;
					toTraverse.push_back(adjList[0]);
		}	}	}	
		else
		{
			if((adjList[nodeIndex+1]!=NULL)&&(adjList[nodeIndex+1]->isClear())&&(adjList[nodeIndex+1]->notVisited()))
			{   
				//make sure it isn't the previous node or start node
				if((prevNodeIndex!=nodeIndex+1)&&(adjList[nodeIndex+1]->getID()!=startNodeID))
				{
					//make sure it isn't adjacent to prevNode
					//only checking (nodeIndex+1)+1, not (nodeIndex+1)-1
					if((nodeIndex+2==8)&&(prevNodeIndex!=0))
					{
//						std::cout<< "directly-adj+acent node "<< adjList[nodeIndex+1]->getRow() << "," << adjList[nodeIndex+1]->getColumn() << " is walkable." <<std::endl;
						toTraverse.push_back(adjList[nodeIndex+1]);
					}
					else if(nodeIndex+2!=prevNodeIndex)
					{
//						std::cout<< "directly-adjacent node "<< adjList[nodeIndex+1]->getRow() << "," << adjList[nodeIndex+1]->getColumn() << " is walkable." <<std::endl;
						toTraverse.push_back(adjList[nodeIndex+1]);
		}	}	}	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//counter-clockwise node
		if(nodeIndex==0)
		{
			if((adjList[7]!=NULL)&&(adjList[7]->isClear())&&(adjList[7]->notVisited()))
			{   
				//make sure it isn't the previous node, nor the start node, nor the nodes adjacent to the previous node
				if((prevNodeIndex!=7)&&(adjList[7]->getID()!=startNodeID)&&(prevNodeIndex!=6)&&(prevNodeIndex!=0))
				{
//					std::cout<< "directly-adjacent node "<< adjList[7]->getRow() << "," << adjList[7]->getColumn() << " is walkable." <<std::endl;
					toTraverse.push_back(adjList[7]);
		}	}	}	
		else
		{
			if((adjList[nodeIndex-1]!=NULL)&&(adjList[nodeIndex-1]->isClear())&&(adjList[nodeIndex-1]->notVisited()))
			{   
				//make sure it isn't the previous node or start node
				if((prevNodeIndex!=nodeIndex-1)&&(adjList[nodeIndex-1]->getID()!=startNodeID))
				{
					//make sure it isn't adjacent to prevNode
					if((nodeIndex-2==-1)&&(prevNodeIndex!=7))
					{
//						std::cout<< "directly-adjacent node "<< adjList[nodeIndex-1]->getRow() << "," << adjList[nodeIndex-1]->getColumn() << " is walkable." <<std::endl;
						toTraverse.push_back(adjList[nodeIndex-1]);
					}
					if(nodeIndex-2!=prevNodeIndex)
					{
//						std::cout<< "directly-adjacent node "<< adjList[nodeIndex-1]->getRow() << "," << adjList[nodeIndex-1]->getColumn() << " is walkable." <<std::endl;
						toTraverse.push_back(adjList[nodeIndex-1]);
		}	}	}	}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//check direct-diagonal nodes
		//clockwise node
		if(nodeIndex==7)
		{
			if((adjList[1]!=NULL)&&(adjList[1]->isClear())&&(adjList[1]->notVisited()))
			{   
				//make sure it isn't the previous node, nor the start node, nor the nodes adjacent to the previous node
				if((prevNodeIndex!=1)&&(adjList[1]->getID()!=startNodeID)&&(prevNodeIndex!=2)&&(prevNodeIndex!=0))
				{
//					std::cout<< "directly-diagonal node "<< adjList[1]->getRow() << "," << adjList[1]->getColumn() << " is walkable." <<std::endl;
					toTraverse.push_back(adjList[1]);
		}	}	}	
		else if(nodeIndex==6)
		{
			if((adjList[0]!=NULL)&&(adjList[0]->isClear())&&(adjList[0]->notVisited()))
			{   
				//make sure it isn't the previous node, nor the start node, nor the nodes adjacent to the previous node
				if((prevNodeIndex!=0)&&(adjList[0]->getID()!=startNodeID)&&(prevNodeIndex!=7)&&(prevNodeIndex!=1))
				{
//					std::cout<< "directly-diagonal node "<< adjList[0]->getRow() << "," << adjList[0]->getColumn() << " is walkable." <<std::endl;
					toTraverse.push_back(adjList[0]);
		}	}	}	
		else
		{
			if((adjList[nodeIndex+2]!=NULL)&&(adjList[nodeIndex+2]->isClear())&&(adjList[nodeIndex+2]->notVisited()))
			{   
				//make sure it isn't the previous node or start node
				if((prevNodeIndex!=nodeIndex+2)&&(adjList[nodeIndex+2]->getID()!=startNodeID))
				{
					//make sure it isn't adjacent to prevNode
					//check (nodeIndex+2)+1 and (nodeIndex+2)-1
					if(((nodeIndex+3==8)&&(prevNodeIndex!=0))&&(nodeIndex+1!=prevNodeIndex))
					{
//						std::cout<< "directly-diagonal node "<< adjList[nodeIndex+2]->getRow() << "," << adjList[nodeIndex+2]->getColumn() << " is walkable." <<std::endl;
						toTraverse.push_back(adjList[nodeIndex+2]);
					}
					else if ((nodeIndex+3!=prevNodeIndex)&&(nodeIndex+1!=prevNodeIndex))
					{
//						std::cout<< "directly-diagonal node "<< adjList[nodeIndex+2]->getRow() << "," << adjList[nodeIndex+2]->getColumn() << " is walkable." <<std::endl;
						toTraverse.push_back(adjList[nodeIndex+2]);
		}	}	}	}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
		//counter-clockwise node
		if(nodeIndex==0)
		{
			if((adjList[6]!=NULL)&&(adjList[6]->isClear())&&(adjList[6]->notVisited()))
			{   
				//make sure it isn't the previous node, nor the start node, nor the nodes adjacent to the previous node
				if((prevNodeIndex!=6)&&(adjList[6]->getID()!=startNodeID)&&(prevNodeIndex!=7)&&(prevNodeIndex!=5))
				{
//					std::cout<< "directly-diagonal node "<< adjList[6]->getRow() << "," << adjList[6]->getColumn() << " is walkable." <<std::endl;
					toTraverse.push_back(adjList[6]);
		}	}	}	
		else if(nodeIndex==1)
		{
			if((adjList[7]!=NULL)&&(adjList[7]->isClear())&&(adjList[7]->notVisited()))
			{  
				//make sure it isn't the previous node, nor the start node, nor the nodes adjacent to the previous node
				if((prevNodeIndex!=7)&&(adjList[7]->getID()!=startNodeID)&&(prevNodeIndex!=0)&&(prevNodeIndex!=6))
				{
//					std::cout<< "directly-diagonal node "<< adjList[7]->getRow() << "," << adjList[7]->getColumn() << " is walkable." <<std::endl;
					toTraverse.push_back(adjList[7]);
		}	}	}
		else
		{
			if((adjList[nodeIndex-2]!=NULL)&&(adjList[nodeIndex-2]->isClear())&&(adjList[nodeIndex-2]->notVisited()))
			{   
				//make sure it isn't the previous node or start node
				if((prevNodeIndex!=nodeIndex-2)&&(adjList[nodeIndex-2]->getID()!=startNodeID))
				{
					//make sure it isn't adjacent to prevNode
					//check (nodeIndex-2)+1 and (nodeIndex-2)-1
					if((nodeIndex-3==-1)&&(prevNodeIndex!=7)&&(prevNodeIndex!=nodeIndex-1))
					{
//						std::cout<< "directly-diagonal node "<< adjList[nodeIndex-2]->getRow() << "," << adjList[nodeIndex-2]->getColumn() << " is walkable." <<std::endl;
						toTraverse.push_back(adjList[nodeIndex-2]);
					}
					else if((nodeIndex-3!=prevNodeIndex)&&(prevNodeIndex!=nodeIndex-1))
					{
//						std::cout<< "directly-diagonal node "<< adjList[nodeIndex-2]->getRow() << "," << adjList[nodeIndex-2]->getColumn() << " is walkable." <<std::endl;
						toTraverse.push_back(adjList[nodeIndex-2]);
		}	}	}   }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//check adjacent-opposite nodes
		//clockwise node
		if(nodeIndex==7)
		{
			if((adjList[2]!=NULL)&&(adjList[2]->isClear())&&(adjList[2]->notVisited()))
			{   
				//make sure it isn't the previous node, nor the start node, nor the nodes adjacent to the previous node
				if((prevNodeIndex!=2)&&(adjList[2]->getID()!=startNodeID)&&(prevNodeIndex!=3)&&(prevNodeIndex!=1))
				{
//					std::cout<< "adjacent-opposite node "<< adjList[2]->getRow() << "," << adjList[2]->getColumn() << " is walkable." <<std::endl;
					toTraverse.push_back(adjList[2]);
		}	}	}	
		else if(nodeIndex==6)
		{
			if((adjList[1]!=NULL)&&(adjList[1]->isClear())&&(adjList[1]->notVisited()))
			{   
				//make sure it isn't the previous node, nor the start node, nor the nodes adjacent to the previous node
				if((prevNodeIndex!=1)&&(adjList[1]->getID()!=startNodeID)&&(prevNodeIndex!=2)&&(prevNodeIndex!=0))
				{
//					std::cout<< "adjacent-opposite node "<< adjList[1]->getRow() << "," << adjList[1]->getColumn() << " is walkable." <<std::endl;
					toTraverse.push_back(adjList[1]);
		}	}	}
		else if(nodeIndex==5)
		{
			if((adjList[0]!=NULL)&&(adjList[0]->isClear())&&(adjList[0]->notVisited()))
			{   
				//make sure it isn't the previous node, nor the start node, nor the nodes adjacent to the previous node
				if((prevNodeIndex!=0)&&(adjList[0]->getID()!=startNodeID)&&(prevNodeIndex!=7)&&(prevNodeIndex!=1))
				{
//					std::cout<< "adjacent-opposite node "<< adjList[0]->getRow() << "," << adjList[0]->getColumn() << " is walkable." <<std::endl;
					toTraverse.push_back(adjList[0]);
		}	}	}	
		else
		{
			if((adjList[nodeIndex+3]!=NULL)&&(adjList[nodeIndex+3]->isClear())&&(adjList[nodeIndex+3]->notVisited()))
			{   
				//make sure it isn't the previous node or start node
				if((prevNodeIndex!=nodeIndex+3)&&(adjList[nodeIndex+3]->getID()!=startNodeID))
				{
					//make sure it isn't adjacent to prevNode
					//check (nodeIndex+3)+1 and (nodeIndex+3)-1
					if((nodeIndex+4==8)&&(prevNodeIndex!=0)&&(prevNodeIndex!=nodeIndex+2))
					{
//						std::cout<< "adjacent-opposite node "<< adjList[nodeIndex+3]->getRow() << "," << adjList[nodeIndex+3]->getColumn() << " is walkable." <<std::endl;
						toTraverse.push_back(adjList[nodeIndex+3]);
					}
					else if((nodeIndex+4!=prevNodeIndex)&&(nodeIndex+2!=prevNodeIndex))
					{
//						std::cout<< "adjacent-opposite node "<< adjList[nodeIndex+3]->getRow() << "," << adjList[nodeIndex+3]->getColumn() << " is walkable." <<std::endl;
						toTraverse.push_back(adjList[nodeIndex+3]);
		}	}	}	}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
		//counter-clockwise node
		if(nodeIndex==0)
		{
			if((adjList[5]!=NULL)&&(adjList[5]->isClear())&&(adjList[5]->notVisited()))
			{   
				//make sure it isn't the previous node, nor the start node, nor the nodes adjacent to the previous node
				if((prevNodeIndex!=5)&&(adjList[5]->getID()!=startNodeID)&&(prevNodeIndex!=4)&&(prevNodeIndex!=6))
				{
//					std::cout<< "adjacent-opposite node "<< adjList[5]->getRow() << "," << adjList[5]->getColumn() << " is walkable." <<std::endl;
					toTraverse.push_back(adjList[5]);
		}	}	}	
		else if(nodeIndex==1)
		{
			if((adjList[6]!=NULL)&&(adjList[6]->isClear())&&(adjList[6]->notVisited()))
			{  
				//make sure it isn't the previous node, nor the start node, nor the nodes adjacent to the previous node
				if((prevNodeIndex!=6)&&(adjList[6]->getID()!=startNodeID)&&(prevNodeIndex!=7)&&(prevNodeIndex!=5))
				{
//					std::cout<< "adjacent-opposite node "<< adjList[6]->getRow() << "," << adjList[6]->getColumn() << " is walkable." <<std::endl;
					toTraverse.push_back(adjList[6]);
		}	}	}	
		else if(nodeIndex==2)
		{
			if((adjList[7]!=NULL)&&(adjList[7]->isClear())&&(adjList[7]->notVisited()))
			{  
				//make sure it isn't the previous node, nor the start node, nor the nodes adjacent to the previous node
				if((prevNodeIndex!=7)&&(adjList[7]->getID()!=startNodeID)&&(prevNodeIndex!=6)&&(prevNodeIndex!=0))
				{
//					std::cout<< "adjacent-opposite node "<< adjList[7]->getRow() << "," << adjList[7]->getColumn() << " is walkable." <<std::endl;
					toTraverse.push_back(adjList[7]);
		}	}	}
		else
		{
			if((adjList[nodeIndex-3]!=NULL)&&(adjList[nodeIndex-3]->isClear())&&(adjList[nodeIndex-3]->notVisited()))
			{   
				//make sure it isn't the previous node or start node
				if((prevNodeIndex!=nodeIndex-3)&&(adjList[nodeIndex-3]->getID()!=startNodeID))
				{
					//make sure it isn't adjacent to prevNode
					//check (nodeIndex-3)+1 and (nodeIndex-3)-1
					if((nodeIndex-4==-1)&&(prevNodeIndex!=7)&&(prevNodeIndex!=nodeIndex-2))
					{
//						std::cout<< "adjacent-opposite node "<< adjList[nodeIndex-3]->getRow() << "," << adjList[nodeIndex-3]->getColumn() << " is walkable." <<std::endl;
						toTraverse.push_back(adjList[nodeIndex-3]);
					}
					else if((prevNodeIndex!=nodeIndex-4)&&(prevNodeIndex!=nodeIndex-2))
					{
//						std::cout<< "adjacent-opposite node "<< adjList[nodeIndex-3]->getRow() << "," << adjList[nodeIndex-3]->getColumn() << " is walkable." <<std::endl;
						toTraverse.push_back(adjList[nodeIndex-3]);
			}	}	}	}	
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//check the direct-accross node.
		if(nodeIndex==0)
		{
			if((adjList[4]!=NULL)&&(adjList[4]->isClear())&&(adjList[4]->notVisited()))
			{ 
				//make sure it isn't the previous node, nor the start node, nor the nodes adjacent to the previous node
				if((prevNodeIndex!=4)&&(adjList[4]->getID()!=startNodeID)&&(prevNodeIndex!=3)&&(prevNodeIndex!=5))
				{
//					std::cout<< "direct-accross node "<< adjList[4]->getRow() << "," << adjList[4]->getColumn() << " is walkable." <<std::endl;
					toTraverse.push_back(adjList[4]);
		}	}	}	
		else if(nodeIndex==1)
		{
			if((adjList[5]!=NULL)&&(adjList[5]->isClear())&&(adjList[5]->notVisited()))
			{ 
				//make sure it isn't the previous node, nor the start node, nor the nodes adjacent to the previous node
				if((prevNodeIndex!=5)&&(adjList[5]->getID()!=startNodeID)&&(prevNodeIndex!=4)&&(prevNodeIndex!=6))
				{
//					std::cout<< "direct-accross node "<< adjList[5]->getRow() << "," << adjList[5]->getColumn() << " is walkable." <<std::endl;
					toTraverse.push_back(adjList[5]);
		}	}	}
		else if(nodeIndex==2)
		{
			if((adjList[6]!=NULL)&&(adjList[6]->isClear())&&(adjList[6]->notVisited()))
			{ 
				//make sure it isn't the previous node, nor the start node, nor the nodes adjacent to the previous node
				if((prevNodeIndex!=6)&&(adjList[6]->getID()!=startNodeID)&&(prevNodeIndex!=7)&&(prevNodeIndex!=5))
				{
//					std::cout<< "direct-accross node "<< adjList[6]->getRow() << "," << adjList[6]->getColumn() << " is walkable." <<std::endl;
					toTraverse.push_back(adjList[6]);
		}	}	}	
		else if(nodeIndex==3)
		{
			if((adjList[7]!=NULL)&&(adjList[7]->isClear())&&(adjList[7]->notVisited()))
			{ 
				//make sure it isn't the previous node, nor the start node, nor the nodes adjacent to the previous node
				if((prevNodeIndex!=7)&&(adjList[7]->getID()!=startNodeID)&&(prevNodeIndex!=0)&&(prevNodeIndex!=6))
				{
//					std::cout<< "direct-accross node "<< adjList[7]->getRow() << "," << adjList[7]->getColumn() << " is walkable." <<std::endl;
					toTraverse.push_back(adjList[7]);
		}	}	}
		else if((adjList[nodeIndex-4]!=NULL)&&(adjList[nodeIndex-4]->isClear())&&(adjList[nodeIndex-4]->notVisited()))
		{
			if((prevNodeIndex!=nodeIndex-4)&&(adjList[nodeIndex-4]->getID()!=startNodeID))
			{
				//make sure it isn't adjacent to prevNode
				//check (nodeIndex-4)+1 and (nodeIndex-4)-1
				if((nodeIndex-5==-1)&&(prevNodeIndex!=7)&&(prevNodeIndex!=nodeIndex-3))
				{
//					std::cout<< "direct-accross node "<< adjList[nodeIndex-4]->getRow() << "," << adjList[nodeIndex-4]->getColumn() << " is walkable." <<std::endl;
					toTraverse.push_back(adjList[nodeIndex-4]);
				}
				else if((prevNodeIndex!=nodeIndex-5)&&(prevNodeIndex!=nodeIndex-3))
				{
//					std::cout<< "direct-accross node "<< adjList[nodeIndex-4]->getRow() << "," << adjList[nodeIndex-4]->getColumn() << " is walkable." <<std::endl;
					toTraverse.push_back(adjList[nodeIndex-4]);
	}	}	}	}
	if(toTraverse.empty())
	{
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//have we reached a dead end? start backtracking
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//		std::cout<< "A dead end was reached." <<std::endl;
		//if we have backtracked to the starter node, there is no path to the goal
		if(currentNode->getPrevNode() == NULL)
		{   
//			std::cout<< "No path was found!" <<std::endl;
			enemyGrid->softReset();
			return false;
		}
//		std::cout<< "Turning around..." <<std::endl;
		//otherwise, backup and pick another direction
		toReturn = findPath(currentNode->getPrevNode(), goalNode, (costSoFar - currentNode->getCost()));
	}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//Start following the chosen path(s)!
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	for(GridNode* toVisit : toTraverse)//for each node in list of nodes to traverse
	{
		toVisit->setPrevNode(currentNode);
//		std::cout<< "now at node "<< currentNode->getRow() << "," << currentNode->getColumn() <<std::endl;
//		std::cout<< "about to visit "<< toVisit->getRow() << "," << toVisit->getColumn() <<std::endl;
		toReturn = findPath(toVisit, goalNode, (costSoFar + currentNode->getCost()));			
	}
//	std::cout<<"...backtracking to an unexplored branch... "<<std::endl;
	currentNode->reset(); //sets prevNode to NULL to avoid complications

	//don't forget to return whether or not a path has been found
	return toReturn;
}



