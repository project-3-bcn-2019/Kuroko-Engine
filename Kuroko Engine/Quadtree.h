#ifndef QUAD_TREE
#define QUAD_TREE
#include <list>
#include "Math.h"

class GameObject;
template<class PRIMITIVE>
struct QuadTreeNode {

	QuadTreeNode(AABB limits); // Create nodes with no objects
	QuadTreeNode(AABB limits, std::list<GameObject*> objects); // Create nodes with objects

	void CollectIntersections(std::list<GameObject*> found_obj, const PRIMITIVE& primitive);
	void Split(); // creates childs and passes them the objects contained on this one
	QuadTreeNode* childs[4];
	AABB box;
	bool is_leaf = true;
	std::list<GameObject*> objects;
};



template<class PRIMITIVE>
class Quadtree {
public:
	Quadtree(AABB limits); 	
	Quadtree(std::list<GameObject*> objects);	  // Adaptive
	~Quadtree();
	void Create(AABB limits); 	
	void Create(std::list<GameObject*> objects);	  // Adaptive
	void Insert(GameObject* object);			  // Insert an object into the quadtree so it blends to fit it
	void Fill(std::list<GameObject*> objects);    //Multiple "Inserts" at once
	void Intersect(std::list<GameObject*> objects, PRIMITIVE primitive); 	// objects: List to be filled
	QuadTreeNode* root; // Root of the quadtree
	int max_splits; 	// How many splits allowed
	int bucket_size;	// How much items can be held in a QuadTreeNode
};





#endif 