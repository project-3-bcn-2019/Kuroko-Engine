#include "ComponentAnimator.h"
#include "Application.h"
#include "ModuleResourcesManager.h"
#include "ResourceAnimationGraph.h"

ComponentAnimator::ComponentAnimator(JSON_Object * deff, GameObject * parent)
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
