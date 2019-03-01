#include "ComponentScript.h"
#include "Wren/wren.hpp"
#include "Application.h"
#include "ModuleResourcesManager.h"
#include "ModuleTimeManager.h"

#include "ModuleInput.h"
#include "GameObject.h"
#include "Applog.h"


ComponentScript::ComponentScript(GameObject* g_obj, uint resource_uuid) : Component(g_obj, SCRIPT)
{
	if(resource_uuid != 0)
		assignScriptResource(resource_uuid);
}

ComponentScript::ComponentScript(JSON_Object * deff, GameObject * parent): Component(parent, SCRIPT) {
	script_resource_uuid = App->resources->getResourceUuid(json_object_get_string(deff, "script"));

	assignScriptResource(script_resource_uuid);
	
	JSON_Array* variables = json_object_get_array(deff, "variables");

	if (!instance_data)
		return;

	for (int i = 0; i < json_array_get_count(variables); i++) {
		JSON_Object* variable = json_array_get_object(variables, i);

		std::string variable_name = json_object_get_string(variable, "name");
		
		// Look for the variable in the instance data
		for (auto it = instance_data->vars.begin(); it != instance_data->vars.end(); it++) {
			if ((*it).getName() == variable_name) { // If the variable is found fill it with information
				std::string type_string = json_object_get_string(variable, "type");

				(*it).setPublic(json_object_get_boolean(variable, "public"));
				(*it).setForcedType(json_object_get_boolean(variable, "type_forced"));

				if (type_string == "number") {
					Var value;
					value.value_number = json_object_get_number(variable, "value");
					(*it).setType(ImportedVariable::WREN_NUMBER);
					(*it).SetValue(value, ImportedVariable::WREN_NUMBER);
				}
				if (type_string == "bool") {
					Var value;
					value.value_bool = json_object_get_boolean(variable, "value");
					(*it).setType(ImportedVariable::WREN_BOOL);
					(*it).SetValue(value, ImportedVariable::WREN_BOOL);
				}
				if (type_string == "string") {
					(*it).value_string = json_object_get_string(variable, "value");
					(*it).setType(ImportedVariable::WREN_STRING);
				}

				(*it).setEdited(true);
			}
		}
	}
}

void ComponentScript::assignScriptResource(uint resource_uuid)
{
	if (instance_data)
		CleanUp();

	script_resource_uuid = resource_uuid;
	App->resources->assignResource(script_resource_uuid);

	LoadResource();
}

void ComponentScript::LoadResource() {

	ResourceScript* script_r = (ResourceScript*)App->resources->getResource(script_resource_uuid);

	if (!script_r) {
		app_log->AddLog("Couldn't load script, resource not found");
		script_name = "Unknown";
		instance_data = nullptr;
		return;
	}
	ScriptData* base_script_data = script_r->getData();
	script_name = script_r->class_name.c_str();

	if (!base_script_data) {
		app_log->AddLog("Couldn't load script, resource is invalid");
		instance_data = nullptr;
		return;
	}

	instance_data = new ScriptData();

	instance_data->class_name = base_script_data->class_name;
	instance_data->vars = base_script_data->vars;
	instance_data->methods = base_script_data->methods;
	instance_data->class_handle = App->scripting->GetHandlerToClass(instance_data->class_name.c_str(), instance_data->class_name.c_str());

	// If the game is playing the script should start executing right away
	if(App->time->getGameState() == GameState::PLAYING) 
		instance_data->setState(ScriptState::SCRIPT_STARTING);

	LinkScriptToObject();

	App->scripting->loaded_instances.push_back(instance_data);

}
void ComponentScript::LinkScriptToObject() {

	WrenVM* vm = App->scripting->vm;
	WrenHandle* class_handle = instance_data->class_handle;
	wrenEnsureSlots(vm, 2); // One for class and other for GO

	// Set instance
	wrenSetSlotHandle(vm, 0, class_handle);
	// Set argument
	wrenSetSlotDouble(vm, 1, getParent()->getUUID());
	// Call setter
	wrenCall(vm, App->scripting->base_signatures.at("gameObject=(_)"));

}



void ComponentScript::CleanUp() {
	App->scripting->loaded_instances.remove(instance_data);
	delete instance_data;
}

void ComponentScript::Save(JSON_Object * config) {
	json_object_set_string(config, "type", "script");

	ResourceScript* res_script = (ResourceScript*)App->resources->getResource(script_resource_uuid);

	if(res_script)
		json_object_set_string(config, "script", res_script->asset.c_str()); // Gives us all we need but the variables
	else
		json_object_set_string(config, "script", "missing reference"); 

	// Store the variables
	if (!instance_data) {
		return;
	}
	// Create and fill array
	JSON_Value* vars_array = json_value_init_array();
	for (auto it = instance_data->vars.begin(); it != instance_data->vars.end(); it++) {

		if (strcmp((*it).getName().c_str(), "gameObject") == 0) // Don't save the gameObject variable, is going to be relinked in load
			continue;

		JSON_Value* var = json_value_init_object();

		json_object_set_string(json_object(var), "name", (*it).getName().c_str());
		switch ((*it).getType()) {
		case ImportedVariable::WREN_NUMBER:
			json_object_set_string(json_object(var), "type", "number");
			json_object_set_number(json_object(var), "value", (*it).GetValue().value_number);
			break;
		case ImportedVariable::WREN_BOOL:
			json_object_set_string(json_object(var), "type", "bool");
			json_object_set_boolean(json_object(var), "value", (*it).GetValue().value_bool);
			break;
		case ImportedVariable::WREN_STRING:
			json_object_set_string(json_object(var), "type", "string");
			json_object_set_string(json_object(var), "value", (*it).value_string.c_str());
			break;
		}

		json_object_set_boolean(json_object(var), "public", (*it).isPublic());
		json_object_set_boolean(json_object(var), "type_forced", (*it).isTypeForced());

		json_array_append_value(json_array(vars_array), var);
	}

	json_object_set_value(config, "variables", vars_array);
}


ComponentScript::~ComponentScript()
{
	CleanUp();
}

