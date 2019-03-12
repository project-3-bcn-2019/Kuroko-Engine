#include "PanelObjectInspector.h"
#include "Application.h"
#include "ModuleScene.h"
#include "GameObject.h"
#include "ModuleScripting.h"
#include "ModuleUI.h"
#include "ModuleResourcesManager.h"
#include "ModuleAudio.h"
#include "Include_Wwise.h"

#include "Component.h"
#include "ComponentScript.h"
#include "ComponentAudioSource.h"


PanelObjectInspector::PanelObjectInspector(const char * name, bool active): Panel(name, active)
{
}

PanelObjectInspector::~PanelObjectInspector()
{
}

void PanelObjectInspector::Draw()
{
	ImGui::Begin("Object Inspector", &active);
	//ImGui::PushFont(ui_fonts[REGULAR]);

	static bool select_script = false;
	static bool select_audio = false;
	if (App->scene->selected_obj.size() == 1) {
		GameObject* selected_obj = (*App->scene->selected_obj.begin());

		if (selected_obj)
		{
			ImGui::Text("Name: %s", selected_obj->getName().c_str());

			if (selected_obj->is_UI){//if is UI
				ImGui::SameLine(0.f, 10.0f);
				ImGui::TextColored(ImVec4(0.25f, 0.25f, 0.25f, 1), "UI GameObject");
			}

			ImGui::Checkbox("Active", &selected_obj->is_active);
			
			if (!selected_obj->is_UI) { // if it is not UI
				ImGui::SameLine();
				if (ImGui::Checkbox("Static", &selected_obj->is_static)) // If an object is set/unset static, reload the quadtree
					App->scene->quadtree_reload = true;
				DrawTagSelection(selected_obj);
				// Add a new tag
				static char new_tag[64];
				ImGui::InputText("New Tag", new_tag, 64);
				if (ImGui::Button("Add Tag")) {
					App->scripting->tags.push_back(new_tag);
					for (int i = 0; i < 64; i++)
						new_tag[i] = '\0';

				}
			}
			
			if (ImGui::CollapsingHeader("Add component"))
			{
				if (ImGui::Button("Add Script")) select_script = true;

				if (!selected_obj->is_UI) {
					if (ImGui::Button("Add Mesh"))	selected_obj->addComponent(MESH);
					if (ImGui::Button("Add Camera"))  selected_obj->addComponent(CAMERA);
					if (ImGui::Button("Add Animation")) selected_obj->addComponent(ANIMATION);
					if (ImGui::Button("Add Animation Event")) selected_obj->addComponent(ANIMATION_EVENT);
					if (ImGui::Button("Add Audio Source")) select_audio = true;
					if (ImGui::Button("Add Listener")) selected_obj->addComponent(AUDIOLISTENER);
					if (ImGui::Button("Add Billboard")) selected_obj->addComponent(BILLBOARD);
					if (ImGui::Button("Add Particle Emitter")) selected_obj->addComponent(PARTICLE_EMITTER);
					if (ImGui::Button("Add Physic Object Properties")) selected_obj->addComponent(PHYSICS);
					if (ImGui::Button("Add Trigger")) selected_obj->addComponent(TRIGGER);
					if (ImGui::Button("Add Animator")) selected_obj->addComponent(ANIMATOR);
				}
			}

			std::list<Component*> components;
			selected_obj->getComponents(components);

			std::list<Component*> components_to_erase;
			int id = 0;
			for (std::list<Component*>::iterator it = components.begin(); it != components.end(); it++) {
				(*it)->DrawInspector(id);
				if (!App->gui->DrawComponent(*(*it), id))
					components_to_erase.push_back(*it);
				id++;
			}

			for (std::list<Component*>::iterator it = components_to_erase.begin(); it != components_to_erase.end(); it++)
				selected_obj->removeComponent(*it);

			if (select_script) {
				std::list<resource_deff> script_res;
				App->resources->getScriptResourceList(script_res);

				ImGui::Begin("Script selector", &select_script);
				for (auto it = script_res.begin(); it != script_res.end(); it++) {
					resource_deff script_deff = (*it);
					if (ImGui::MenuItem(script_deff.asset.c_str())) {
						ComponentScript* c_script = (ComponentScript*)selected_obj->addComponent(SCRIPT);
						c_script->assignScriptResource(script_deff.uuid);
						select_script = false;
						break;
					}
				}

				ImGui::End();
			}

			if (select_audio)
			{
				ImGui::Begin("Select Audio Event", &select_audio);
				if (ImGui::MenuItem("NONE"))
				{
					selected_obj->addComponent(AUDIOSOURCE);
					select_audio = false;
				}
				for (auto it = App->audio->events.begin(); it != App->audio->events.end(); it++) {

					if (ImGui::MenuItem((*it).c_str())) {
						ComponentAudioSource* c_source = (ComponentAudioSource*)selected_obj->addComponent(AUDIOSOURCE);
						c_source->SetSoundID(AK::SoundEngine::GetIDFromString((*it).c_str()));
						c_source->SetSoundName((*it).c_str());
						select_audio = false;
						break;
					}
				}
				ImGui::End();
			}
		}
	}

	//ImGui::PopFont();
	ImGui::End();
}

void PanelObjectInspector::DrawChildedInspector(GameObject* object)
{
	ImGui::BeginChild("Object Inspector##ChildedInsp", ImVec2(0,0), true);
	//ImGui::PushFont(ui_fonts[REGULAR]);

	static bool select_script = false;
	static bool select_audio = false;
	if (object != nullptr) {

			ImGui::Text("Name: %s", object->getName().c_str());

			if (object->is_UI) {//if is UI
				ImGui::SameLine(0.f, 10.0f);
				ImGui::TextColored(ImVec4(0.25f, 0.25f, 0.25f, 1), "UI GameObject##ChildedInsp");
			}

			ImGui::Checkbox("Active##ChildedInsp", &object->is_active);

			if (!object->is_UI) { // if it is not UI
				ImGui::SameLine();
				if (ImGui::Checkbox("Static##ChildedInsp", &object->is_static)) // If an object is set/unset static, reload the quadtree
					App->scene->quadtree_reload = true;
				DrawTagSelection(object);
				// Add a new tag
				static char new_tag[64];
				ImGui::InputText("New Tag##ChildedInsp", new_tag, 64);
				if (ImGui::Button("Add Tag##ChildedInsp")) {
					App->scripting->tags.push_back(new_tag);
					for (int i = 0; i < 64; i++)
						new_tag[i] = '\0';

				}
			}

			if (ImGui::CollapsingHeader("Add component##ChildedInsp"))
			{
				if (ImGui::Button("Add Script##ChildedInsp")) select_script = true;

				if (!object->is_UI) {
					if (ImGui::Button("Add Mesh##ChildedInsp"))	object->addComponent(MESH);
					if (ImGui::Button("Add Camera##ChildedInsp"))  object->addComponent(CAMERA);
					if (ImGui::Button("Add Animation##ChildedInsp")) object->addComponent(ANIMATION);
					if (ImGui::Button("Add Animation Event##ChildedInsp")) object->addComponent(ANIMATION_EVENT);
					if (ImGui::Button("Add Audio Source##ChildedInsp")) select_audio = true;
					if (ImGui::Button("Add Listener##ChildedInsp")) object->addComponent(AUDIOLISTENER);
					if (ImGui::Button("Add Billboard##ChildedInsp")) object->addComponent(BILLBOARD);
					if (ImGui::Button("Add Particle Emitter##ChildedInsp")) object->addComponent(PARTICLE_EMITTER);
					if (ImGui::Button("Add Physic Object Properties##ChildedInsp")) object->addComponent(PHYSICS);
					if (ImGui::Button("Add Animator##ChildedInsp")) object->addComponent(ANIMATOR);
				}
			}

			std::list<Component*> components;
			object->getComponents(components);

			std::list<Component*> components_to_erase;
			int id = 0;
			for (std::list<Component*>::iterator it = components.begin(); it != components.end(); it++) {
				if (!App->gui->DrawComponent(*(*it), id))
					components_to_erase.push_back(*it);
				id++;
			}

			for (std::list<Component*>::iterator it = components_to_erase.begin(); it != components_to_erase.end(); it++)
				object->removeComponent(*it);

			if (select_script) {
				std::list<resource_deff> script_res;
				App->resources->getScriptResourceList(script_res);

				ImGui::Begin("Script selector##ChildedInsp", &select_script);
				for (auto it = script_res.begin(); it != script_res.end(); it++) {
					resource_deff script_deff = (*it);
					if (ImGui::MenuItem(script_deff.asset.c_str())) {
						ComponentScript* c_script = (ComponentScript*)object->addComponent(SCRIPT);
						c_script->assignScriptResource(script_deff.uuid);
						select_script = false;
						break;
					}
				}

				ImGui::End();
			}

			if (select_audio)
			{
				ImGui::Begin("Select Audio Event##ChildedInsp", &select_audio);
				if (ImGui::MenuItem("NONE##ChildedInsp"))
				{
					object->addComponent(AUDIOSOURCE);
					select_audio = false;
				}
				for (auto it = App->audio->events.begin(); it != App->audio->events.end(); it++) {

					if (ImGui::MenuItem((*it).c_str())) {
						ComponentAudioSource* c_source = (ComponentAudioSource*)object->addComponent(AUDIOSOURCE);
						c_source->SetSoundID(AK::SoundEngine::GetIDFromString((*it).c_str()));
						c_source->SetSoundName((*it).c_str());
						select_audio = false;
						break;
					}
				}
				ImGui::End();
			}
		}

	//ImGui::PopFont();
	ImGui::EndChild();
}

void PanelObjectInspector::DrawTagSelection(GameObject* object) {

	std::string object_tag = object->tag; // Current tag
	int inx = 0;						  // Index of the current tag


	std::string posible_tags; // All the tags in the same string
	bool inx_found = false; // Stop when tag is found

	for (auto it = App->scripting->tags.begin(); it != App->scripting->tags.end(); it++) {
		// Store every tag in the same string
		posible_tags += (*it);
		posible_tags += '\0';

		// Figure out which inx is the tag of the gameobject
		if (object_tag == (*it))
			inx_found = true;
		if (!inx_found) {
			inx++;
		}
	}
	if (ImGui::Combo("Tag selector", &inx, posible_tags.c_str())) {
		// Out of the selected index, extract the "tag" of the gameobject and return it
		int inx_it = 0;
		for (auto it = App->scripting->tags.begin(); it != App->scripting->tags.end(); it++) {
			if (inx_it == inx) {
				object->tag = (*it);
				break;
			}
			inx_it++;
		}
	}

}