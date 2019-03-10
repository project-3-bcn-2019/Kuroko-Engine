#include "ComponentAnimator.h"
#include "Application.h"
#include "ModuleResourcesManager.h"
#include "ResourceAnimationGraph.h"

ComponentAnimator::ComponentAnimator(JSON_Object* deff, GameObject * parent): Component(parent, ANIMATOR)
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

	loadValues(deff);

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

void ComponentAnimator::loadValues(JSON_Object * deff)
{
	JSON_Array* JSON_ints = json_object_get_array(deff, "ints");
	for (int i = 0; i < json_array_get_count(JSON_ints); ++i)
	{
		JSON_Object* value = json_array_get_object(JSON_ints, i);
		uint uuid = json_object_get_number(value, "uuid");
		int num = json_object_get_number(value, "value");
		ints.insert(std::pair<uint, int>(uuid, num));
	}

	JSON_Array* JSON_floats = json_object_get_array(deff, "floats");
	for (int i = 0; i < json_array_get_count(JSON_floats); ++i)
	{
		JSON_Object* value = json_array_get_object(JSON_floats, i);
		uint uuid = json_object_get_number(value, "uuid");
		float num = json_object_get_number(value, "value");
		floats.insert(std::pair<uint, float>(uuid, num));
	}

	JSON_Array* JSON_strings = json_object_get_array(deff, "strings");
	for (int i = 0; i < json_array_get_count(JSON_strings); ++i)
	{
		JSON_Object* value = json_array_get_object(JSON_strings, i);
		uint uuid = json_object_get_number(value, "uuid");
		std::string string = json_object_get_string(value, "value");
		strings.insert(std::pair<uint, std::string>(uuid, string));
	}

	JSON_Array* JSON_bools = json_object_get_array(deff, "bools");
	for (int i = 0; i < json_array_get_count(JSON_bools); ++i)
	{
		JSON_Object* value = json_array_get_object(JSON_bools, i);
		uint uuid = json_object_get_number(value, "uuid");
		bool boolean = json_object_get_boolean(value, "value");
		bools.insert(std::pair<uint, bool>(uuid, boolean));
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

void ComponentAnimator::removeValue(variableType type, uint uuid)
{
	switch (type)
	{
	case VAR_INT:
		ints.erase(uuid);
		break;
	case VAR_FLOAT:
		floats.erase(uuid);
		break;
	case VAR_STRING:
		strings.erase(uuid);
		break;
	case VAR_BOOL:
		bools.erase(uuid);
		break;
	}
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
			
			JSON_Value* JSON_ints = json_value_init_array();
			for (std::map<uint, int>::iterator it_i = ints.begin(); it_i != ints.end(); ++it_i)
			{
				JSON_Value* value = json_value_init_object();
				json_object_set_number(json_object(value), "uuid", (*it_i).first);
				json_object_set_number(json_object(value), "value", (*it_i).second);
				json_array_append_value(json_array(JSON_ints), value);
			}
			json_object_set_value(config, "ints", JSON_ints);

			JSON_Value* JSON_floats = json_value_init_array();
			for (std::map<uint, float>::iterator it_f = floats.begin(); it_f != floats.end(); ++it_f)
			{
				JSON_Value* value = json_value_init_object();
				json_object_set_number(json_object(value), "uuid", (*it_f).first);
				json_object_set_number(json_object(value), "value", (*it_f).second);
				json_array_append_value(json_array(JSON_floats), value);
			}
			json_object_set_value(config, "floats", JSON_floats);

			JSON_Value* JSON_strings = json_value_init_array();
			for (std::map<uint, std::string>::iterator it_s = strings.begin(); it_s != strings.end(); ++it_s)
			{
				JSON_Value* value = json_value_init_object();
				json_object_set_number(json_object(value), "uuid", (*it_s).first);
				json_object_set_string(json_object(value), "value", (*it_s).second.c_str());
				json_array_append_value(json_array(JSON_strings), value);
			}
			json_object_set_value(config, "strings", JSON_strings);

			JSON_Value* JSON_bools = json_value_init_array();
			for (std::map<uint, bool>::iterator it_b = bools.begin(); it_b != bools.end(); ++it_b)
			{
				JSON_Value* value = json_value_init_object();
				json_object_set_number(json_object(value), "uuid", (*it_b).first);
				json_object_set_boolean(json_object(value), "value", (*it_b).second);
				json_array_append_value(json_array(JSON_bools), value);
			}
			json_object_set_value(config, "bools", JSON_bools);
		}
		else
		{
			json_object_set_string(config, "graph_name", "missing reference");
			json_object_set_string(config, "Parent3dObject", "missing reference");
		}
	}
}