#include "ComponentCamera.h"
#include "Application.h"
#include "Camera.h"
#include "ComponentTransform.h"
#include "Transform.h"
#include "GameObject.h"
#include "ModuleScene.h"
#include "ModuleCamera3D.h"

#include "ImGui/imgui.h"

#include "MathGeoLib\Math\Quat.h"

ComponentCamera::ComponentCamera(GameObject* parent, Camera* camera) : Component(parent, CAMERA), camera(camera)
{
	transform = (ComponentTransform*)parent->getComponent(TRANSFORM);
	camera->attached_to = this;
}

ComponentCamera::ComponentCamera(JSON_Object* deff, GameObject* parent) : Component(parent, CAMERA){


	is_active = json_object_get_boolean(deff, "active");

	JSON_Array* offset_array = json_object_get_array(deff, "offset");
	offset.x = json_array_get_number(offset_array, 0);
	offset.y = json_array_get_number(offset_array, 1);
	offset.z = json_array_get_number(offset_array, 2);

	// Load camera
	float3 pos(json_object_dotget_number(deff, "camera.pos_x"), json_object_dotget_number(deff, "camera.pos_y"), json_object_dotget_number(deff, "camera.pos_z"));
	float3 reference(json_object_dotget_number(deff, "camera.ref_x"), json_object_dotget_number(deff, "camera.ref_y"), json_object_dotget_number(deff, "camera.ref_z"));
	math::FrustumType type = (math::FrustumType)(uint)json_object_dotget_number(deff, "camera.frustum_type");
	camera = new Camera(pos, reference, type);

	// Load bools
	camera->active = json_object_dotget_boolean(deff, "camera.active");
	camera->draw_depth = json_object_dotget_boolean(deff, "camera.draw_depth");
	camera->draw_frustum = json_object_dotget_boolean(deff, "camera.draw_frustum");
	camera->draw_in_UI = json_object_dotget_boolean(deff, "camera.draw_in_UI");

	transform = (ComponentTransform*)parent->getComponent(TRANSFORM);
	camera->attached_to = this;

	if (json_object_get_boolean(deff, "game_camera"))
	{
		App->camera->game_camera = camera;
		if (App->is_game)
			is_active = true;
	}
}

ComponentCamera::~ComponentCamera()
{
	if (camera == App->camera->override_editor_cam_culling) App->camera->override_editor_cam_culling = nullptr;
	if (camera == App->camera->game_camera)					App->camera->game_camera = App->camera->editor_camera;
	App->camera->game_cameras.remove(camera);
	delete camera;
}

bool ComponentCamera::Update(float dt)
{
	float3 displacement = camera->getFrustum()->pos;
	camera->getFrustum()->pos = transform->local->getPosition() + offset;
	displacement = camera->getFrustum()->pos - displacement;
	camera->Reference += displacement;
	camera->Reference = camera->getFrustum()->pos + (transform->local->Forward().Normalized() * (camera->Reference - camera->getFrustum()->pos).Length());
	camera->X = transform->local->Right();
	camera->Y = transform->local->Up();
	camera->Z = transform->local->Forward();
	camera->updateFrustum();
	
	getCamera()->active = getCamera()->draw_in_UI;

	return true;
}

bool ComponentCamera::DrawInspector(int id)
{
	if (ImGui::CollapsingHeader("Camera"))
	{
		static bool camera_active;
		camera_active = isActive();

		if (ImGui::Checkbox("Active##active camera", &camera_active))
		{
			setActive(camera_active);
			App->scene->AskAutoSaveScene();
		}
		if (ImGui::Checkbox("Draw camera view", &getCamera()->draw_in_UI))
			App->scene->AskAutoSaveScene();

		if (ImGui::Checkbox("Draw frustum", &getCamera()->draw_frustum))
			App->scene->AskAutoSaveScene();

		if (ImGui::Checkbox("Draw depth", &getCamera()->draw_depth))
			App->scene->AskAutoSaveScene();

		if (ImGui::Checkbox("Interpolated movement", &getCamera()->interpolating))
			App->scene->AskAutoSaveScene();

		if (getCamera()->interpolating)
		{
			if(ImGui::InputFloat("interpolation speed", &getCamera()->interpolating_speed))
				App->scene->AskAutoSaveScene();
		}

		static bool overriding;
		overriding = (getCamera() == App->camera->override_editor_cam_culling);
		if (ImGui::Checkbox("Override Frustum Culling", &overriding))
		{
			if (!overriding)	App->camera->override_editor_cam_culling = nullptr;
			else				App->camera->override_editor_cam_culling = getCamera();
			App->scene->AskAutoSaveScene();
		}

		if (camera_active)
		{
			static bool changing_value_x = false;
			static bool changing_value_y = false;
			static bool changing_value_z = false;
			static float3 _offset;
			_offset = offset;
			ImGui::Text("Offset:");
			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
			if (ImGui::DragFloat("##o x", &_offset.x, 0.01f, -1000.0f, 1000.0f, "%.02f"))
				changing_value_x = true;
			else if (changing_value_x && !ImGui::IsItemActive())
			{
				App->scene->AskAutoSaveScene();
				changing_value_x = false;
			}

			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
			if (ImGui::DragFloat("##o y", &_offset.y, 0.01f, -1000.0f, 1000.0f, "%.02f"))
				changing_value_y = true;
			else if (changing_value_y && !ImGui::IsItemActive())
			{
				App->scene->AskAutoSaveScene();
				changing_value_y = false;
			}

			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
			if (ImGui::DragFloat("##o z", &_offset.z, 0.01f, -1000.0f, 1000.0f, "%.02f"))
				changing_value_z = true;
			else if (changing_value_z && !ImGui::IsItemActive())
			{
				App->scene->AskAutoSaveScene();
				changing_value_z = false;
			}

			offset = _offset;
		}

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.f, 0.f, 0.f, 1.f)); ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.f, 0.2f, 0.f, 1.f));
		if (ImGui::Button("Remove##Remove camera")) {
			App->scene->AskAutoSaveScene();
			ImGui::PopStyleColor(); ImGui::PopStyleColor();
			return false;
		}
		ImGui::PopStyleColor(); ImGui::PopStyleColor();
	}

	return true;
}

void ComponentCamera::Save(JSON_Object* config) {

	json_object_set_string(config, "type", "camera");
	json_object_set_boolean(config, "active", is_active);
	JSON_Value* array_offset = json_value_init_array();
	json_array_append_number(json_array(array_offset), offset.x);
	json_array_append_number(json_array(array_offset), offset.y);
	json_array_append_number(json_array(array_offset), offset.z);
	json_object_set_value(config, "offset", array_offset);

	JSON_Value* camera_value = json_value_init_object();
	camera->Save(json_object(camera_value));
	json_object_set_value(config, "camera", camera_value);
	json_object_set_boolean(config, "game_camera", App->camera->game_camera == camera);
}