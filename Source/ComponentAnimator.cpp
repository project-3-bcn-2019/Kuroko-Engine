#include "ComponentAnimator.h"
#include "Application.h"
#include "ModuleResourcesManager.h"
#include "ResourceAnimationGraph.h"

ComponentAnimator::ComponentAnimator(JSON_Object * deff, GameObject * parent): Component(parent, ANIMATOR)
{
	const char* parent3dobject = json_object_get_string(deff, "Parent3dObject");
	if (App->is_game && !App->debug_game)
	{
		graph_resource_uuid = App->resources->getResourceUuid(json_object_get_string(deff, "graph_name"), R_ANIMATIONGRAPH);
	}
	else if (parent3dobject) // Means that is being loaded from a scene
		graph_resource_uuid = App->resources->getAnimationGraphResourceUuid(parent3dobject, json_object_get_string(deff, "graph_name"));
	else // Means it is being loaded from a 3dObject binary
		graph_resource_uuid = json_object_get_number(deff, "graph_resource_uuid");

	App->resources->assignResource(graph_resource_uuid);
}

ComponentAnimator::~ComponentAnimator()
{
}

void ComponentAnimator::setAnimationGraphResource(uint uuid)
{
	App->resources->deasignResource(graph_resource_uuid);
	graph_resource_uuid = uuid;
	App->resources->assignResource(graph_resource_uuid);

	//Clear lists
	ints.clear();
	floats.clear();
	strings.clear();
	bools.clear();

	ResourceAnimationGraph* graph = (ResourceAnimationGraph*)App->resources->getResource(graph_resource_uuid);
	if (graph != nullptr)
	{
		for (std::list<Variable*>::iterator it = graph->blackboard.begin(); it != graph->blackboard.end(); ++it)
		{
			switch ((*it)->type)
			{
			case VAR_INT:
				ints.insert(std::pair<uint, int>((*it)->uuid, 0));
				break;
			case VAR_FLOAT:
				floats.insert(std::pair<uint, float>((*it)->uuid, 0.0f));
				break;
			case VAR_STRING:
				strings.insert(std::pair<uint, std::string>((*it)->uuid, ""));
				break;
			case VAR_BOOL:
				bools.insert(std::pair<uint, bool>((*it)->uuid, false));
				break;
			}
		}
	}
}

void ComponentAnimator::setInt(uint uuid, int value)
{
	ints[uuid] = value;
}

int* ComponentAnimator::getInt(uint uuid)
{
	if (ints.find(uuid) != ints.end())
		return &ints[uuid];
	return nullptr;
}

void ComponentAnimator::setFloat(uint uuid, float value)
{
	floats[uuid] = value;
}

float* ComponentAnimator::getFloat(uint uuid)
{
	if (floats.find(uuid) != floats.end())
		return &floats[uuid];
	return nullptr;
}

void ComponentAnimator::setString(uint uuid, const char * value)
{
	strings[uuid] = value;
}

std::string* ComponentAnimator::getString(uint uuid)
{
	if (strings.find(uuid) != strings.end())
		return &strings[uuid];
	return nullptr;
}

void ComponentAnimator::setBool(uint uuid, bool value)
{
	bools[uuid] = value;
}

bool* ComponentAnimator::getBool(uint uuid)
{
	if (bools.find(uuid) != bools.end())
		return &bools[uuid];
	return nullptr;
}

void ComponentAnimator::Save(JSON_Object * config)
{
	json_object_set_string(config, "type", "animator");

	if (graph_resource_uuid != 0)
	{
		ResourceAnimationGraph* res_graph = (ResourceAnimationGraph*)App->resources->getResource(graph_resource_uuid);
		if (res_graph)
		{
			json_object_set_string(config, "graph_name", res_graph->asset.c_str());
			json_object_set_string(config, "Parent3dObject", res_graph->Parent3dObject.c_str());
		}
		else
		{
			json_object_set_string(config, "graph_name", "missing reference");
			json_object_set_string(config, "Parent3dObject", "missing reference");
		}
	}
}
