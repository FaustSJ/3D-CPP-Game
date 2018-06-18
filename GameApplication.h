#ifndef __GameApplication_h_
#define __GameApplication_h_

#include "BaseApplication.h"
#include "Agent.h"
#include "Enemy.h"
#include "Grid.h"
//class Grid;
//class GridNode;
//class GridRows;

class GameApplication : public BaseApplication
{
private:
	Agent* agent; // store a pointer to the character
	Enemy* enemy;
	std::list<Agent*> agentList; // Lecture 5: now a list of agents
	std::list<Enemy*> enemyList;
	Grid* grid; // store the grid mapping the visual plane
	int rowsGA;
	int colsGA;
	int destination; //remembers which map/level is currently in use
	int compass; //1, 2, 3, 4 for N, E, S, W respectively
	GridNode* agPosNode; //Node marking the agen't final position post-fall
	bool holdLevel; //the game won't start unless the player reads and clicks the button
	bool firstLoad;
	Ogre::Vector3 initialPlayerPosition;


	Ogre::SceneNode* wall;
	Ogre::Entity* wallMesh;
	Ogre::SceneNode* goal;
	Ogre::Entity* goalMesh;
public:
    GameApplication(void);
    virtual ~GameApplication(void);

	void loadEnv(std::string fileName);			// Load the buildings or ground plane, etc.
	void setupEnv();		// Set up the lights, shadows, etc

	void addTime(Ogre::Real deltaTime);		// update the game state

	//////////////////////////////////////////////////////////////////////////
	// Lecture 4: keyboard interaction
	// moved from base application
	// OIS::KeyListener
    bool keyPressed( const OIS::KeyEvent &arg );
    bool keyReleased( const OIS::KeyEvent &arg );
    // OIS::MouseListener
    bool mouseMoved( const OIS::MouseEvent &arg );
    bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
    bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
	////////////////////////////////////////////////////////////////////////////

	bool bLMouseDown, bRMouseDown;		//true if mouse buttons are held down
	Ogre::SceneNode *mCurrentObject;	//pointer to our currently selected object
	virtual void buttonHit(OgreBites::Button* b); // Lecture 12GUI

protected:
    virtual void createScene(void);
	virtual void createGUI(void); // Lecture 12GUI
	
	OgreBites::ParamsPanel* mParamsPanel; // Lecture 12GUI
//	OgreBites::Button* setLevel;
	OgreBites::TextBox* infoBox;
};

#endif // #ifndef __TutorialApplication_h_
