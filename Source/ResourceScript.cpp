#include "ResourceScript.h"
#include "Wren/wren.hpp"
#include "Application.h"
#include "ModuleScripting.h"
#include "ScriptData.h"
#include "Applog.h"


ResourceScript::ResourceScript(resource_deff deff): Resource(deff)  {

	UpdateCode();
}

void ResourceScript::UpdateCode() {
	// Get the binary
	JSON_Value* script_binary = json_parse_file(binary.c_str());

	// Read code and class name
	const char* code_ptr = json_object_get_string(json_object(script_binary), "code");
	const char* class_name_ptr = json_object_get_string(json_object(script_binary), "class_name");
	
	if (code_ptr) code = code_ptr;	
	if (class_name_ptr) class_name	= class_name_ptr;
	
	invalid_resource = (!(code_ptr && class_name_ptr));

	json_value_free(script_binary);
}

void ResourceScript::Compile() {

	UpdateCode();

	if(!invalid_resource)
		invalid_resource = !App->scripting->CompileIntoVM(class_name.c_str(), code.c_str()); // If return false invalidate resource
}

void ResourceScript::GenerateScript() {
	if (!invalid_resource)
		data = App->scripting->GenerateScript(class_name.c_str());
	else
		data = nullptr;
}

void ResourceScript::CleanUp()
{
	if(data){

		// Release all getters
		for (auto it = data->vars.begin(); it != data->vars.end(); it++) {
			App->scripting->ReleaseHandler((*it).getGetter());
		}
		// Release all setters
		for (auto it = data->methods.begin(); it != data->methods.end(); it++) {
			App->scripting->ReleaseHandler((*it).getWrenHandle());
		}

		delete data;
	}


}




ResourceScript::~ResourceScript() {
	CleanUp();
}


