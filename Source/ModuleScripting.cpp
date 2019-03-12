#include "ModuleScripting.h"
#include "Wren/wren.hpp"
#include "Wren/wren_value.h"
#include "Wren/wren_vm.h"
#include "Applog.h"
#include "ModuleInput.h"
#include "ScriptData.h"
#include "ModuleScene.h"
#include "ModuleInput.h"
#include "ModuleResourcesManager.h"
#include "ResourceAnimationGraph.h"
#include "ModuleTimeManager.h"
#include "Include_Wwise.h"
// May be better to manage in scene
#include "GameObject.h"
#include "Transform.h"
#include "ComponentTransform.h"
#include "ComponentAudioSource.h"
#include "ComponentAnimation.h"
#include "ComponentPhysics.h"
#include "ComponentParticleEmitter.h"
#include "ComponentScript.h"
#include "ComponentButtonUI.h"
#include "ComponentCheckBoxUI.h"
#include "ComponentTextUI.h"
#include "ComponentProgressBarUI.h"
#include "ComponentAnimator.h"
#include "ComponentTrigger.h"



// Object comunicator
void SetGameObjectPos(WrenVM* vm);
void ModGameObjectPos(WrenVM* vm);
void lookAt(WrenVM* vm);
void rotate(WrenVM* vm);

void getGameObjectPosX(WrenVM* vm);
void getGameObjectPosY(WrenVM* vm);
void getGameObjectPosZ(WrenVM* vm);
void getGameObjectPitch(WrenVM* vm);
void getGameObjectYaw(WrenVM* vm);
void getGameObjectRoll(WrenVM* vm);

void KillGameObject(WrenVM* vm);
void MoveGameObjectForward(WrenVM* vm);
void GetComponentUUID(WrenVM* vm);
void GetCollisions(WrenVM* vm);
void GetScript(WrenVM* vm);

// Engine comunicator
void ConsoleLog(WrenVM* vm); 
void InstantiatePrefab(WrenVM* vm);
void getTime(WrenVM* vm);
void BreakPoint(WrenVM* vm);
void FindGameObjectByTag(WrenVM* vm);
void LoadScene(WrenVM* vm);

// Math
void sqrt(WrenVM* vm);
void angleBetween(WrenVM* vm);

//Time
void GetDeltaTime(WrenVM* vm);
void GetTimeScale(WrenVM* vm);
void SetTimeScale(WrenVM* vm);

// Input comunicator
void getKey(WrenVM* vm);
void getButton(WrenVM* vm);
void getAxes(WrenVM* vm);
void getMouseRaycastX(WrenVM* vm);
void getMouseRaycastY(WrenVM* vm);
void getMouseRaycastZ(WrenVM* vm);

// Audio
void SetSound(WrenVM* vm);
void PlayAudio(WrenVM* vm);
void PauseAudio(WrenVM* vm);
void ResumeAudio(WrenVM* vm);
void StopAudio(WrenVM* vm);

// Animation
	// Animation
void SetAnimation(WrenVM* vm);
void PlayAnimation(WrenVM* vm);
void PauseAnimation(WrenVM* vm);
void ResetAnimation(WrenVM* vm);
	// Animation graph
void SetInt(WrenVM* vm);
void GetInt(WrenVM* vm);
void SetFloat(WrenVM* vm);
void GetFloat(WrenVM* vm);
void SetString(WrenVM* vm);
void GetString(WrenVM* vm);
void SetBool(WrenVM* vm);
void GetBool(WrenVM* vm);


// Particles
void CreateParticles(WrenVM* vm);

// UI

	// Button
void ButtonGetState(WrenVM* vm);
	// Checkbox
void CheckboxIsPressed(WrenVM* vm);
	// Text
void UIText_SetText(WrenVM* vm);
	// Progress Bar
void SetProgress(WrenVM* vm);

// Physics
void SetSpeed(WrenVM* vm);


WrenForeignMethodFn bindForeignMethod(WrenVM* vm, const char* module, const char* className, bool isStatic, const char* signature); // Wren foraign methods
WrenForeignClassMethods bindForeignClass(WrenVM* vm, const char* module, const char* className);
char* loadModule(WrenVM* vm, const char* name);
void* reallocate(void* memory, size_t newSize);
void error(WrenVM* vm, WrenErrorType type, const char* module, int line, const char* message);
void write(WrenVM* vm, const char* text);

ModuleScripting::ModuleScripting(Application* app, bool start_enabled) : Module(app, start_enabled) 
{ 
	name = "scripting";
}


bool ModuleScripting::Init(const JSON_Object* config) 
{

	WrenConfiguration wconfig;
	wrenInitConfiguration(&wconfig);

	wconfig.bindForeignClassFn = bindForeignClass;
	wconfig.bindForeignMethodFn = bindForeignMethod;
	wconfig.loadModuleFn = loadModule;
	wconfig.writeFn = write;
	wconfig.errorFn = error;

	LoadConfig(config);

	tags.push_back("undefined");

	//wconfig.reallocateFn = reallocate;

	if (vm = wrenNewVM(&wconfig))
	{
		// Create base signature handles to call in every existing class
		base_signatures.insert(std::make_pair(std::string("Start()"), wrenMakeCallHandle(vm, "Start()")));
		base_signatures.insert(std::make_pair(std::string("Update()"), wrenMakeCallHandle(vm, "Update()")));
		base_signatures.insert(std::make_pair(std::string("new()"), wrenMakeCallHandle(vm, "new()")));
		base_signatures.insert(std::make_pair(std::string("gameObject=(_)"), wrenMakeCallHandle(vm, "gameObject=(_)")));

		
		object_linker_code = App->fs.GetFileString(OBJECT_LINKER_PATH);
		audio_code = App->fs.GetFileString(AUDIO_PATH);
		animation_code = App->fs.GetFileString(ANIMATION_PATH);
		particles_code = App->fs.GetFileString(PARTICLES_PATH);
		UI_code = App->fs.GetFileString(UI_PATH);
		physics_code = App->fs.GetFileString(PHYSICS_PATH);
		return true;
	}
	else
		return false;
}

update_status ModuleScripting::Update(float dt)
{
	// Iterate all script instances. Change the variables if edited and update them so they match value in script
	for (auto instance = loaded_instances.begin(); instance != loaded_instances.end(); instance++)
	{
		for (auto var = (*instance)->vars.begin(); var != (*instance)->vars.end(); var++)
		{
			// Update variables in script with editor values
			if (var->isEdited())
			{
				wrenEnsureSlots(vm, 2);
				wrenSetSlotHandle(vm, 0, (*instance)->class_handle);

				switch ((*var).getType())
				{
				case ImportedVariable::WREN_BOOL:
					wrenSetSlotBool(vm, 1, (*var).GetValue().value_bool);
					break;
				case ImportedVariable::WREN_NUMBER:
					wrenSetSlotDouble(vm, 1, (*var).GetValue().value_number);
					break;
				case ImportedVariable::WREN_STRING:
					wrenSetSlotString(vm, 1, (*var).value_string.c_str());
					break;
				}
				for (auto setter = (*instance)->methods.begin(); setter != (*instance)->methods.end(); setter++)
				{
					if ((*setter).getName() == (*var).getName() + "=(_)")
						wrenCall(vm, (*setter).getWrenHandle());
				}

				var->setEdited(false);
			}
			// Update editor values with variables in script
			wrenEnsureSlots(vm, 1);
			wrenSetSlotHandle(vm, 0, (*instance)->class_handle);
			wrenCall(vm, (*var).getGetter());
			Var script_value;
			switch ((*var).getType()) {
			case ImportedVariable::WREN_BOOL:
				script_value.value_bool = wrenGetSlotBool(vm, 0);
				(*var).SetValue(script_value, (*var).getType());
				break;
			case ImportedVariable::WREN_NUMBER:
				script_value.value_number = wrenGetSlotDouble(vm, 0);
				(*var).SetValue(script_value, (*var).getType());
				break;
			case ImportedVariable::WREN_STRING:
				static char buf[256] = "";
				strcpy(buf, wrenGetSlotString(vm, 0));
				(*var).value_string = buf;
				break;
			}

		}

		if ((*instance)->getState() == SCRIPT_STARTING)
		{
			for (auto it = (*instance)->methods.begin(); it != (*instance)->methods.end(); it++)
			{
				if ((*it).getName() == "Start()")
				{
					wrenEnsureSlots(vm, 1);
					wrenSetSlotHandle(vm, 0, (*instance)->class_handle);
					wrenCall(vm, (*it).getWrenHandle());
					break;
				}
			}
			(*instance)->setState(SCRIPT_UPDATING);
		}
		else if ((*instance)->getState() == SCRIPT_UPDATING)
		{
			for (auto it = (*instance)->methods.begin(); it != (*instance)->methods.end(); it++)
			{
				if ((*it).getName() == "Update()")
				{
					wrenEnsureSlots(vm, 1);
					wrenSetSlotHandle(vm, 0, (*instance)->class_handle);
					wrenCall(vm, (*it).getWrenHandle());
					break;
				}
			}
		}
	}
	return UPDATE_CONTINUE;
}


void ModuleScripting::StartInstances() { for (auto it : loaded_instances) (*it).setState(SCRIPT_STARTING); }
void ModuleScripting::PauseInstances() { for (auto it : loaded_instances) (*it).setState(SCRIPT_PAUSED); }
void ModuleScripting::StopInstances() { for (auto it : loaded_instances)  (*it).setState(SCRIPT_STOPPED); }

std::string ModuleScripting::enum2component(Component_type type) {

	switch (type) {
		case Component_type::ANIMATION:
			return "Animation";
		case Component_type::ANIMATION_EVENT:
			return "Animation Event";
		case Component_type::ANIMATOR:
			return "Animator";
		case Component_type::AUDIOLISTENER:
			return "Audio Listener";
		case Component_type::AUDIOSOURCE:
			return "Audio Source";
		case Component_type::BILLBOARD:
			return "Billboard";
		case Component_type::BONE:
			return "Bone";
		case Component_type::CAMERA:
			return "Camera";
		case Component_type::CANVAS:
			return "Canvas";
		case Component_type::PHYSICS:
			return "physics";
		case Component_type::C_AABB:
			return "AABB";
		case Component_type::MESH:
			return "Mesh";
		case Component_type::PARTICLE_EMITTER:
			return "Particle Emitter";
		case Component_type::RECTTRANSFORM:
			return "Rect Transform";
		case Component_type::SCRIPT:
			return "Script";
		case Component_type::TRANSFORM:
			return "Transform";
		case Component_type::UI_BUTTON:
			return "Button";
		case Component_type::UI_CHECKBOX:
			return "Checkbox";
		case Component_type::UI_IMAGE:
			return "Image";
		case Component_type::UI_PROGRESSBAR:
			return "Progress Bar";
		case Component_type::UI_TEXT:
			return "Text";

	}
	return std::string();
}


void ModuleScripting::SaveConfig(JSON_Object * config) const {
	JSON_Array* tags_arr = json_array(json_value_init_array());

	for (auto it = tags.begin(); it != tags.end(); it++) {
		if ((*it) == "undefined") // Hardcoded value, omit saving to avoid repetition
			continue;
		json_array_append_string(tags_arr, (*it).c_str());
	}

	json_object_set_value(config, "tags", json_array_get_wrapping_value(tags_arr));

}
void ModuleScripting::LoadConfig(const JSON_Object * config) {
	tags.clear();

	JSON_Array* tags_arr = json_object_get_array(config, "tags");

	for (int i = 0; i != json_array_get_count(tags_arr); i++)
		tags.push_back(json_array_get_string(tags_arr, i));
}


bool ModuleScripting::CleanUp() 
{
	for(auto it = base_signatures.begin(); it != base_signatures.end(); it++)
		wrenReleaseHandle(vm, (*it).second);

	base_signatures.clear();

	// Resources already hold pointers to the original handlers
	//for (int i = 0; i < loaded_scripts.size(); i++)
	//	delete loaded_scripts[i];

	wrenFreeVM(vm);
	loaded_scripts.clear();
	loaded_instances.clear();

	return true;
}


ScriptData* ModuleScripting::GenerateScript(const char* file_name_c)
{
	std::string file_name = file_name_c;

	ScriptData* script = new ScriptData();
	script->class_name = file_name;
	script->class_handle = GetHandlerToClass(file_name.c_str(), file_name.c_str());
		
	std::vector<std::string> methods = GetMethodsFromClassHandler(script->class_handle);
	for (int i = 0; i < methods.size(); i++)
	{
		std::string method_name = methods[i];
		if (method_name.find("(") != std::string::npos)					
		{
			if (method_name.find("=") == std::string::npos)				// method
			{
				ImportedVariable::WrenDataType return_type_test = ImportedVariable::WREN_NUMBER;
				std::string var_name_test = "test";
				std::vector<ImportedVariable> args;
				float var_value_test = 0.0f;

				size_t offset = method_name.find_first_of("(");
				for (auto it = method_name.find_first_of("_", offset + 1); it != std::string::npos; it = method_name.find_first_of("_", offset + 1))
				{
					offset = it;
					args.push_back(ImportedVariable(var_name_test.c_str(), return_type_test, (void*)&var_value_test, nullptr)); // Don't call getters on arguments
				}

				script->methods.push_back(ImportedMethod(method_name, return_type_test, args, wrenMakeCallHandle(vm, method_name.c_str())));
			}													 
			else	
				// setter
				script->methods.push_back(ImportedMethod(method_name, wrenMakeCallHandle(vm, method_name.c_str())));
		}						
		else															// var
		{

			float value_test = 0.0f;
			ImportedVariable var(method_name.c_str(), ImportedVariable::WrenDataType::WREN_NUMBER, &value_test, wrenMakeCallHandle(vm, method_name.c_str()));

			if (method_name == "gameObject")
				var.setPublic(false);

			script->vars.push_back(var);
		}
	}

	loaded_scripts.push_back(script);
	return script;
}

bool ModuleScripting::CompileIntoVM(const char* module, const char* code) 
{
	return (wrenInterpret(vm, module, code) == WREN_RESULT_SUCCESS);
}

WrenHandle* ModuleScripting::GetHandlerToClass(const char* module, const char* class_name) 
{
	// Get static class
	wrenEnsureSlots(vm, 1);
	wrenGetVariable(vm, module, class_name, 0); // Class is now on slot 0
	

	// Call constructor to get instance
	wrenCall(vm, base_signatures.at("new()")); // instance of the class is now at slot 0
	
	// Get the prize
	return wrenGetSlotHandle(vm, 0);
}

void ModuleScripting::ReleaseHandler(WrenHandle* handle)
{
	wrenReleaseHandle(vm, handle);
}

WrenHandle* ModuleScripting::getHandlerToMethod(const char* method_name)
{
	return wrenMakeCallHandle(vm, method_name);
}

std::vector<std::string> ModuleScripting::GetMethodsFromClassHandler(WrenHandle * wrenClass) {

	if (!IS_OBJ(wrenClass->value)) {
		app_log->AddLog("Trying to get methods from a non object handler");
		return std::vector<std::string>();
	}

	std::vector<std::string> ret;
	Obj* cls = AS_OBJ(wrenClass->value);

	for (int i = 0; i < cls->classObj->methods.count; ++i) 
	{
		Method& method = cls->classObj->methods.data[i];

		if (method.type != METHOD_PRIMITIVE && method.type != METHOD_FOREIGN && method.type != METHOD_NONE ) 
			ret.push_back(method.as.closure->fn->debug->name);
	}

	return ret;
}


// Wren callbacks  ======================================================================

char* loadModule(WrenVM* vm, const char* name) {
	char* ret = nullptr;
	if (strcmp(name, "ObjectLinker") == 0) {

		int string_size = strlen(App->scripting->object_linker_code.c_str()) + 1; // 1 for the /0
		ret = new char[string_size];
		strcpy(ret, App->scripting->object_linker_code.c_str());
		ret[string_size - 1] = '\0';
	}

	if (strcmp(name, "Audio") == 0) {
		int string_size = strlen(App->scripting->audio_code.c_str()) + 1; // 1 for the /0
		ret = new char[string_size];
		strcpy(ret, App->scripting->audio_code.c_str());
		ret[string_size - 1] = '\0';
	}

	if (strcmp(name, "Animation") == 0) {
		int string_size = strlen(App->scripting->animation_code.c_str()) + 1; // 1 for the /0
		ret = new char[string_size];
		strcpy(ret, App->scripting->animation_code.c_str());
		ret[string_size - 1] = '\0';
	}

	if (strcmp(name, "Particles") == 0) {
		int string_size = strlen(App->scripting->particles_code.c_str()) + 1; // 1 for the /0
		ret = new char[string_size];
		strcpy(ret, App->scripting->particles_code.c_str());
		ret[string_size - 1] = '\0';
	}

	if (strcmp(name, "UI") == 0) {
		int string_size = strlen(App->scripting->UI_code.c_str()) + 1; // 1 for the /0
		ret = new char[string_size];
		strcpy(ret, App->scripting->UI_code.c_str());
		ret[string_size - 1] = '\0';
	}

	if (strcmp(name, "Physics") == 0) {
		int string_size = strlen(App->scripting->physics_code.c_str()) + 1; // 1 for the /0
		ret = new char[string_size];
		strcpy(ret, App->scripting->physics_code.c_str());
		ret[string_size - 1] = '\0';
	}
	return ret;
}

void write(WrenVM* vm, const char* text)
{
	app_log->AddLog(text);
}

void error(WrenVM* vm, WrenErrorType type, const char* module, int line, const char* message)
{
	if (type == WrenErrorType::WREN_ERROR_COMPILE)
		app_log->AddLog("Couldn't compile %s. %s error in %i line", module, message, line);
	else if (type == WrenErrorType::WREN_ERROR_RUNTIME)
		app_log->AddLog("Error when running %s. %s error in %i line", module, message, line);
}


void* reallocate(void* memory, size_t newSize) {

	if (!memory) {
		return malloc(newSize);
	}
	else{
		if (newSize != 0)
			return realloc(memory, newSize);
		else
			free(memory);
	}

	return nullptr;
}

WrenForeignClassMethods bindForeignClass(WrenVM* vm, const char* module, const char* className)
{
	WrenForeignClassMethods methods;

	//if (strcmp(className, "File") == 0) {
	//	methods->allocate = fileAllocate;
	//	methods->finalize = fileFinalize;
	//}
	//else {
	//	// Unknown class.
	methods.allocate = NULL;
	methods.finalize = NULL;
	//}

	return methods;
}

WrenForeignMethodFn bindForeignMethod(WrenVM* vm, const char* module, const char* className, bool isStatic, const char* signature) // Wren foraign methods
{

	// OBJECT COMUNICATOR
	if(strcmp(module, "ObjectLinker") == 0){
		if(strcmp(className, "ObjectComunicator") == 0){
			if (strcmp(signature, "C_setPos(_,_,_,_)") == 0) {
				return SetGameObjectPos; //  C function for ObjectComunicator.C_setPos
			}
			if (strcmp(signature, "C_modPos(_,_,_,_)") == 0) {
				return ModGameObjectPos; // C function for ObjectComunicator.C_modPos
			}
			if (strcmp(signature, "C_rotate(_,_,_,_)") == 0) {
				return rotate;//Rotates the game object x degrees in each axis
			}
			if (strcmp(signature, "C_lookAt(_,_,_,_)") == 0) {
				return lookAt; // C function for ObjectComunicator.C_LookAt
			}
			if (strcmp(signature, "C_getPosX(_,_)") == 0) {
				return getGameObjectPosX; // C function for ObjectComunicator.C_getPosX
			}
			if (strcmp(signature, "C_getPosY(_,_)") == 0) {
				return getGameObjectPosY; // C function for ObjectComunicator.C_getPosY
			}
			if (strcmp(signature, "C_getPosZ(_,_)") == 0) {
				return getGameObjectPosZ; // C function for ObjectComunicator.C_getPosZ
			}
			if (strcmp(signature, "C_getPitch(_)") == 0) {
				return getGameObjectPitch; // C function for ObjectComunicator.C_getPitch
			}
			if (strcmp(signature, "C_getYaw(_)") == 0) {
				return getGameObjectYaw; // C function for ObjectComunicator.C_getYaw
			}
			if (strcmp(signature, "C_getRoll(_)") == 0) {
				return getGameObjectRoll; // C function for ObjectComunicator.C_getRoll
			}
			if (strcmp(signature, "C_Kill(_)") == 0) {
				return KillGameObject; // C function for ObjectComunicator.C_Kill
			}
			if (strcmp(signature, "C_MoveForward(_,_)") == 0) {
				return MoveGameObjectForward; // C function for ObjectComunicator.C_MoveForward
			}
			if (strcmp(signature, "C_GetComponentUUID(_,_)") == 0) {
				return GetComponentUUID; // C function for ObjectComunicator.C_GetComponentUUID
			}
			if (strcmp(signature, "C_GetCollisions(_)") == 0) {
				return GetCollisions; // C function for ObjectComunicator.C_GetCollisions
			}
			if (strcmp(signature, "C_GetScript(_,_)") == 0) {
				return GetScript; // C function for ObjectComunicator.C_GetScript
			}
		}
		if (strcmp(className, "Math") == 0) {
			if (isStatic && strcmp(signature, "C_sqrt(_)") == 0)
				return sqrt; // C function for Math.C_sqrt(_)

			if (isStatic && strcmp(signature, "C_angleBetween(_,_,_,_,_,_)") == 0)
				return angleBetween;
		
		}
		if (strcmp(className, "Time") == 0) {
			if (isStatic && strcmp(signature, "C_GetDeltaTime()") == 0) {
				return GetDeltaTime;
			}
			if (isStatic && strcmp(signature, "C_GetTimeScale()") == 0) {
				return GetTimeScale;
			}
			if(isStatic && strcmp(signature,"C_SetTimeScale(_)") == 0){
				return SetTimeScale;
			}
		}
		if (strcmp(className, "EngineComunicator") == 0) {
			if (isStatic && strcmp(signature, "consoleOutput(_)") == 0)
				return ConsoleLog; // C function for EngineComunicator.consoleOutput
			if (isStatic && strcmp(signature, "C_Instantiate(_,_,_,_,_,_,_)") == 0)
				return InstantiatePrefab; // C function for EngineComunicator.Instantiate
			if (isStatic && strcmp(signature, "getTime()") == 0)
				return getTime;
			if (isStatic && strcmp(signature, "BreakPoint(_,_,_)") == 0)
				return BreakPoint;
			if (isStatic && strcmp(signature, "C_FindGameObjectsByTag(_)") == 0)
				return FindGameObjectByTag;
			if (isStatic && strcmp(signature, "LoadScene(_)") == 0)
				return LoadScene;
		}
		if (strcmp(className, "InputComunicator") == 0) {
			if (isStatic && strcmp(signature, "getKey(_,_)") == 0)
				return getKey; // C function for InputComunicator.getKey
			if (isStatic && strcmp(signature, "getMouseRaycastX()") == 0)
				return getMouseRaycastX; // C function for InputComunicator.getMouseRaycastX
			if (isStatic && strcmp(signature, "getMouseRaycastY()") == 0)
				return getMouseRaycastY; // C function for InputComunicator.getMouseRaycastY
			if (isStatic && strcmp(signature, "getMouseRaycastZ()") == 0)
				return getMouseRaycastZ; // C function for InputComunicator.getMouseRaycastZ
			if (isStatic && strcmp(signature, "getButton(_,_,_)") == 0)
				return getButton;
			if (isStatic && strcmp(signature, "getAxis(_,_)") == 0)
				return getAxes;
		}
	}
	// AUDIO
	if (strcmp(module, "Audio") == 0) {
		if (strcmp(className, "AudioSourceComunicator") == 0) {
			if (isStatic && strcmp(signature, "C_SetSound(_,_,_)") == 0)
				return SetSound;
			if (isStatic && strcmp(signature, "C_Play(_,_)") == 0)
				return PlayAudio;
			if (isStatic && strcmp(signature, "C_Pause(_,_)") == 0)
				return PauseAudio;
			if (isStatic && strcmp(signature, "C_Resume(_,_)") == 0)
				return ResumeAudio;
			if (isStatic && strcmp(signature, "C_Stop(_,_)") == 0)
				return StopAudio;
		}
	}
	// ANIMATION
	if (strcmp(module, "Animation") == 0) {
		if (strcmp(className, "AnimationComunicator") == 0) {
			if (isStatic && strcmp(signature, "C_SetAnimation(_,_,_)") == 0)
				return SetAnimation;
			if (isStatic && strcmp(signature, "C_Play(_,_)") == 0)
				return PlayAnimation;
			if (isStatic && strcmp(signature, "C_Pause(_,_)") == 0)
				return PauseAnimation;
			if (isStatic && strcmp(signature, "C_ResetAnimation(_,_)") == 0)
				return ResetAnimation;

			if (isStatic && strcmp(signature, "C_SetInt(_,_,_,_)") == 0)
				return SetInt;
			if (isStatic && strcmp(signature, "C_GetInt(_,_,_)") == 0)
				return GetInt;

			if (isStatic && strcmp(signature, "C_SetFloat(_,_,_,_)") == 0)
				return SetFloat;
			if (isStatic && strcmp(signature, "C_GetFloat(_,_,_)") == 0)
				return GetFloat;

			if (isStatic && strcmp(signature, "C_SetString(_,_,_,_)") == 0)
				return SetString;
			if (isStatic && strcmp(signature, "C_GetString(_,_,_)") == 0)
				return GetString;

			if (isStatic && strcmp(signature, "C_SetBool(_,_,_,_)") == 0)
				return SetBool;
			if (isStatic && strcmp(signature, "C_GetBool(_,_,_)") == 0)
				return GetBool;


		}
	}

	// Particles
	if (strcmp(module, "Particles") == 0) {
		if (strcmp(className, "ParticleComunicator") == 0) {
			if (isStatic && strcmp(signature, "C_CreateParticles(_,_,_)") == 0)
				return CreateParticles;
		}
	}

	// UI
	if (strcmp(module, "UI") == 0) {
		if (strcmp(className, "ButtonComunicator") == 0) {
			if (isStatic && strcmp(signature, "C_ButtonGetState(_,_)") == 0)
				return ButtonGetState;
		}
		if (strcmp(className, "CheckboxComunicator") == 0) {
			if (isStatic && strcmp(signature, "C_CheckboxIsPressed(_,_)") == 0)
				return CheckboxIsPressed;
		}
		if (strcmp(className, "TextComunicator") == 0) {
			if (isStatic && strcmp(signature, "C_SetText(_,_,_)") == 0)
				return UIText_SetText;
		}
		if (strcmp(className, "ProgressBarComunicator") == 0) {
			if (isStatic && strcmp(signature, "C_SetProgress(_,_,_)") == 0)
				return SetProgress;
		}
	}

	// Physics
	if (strcmp(module, "Physics") == 0) {
		if (strcmp(className, "PhysicsComunicator") == 0) {
			if (isStatic && strcmp(signature, "C_SetSpeed(_,_,_,_,_)") == 0)
				return SetSpeed;
		}
	}

}

// C++ functions to be called from wren  ==================================================


// Object Comunicator
void SetGameObjectPos(WrenVM* vm) {

	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);

	float x = wrenGetSlotDouble(vm, 2);
	float y = wrenGetSlotDouble(vm, 3);
	float z = wrenGetSlotDouble(vm, 4);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);
	if (!go) {
		app_log->AddLog("Script asking for none existing resource");
		return;
	}

	ComponentTransform* c_trans = (ComponentTransform*)go->getComponent(TRANSFORM);
	c_trans->local->setPosition(float3(x, y, z));
	c_trans->LocalToGlobal();

}
void ModGameObjectPos(WrenVM* vm) {

	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);

	float x = wrenGetSlotDouble(vm, 2);
	float y = wrenGetSlotDouble(vm, 3);
	float z = wrenGetSlotDouble(vm, 4);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);
	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentTransform* c_trans = (ComponentTransform*)go->getComponent(TRANSFORM);

	float3 mod_trans_pos = c_trans->local->getPosition() + float3(x, y, z);
	c_trans->local->setPosition(mod_trans_pos);
}
void lookAt(WrenVM* vm) {

	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);

	float3 target = { (float)wrenGetSlotDouble(vm, 2), (float)wrenGetSlotDouble(vm, 3), (float)wrenGetSlotDouble(vm, 4) };

	GameObject* go = App->scene->getGameObject(gameObjectUUID);
	if (!go) {
		app_log->AddLog("Script asking for not existing gameObject");
		return;
	}

	ComponentTransform* c_trans = (ComponentTransform*)go->getComponent(TRANSFORM);
	c_trans->local->LookAt(float3(c_trans->global->getPosition().x, c_trans->global->getPosition().y, c_trans->global->getPosition().z), target);
}

void rotate(WrenVM* vm) {
	//HARDCODED
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	float3 rotation = { (float)wrenGetSlotDouble(vm, 2)*DEGTORAD, (float)wrenGetSlotDouble(vm, 3)*DEGTORAD, (float)wrenGetSlotDouble(vm, 4)*DEGTORAD };
	if (math::IsNan(rotation.y))
	{
		rotation.y = 0;
	}
	GameObject* go = App->scene->getGameObject(gameObjectUUID);
	if (!go) {
		app_log->AddLog("Script asking for not existing gameObject");
		return;
	}
	float2 rot2D = { rotation.x,-rotation.y };
	ComponentTransform* c_trans = (ComponentTransform*)go->getComponent(TRANSFORM);
	/*float3 final_rotation = c_trans->local->getRotationEuler() + rotation;*/
	Quat rot = Quat::identity;

	rot = rot.FromEulerXYZ(0,rot2D.AimedAngle()+90*DEGTORAD,0);
	c_trans->local->setRotation(rot);
	//c_trans->local->RotateAroundAxis(c_trans->local->Up(), rotation.y);

//	c_trans->local->setRotationEuler(final_rotation);
}

void getGameObjectPosX(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	const char* mode = wrenGetSlotString(vm, 2);
	GameObject* go = App->scene->getGameObject(gameObjectUUID);
	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentTransform* c_trans = (ComponentTransform*)go->getComponent(TRANSFORM);

	float ret = 0;

	if (strcmp(mode, "global") == 0)
		ret = c_trans->global->getPosition().x;
	else
		ret = c_trans->local->getPosition().x;

	wrenSetSlotDouble(vm, 0, ret);
}
void getGameObjectPosY(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	const char* mode = wrenGetSlotString(vm, 2);
	GameObject* go = App->scene->getGameObject(gameObjectUUID);
	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentTransform* c_trans = (ComponentTransform*)go->getComponent(TRANSFORM);

	float ret = 0;

	if (strcmp(mode, "global") == 0)
		ret = c_trans->global->getPosition().y;
	else
		ret = c_trans->local->getPosition().y;

	wrenSetSlotDouble(vm, 0, ret);
}
void getGameObjectPosZ(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	const char* mode = wrenGetSlotString(vm, 2);
	GameObject* go = App->scene->getGameObject(gameObjectUUID);
	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentTransform* c_trans = (ComponentTransform*)go->getComponent(TRANSFORM);

	float ret = 0;

	if (strcmp(mode, "global") == 0)
		ret = c_trans->global->getPosition().z;
	else
		ret = c_trans->local->getPosition().z;

	wrenSetSlotDouble(vm, 0, ret);
}
void getGameObjectPitch(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);
	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentTransform* c_trans = (ComponentTransform*)go->getComponent(TRANSFORM);

	wrenSetSlotDouble(vm, 0, c_trans->local->getRotationEuler().x);
}
void getGameObjectYaw(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);
	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentTransform* c_trans = (ComponentTransform*)go->getComponent(TRANSFORM);

	wrenSetSlotDouble(vm, 0, c_trans->local->getRotationEuler().y);
}
void getGameObjectRoll(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);
	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentTransform* c_trans = (ComponentTransform*)go->getComponent(TRANSFORM);

	wrenSetSlotDouble(vm, 0, c_trans->local->getRotationEuler().z);
}

void KillGameObject(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);
	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	App->scene->deleteGameObject(go);
}
void MoveGameObjectForward(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	float speed = wrenGetSlotDouble(vm, 2);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);
	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentTransform* c_trans = (ComponentTransform*)go->getComponent(TRANSFORM);

	float3 mod_trans_pos = c_trans->local->getPosition() + (c_trans->local->Forward() * speed);
	c_trans->local->setPosition(mod_trans_pos);

}
void GetComponentUUID(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	Component_type type = (Component_type)(int)wrenGetSlotDouble(vm, 2);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);

	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	Component* component = go->getComponent(type);

	if (!component) {
		app_log->AddLog("Game Object named %s has no %s", go->getName().c_str(), App->scripting->enum2component(type).c_str());
		return;
	}

	wrenSetSlotDouble(vm, 0, component->getUUID());
}
void GetCollisions(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);

	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentTrigger* component = (ComponentTrigger*)go->getComponent(PHYSICS);

	if (!component) {
		app_log->AddLog("Game Object %s has no ComponentColliderCube", go->getName().c_str());
		return;
	}

	// Get all the colliding GameObjects
	std::list<GameObject*> colliding_go = component->colliding;

	// Initialized list
	wrenSetSlotNewList(vm, 0);

	// Fill wren list
	wrenEnsureSlots(vm, 3);
	for (auto it = colliding_go.begin(); it != colliding_go.end(); it++) {
		wrenSetSlotDouble(vm, 2, (*it)->getUUID());
		wrenInsertInList(vm, 0, -1, 2);
	}

	// retrun the list in slot 0
}

void GetScript(WrenVM* vm) { // Could be faster if instances had a pointer to the parent (maybe)
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	std::string script_name = wrenGetSlotString(vm, 2);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);

	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}


	ComponentScript* component =  (ComponentScript*)go->getScriptComponent(script_name);

	if (!component) {
		app_log->AddLog("Game Object %s has no ComponentScript named %s", go->getName().c_str(), script_name.c_str());
		return;
	}
	
	wrenSetSlotHandle(vm, 0, component->instance_data->class_handle);
}
// Engine comunicator
void ConsoleLog(WrenVM* vm)
{
	const char* message = wrenGetSlotString(vm, 1);
	app_log->AddLog(message);
}
void InstantiatePrefab(WrenVM* vm) {

	std::string prefab_name = wrenGetSlotString(vm, 1);

	float x = wrenGetSlotDouble(vm, 2);
	float y = wrenGetSlotDouble(vm, 3);
	float z = wrenGetSlotDouble(vm, 4);

	float pitch = wrenGetSlotDouble(vm, 5);
	float yaw = wrenGetSlotDouble(vm, 6);
	float roll = wrenGetSlotDouble(vm, 7);

	std::string prefab_path = App->resources->getPrefabPath(prefab_name.c_str());

	App->scene->AskPrefabLoadFile(prefab_path.c_str(), float3(x, y, z), float3(pitch, yaw, roll));
}
void getTime(WrenVM* vm) {
	wrenSetSlotDouble(vm, 0, SDL_GetTicks());
}
void BreakPoint(WrenVM* vm) {
	std::string message = wrenGetSlotString(vm, 1);
	WrenType type = wrenGetSlotType(vm, 2);
	std::string var_name = wrenGetSlotString(vm, 3);
	int number = 0;
	std::string str;
	bool boolean = false;

	switch (type) {
	case WREN_TYPE_NUM:
		number = wrenGetSlotDouble(vm, 2);
		break;
	case WREN_TYPE_BOOL:
		boolean = wrenGetSlotBool(vm, 2);
		break;
	case WREN_TYPE_STRING:
		str = wrenGetSlotString(vm, 2);
		break;
	case WREN_TYPE_UNKNOWN:
		app_log->AddLog("Variable in the break point is not a C native type");
		return;
	}

	return; // PUT A BREAK POINT HERE TO SEE THE VALUE AND NAME OF THE VARIABLE
}
void FindGameObjectByTag(WrenVM* vm) {
	std::string tag = wrenGetSlotString(vm, 1);

	std::list<GameObject*> tagged_gos = App->scene->getGameObjectsByTag(tag);

	// Initialized list
	wrenSetSlotNewList(vm, 0);

	// Fill list
	wrenEnsureSlots(vm, 3);
	for (auto it = tagged_gos.begin(); it != tagged_gos.end(); it++) {
		wrenSetSlotDouble(vm, 2, (*it)->getUUID());
		wrenInsertInList(vm, 0, -1, 2);
	}


	// retrun the list in slot 0
}

void LoadScene(WrenVM* vm) {
	std::string scene_name = wrenGetSlotString(vm, 1);

	std::string scene_path = App->resources->getScenePath(scene_name.c_str());
	App->scene->AskSceneLoadFile((char*)scene_path.c_str());
}

// Math
void sqrt(WrenVM* vm) {
	int number = wrenGetSlotDouble(vm, 1);
	wrenSetSlotDouble(vm, 0, sqrt(number));
}

void angleBetween(WrenVM * vm)
{
	float3 vector1 = { (float)wrenGetSlotDouble(vm, 1), (float)wrenGetSlotDouble(vm, 2), (float)wrenGetSlotDouble(vm, 3) };
	float3 vector2 = { (float)wrenGetSlotDouble(vm, 4), (float)wrenGetSlotDouble(vm, 5), (float)wrenGetSlotDouble(vm, 6) };

	float angle = vector1.AngleBetween(vector2);
	angle = angle * RADTODEG;
	wrenSetSlotDouble(vm, 0, angle);
}

// Time
void GetDeltaTime(WrenVM * vm)
{
	wrenSetSlotDouble(vm, 0, App->time->getDeltaTime());
}
void GetTimeScale(WrenVM * vm)
{
	wrenSetSlotDouble(vm, 0, App->time->getTimeScale());
}
void SetTimeScale(WrenVM * vm)
{
	App->time->setTimeScale(wrenGetSlotDouble(vm, 1));
}

// Input
void getKey(WrenVM* vm) {
	int pressed_key = wrenGetSlotDouble(vm, 1);
	int mode = wrenGetSlotDouble(vm, 2);

	bool key_pressed = App->input->GetKey(pressed_key) == mode; 

	wrenSetSlotBool(vm, 0,  key_pressed);
}
void getButton(WrenVM* vm) {
	int controller_id = wrenGetSlotDouble(vm, 1);
	CONTROLLER_BUTTON button = (CONTROLLER_BUTTON)(uint)wrenGetSlotDouble(vm, 2);
	KEY_STATE mode = (KEY_STATE)(uint)wrenGetSlotDouble(vm, 3);

	bool ret = false;

	if (controller_id != -1)
		ret = App->input->getControllerButton(controller_id, button, mode);
	else
		ret = App->input->getFirstControllerButton(button, mode);


	wrenSetSlotBool(vm, 0, ret);

}
void getAxes(WrenVM* vm) {
	int controller_id = wrenGetSlotDouble(vm, 1);
	SDL_GameControllerAxis axis = (SDL_GameControllerAxis)(uint)wrenGetSlotDouble(vm, 2);

	float ret = 0;
	if (controller_id != -1)
		ret = App->input->getControllerAxis(controller_id, axis);
	else
		ret = App->input->getFirstControllerAxis(axis);

	wrenSetSlotDouble(vm, 0, ret);
}
void getMouseRaycastX(WrenVM* vm)
{
	float3 hit = App->scene->MousePickingHit();
	wrenSetSlotDouble(vm, 0, hit.x);
}
void getMouseRaycastY(WrenVM* vm)
{
	float3 hit = App->scene->MousePickingHit();
	wrenSetSlotDouble(vm, 0, hit.y);
}
void getMouseRaycastZ(WrenVM* vm)
{
	float3 hit = App->scene->MousePickingHit();
	wrenSetSlotDouble(vm, 0, hit.z);
}

// Audio
void SetSound(WrenVM * vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	uint componentUUID = wrenGetSlotDouble(vm, 2);
	std::string sound_string = wrenGetSlotString(vm, 3);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);

	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentAudioSource* component = (ComponentAudioSource*)go->getComponentByUUID(componentUUID);

	if (!component) {
		app_log->AddLog("Game Object %s has no ComponentAudioSource with %i uuid", go->getName().c_str(), componentUUID);
		return;
	}

	component->sound_ID = AK::SoundEngine::GetIDFromString(sound_string.c_str());
}
void PlayAudio(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	uint componentUUID = wrenGetSlotDouble(vm, 2);


	GameObject* go = App->scene->getGameObject(gameObjectUUID);

	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentAudioSource* component = (ComponentAudioSource*)go->getComponentByUUID(componentUUID);

	if (!component) {
		app_log->AddLog("Game Object %s has no ComponentAudioSource with %i uuid", go->getName().c_str(), componentUUID);
		return;
	}

	component->sound_go->PlayEvent(component->sound_ID);
}
void PauseAudio(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	uint componentUUID = wrenGetSlotDouble(vm, 2);


	GameObject* go = App->scene->getGameObject(gameObjectUUID);

	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentAudioSource* component = (ComponentAudioSource*)go->getComponentByUUID(componentUUID);

	if (!component) {
		app_log->AddLog("Game Object %s has no ComponentAudioSource with %i uuid", go->getName().c_str(), componentUUID);
		return;
	}

	component->sound_go->PauseEvent(component->sound_ID);
}
void ResumeAudio(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	uint componentUUID = wrenGetSlotDouble(vm, 2);
	uint sound_ID = wrenGetSlotDouble(vm, 3);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);

	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentAudioSource* component = (ComponentAudioSource*)go->getComponentByUUID(componentUUID);

	if (!component) {
		app_log->AddLog("Game Object %s has no ComponentAudioSource with %i uuid", go->getName().c_str(), componentUUID);
		return;
	}

	component->sound_go->ResumeEvent(component->sound_ID);
}
void StopAudio(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	uint componentUUID = wrenGetSlotDouble(vm, 2);
	uint sound_ID = wrenGetSlotDouble(vm, 3);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);

	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentAudioSource* component = (ComponentAudioSource*)go->getComponentByUUID(componentUUID);

	if (!component) {
		app_log->AddLog("Game Object %s has no ComponentAudioSource with %i uuid", go->getName().c_str(), componentUUID);
		return;
	}

	component->sound_go->StopEvent(component->sound_ID);
}

// Animation
void SetAnimation(WrenVM* vm) {

	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	uint componentUUID = wrenGetSlotDouble(vm, 2);
	std::string animation_string = wrenGetSlotString(vm, 3);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);

	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentAnimation* component = (ComponentAnimation*)go->getComponentByUUID(componentUUID);

	if (!component) {
		app_log->AddLog("Game Object %s has no ComponentAnimation with %i uuid", go->getName().c_str(), componentUUID);
		return;
	}

	uint animation_uuid = App->resources->getAnimationResourceUuid(animation_string.c_str());
	component->setAnimationResource(animation_uuid);
}
void PlayAnimation(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	uint componentUUID = wrenGetSlotDouble(vm, 2);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);

	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentAnimation* component = (ComponentAnimation*)go->getComponentByUUID(componentUUID);

	if (!component) {
		app_log->AddLog("Game Object %s has no ComponentAnimation with %i uuid", go->getName().c_str(), componentUUID);
		return;
	}

	component->Play();
}
void PauseAnimation(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	uint componentUUID = wrenGetSlotDouble(vm, 2);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);

	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentAnimation* component = (ComponentAnimation*)go->getComponentByUUID(componentUUID);

	if (!component) {
		app_log->AddLog("Game Object %s has no ComponentAnimation with %i uuid", go->getName().c_str(), componentUUID);
		return;
	}

	component->Pause();
}

void ResetAnimation(WrenVM * vm)
{
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	uint componentUUID = wrenGetSlotDouble(vm, 2);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);

	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentAnimation* component = (ComponentAnimation*)go->getComponentByUUID(componentUUID);

	if (!component) {
		app_log->AddLog("Game Object %s has no ComponentAnimation with %i uuid", go->getName().c_str(), componentUUID);
		return;
	}

	component->Reset();
}
// Animation graph
void SetInt(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	uint componentUUID = wrenGetSlotDouble(vm, 2);
	std::string name = wrenGetSlotString(vm, 3);
	int value = wrenGetSlotDouble(vm, 4);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);

	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentAnimator* component = (ComponentAnimator*)go->getComponentByUUID(componentUUID);

	if (!component) {
		app_log->AddLog("Game Object %s has no ComponentAnimation with %i uuid", go->getName().c_str(), componentUUID);
		return;
	}

	ResourceAnimationGraph* res = (ResourceAnimationGraph*)App->resources->getResource(component->getAnimationGraphResource());

	if (!res) {
		app_log->AddLog("Couldn't find the Animator's graph %i uuid in resources", component->getAnimationGraphResource());
		return;
	}

	uint var_uuid = res->getVariableUUID(name.c_str(), variableType::VAR_INT);

	component->setInt(var_uuid, value);
}
void GetInt(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	uint componentUUID = wrenGetSlotDouble(vm, 2);
	std::string name = wrenGetSlotString(vm, 3);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);

	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentAnimator* component = (ComponentAnimator*)go->getComponentByUUID(componentUUID);

	if (!component) {
		app_log->AddLog("Game Object %s has no ComponentAnimation with %i uuid", go->getName().c_str(), componentUUID);
		return;
	}

	ResourceAnimationGraph* res = (ResourceAnimationGraph*)App->resources->getResource(component->getAnimationGraphResource());

	if (!res) {
		app_log->AddLog("Couldn't find the Animator's graph %i uuid in resources", component->getAnimationGraphResource());
		return;
	}

	uint var_uuid = res->getVariableUUID(name.c_str(), variableType::VAR_INT);

	wrenSetSlotDouble(vm, 0, *component->getInt(var_uuid));
}
void SetFloat(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	uint componentUUID = wrenGetSlotDouble(vm, 2);
	std::string name = wrenGetSlotString(vm, 3);
	float value = wrenGetSlotDouble(vm, 4);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);

	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentAnimator* component = (ComponentAnimator*)go->getComponentByUUID(componentUUID);

	if (!component) {
		app_log->AddLog("Game Object %s has no ComponentAnimation with %i uuid", go->getName().c_str(), componentUUID);
		return;
	}

	ResourceAnimationGraph* res = (ResourceAnimationGraph*)App->resources->getResource(component->getAnimationGraphResource());

	if (!res) {
		app_log->AddLog("Couldn't find the Animator's graph %i uuid in resources", component->getAnimationGraphResource());
		return;
	}

	uint var_uuid = res->getVariableUUID(name.c_str(), variableType::VAR_FLOAT);

	component->setFloat(var_uuid, value);
}
void GetFloat(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	uint componentUUID = wrenGetSlotDouble(vm, 2);
	std::string name = wrenGetSlotString(vm, 3);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);

	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentAnimator* component = (ComponentAnimator*)go->getComponentByUUID(componentUUID);

	if (!component) {
		app_log->AddLog("Game Object %s has no ComponentAnimation with %i uuid", go->getName().c_str(), componentUUID);
		return;
	}

	ResourceAnimationGraph* res = (ResourceAnimationGraph*)App->resources->getResource(component->getAnimationGraphResource());

	if (!res) {
		app_log->AddLog("Couldn't find the Animator's graph %i uuid in resources", component->getAnimationGraphResource());
		return;
	}

	uint var_uuid = res->getVariableUUID(name.c_str(), variableType::VAR_FLOAT);

	wrenSetSlotDouble(vm, 0, *component->getFloat(var_uuid));
}
void SetString(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	uint componentUUID = wrenGetSlotDouble(vm, 2);
	std::string name = wrenGetSlotString(vm, 3);
	std::string value = wrenGetSlotString(vm, 4);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);

	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentAnimator* component = (ComponentAnimator*)go->getComponentByUUID(componentUUID);

	if (!component) {
		app_log->AddLog("Game Object %s has no ComponentAnimation with %i uuid", go->getName().c_str(), componentUUID);
		return;
	}

	ResourceAnimationGraph* res = (ResourceAnimationGraph*)App->resources->getResource(component->getAnimationGraphResource());

	if (!res) {
		app_log->AddLog("Couldn't find the Animator's graph %i uuid in resources", component->getAnimationGraphResource());
		return;
	}

	uint var_uuid = res->getVariableUUID(name.c_str(), variableType::VAR_STRING);

	component->setString(var_uuid, value.c_str());


}
void GetString(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	uint componentUUID = wrenGetSlotDouble(vm, 2);
	std::string name = wrenGetSlotString(vm, 3);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);

	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentAnimator* component = (ComponentAnimator*)go->getComponentByUUID(componentUUID);

	if (!component) {
		app_log->AddLog("Game Object %s has no ComponentAnimation with %i uuid", go->getName().c_str(), componentUUID);
		return;
	}

	ResourceAnimationGraph* res = (ResourceAnimationGraph*)App->resources->getResource(component->getAnimationGraphResource());

	if (!res) {
		app_log->AddLog("Couldn't find the Animator's graph %i uuid in resources", component->getAnimationGraphResource());
		return;
	}

	uint var_uuid = res->getVariableUUID(name.c_str(), variableType::VAR_STRING);

	wrenSetSlotString(vm, 0, (*component->getString(var_uuid)).c_str());
}
void SetBool(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	uint componentUUID = wrenGetSlotDouble(vm, 2);
	std::string name = wrenGetSlotString(vm, 3);
	bool value = wrenGetSlotBool(vm, 4);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);

	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentAnimator* component = (ComponentAnimator*)go->getComponentByUUID(componentUUID);

	if (!component) {
		app_log->AddLog("Game Object %s has no ComponentAnimation with %i uuid", go->getName().c_str(), componentUUID);
		return;
	}

	ResourceAnimationGraph* res = (ResourceAnimationGraph*)App->resources->getResource(component->getAnimationGraphResource());

	if (!res) {
		app_log->AddLog("Couldn't find the Animator's graph %i uuid in resources", component->getAnimationGraphResource());
		return;
	}

	uint var_uuid = res->getVariableUUID(name.c_str(), variableType::VAR_BOOL);

	component->setBool(var_uuid, value);
}
void GetBool(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	uint componentUUID = wrenGetSlotDouble(vm, 2);
	std::string name = wrenGetSlotString(vm, 3);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);

	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentAnimator* component = (ComponentAnimator*)go->getComponentByUUID(componentUUID);

	if (!component) {
		app_log->AddLog("Game Object %s has no ComponentAnimation with %i uuid", go->getName().c_str(), componentUUID);
		return;
	}

	ResourceAnimationGraph* res = (ResourceAnimationGraph*)App->resources->getResource(component->getAnimationGraphResource());

	if (!res) {
		app_log->AddLog("Couldn't find the Animator's graph %i uuid in resources", component->getAnimationGraphResource());
		return;
	}

	uint var_uuid = res->getVariableUUID(name.c_str(), variableType::VAR_BOOL);

	bool read = *component->getBool(var_uuid);
	wrenSetSlotBool(vm, 0, read);
}
// Particles
void CreateParticles(WrenVM* vm) {

	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	uint componentUUID = wrenGetSlotDouble(vm, 2);
	uint particles = wrenGetSlotDouble(vm, 3);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);

	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentParticleEmitter* component = (ComponentParticleEmitter*)go->getComponentByUUID(componentUUID);

	if (!component) {
		app_log->AddLog("Game Object %s has no ComponentParticles with %i uuid", go->getName().c_str(), componentUUID);
		return;
	}

	for(int i = 0; i < particles; i++)
		component->CreateParticle();
}

// UI

	// Button
void ButtonGetState(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	uint componentUUID = wrenGetSlotDouble(vm, 2);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);

	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentButtonUI* component = (ComponentButtonUI*)go->getComponentByUUID(componentUUID);

	if (!component) {
		app_log->AddLog("Game Object %s has no ComponentButton with %i uuid", go->getName().c_str(), componentUUID);
		return;
	}

	wrenSetSlotDouble(vm, 0, (int)component->getState());
}

// Checkbox
void CheckboxIsPressed(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	uint componentUUID = wrenGetSlotDouble(vm, 2);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);

	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentCheckBoxUI* component = (ComponentCheckBoxUI*)go->getComponentByUUID(componentUUID);

	if (!component) {
		app_log->AddLog("Game Object %s has no ComponentCheckbox with %i uuid", go->getName().c_str(), componentUUID);
		return;
	}

	wrenSetSlotBool(vm, 0, component->isPressed());
}

// Text

void UIText_SetText(WrenVM* vm) {

	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	uint componentUUID = wrenGetSlotDouble(vm, 2);
	std::string message = wrenGetSlotString(vm, 3);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);

	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentTextUI* component = (ComponentTextUI*)go->getComponentByUUID(componentUUID);

	if (!component) {
		app_log->AddLog("Game Object %s has no ComponentText with %i uuid", go->getName().c_str(), componentUUID);
		return;
	}

	component->SetText(message.c_str());
}

// Progress Bar

void SetProgress(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	uint componentUUID = wrenGetSlotDouble(vm, 2);
	uint percentage = wrenGetSlotDouble(vm, 3);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);

	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentProgressBarUI* component = (ComponentProgressBarUI*)go->getComponentByUUID(componentUUID);

	if (!component) {
		app_log->AddLog("Game Object %s has no ComponentText with %i uuid", go->getName().c_str(), componentUUID);
		return;
	}

component->setPercent(percentage);
}


// Physics
void SetSpeed(WrenVM* vm) {
	uint gameObjectUUID = wrenGetSlotDouble(vm, 1);
	uint componentUUID = wrenGetSlotDouble(vm, 2);
	int x = wrenGetSlotDouble(vm, 3);
	int y = wrenGetSlotDouble(vm, 4);
	int z = wrenGetSlotDouble(vm, 5);

	GameObject* go = App->scene->getGameObject(gameObjectUUID);

	if (!go) {
		app_log->AddLog("Script asking for none existing gameObject");
		return;
	}

	ComponentPhysics* component = (ComponentPhysics*)go->getComponentByUUID(componentUUID);

	if (!component) {
		app_log->AddLog("Game Object %s has no ComponentText with %i uuid", go->getName().c_str(), componentUUID);
		return;
	}

	component->SetSpeed(x, y, z);
}