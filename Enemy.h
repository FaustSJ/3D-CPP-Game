#ifndef ENEMY_H
#define ENEMY_H
#include "BaseApplication.h"
#include <deque>
#include "Grid.h"
//class Grid;
//class GridNode;
//class GridRows;

class Enemy
{
private:
	Ogre::SceneManager* mSceneMgr;		// pointer to scene graph
	Ogre::SceneNode* mBodyNode;			
	Ogre::Entity* mBodyEntity;
	float height;						// height the character should be moved up
	float scale;						// scale of character from original model
	Grid* enemyGrid;	//for pathfinding
	int totalDH;	//for pathfinding
	int startNodeID; //for pathfinding
	bool ready;

	//these nodes mark the two ends of the enemy's travel path
	GridNode *pathNodeFrom;
	GridNode *pathNodeTo;


	// all of the animations our character has, and a null ID
	// some of these affect separate body parts and will be blended together
	
	// Die Idle Shoot Slump Walk
	enum AnimID
	{
		ANIM_IDLE_BASE,
		ANIM_IDLE_TOP,
		ANIM_RUN_BASE,
		ANIM_RUN_TOP,
		ANIM_HANDS_CLOSED,
		ANIM_HANDS_RELAXED,
		ANIM_DRAW_SWORDS,
		ANIM_SLICE_VERTICAL,
		ANIM_SLICE_HORIZONTAL,
		ANIM_DANCE,
		ANIM_JUMP_START,
		ANIM_JUMP_LOOP,
		ANIM_JUMP_END,
		ANIM_NONE
	};


	Ogre::AnimationState* mAnims[13];		// master animation list
	AnimID mBaseAnimID;						// current base (full- or lower-body) animation
	AnimID mTopAnimID;						// current top (upper-body) animation
	bool mFadingIn[13];						// which animations are fading in
	bool mFadingOut[13];					// which animations are fading out
	Ogre::Real mTimer;						// general timer to see how long animations have been playing
	Ogre::Real mVerticalVelocity;			// for jumping

	void setupAnimations();					// load this character's animations
	void fadeAnimations(Ogre::Real deltaTime);				// blend from one animation to another
	void updateAnimations(Ogre::Real deltaTime);			// update the animation frame


	// for locomotion
	Ogre::Real mDistance;					// The distance the enemy has left to travel
	Ogre::Vector3 mDirection;				// The direction the object is moving
	Ogre::Vector3 mDestination;				// The destination the object is moving towards
	std::deque<Ogre::Vector3> mWalkList;	// The list of points we are walking to
	Ogre::Real mWalkSpeed;					// The speed at which the object is moving
	bool nextLocation();					// Is there another destination?
	void updateLocomote(Ogre::Real deltaTime);			// update the character's walking
	void setDirection();
	bool findPath(GridNode* currentNode, GridNode* goalNode, int costSoFar);
	void setWayPoints(GridNode* gn);

	//////////////////////////////////////////////
	// Lecture 4
	bool procedural;						// Is this character performing a procedural animation
    //////////////////////////////////////////////
public:
	Enemy(Ogre::SceneManager* SceneManager, std::string name, std::string filename, float height, float scale);
	~Enemy();
	void setPosition(float x, float y, float z);
	Ogre::Vector3 getPosition();

	void update(Ogre::Real deltaTime);		// update the enemy

	void setBaseAnimation(AnimID id, bool reset = false);	// choose animation to display
	void setTopAnimation(AnimID id, bool reset = false);

	Ogre::AxisAlignedBox getEnemyBoundingBox();

	void walkTo(GridNode* gn, Grid* g);
};
#endif
