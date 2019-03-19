#include "ScriptData.h"
#include "ModuleScripting.h"
#include "Application.h"

ImportedVariable::ImportedVariable(const char* name, WrenDataType type, void* _value, WrenHandle* _getter)
{
	var_name = name;
	data_type = type;
	getter = _getter;

	bool* var_bool = nullptr;
	float* var_float = nullptr;

	switch (type)
	{
	case WREN_BOOL:
		var_bool = (bool*)_value;
		value.value_bool = *var_bool;
		break;
	case WREN_NUMBER:
		var_float = (float*)_value;
		value.value_number = *var_float;
		break;
	case WREN_STRING:
		value_string = (char*)_value;
		break;
	}
}

void ImportedVariable::SetValue(Var value, WrenDataType value_type)
{
	if (!isTypeForced() || value_type == getType())
		this->value = value;
}


ScriptData::~ScriptData()
{
	class_name.clear();

	vars.clear();
	methods.clear();
	if (class_handle)
		App->scripting->ReleaseHandler(class_handle);
}


