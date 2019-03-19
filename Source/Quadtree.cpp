#include "Quadtree.h"
#include "GameObject.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ComponentAABB.h"

// Quadtree

Quadtree::Quadtree(AABB limits, int bucket_size, int max_splits): bucket_size(bucket_size), max_splits(max_splits) {
	root = new QuadTreeNode(limits);
}


Quadtree::Quadtree(std::list<GameObject*> objects) { // Adaptive (not for now)

}


Quadtree::~Quadtree() {
	delete root;
}


void Quadtree::Create(AABB limits, int bucket_size, int max_splits) {
	
	delete root;
	root = new QuadTreeNode(limits);
	this->bucket_size = bucket_size;
	this->max_splits = max_splits;
}


bool Quadtree::Insert(GameObject* object) {
	return root->AddObject(object, bucket_size, max_splits);
}


void Quadtree::Fill(std::list<GameObject*> objects) {
	for (auto it = objects.begin(); it != objects.end(); it++)
		Insert(*it);
}
void Quadtree::Empty() {
	root->objects.clear();
	if (!root->is_leaf) {
		for (int i = 0; i < 4; i++) {
			delete root->childs[i];
			root->childs[i] = nullptr;
		}
	}
	root->is_leaf = true;
}

int Quadtree::Intersect(std::list<GameObject*>& found_obj, Frustum primitive) {
	int checks = 0;
	root->CollectIntersections(found_obj, primitive, checks);
	return checks;
}



void Quadtree::DebugDraw() {
	root->Draw();
}

// Quadtree node
QuadTreeNode::QuadTreeNode(AABB limits) {
	box = limits;
	is_leaf = true;
	node_depth = 0;
	for (int i = 0; i < 4; i++)
		childs[i] = nullptr;
}

QuadTreeNode::~QuadTreeNode() {
	// Make sure to delete children of deleted nodes
	if (!is_leaf) {
		for (int i = 0; i < 4; i++) 
			delete childs[i];
	}
}

void QuadTreeNode::CollectIntersections(std::list<GameObject*>& found_obj, const Frustum& primitive, int& checks) {

	checks++;
	if (box.Intersects(primitive)) {
			if (!is_leaf) {													// If not leaf call the function for all childs
				for (int i = 0; i < 4; i++)
				childs[i]->CollectIntersections(found_obj, primitive, checks);
			}
			else{
				for (auto it = objects.begin(); it != objects.end(); it++) {
					if(std::find(found_obj.begin(), found_obj.end(), (*it)) == found_obj.end()) // Don't push go that already are in the list
						found_obj.push_back((*it));
				}
							
			}
	}


}


void QuadTreeNode::Split() {

	// Top left
	float quarter_x = box.HalfSize().x / 2;
	float quarter_y = box.HalfSize().z / 2;
	float3 centre = box.CenterPoint();


	AABB split_box;
	float3 new_centre;
	float3 new_box_size = float3(box.HalfSize().x, box.HalfSize().y * 2, box.HalfSize().z);

	// Top left
	new_centre = box.Centroid() + float3(-quarter_x, 0, quarter_y);
	split_box.SetFromCenterAndSize(new_centre, new_box_size);
	childs[0] = new QuadTreeNode(split_box);

	// Top right
	new_centre = box.Centroid() + float3(quarter_x, 0, quarter_y);
	split_box.SetFromCenterAndSize(new_centre, new_box_size);
	childs[1] = new QuadTreeNode(split_box);

	//Bottom left
	new_centre = box.Centroid() + float3(-quarter_x, 0, -quarter_y);
	split_box.SetFromCenterAndSize(new_centre, new_box_size);
	childs[2] = new QuadTreeNode(split_box);

	//Bottom right
	new_centre = box.Centroid() + float3(quarter_x, 0, -quarter_y);
	split_box.SetFromCenterAndSize(new_centre, new_box_size);
	childs[3] = new QuadTreeNode(split_box);

	// Set their node depth
	for (int i = 0; i < 4; i++) {
		childs[i]->node_depth = node_depth + 1;
	}

}

bool QuadTreeNode::AddObject(GameObject * obj, int bucket_size, int max_splits) {

	// If the gameobject is not in this node we do nothing
	AABB check_box = *((ComponentAABB*)obj->getComponent(C_AABB))->getAABB();
	if (!box.Intersects(check_box))
		return false;

	if(is_leaf && (objects.size() < bucket_size || node_depth >= max_splits))// If the node is a leaf bucket size accepts another object or has reached its maximum depth
		objects.push_back(obj);				
	else {
		if (is_leaf) {						// If it is a leaf with its bucket size full, we crate children and empty the object list into them
			is_leaf = false;
			Split();		
			for (auto it = objects.begin(); it != objects.end(); it++)
				for (int i = 0; i < 4; i++)
					childs[i]->AddObject(*it, bucket_size, max_splits);	// If a child can already hold the object break the loop

			objects.clear();
		}
											
		for (int i = 0; i < 4; i++)									// We fill childs with the original object we wanted to add.
			childs[i]->AddObject(obj, bucket_size, max_splits);				// If a child can already hold the object break the loop
		
	}
	return true;

}

int QuadTreeNode::getNumObj() {
	return objects.size();
}

void QuadTreeNode::Draw() {
	App->renderer3D->DrawDirectAABB(box);

	for (int i = 0; i < 4; i++)
		if (childs[i]) childs[i]->Draw();
	
}
