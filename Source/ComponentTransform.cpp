#include "ComponentTransform.h"
#include "GameObject.h"
#include "Transform.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleDebug.h"

#include "ImGui/imgui.h"

#include "glew-2.1.0\include\GL\glew.h"

ComponentTransform::ComponentTransform(GameObject* parent, const Quat& rot, const float3& pos, const float3& scl) : Component(parent, TRANSFORM)
{
	local = new Transform();
	global = new Transform();

	local->setPosition(pos);
	local->setRotation(rot);
	local->setScale(scl);
	local->CalculateMatrix();

	LocalToGlobal();
}

ComponentTransform::ComponentTransform(GameObject* parent, const float3& euler_axis, const float3& pos, const float3& scl) : Component(parent, TRANSFORM)
{
	local = new Transform();
	global = new Transform();

	local->setPosition(pos);
	local->setRotation(Quat::FromEulerXYZ(euler_axis.x, euler_axis.y, euler_axis.z));
	local->setScale(scl);
	local->CalculateMatrix();

	LocalToGlobal();
}

ComponentTransform::ComponentTransform(GameObject* parent, const ComponentTransform& transform) : Component(parent, TRANSFORM)
{
	local = new Transform(*transform.local);
	global = new Transform(*transform.global);
}

ComponentTransform::ComponentTransform(JSON_Object* deff, GameObject* parent) : Component(parent, TRANSFORM) {
	local = new Transform(json_object_get_object(deff, "local"));
	global = new Transform();
	LocalToGlobal();
}

ComponentTransform::ComponentTransform(GameObject* parent) : Component(parent, TRANSFORM)
{
	local = new Transform();
	global = new Transform();
}

ComponentTransform::~ComponentTransform()
{
	if (local) delete local;
	if (global) delete global;
}

bool ComponentTransform::Update(float dt)
{
	if (mode == GLOBAL)
	{
		global->CalculateMatrix();
		GlobalToLocal();
	}
	else
	{
		local->CalculateMatrix();
		LocalToGlobal();
	}


	return true;
}

Transform* ComponentTransform::getInheritedTransform()
{
	if (GameObject* parent_obj = getParent()->getParent())
		return ((ComponentTransform*)parent_obj->getComponent(TRANSFORM))->global;
	else 
		return nullptr;
}

void ComponentTransform::GlobalToLocal()
{
	if (Transform* inh_transform = getInheritedTransform())
	{
		local->mat = inh_transform->getMatrix().Inverted() * global->getMatrix();
		local->mat.Decompose(local->position, local->rotation, local->scale);
	}
	else
		local->Set(global->getPosition(), global->getRotation(), global->getScale());

	local->CalculateMatrix();
}

void ComponentTransform::LocalToGlobal()
{
	if (Transform* inh_transform = getInheritedTransform())
	{
		global->mat = inh_transform->getMatrix()* local->getMatrix();
		global->mat.Decompose(global->position, global->rotation, global->scale);
	}
	else
		global->Set(local->getPosition(), local->getRotation(), local->getScale());

	global->CalculateMatrix();
}

void ComponentTransform::Draw() const
{
	if (draw_axis)
		App->debug->directDrawAxis(global->getPosition(), global->getRotation());
}

bool ComponentTransform::DrawInspector(int id)
{
	if (ImGui::CollapsingHeader("Transform"))
	{
		static bool want_autosave = false;
		ImGui::TextWrapped("Drag the parameters to change them, or ctrl+click on one of them to set it's value");
		/*ComponentTransform* c_trans = (ComponentTransform*)&component;*/

		static float3 position;
		static float3 rotation;
		static float3 scale;

		Transform* transform = nullptr;

		if (this->getMode() == LOCAL)
		{
			transform = this->local;
			ImGui::Text("Current mode: Local");
			ImGui::SameLine();
			if (ImGui::Button("Global"))
				this->setMode(GLOBAL);
		}
		else
		{
			transform = this->global;
			ImGui::Text("Current mode: Global");
			ImGui::SameLine();
			if (ImGui::Button("Local"))
				this->setMode(LOCAL);
		}

		position = transform->getPosition();
		rotation = transform->getRotationEuler();
		scale = transform->getScale();

		//position
		ImGui::Text("Position:");
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		if (!this->constraints[0][0]) {
			static bool changing_value = false;
			if (ImGui::DragFloat("##p x", &position.x, 0.01f, 0.0f, 0.0f, "%.02f"))
				changing_value = true;
			else if (changing_value && !ImGui::IsItemActive())
			{
				want_autosave = true;
				changing_value = false;
			}
		}
		else								ImGui::Text("%.2f", position.x);

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		if (!this->constraints[0][1]) {
			static bool changing_value = false;
			if (ImGui::DragFloat("##p y", &position.y, 0.01f, 0.0f, 0.0f, "%.02f"))
				changing_value = true;
			else if (changing_value && !ImGui::IsItemActive())
			{
				want_autosave = true;
				changing_value = false;
			}
		}
		else								ImGui::Text("%.2f", position.y);

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		if (!this->constraints[0][2]) {
			static bool changing_value = false;
			if (ImGui::DragFloat("##p z", &position.z, 0.01f, 0.0f, 0.0f, "%.02f"))
				changing_value = true;
			else if (changing_value && !ImGui::IsItemActive())
			{
				want_autosave = true;
				changing_value = false;
			}
		}
		else								ImGui::Text("%.2f", position.z);

		//rotation
		ImGui::Text("Rotation:");
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		if (!this->constraints[1][0]) {
			static bool changing_value = false;
			if (ImGui::DragFloat("##r x", &rotation.x, 0.2f, -180.0f, 180.0f, "%.02f"))
				changing_value = true;
			else if (changing_value && !ImGui::IsItemActive())
			{
				want_autosave = true;
				changing_value = false;
			}
		}
		else								ImGui::Text("%.2f", rotation.z);

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		if (!this->constraints[1][1]) {
			static bool changing_value = false;
			if (ImGui::DragFloat("##r y", &rotation.y, 0.2f, -180.0f, 180.0f, "%.02f"))
				changing_value = true;
			else if (changing_value && !ImGui::IsItemActive())
			{
				want_autosave = true;
				changing_value = false;
			}
		}
		else								ImGui::Text("%.2f", rotation.x);

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		if (!this->constraints[1][2]) {
			static bool changing_value = false;
			if (ImGui::DragFloat("##r z", &rotation.z, 0.2f, -180.0f, 180.0f, "%.02f"))
				changing_value = true;
			else if (changing_value && !ImGui::IsItemActive())
			{
				want_autosave = true;
				changing_value = false;
			}
		}
		else								ImGui::Text("%.2f", rotation.y);

		//scale
		ImGui::Text("   Scale:");
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		if (!this->constraints[2][0]) {
			static bool changing_value = false;
			if (ImGui::DragFloat("##s x", &scale.x, 0.01f, -1000.0f, 1000.0f, "%.02f"))
				changing_value = true;
			else if (changing_value && !ImGui::IsItemActive())
			{
				want_autosave = true;
				changing_value = false;
			}
		}
		else								ImGui::Text("%.2f", scale.x);

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		if (!this->constraints[2][1]) {
			static bool changing_value = false;
			if (ImGui::DragFloat("##s y", &scale.y, 0.01f, -1000.0f, 1000.0f, "%.02f"))
				changing_value = true;
			else if (changing_value && !ImGui::IsItemActive())
			{
				want_autosave = true;
				changing_value = false;
			}
		}
		else								ImGui::Text("%.2f", scale.y);

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
		if (!this->constraints[2][2]) {
			static bool changing_value = false;
			if (ImGui::DragFloat("##s z", &scale.z, 0.01f, -1000.0f, 1000.0f, "%.02f"))
				changing_value = true;
			else if (changing_value && !ImGui::IsItemActive())
			{
				want_autosave = true;
				changing_value = false;
			}
		}
		else								ImGui::Text("%.2f", scale.z);

		if (ImGui::Button("Reset Transform"))
		{
			position = float3::zero; rotation = float3::zero, scale = float3::one;
			want_autosave = true;
		}

		ImGui::Checkbox("Draw axis", &this->draw_axis);

		if (ImGui::CollapsingHeader("	Constraints"))
		{
			ImGui::Text("Position:");
			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
			ImGui::Checkbox("##constraint p x", &this->constraints[0][0]);

			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
			ImGui::Checkbox("##constraint p y", &this->constraints[0][1]);

			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
			ImGui::Checkbox("##constraint p z", &this->constraints[0][2]);


			ImGui::Text("Rotation:");
			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
			ImGui::Checkbox("##constraint r x", &this->constraints[1][0]);

			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
			ImGui::Checkbox("##constraint r y", &this->constraints[1][1]);

			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
			ImGui::Checkbox("##constraint r z", &this->constraints[1][2]);


			ImGui::Text("   Scale:");
			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
			ImGui::Checkbox("##constraint s x", &this->constraints[2][0]);

			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
			ImGui::Checkbox("##constraint s y", &this->constraints[2][1]);

			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
			ImGui::Checkbox("##constraint s z", &this->constraints[2][2]);
		}

		transform->setPosition(position);
		transform->setRotationEuler(rotation);
		transform->setScale(scale);

		if (want_autosave)
		{
			App->scene->AskAutoSaveScene();
			want_autosave = false;
		}
	}

	return true;
}

void ComponentTransform::Save(JSON_Object* config) {
	// Component has an object called transform, which has the two transforms as attributes
	JSON_Value* local_trans = json_value_init_object();
	JSON_Value* global_trans = json_value_init_object();
	// Set component type
	json_object_set_string(config, "type", "transform");

	local->Save(json_object(local_trans));
	global->Save(json_object(global_trans));

	json_object_set_value(config, "local", local_trans);
	json_object_set_value(config, "global", global_trans);
}

