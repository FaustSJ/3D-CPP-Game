#include "Grid.h"
#include <iostream>
#include <fstream>

///////////////////////CONSTRUCTORS/////////////////////////////////////////
// create a node
GridNode::GridNode(int nID, int row, int column, bool isC)
{
	this->clear = isC;
	this->visited = false;	//for pathfinding

	this->rCoord = row;
	this->cCoord = column;

	this->entity = NULL;
	this->prevNode = NULL;	//for pathfinding

	this->heuristic = -1;
	this->cost = 0;

	if (isC)
		this->contains = '.';
	else
		this->contains = 'B';
}

// default constructor
GridNode::GridNode()
{
	nodeID = -999;			// mark these as currently invalid
	this->prevNode = NULL;	//for pathfinding
	this->clear = true;
	this->visited = false;	//for pathfinding
	this->cost=0;
	this->heuristic = -1;
	this->contains = '.';
} 

// destroy a node
GridNode::~GridNode()
{
	delete(this->prevNode);
	this->prevNode = NULL;
}

////////////////////GETTERS////////////////////////////////////////////
// get the x and y coordinate of the node
int GridNode::getRow()
{
	return rCoord;
}

int GridNode::getColumn()
{
	return cCoord;
}

//resets the grid node for the next path search implementation
void GridNode::reset()
{
	this->visited = false;
	this->prevNode = NULL;
	this->cost = 0;
	this->heuristic = -1;
}

void GridNode::softReset()
{
	this->visited = false;
}

//for pathfinding
GridNode* GridNode::getPrevNode()
{
	return this->prevNode;
}

// return the position of this node given the number of rows and columns as parameters
Ogre::Vector3 GridNode::getPosition(int rows, int cols)
{
	Ogre::Vector3 t;
	t.z = (rCoord * NODESIZE) - (rows * NODESIZE)/2.0 + (NODESIZE/2.0); 
	t.y = 0; 
	t.x = (cCoord * NODESIZE) - (cols * NODESIZE)/2.0 + (NODESIZE/2.0); 
	return t;
}

int GridNode::getHeuristic()
{
	return this->heuristic;
}

int GridNode::getCost()
{
	return this->cost;
}

//for pathfinding
bool GridNode::notVisited()
{
	return !(this->visited);
}

// is the node walkable
bool GridNode::isClear()
{
	return this->clear;
}

bool GridNode::isHeuristicSet()
{
	if(this->heuristic != -1)
		return true;
	return false;
}

/////////////////////////SETTERS///////////////////////////////////////
// set the node id
void GridNode::setID(int id)
{
	this->nodeID = id;
}

// set the node as walkable
void GridNode::setClear()
{
	this->clear = true;
	this->contains = '.';

}

//for pathfinding
void GridNode::setPrevNode(GridNode* pn)
{
	this->prevNode = pn;
}

// set the node as occupied
void GridNode::setOccupied()
{
	this->clear = false;
	this->contains = 'B';
}


//for pathfinding
void GridNode::setVisited()
{
	this->visited = true;
}

// set the x coordinate
void GridNode::setRow(int r)
{
	this->rCoord = r;
}

// set the y coordinate
void GridNode::setColumn(int c)
{
	this->cCoord = c;
}

void GridNode::setHeuristic(int i)
{
	this->heuristic = i;
}

void GridNode::setCost(int i)
{
	this->cost = i;
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// create a grid
Grid::Grid(Ogre::SceneManager* mSceneMgr, int numRows, int numCols)
{
	this->mSceneMgr = mSceneMgr; 

	assert(numRows > 0 && numCols > 0);
	this->nRows = numRows;
	this->nCols = numCols;

	data.resize(numCols, GridRow(numRows));
		
	// put the coordinates in each node
	int count = 0;
	for (int i = 0; i < numRows; i++)
		for (int j = 0; j < numCols; j++)
		{
			GridNode *n = this->getNode(i,j);
			n->setRow(i);
			n->setColumn(j);
			n->setID(count);
			count++;
		}
}

/////////////////////////////////////////
// destroy a grid
Grid::~Grid()
{
//	if(mSceneMgr!=NULL)
//	{
//		mSceneMgr->clearScene();
//		delete(mSceneMgr);
//		mSceneMgr = NULL;
//	}
};  														

//return all of the gridNodes to their former states (before pathfinding)
void Grid::reset()
{
	for (int i = 0; i < nRows; i++)
		for (int j = 0; j < nCols; j++)
		{
			this->data[j].data[i].reset();
		}
}

void Grid::softReset()
{
	for (int i = 0; i < nRows; i++)
		for (int j = 0; j < nCols; j++)
		{
			this->data[j].data[i].softReset();
		}
}

////////////////////////////////////////////////////////////////
// get the node specified 
GridNode* Grid::getNode(int r, int c)
{
//std::cout<<"ROWS: "<<nRows<<" COLS: "<<nCols<<std::endl;
//std::cout<<"GIVEN r, c "<<r<<", "<<c<<std::endl;
	if (r >= nRows || c >= nCols || r < 0 || c < 0)
		return NULL;
	GridNode* gn = &this->data[c].data[r];
	return gn;//&this->data[c].data[r];
}

/*
// get adjacent nodes;
GridNode* Grid::getNorthNode(GridNode* n)
{
	return getNode(n->getRow()+1, n->getColumn());
}

GridNode* Grid::getSouthNode(GridNode* n)
{
	return getNode(n->getRow()-1, n->getColumn());
}

GridNode* Grid::getEastNode(GridNode* n)
{
	return getNode(n->getRow(), n->getColumn()+1);
}

GridNode* Grid::getWestNode(GridNode* n)
{
	return getNode(n->getRow(), n->getColumn()-1);
}

GridNode* Grid::getNENode(GridNode* n)  
{
	return getNode(n->getRow()+1, n->getColumn()+1);
}

GridNode* Grid::getNWNode(GridNode* n) 
{
	return getNode(n->getRow()+1, n->getColumn()-1);
}

GridNode* Grid::getSENode(GridNode* n) 
{
	return getNode(n->getRow()-1, n->getColumn()+1);
}

GridNode* Grid::getSWNode(GridNode* n) 
{
	return getNode(n->getRow()+1, n->getColumn()-1);
}

*/
std::vector<GridNode*> Grid::getWalkableAdjacentNodes(GridNode* n)
{
	std::vector<GridNode*> walkableNodes;
	GridNode* nNode = getNode(n->getRow()+1, n->getColumn()); //north
	GridNode* sNode = getNode(n->getRow()-1, n->getColumn()); //south
	GridNode* eNode = getNode(n->getRow(), n->getColumn()+1); //east
	GridNode* wNode = getNode(n->getRow(), n->getColumn()-1); //west
	GridNode* neNode = getNode(n->getRow()+1, n->getColumn()+1); //NE
	GridNode* nwNode = getNode(n->getRow()+1, n->getColumn()-1); //NW
	GridNode* seNode = getNode(n->getRow()-1, n->getColumn()+1); //SE
	GridNode* swNode = getNode(n->getRow()-1, n->getColumn()-1); //SW

	/*
	Before being added to the list, each GridNode is given a cost.
	Cross nodes are given the value of 10 while diagonal nodes are given 14.
	The values of 10 and 14 correpond to the side-hypotenuse size ratio of 1:1.4
	*/

	//added in a clockwise fashion
	if(nNode!=NULL){   nNode->setCost(10);}
	walkableNodes.push_back(nNode);
	if(neNode!=NULL){   neNode->setCost(14);}
	walkableNodes.push_back(neNode);
	if(eNode!=NULL){   eNode->setCost(10);}
	walkableNodes.push_back(eNode);
	if(seNode!=NULL){   seNode->setCost(14);}
	walkableNodes.push_back(seNode);
	if(sNode!=NULL){   sNode->setCost(10);}
	walkableNodes.push_back(sNode);
	if(swNode!=NULL){   swNode->setCost(14);}
	walkableNodes.push_back(swNode);
	if(wNode!=NULL){   wNode->setCost(10);}
	walkableNodes.push_back(wNode);
	if(nwNode!=NULL){   nwNode->setCost(14);}
	walkableNodes.push_back(nwNode);
	

	return walkableNodes;
//the below code was used before wall-following was featured.
	//if(nNode != NULL)//&&(nNode->isClear())) //north
	//{
	//	nNode->setCost(10);
	//	walkableNodes.push_back(nNode);
	//}
	//if(sNode != NULL)//&&(sNode->isClear())) //south
	//{
	//	sNode->setCost(10);
	//	walkableNodes.push_back(sNode);
	//}
	//if(eNode != NULL)//&&(eNode->isClear())) //east
	//{
	//	eNode->setCost(10);
	//	walkableNodes.push_back(eNode);
	//}
	//if(wNode != NULL)//&&(wNode->isClear())) //west
	//{
	//	wNode->setCost(10);
	//	walkableNodes.push_back(wNode);
	//}

	//if(neNode != NULL)//&&(neNode->isClear())) //NE
	//{
	//	//we cannot reach diagonal nodes if both adjacent nodes are impassible
	//	if(((nNode!=NULL)&&(nNode->isClear()))&&((eNode!=NULL)&&(eNode->isClear())))
	//	{
	//		neNode->setCost(14);
	//		walkableNodes.push_back(neNode);
	//	}
	//}
	//if(nwNode != NULL)//&&(nwNode->isClear())) //NW
	//{
	//	//we cannot reach diagonal nodes if both adjacent nodes are impassible
	//	if(((nNode!=NULL)&&(nNode->isClear()))&&((wNode!=NULL)&&(wNode->isClear())))
	//	{
	//		nwNode->setCost(14);
	//		walkableNodes.push_back(nwNode);
	//	}
	//}
	//if(seNode != NULL)//&&(seNode->isClear())) //SE
	//{
	//	//we cannot reach diagonal nodes if both adjacent nodes are impassible
	//	if(((sNode!=NULL)&&(sNode->isClear()))&&((eNode!=NULL)&&(eNode->isClear())))
	//	{
	//		seNode->setCost(14);
	//		walkableNodes.push_back(seNode);
	//	}
	//}
	//if(swNode != NULL)//&&(swNode->isClear())) //SW
	//{
	//	//we cannot reach diagonal nodes if both adjacent nodes are impassible
	//	if(((sNode!=NULL)&&(sNode->isClear()))&&((wNode!=NULL)&&(wNode->isClear())))
	//	{
	//		swNode->setCost(14);
	//		walkableNodes.push_back(swNode);
	//	}
	//}
}

////////////////////////////////////////////////////////////////
//get distance between between two nodes
int Grid::getDistance(GridNode* node1, GridNode* node2)
{
	int r1 = node1->getRow();
	int c1 = node1->getColumn();
	int r2 = node2->getRow();
	int c2 = node2->getColumn();

	int rr = r1 - r2;
	if(rr<0)
		rr *= -1;

	int cc = c1 - c2;
	if(cc<0)
		cc *= -1;

	return ((cc+rr)*10);
}

int Grid::getRows()
{
	return nRows;
}
int Grid::getCols()
{
	return nCols;
}

///////////////////////////////////////////////////////////////////////////////
// Print out the grid in ASCII
void Grid::printToFile()
{
	std::string path = __FILE__; //gets the current cpp file's path with the cpp file
	path = path.substr(0,1+path.find_last_of('\\')); //removes filename to leave path
	path+= "Grid.txt"; //if txt file is in the same directory as cpp file
	std::ofstream outFile;
	outFile.open(path);

	if (!outFile.is_open()) // oops. there was a problem opening the file
	{
		std::cout << "ERROR, FILE COULD NOT BE OPENED" << std::endl;	
		return;
	}

	for (int i = 0; i < nRows; i++)
	{
		for (int j = 0; j < nCols; j++)
		{
			outFile << this->getNode(i, j)->contains << " ";
		}
		outFile << std::endl;
	}
	outFile.close();
}

// load and place a model in a certain location.
void Grid::loadObject(std::string name, std::string filename, int row, int height, int col, float scale)
{
	using namespace Ogre;

	if (row >= nRows || col >= nCols || row < 0 || col < 0)
		return;

	Entity *ent = mSceneMgr->createEntity(name, filename);
    SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode(name,
        Ogre::Vector3(0.0f, 0.0f,  0.0f));
    node->attachObject(ent);
    node->setScale(scale, scale, scale);


	GridNode* gn = this->getNode(row, col);
	node->setPosition(getPosition(row, col)); 
	node->setPosition(getPosition(row, col).x, height, getPosition(row, col).z);
//	gn->setOccupied();
	gn->entity = ent;
}

////////////////////////////////////////////////////////////////////////////
// Added this method and changed GridNode version to account for varying floor 
// plane dimensions. Assumes each grid is centered at the origin.
// It returns the center of each square. 
Ogre::Vector3 Grid::getPosition(int r, int c)	
{
	Ogre::Vector3 t;
	t.z = (r * NODESIZE) - (this->nRows * NODESIZE)/2.0 + NODESIZE/2.0; 
	t.y = 0; 
	t.x = (c * NODESIZE) - (this->nCols * NODESIZE)/2.0 + NODESIZE/2.0; 

	return t;
}

Ogre::Vector3 Grid::getGridPosition(int r, int c)
{
	Ogre::Vector3 t;
	t.z = (r - NODESIZE/2.0 + (this->nRows * NODESIZE)/2.0 )/NODESIZE;
	t.y = 0;
	t.x = (c - NODESIZE/2.0 + (this->nCols * NODESIZE)/2.0 )/NODESIZE;

	return t;
}