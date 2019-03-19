#pragma once
#include <string>
#include <vector>

struct WrenHandle;

union Var {
	bool value_bool = false;
	float value_number;
};

struct ImportedVariable
{
	enum WrenDataType { WREN_UNKNOWN, WREN_BOOL, WREN_STRING, WREN_NUMBER };

	ImportedVariable(const char* name, WrenDataType type, void* value, WrenHandle* _getter);
private:
	Var value;
	std::string var_name;
	WrenDataType data_type = WREN_UNKNOWN;
	bool is_public = true;
	bool edited = false;
	bool forced_type = false;
	WrenHandle* getter = nullptr;

public:
	std::string getName()	{ return var_name; }
	Var GetValue()			{ return value; }
	std::string value_string;
	WrenHandle* getGetter()  { return getter; }
	WrenDataType getType()	{ return data_type; }
	bool isPublic()			{ return is_public; }
	bool isEdited()			{ return edited; }
	bool isTypeForced()		{ return forced_type; }

	void SetValue(Var value, WrenDataType value_type = WREN_UNKNOWN);   // value type used to check value dataype when forced type is enabled.

	void setType(WrenDataType type) { data_type = type; }
	void setPublic(bool value)		{ is_public = value; }
	void setEdited(bool value)		{ edited = value; }
	void setForcedType(bool value)	{ forced_type = value; }

};


struct ImportedMethod
{
	friend class ComponentScript;
	ImportedMethod(std::string name, WrenHandle* handle) : method_name(name), method_handle(handle) {};  // for setters

private:
	std::string method_name;
	WrenHandle* method_handle = nullptr;

public:
	std::string getName() { return method_name; };
	WrenHandle* getWrenHandle() { return method_handle; };
};

enum ScriptState { SCRIPT_STARTING, SCRIPT_UPDATING, SCRIPT_CLOSING, SCRIPT_PAUSED, SCRIPT_STOPPED};

struct ScriptData
{
	~ScriptData();
	ScriptState getState() { return state; }
	void setState(ScriptState st) { state = st; }

	std::string class_name;

	WrenHandle* class_handle = nullptr;

	std::vector<ImportedVariable> vars;
	std::vector<ImportedMethod>  methods;

private:
	ScriptState state = SCRIPT_STOPPED;
};
