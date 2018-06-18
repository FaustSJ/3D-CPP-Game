#include "GameApplication.h"
//#include "Grid.h" // Lecture 5
#include <fstream>
#include <sstream>
#include <iostream>
#include <map> 

//-------------------------------------------------------------------------------------
GameApplication::GameApplication(void):
bLMouseDown(false),
bRMouseDown(false)
{
	agent = NULL; // Init member data
	enemy = NULL;
	grid = NULL;
	rowsGA = 0;
	colsGA = 0;
	destination = 1;
	compass = 1;
//	initialCameraDirection = mCamera->getOrientation();
	holdLevel = true;
	firstLoad = true;
}
//-------------------------------------------------------------------------------------
GameApplication::~GameApplication(void)
{
	if (agent != NULL)  // clean up memory
		delete agent; 
	if (grid != NULL)  
		delete grid;
	if(enemy != NULL)
		delete enemy;
//	if(wall != NULL)
//		delete wall;
	if(wallMesh != NULL)
//		delete wallMesh;
	if(agPosNode != NULL)
//		delete agPosNode;
	grid = NULL;
	agent = NULL;
	enemy = NULL;
	wall = NULL;
	wallMesh = NULL;
	agPosNode = NULL;
}

//-------------------------------------------------------------------------------------
void GameApplication::createScene(void)
{
    loadEnv("level001.txt");
	setupEnv();

	//but we also want to set up our raySceneQuery after everything has been initialized
	mRayScnQuery = mSceneMgr->createRayQuery(Ogre::Ray());
}

void GameApplication::createGUI(void)
{
	//////////////////////////////////////////////////////////////////////////////////
	// Lecture 12GUI
	if (mTrayMgr == NULL) return;
	using namespace OgreBites;
	
	//making the button
	//Button* b = mTrayMgr->createButton(TL_TOP, "MyButton", "OK", 120.0);
	//b->show();

	//The font must be loaded beforehand in order for it to fit in the textbox
	Ogre::FontManager& fm = Ogre::FontManager::getSingleton();
	Ogre::FontPtr valueFontPtr = fm.getByName("SdkTrays/Value");
	if (!valueFontPtr.isNull()) {
	   valueFontPtr.getPointer()->load();
	}

	//making the textbox
	infoBox = mTrayMgr->createTextBox(TL_CENTER, "Info", "Attention!!", 500, 300);
	infoBox->setText("Welcome to the Gravity Maze!\n\nInstructions:\nYou are the brown circle (or barrel).\nUse the [<-] and [->] arrows to rotate your environment and get to the blue orb!\n\nPress [Enter] to start");
	infoBox->show();
	
	mTrayMgr->showAll();

	//////////////////////////////////////////////////////////////////////////////////
}

//////////////////////////////////////////////////////////////////
// Lecture 5: Returns a unique name for loaded objects and agents
std::string getNewName() // return a unique name 
{
	static int count = 0;	// keep counting the number of objects

	std::string s;
	std::stringstream out;	// a stream for outputing to a string
	out << count++;			// make the current count into a string
	s = out.str();

	return "object_" + s;	// append the current count onto the string
}

// Lecture 5: Load level from file!
// Load the buildings or ground plane, etc
void GameApplication::loadEnv(std::string fileName)
{
	using namespace Ogre;	// use both namespaces
	using namespace std;

	mSceneMgr->destroyAllEntities();

	class readEntity // need a structure for holding entities
	{
	public:
		string filename;
		float y;
		float scale;
		float orient;
		bool agent;
		bool enemy;
	};

	ifstream inputfile;		// Holds a pointer into the file

	string path = __FILE__; //gets the current cpp file's path with the cpp file
	path = path.substr(0,1+path.find_last_of('\\')); //removes filename to leave path
	path+= fileName;//"level001.txt"; //if txt file is in the same directory as cpp file
	inputfile.open(path);

	//inputfile.open("D:/CS425-2012/Lecture 8/GameEngine-loadLevel/level001.txt"); // bad explicit path!!!
	if (!inputfile.is_open()) // oops. there was a problem opening the file
	{
		cout << "ERROR, FILE COULD NOT BE OPENED" << std::endl;	// Hmm. No output?
		return;
	}

	// the file is open
	int x,z;
	inputfile >> x >> z;	// read in the dimensions of the grid
	string matName;
	inputfile >> matName;	// read in the material name

	//store the dimensions in field variables
	rowsGA = z;
	colsGA = x;

	//creates a unique name for any floor meshes created, based on their dimensions
	const string floorMeshName = to_string(x)+to_string(z)+"floor";

	// create floor mesh using the dimension read
	MeshManager::getSingleton().createPlane(floorMeshName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
		Plane(Vector3::UNIT_Y, 0), x*NODESIZE, z*NODESIZE, x, z, true, 1, x, z, Vector3::UNIT_Z);

	//create a floor entity, give it material, and place it at the origin
	Entity* floor = mSceneMgr->createEntity("Floor", floorMeshName);
	floor->setMaterialName(matName);
	floor->setCastShadows(false);
	//mSceneMgr->getRootSceneNode()->attachObject(floor);
	Ogre::SceneNode* pNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    pNode->attachObject(floor);

	cout<< "making the grid"<<endl;
	grid = new Grid(mSceneMgr, z, x); // Set up the grid. z is rows, x is columns
	
	string buf;
	inputfile >> buf;	// Start looking for the Objects section
	while  (buf != "Objects")
		inputfile >> buf;
	if (buf != "Objects")	// Oops, the file must not be formated correctly
	{
		cout << "ERROR: Level file error" << endl;
		return;
	}

	// read in the objects
	readEntity *rent = new readEntity();	// hold info for one object
	std::map<string,readEntity*> objs;		// hold all object and agent types;
	while (!inputfile.eof() && buf != "Characters") // read through until you find the Characters section
	{ 
		inputfile >> buf;			// read in the char
		if (buf != "Characters")
		{
			inputfile >> rent->filename >> rent->y >> rent->orient >> rent->scale;  // read the rest of the line
			rent->agent = false;		// these are objects
			rent->enemy = false;
			objs[buf] = rent;			// store this object in the map
			rent = new readEntity();	// create a new instance to store the next object
		}
	}

	while  (buf != "Characters")	// get through any junk
		inputfile >> buf;
	
	// Read in the characters
	while (!inputfile.eof() && buf != "Enemies") // Read through until the world section
	{
		inputfile >> buf;		// read in the char
		if (buf != "Enemies")
		{
			inputfile >> rent->filename >> rent->y >> rent->scale; // read the rest of the line
			rent->agent = true;			// this is an agent
			rent->enemy = false;
			objs[buf] = rent;			// store the agent in the map
			rent = new readEntity();	// create a new instance to store the next object
		}
	}

	while  (buf != "Enemies")	// get through any junk
		inputfile >> buf;

	//Read in the enemies
	while (!inputfile.eof() && buf != "World") // Read through until the world section
	{
		inputfile >> buf;		// read in the char
		if (buf != "World")
		{
			inputfile >> rent->filename >> rent->y >> rent->scale; // read the rest of the line
			rent->enemy = true;			// this is an enemy
			rent->agent = false;
			objs[buf] = rent;			// store the agent in the map
			rent = new readEntity();	// create a new instance to store the next object
		}
	}
	delete rent; // we didn't need the last one

	// read through the placement map
	char c;
	for (int i = 0; i < z; i++)			// down (row)
	{
		for (int j = 0; j < x; j++)		// across (column)
		{
			inputfile >> c;			// read one char at a time
			buf = c + '\0';			// convert char to string
			rent = objs[buf];		// find cooresponding object or agent
			if (rent != NULL)		// it might not be an agent or object
				if (rent->agent)	// if it is an agent...
				{
					agent = new Agent(this->mSceneMgr, getNewName(), rent->filename, rent->y, rent->scale);
					agentList.push_back(agent);
					agent->setPosition(grid->getPosition(i,j).x, 0, grid->getPosition(i,j).z);
					initialPlayerPosition = Ogre::Vector3(grid->getPosition(i,j).x, 0, grid->getPosition(i,j).z);
					// If we were using different characters, we'd have to deal with 
					// different animation clips. 
				}
				else if(rent->enemy)
				{
					enemy = new Enemy(this->mSceneMgr, getNewName(), rent->filename, rent->y, rent->scale);
					enemyList.push_back(enemy);
					enemy->setPosition(grid->getPosition(i,j).x, 0, grid->getPosition(i,j).z);
				}
				else	// Load objects
				{
					grid->loadObject(getNewName(), rent->filename, i, rent->y, j, rent->scale);
				}
			else // not an object or agent
			{
				if (c == 'w') // create a wall
				{
					Entity* ent = mSceneMgr->createEntity(getNewName(), Ogre::SceneManager::PT_CUBE);
					ent->setMaterialName("Examples/RustySteel");
					Ogre::SceneNode* mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
					mNode->attachObject(ent);
					mNode->scale(0.1f,0.2f,0.1f); // cube is 100 x 100
					grid->getNode(i,j)->setOccupied();  // indicate that agents can't pass through
					mNode->setPosition(grid->getPosition(i,j).x, 10.0f, grid->getPosition(i,j).z);
				}
				else if (c == 'e')
				{
					ParticleSystem::setDefaultNonVisibleUpdateTimeout(5);  // set nonvisible timeout
					ParticleSystem* ps = mSceneMgr->createParticleSystem(getNewName(), "Examples/PurpleFountain");
					Ogre::SceneNode* mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
					mNode->attachObject(ps);
					mNode->setPosition(grid->getPosition(i,j).x, 0.0f, grid->getPosition(i,j).z);
				}
			}
		}
	}

	//make collision box
	wallMesh = mSceneMgr->createEntity(getNewName(), Ogre::SceneManager::PT_CUBE);
	wallMesh->setMaterialName("Examples/RustySteel");
	wall = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	wall->setPosition(grid->getPosition(0,0).x, 10.0f, grid->getPosition(0,0).z); //hide it in a corner
	wall->scale(0.11f,0.11f,0.01f);
	wall->attachObject(wallMesh);
	
	//set up the goal
	goalMesh = mSceneMgr->createEntity(getNewName(), Ogre::SceneManager::PT_SPHERE);
	goalMesh->setMaterialName("Examples/WaterStream");
	goal = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	goal->scale(0.05f,0.05f,0.05f);
	goal->attachObject(goalMesh);

	if(destination==1)
		goal->setPosition(grid->getPosition(6, 6).x, 5.0f, grid->getPosition(6, 6).z); 
	if(destination==2)
		goal->setPosition(grid->getPosition(7, 5).x, 5.0f, grid->getPosition(7, 5).z); 
	if(destination==3)
		goal->setPosition(grid->getPosition(13, 13).x, 5.0f, grid->getPosition(13, 13).z); 

	//get the enemies moving along individual paths
	int count = 0;
	for(std::list<Enemy*>::iterator en = enemyList.begin(); en != enemyList.end(); ++en)
	{
		count++;
		if(count==1)
		{
			if(destination==2)
				(*en)->walkTo(grid->getNode(8, 13), grid);
			if(destination==3)
				(*en)->walkTo(grid->getNode(4, 10), grid);
		}
		if(count==2)
		{
			if(destination==2)
				(*en)->walkTo(grid->getNode(5, 2), grid);
		}

	}


	// delete all of the readEntities in the objs map
	rent = objs["s"]; // just so we can see what is going on in memory (delete this later)
	
	std::map<string,readEntity*>::iterator it;
	for (it = objs.begin(); it != objs.end(); it++) // iterate through the map
	{
		delete (*it).second; // delete each readEntity
	}
	objs.clear(); // calls their destructors if there are any. (not good enough)
	
	inputfile.close();
//	grid->printToFile(); // see what the initial grid looks like.
}

// Set up lights, shadows, etc
void GameApplication::setupEnv()
{
	using namespace Ogre;

	// set shadow properties
	mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
	mSceneMgr->setShadowColour(ColourValue(0.5, 0.5, 0.5));
	mSceneMgr->setShadowTextureSize(1024);
	mSceneMgr->setShadowTextureCount(1);

	// disable default camera control so the character can do its own 
	mCameraMan->setStyle(OgreBites::CS_FREELOOK); // CS_FREELOOK, CS_ORBIT, CS_MANUAL

	// use small amount of ambient lighting
	mSceneMgr->setAmbientLight(ColourValue(0.3f, 0.3f, 0.3f));

	// add a bright light above the scene
	Light* light = mSceneMgr->createLight();
	light->setType(Light::LT_POINT);
	light->setPosition(-10, 40, 20);
	light->setSpecularColour(ColourValue::White);

	//mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8); // Lecture 4
}

void GameApplication::addTime(Ogre::Real deltaTime)
{
	if(!holdLevel)
	{
		//Update the agent
		agentList.front()->update(deltaTime);

		//update the enemies
		std::list<Enemy*>::iterator iter;
		for (iter = enemyList.begin(); iter != enemyList.end(); iter++)
			if (*iter != NULL)
			{
				(*iter)->update(deltaTime);
				if(agentList.front()->getAgentBoundingBox().intersects((*iter)->getEnemyBoundingBox()))
				{
					holdLevel = true;
	//				agentList.front()->setPosition(initialPlayerPosition.x, initialPlayerPosition.y, initialPlayerPosition.z);
					infoBox->clearText();
					infoBox->setText("Argh!! The ogre tripped over you and crushed you to pieces.\n\nPress [Enter] to try again");
					mTrayMgr->showAll();
					break;
				}
				
			}

		//check for collisions
		if (wallMesh->getWorldBoundingBox().intersects(agentList.front()->getAgentBoundingBox()))
		{
			agentList.front()->reset(agPosNode);
		}
		if(goalMesh->getWorldBoundingBox().intersects(agentList.front()->getAgentBoundingBox()))
		{
			if(destination==1)
			{
				//go to level 2
				holdLevel = true;
				destination = 2;
				infoBox->clearText();
				infoBox->setText("Now that you've gotten the hang of it, let's add some ogres! Simply try to avoid them, alright?\n\nPress [Enter] to start");
				mTrayMgr->showAll();
			}
			else if(destination==2)
			{
				//go to level 3
				holdLevel = true;
				destination = 3;
				infoBox->clearText();
				infoBox->setText("For the final level, let's spice things up by moving the orb around.\nCatch it if you can!\n\nPress [Enter] to start");
				mTrayMgr->showAll();
			}
			else if(destination==3)
			{
				//They won the game!
				holdLevel = true;
				destination = 1;
				infoBox->clearText();
				infoBox->setText("Holy Cow, you managed to win!!\n\nPress [Enter] to start over, if you want.");
				mTrayMgr->showAll();
			}
		}

		//if we're on level 3, occasionally move around the goal
		if(destination==3)
		{
			bool stillLooking = true;
			int moveTime = rand() % 300 + 1;

			while(stillLooking && moveTime==1)
			{
				int newSpot = rand() % 5 + 1;
				Ogre::Vector3 agentPos = grid->getGridPosition(agentList.front()->getPosition().z, agentList.front()->getPosition().x);
				if(newSpot==1)//(13, 13)
				{
					if((!(agentPos.z==13 && agentPos.x==13))&&(!(agentPos.z==13 && agentPos.x==11))&&(!(agentPos.z==11 && agentPos.x==13)))
					{
						stillLooking = false;
						goal->setPosition(grid->getPosition(13,13).z, 5.0f, grid->getPosition(13,13).x);
					}
				}
				else if(newSpot==2)//(13, 1)
				{
					if((!(agentPos.z==13 && agentPos.x==1))&&(!(agentPos.z==11 && agentPos.x==1))&&(!(agentPos.z==13 && agentPos.x==3)))
					{
						stillLooking = false;
						goal->setPosition(grid->getPosition(13,1).z, 5.0f, grid->getPosition(13,1).x);
					}
				}
				else if(newSpot==3)//(1, 13)
				{
					if((!(agentPos.z==1 && agentPos.x==13))&&(!(agentPos.z==1 && agentPos.x==11))&&(!(agentPos.z==3 && agentPos.x==13)))
					{
							stillLooking = false;
						goal->setPosition(grid->getPosition(1,13).z, 5.0f, grid->getPosition(1,13).x);
					}
				}
				else if(newSpot==4) //(1, 1)
				{
					if((!(agentPos.z==1 && agentPos.x==1))&&(!(agentPos.z==3 && agentPos.x==1))&&(!(agentPos.z==1 && agentPos.x==3)))
					{
						stillLooking = false;
						goal->setPosition(grid->getPosition(1,1).z, 5.0f, grid->getPosition(1,1).x);
}	}	}	}	}	}	

bool GameApplication::keyPressed( const OIS::KeyEvent &arg ) // Moved from BaseApplication
{
    if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up

    if (arg.key == OIS::KC_F)   // toggle visibility of advanced frame stats
    {
        mTrayMgr->toggleAdvancedFrameStats();
    }
    else if (arg.key == OIS::KC_G)   // toggle visibility of even rarer debugging details
    {
        if (mDetailsPanel->getTrayLocation() == OgreBites::TL_NONE)
        {
            mTrayMgr->moveWidgetToTray(mDetailsPanel, OgreBites::TL_TOPRIGHT, 0);
            mDetailsPanel->show();
        }
        else
        {
            mTrayMgr->removeWidgetFromTray(mDetailsPanel);
            mDetailsPanel->hide();
        }
    }
    else if (arg.key == OIS::KC_T)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::TextureFilterOptions tfo;
        unsigned int aniso;

        switch (mDetailsPanel->getParamValue(9).asUTF8()[0])
        {
        case 'B':
            newVal = "Trilinear";
            tfo = Ogre::TFO_TRILINEAR;
            aniso = 1;
            break;
        case 'T':
            newVal = "Anisotropic";
            tfo = Ogre::TFO_ANISOTROPIC;
            aniso = 8;
            break;
        case 'A':
            newVal = "None";
            tfo = Ogre::TFO_NONE;
            aniso = 1;
            break;
        default:
            newVal = "Bilinear";
            tfo = Ogre::TFO_BILINEAR;
            aniso = 1;
        }

        Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
        Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(aniso);
        mDetailsPanel->setParamValue(9, newVal);
    }
    else if (arg.key == OIS::KC_R)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::PolygonMode pm;

        switch (mCamera->getPolygonMode())
        {
        case Ogre::PM_SOLID:
            newVal = "Wireframe";
            pm = Ogre::PM_WIREFRAME;
            break;
        case Ogre::PM_WIREFRAME:
            newVal = "Points";
            pm = Ogre::PM_POINTS;
            break;
        default:
            newVal = "Solid";
            pm = Ogre::PM_SOLID;
        }

        mCamera->setPolygonMode(pm);
        mDetailsPanel->setParamValue(10, newVal);
    }
    else if(arg.key == OIS::KC_F5)   // refresh all textures
    {
        Ogre::TextureManager::getSingleton().reloadAll();
    }
    else if (arg.key == OIS::KC_SYSRQ)   // take a screenshot
    {
        mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");
    }
    else if (arg.key == OIS::KC_ESCAPE)
    {
        mShutDown = true;
    }
	else if (arg.key == OIS::KC_RETURN)
	{
		holdLevel = false;
		mTrayMgr->hideAll();

		if(!firstLoad)
		{
			//clear the viewing plane
			mSceneMgr->getRootSceneNode()->removeAndDestroyAllChildren();
			delete(grid);
			agentList.clear();
			enemyList.clear();
	
			//redraw everything
			if(destination==1)
				loadEnv("level001.txt");
			else if(destination==2)
				loadEnv("level002.txt");
			else if(destination==3)
				loadEnv("level003.txt");
			setupEnv();

			//reset the camera
			if (compass==2)
			{
				mCamera->roll(Ogre::Degree(90));
			}
			else if (compass==3)
			{
				mCamera->roll(Ogre::Degree(180));
			}
			else if(compass==4)
			{
				mCamera->roll(Ogre::Degree(-90));
			}
			compass = 1;
		}

		firstLoad = false;
	}
	else if (arg.key == OIS::KC_LEFT)
	{
		if(!agentList.front()->isMoving()&&(!holdLevel))
		{
			//camera y rotation *= quat 90 degrees
			mCamera->roll(Ogre::Degree(-90));
			compass++; 
			if(compass==5)
				compass = 1;
			//move the collision wall to catch the agent as it falls
			wall->yaw(Ogre::Degree(-90));
			Ogre::Vector3 initialPos = agentList.front()->getPosition();
			if(compass==1)
			{
				//traverse gridNodes row++
				Ogre::Vector3 agentCoor = grid->getGridPosition(agentList.front()->getPosition().z, agentList.front()->getPosition().x);
				
				GridNode *initialNode = grid->getNode(agentCoor.z, agentCoor.x);
				GridNode *wallNode = NULL;
				bool wallFound = false;
				int count = 0;
				while(!wallFound)
				{
					count++;
					wallNode = grid->getNode(initialNode->getRow()+count, initialNode->getColumn());
					if(!wallNode->isClear())
						wallFound = true;
				}
				Ogre::Vector3 newPosition = wallNode->getPosition(grid->getRows(), grid->getCols());
				wall->setPosition(newPosition.x, newPosition.y+5, newPosition.z-5);
				agPosNode = grid->getNode(initialNode->getRow()+count-1, initialNode->getColumn());
			}
			else if (compass==2)
			{
				//traverse gridNodes cols--
				Ogre::Vector3 agentCoor = grid->getGridPosition(agentList.front()->getPosition().z, agentList.front()->getPosition().x);
				
				GridNode *initialNode = grid->getNode(agentCoor.z, agentCoor.x);
				GridNode *wallNode = NULL;
				bool wallFound = false;
				int count = 0;
				while(!wallFound)
				{
					count++;
					wallNode = grid->getNode(initialNode->getRow(), initialNode->getColumn()-count);
					if(!wallNode->isClear())
						wallFound = true;
				}
				Ogre::Vector3 newPosition = wallNode->getPosition(grid->getRows(), grid->getCols());
				wall->setPosition(newPosition.x+5, newPosition.y+5, newPosition.z);
				agPosNode = grid->getNode(initialNode->getRow(), initialNode->getColumn()-count+1);
			}
			else if (compass==3)
			{
				//traverse gridNodes row--
				Ogre::Vector3 agentCoor = grid->getGridPosition(agentList.front()->getPosition().z, agentList.front()->getPosition().x);
				
				GridNode *initialNode = grid->getNode(agentCoor.z, agentCoor.x);
				GridNode *wallNode = NULL;
				bool wallFound = false;
				int count = 0;
				while(!wallFound)
				{
					count++;
					wallNode = grid->getNode(initialNode->getRow()-count, initialNode->getColumn());
					if(!wallNode->isClear())
						wallFound = true;
				}
				Ogre::Vector3 newPosition = wallNode->getPosition(grid->getRows(), grid->getCols());
				wall->setPosition(newPosition.x, newPosition.y+5, newPosition.z+5);
				agPosNode = grid->getNode(initialNode->getRow()-count+1, initialNode->getColumn());
			}
			else
			{
				//traverse gridNodes cols++
				Ogre::Vector3 agentCoor = grid->getGridPosition(agentList.front()->getPosition().z, agentList.front()->getPosition().x);
				
				GridNode *initialNode = grid->getNode(agentCoor.z, agentCoor.x);
				GridNode *wallNode = NULL;
				bool wallFound = false;
				int count = 0;
				while(!wallFound)
				{
					count++;
					wallNode = grid->getNode(initialNode->getRow(), initialNode->getColumn()+count);
					if(!wallNode->isClear())
						wallFound = true;
				}
				Ogre::Vector3 newPosition = wallNode->getPosition(grid->getRows(), grid->getCols());
				wall->setPosition(newPosition.x-5, newPosition.y+5, newPosition.z);
				agPosNode = grid->getNode(initialNode->getRow(), initialNode->getColumn()+count-1);
			}
			agentList.front()->blastOff(grid, compass);
		}
	}
	else if (arg.key == OIS::KC_RIGHT)
	{
		if(!agentList.front()->isMoving()&&(!holdLevel))
		{
			//camera y rotation *= quat -90 degrees
			mCamera->roll(Ogre::Degree(90));
			compass--;
			if(compass==0)
				compass = 4;
			//move the collision wall to catch the agent as it falls
			wall->yaw(Ogre::Degree(-90));
			Ogre::Vector3 initialPos = agentList.front()->getPosition();
			if(compass==1)
			{
				//traverse gridNodes row++
				Ogre::Vector3 agentCoor = grid->getGridPosition(agentList.front()->getPosition().z, agentList.front()->getPosition().x);
				
				GridNode *initialNode = grid->getNode(agentCoor.z, agentCoor.x);
				GridNode *wallNode = NULL;
				bool wallFound = false;
				int count = 0;
				while(!wallFound)
				{
					count++;
					wallNode = grid->getNode(initialNode->getRow()+count, initialNode->getColumn());
					if(!wallNode->isClear())
						wallFound = true;
				}
				Ogre::Vector3 newPosition = wallNode->getPosition(grid->getRows(), grid->getCols());
				wall->setPosition(newPosition.x, newPosition.y+5, newPosition.z-5);
				agPosNode = grid->getNode(initialNode->getRow()+count-1, initialNode->getColumn());
			}
			else if (compass==2)
			{
				//traverse gridNodes cols--
				Ogre::Vector3 agentCoor = grid->getGridPosition(agentList.front()->getPosition().z, agentList.front()->getPosition().x);
				
				GridNode *initialNode = grid->getNode(agentCoor.z, agentCoor.x);
				GridNode *wallNode = NULL;
				bool wallFound = false;
				int count = 0;
				while(!wallFound)
				{
					count++;
					wallNode = grid->getNode(initialNode->getRow(), initialNode->getColumn()-count);
					if(!wallNode->isClear())
						wallFound = true;
				}
				Ogre::Vector3 newPosition = wallNode->getPosition(grid->getRows(), grid->getCols());
				wall->setPosition(newPosition.x+5, newPosition.y+5, newPosition.z);
				agPosNode = grid->getNode(initialNode->getRow(), initialNode->getColumn()-count+1);
			}
			else if (compass==3)
			{
				//traverse gridNodes row--
				Ogre::Vector3 agentCoor = grid->getGridPosition(agentList.front()->getPosition().z, agentList.front()->getPosition().x);
				
				GridNode *initialNode = grid->getNode(agentCoor.z, agentCoor.x);
				GridNode *wallNode = NULL;
				bool wallFound = false;
				int count = 0;
				while(!wallFound)
				{
					count++;
					wallNode = grid->getNode(initialNode->getRow()-count, initialNode->getColumn());
					if(!wallNode->isClear())
						wallFound = true;
				}
				Ogre::Vector3 newPosition = wallNode->getPosition(grid->getRows(), grid->getCols());
				wall->setPosition(newPosition.x, newPosition.y+5, newPosition.z+5);
				agPosNode = grid->getNode(initialNode->getRow()-count+1, initialNode->getColumn());
			}
			else
			{
				//traverse gridNodes cols++
				Ogre::Vector3 agentCoor = grid->getGridPosition(agentList.front()->getPosition().z, agentList.front()->getPosition().x);
				
				GridNode *initialNode = grid->getNode(agentCoor.z, agentCoor.x);
				GridNode *wallNode = NULL;
				bool wallFound = false;
				int count = 0;
				while(!wallFound)
				{
					count++;
					wallNode = grid->getNode(initialNode->getRow(), initialNode->getColumn()+count);
					if(!wallNode->isClear())
						wallFound = true;
				}
				Ogre::Vector3 newPosition = wallNode->getPosition(grid->getRows(), grid->getCols());
				wall->setPosition(newPosition.x-5, newPosition.y+5, newPosition.z);
				agPosNode = grid->getNode(initialNode->getRow(), initialNode->getColumn()+count-1);
			}
			agentList.front()->blastOff(grid, compass);
		}
	}
	
   
//    mCameraMan->injectKeyDown(arg);
    return true;
}

bool GameApplication::keyReleased( const OIS::KeyEvent &arg )
{
   // mCameraMan->injectKeyUp(arg);
    return true;
}

bool GameApplication::mouseMoved( const OIS::MouseEvent &arg )
{
    if (mTrayMgr->injectMouseMove(arg)) return true;
//    mCameraMan->injectMouseMove(arg);
    return true;
}

bool GameApplication::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
   //////////////////////////////////////////////////////////////////////////////////////
	// Lecture 12
	if(id == OIS::MB_Left)
	{
		bLMouseDown = true;
	}
	else if (id == OIS::MB_Right)
		bRMouseDown = true;

    if (mTrayMgr->injectMouseUp(arg, id)) return true;
  //  mCameraMan->injectMouseUp(arg, id);
    return true;
}

bool GameApplication::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	// Lecture 12
	if(id  == OIS::MB_Left)
	{
		bLMouseDown = false;
	}
	if(id  == OIS::MB_Right)
	{
		bRMouseDown = false;
	}

    if (mTrayMgr->injectMouseUp(arg, id)) return true;
 //   mCameraMan->injectMouseUp(arg, id);
    return true;
}

// Lecture 12GUI
// Callback method for buttons
void GameApplication::buttonHit(OgreBites::Button* b)
{
	if (b->getName() == "MyButton")
	{
		holdLevel = false;
		mTrayMgr->hideAll();
		return;
	}
}