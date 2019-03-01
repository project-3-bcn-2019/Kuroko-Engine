#ifndef _COMPONENT_MESH
#define _COMPONENT_MESH
#include "Component.h"
#include "Mesh.h"

class Material;

class ComponentMesh : public Component {

public:

	ComponentMesh(GameObject* gameobject) : Component(gameobject, MESH) {};   // empty constructor
	ComponentMesh(JSON_Object* deff, GameObject* parent);
	ComponentMesh(GameObject* gameobject, PrimitiveTypes type) : Component(gameobject, MESH), primitive_type(type) {};
	//ComponentMesh(GameObject* gameobject, Mesh* mesh) : Component(gameobject, MESH), primitive_mesh(mesh) {};

	~ComponentMesh();
	void Draw() const;
	void DrawSelected() const;
	bool getWireframe() const			{ return wireframe; };
	bool getDrawNormals()const			{ return draw_normals; };
	Material* getMaterial()	const		{ return mat; };
	void setWireframe(bool state)		{ wireframe = state; };
	void setDrawNormals(bool state)		{ draw_normals = state; };

	bool Update(float dt);

	Mesh* getMesh()	const;
	uint getMeshResource() { return mesh_resource_uuid; }
	void setMaterial(Material* new_mat) { mat = new_mat; };// Can recieve nullptr
	void setMeshResourceId(uint _mesh_resource_uuid); 
	PrimitiveTypes primitiveString2PrimitiveType(std::string primitive_type_string);
	std::string PrimitiveType2primitiveString(PrimitiveTypes type);

	void Skining() const;

	void Save(JSON_Object* config);

public:

	std::vector<std::string> bones_names;
	std::vector<uint> components_bones;

private:

	Mesh * getMeshFromResource() const;
	bool wireframe = false;
	bool draw_normals = false;

	Material* mat = nullptr;

	uint mesh_resource_uuid = 0;
	PrimitiveTypes primitive_type = PrimitiveTypes::Primitive_None;
};
#endif