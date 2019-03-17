#include "ComponentCamera.h"
#include "Application.h"
#include "Camera.h"
#include "ComponentTransform.h"
#include "Transform.h"
#include "GameObject.h"
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
			setActive(camera_active);

		ImGui::Checkbox("Draw camera view", &getCamera()->draw_in_UI);

		ImGui::Checkbox("Draw frustum", &getCamera()->draw_frustum);

		ImGui::Checkbox("Draw depth", &getCamera()->draw_depth);

		static bool overriding;
		overriding = (getCamera() == App->camera->override_editor_cam_culling);
		if (ImGui::Checkbox("Override Frustum Culling", &overriding))
		{
			if (!overriding)	App->camera->override_editor_cam_culling = nullptr;
			else				App->camera->override_editor_cam_culling = getCamera();
		}

		if (camera_active)
		{
			static float3 _offset;
			_offset = offset;
			ImGui::Text("Offset:");
			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
			ImGui::DragFloat("##o x", &_offset.x, 0.01f, -1000.0f, 1000.0f, "%.02f");

			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
			ImGui::DragFloat("##o y", &_offset.y, 0.01f, -1000.0f, 1000.0f, "%.02f");

			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
			ImGui::DragFloat("##o z", &_offset.z, 0.01f, -1000.0f, 1000.0f, "%.02f");

			offset = _offset;
		}

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.f, 0.f, 0.f, 1.f)); ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.f, 0.2f, 0.f, 1.f));
		if (ImGui::Button("Remove##Remove camera")) {
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

	JSON_Value* camera_value = json_value_init_object();
	camera->Save(json_object(camera_value));
	json_object_set_value(config, "camera", camera_value);
	json_object_set_boolean(config, "game_camera", App->camera->game_camera == camera);
}