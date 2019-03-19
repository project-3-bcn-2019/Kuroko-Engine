#include "ComponentParticleEmitter.h"
#include "Application.h"
#include "ComponentTransform.h"
#include "ComponentMesh.h"
#include "Transform.h"
#include "GameObject.h"
#include "MathGeoLib/MathGeoLib.h"
#include "PCG Random\pcg_basic.h"
#include "ModuleUI.h"
#include "Material.h"
#include "ModuleResourcesManager.h"
#include "ResourceTexture.h"


std::string openFileWID(bool isfile = false);

ComponentParticleEmitter::ComponentParticleEmitter(GameObject* parent) : Component(parent, PARTICLE_EMITTER)
{
	emitterLifetime = -1;

	//Default Values
	speed.max = 5;
	speed.min = 0;

	particleLifetime.max = 5;
	particleLifetime.min = 1;

	startSize.max = 10;
	startSize.min = 9;

	endSize.max = 5;
	endSize.min = 4;

	startSpin.max = 2;
	startSpin.min = 1;

	endSpin.max = 0.5;
	endSpin.min = 0;

	direction = { 0,1,0 };
	gravity = { 1, 0, 0 };
	dirVartiation = 180;

	startColor.max = { 1.0f, 0.0f, 0.0f, 1.0f };
	endColor.max = { 0.0f, 0.0f, 1.0f, 1.0f };

	period = 0.1f;

	maxParticles = 100;

	area.sphere.r = 1;

	area.aabb.minPoint.Set(-0.5, -0.5, -0.5);
	area.aabb.maxPoint.Set(0.5, 0.5, 0.5);

	transform = ((ComponentTransform*)parent->getComponent(TRANSFORM));
	mesh = new Mesh(Primitive_Plane);
	billboard = new Billboard(this, mesh);
}


ComponentParticleEmitter::ComponentParticleEmitter(GameObject* parent, const ComponentParticleEmitter& emitter) : Component(parent, PARTICLE_EMITTER)
{
	setActive(emitter.is_active);
	emisionTime = emitter.emisionTime;
	period = emitter.period;
	maxParticles = emitter.maxParticles;
		   
	speed = emitter.speed;
	emitterLifetime = emitter.emitterLifetime;
	startSize = emitter.startSize;
	endSize = emitter.endSize;
		   
	startSpin = emitter.startSpin;
	endSpin = emitter.endSpin;
	startColor = emitter.startColor;
	endColor = emitter.endColor;
		   
	gravity = emitter.gravity;
	direction = emitter.direction;
	dirVartiation = emitter.dirVartiation;
		   
	area = emitter.area;

	transform = ((ComponentTransform*)parent->getComponent(TRANSFORM));
	mesh = new Mesh(Primitive_Plane);
	billboard = new Billboard(*emitter.billboard);
}


ComponentParticleEmitter::ComponentParticleEmitter(JSON_Object* deff, GameObject* parent) : Component(parent, PARTICLE_EMITTER)
{

	transform = ((ComponentTransform*)parent->getComponent(TRANSFORM));

	//Emitter Info
	emisionTime = json_object_dotget_number(deff, "Emmision Time");
	period = json_object_dotget_number(deff, "Period");
	maxParticles = json_object_dotget_number(deff, "Max Particles");
	emitterLifetime = json_object_dotget_number(deff, "EmitterLifetime");
	script_controlled = json_object_get_boolean(deff, "Script Controlled");
	
	//Particle Info
	speed = LoadRange(deff, "Speed");
	particleLifetime = LoadRange(deff, "ParticleLifetime");
	startSize = LoadRange(deff, "StartSize");
	endSize = LoadRange(deff, "EndSize");
	
	startSpin = LoadRange(deff, "StartSpin");
	endSpin = LoadRange(deff, "EndSpin");
	
	startColor.min = LoadColor(deff, "StartColorMin");
	startColor.max = LoadColor(deff, "StartColorMax");
	
	endColor.min = LoadColor(deff, "EndColorMin");
	endColor.max = LoadColor(deff, "EndColorMax");
	
	gravity = LoadFloat3(deff, "Gravity");
	direction = LoadFloat3(deff, "Direction");
	
	dirVartiation = json_object_dotget_number(deff, "DirVariation");
	
	
	//Area of Spawn
	uint type = json_object_dotget_number(deff, "AreaType");
	area.type = (AreaType)type;
	
	float3 size = LoadFloat3(deff, "AABBSize");
	area.aabb.SetFromCenterAndSize(transform->global->getPosition(), size);
	
	area.sphere.r = json_object_dotget_number(deff, "SphereSize");

	mesh = new Mesh(Primitive_Plane);
	billboard = new Billboard(this, mesh, deff);
	
}


ComponentParticleEmitter::~ComponentParticleEmitter()
{
	for (std::list<Particle*>::iterator item = particles.begin(); item != particles.end();)
	{
		if (*item != nullptr)
		{
			delete *item;
			*item = nullptr;
		}

		item = particles.erase(item);

	}
	if(mesh) delete mesh;
	if (billboard) delete billboard;
}


bool ComponentParticleEmitter::Update(float dt)
{

	if (time <= emitterLifetime || emitterLifetime < 0)
	{
		if (currentParticles <= maxParticles || maxParticles == 0)
			SpawnParticles(dt);

		if (emitterLifetime > 0)
			time += dt;
	}

	UpdateParticles(dt);
	DrawSpawnArea();
	UpdateSpawnAreaPos();

	return true;
}

void ComponentParticleEmitter::Save(JSON_Object * json)
{
	json_object_set_string(json, "type", "particle_emitter");
	json_object_dotset_number(json, "Type", type);
	json_object_dotset_boolean(json, "Active", isActive());

	//Emitter Info
	json_object_dotset_number(json, "Emmision Time", emisionTime);
	json_object_dotset_number(json, "Period", period);
	json_object_dotset_number(json, "Max Particles", maxParticles);
	json_object_dotset_number(json, "EmitterLifetime", emitterLifetime);
	json_object_set_boolean(json, "Script Controlled", script_controlled);

	//Particle Info
	SaveRange(json, "Speed", speed);
	SaveRange(json, "ParticleLifetime", particleLifetime);
	SaveRange(json, "StartSize", startSize);
	SaveRange(json, "EndSize", endSize);

	SaveRange(json, "StartSpin", startSpin);
	SaveRange(json, "EndSpin", endSpin);

	SaveColor(json, "StartColorMin", startColor.min);
	SaveColor(json, "StartColorMax", startColor.max);

	SaveColor(json, "EndColorMin", endColor.min);
	SaveColor(json, "EndColorMax", endColor.max);

	SaveFloat3(json, "Gravity", gravity);
	SaveFloat3(json, "Direction", direction);

	json_object_dotset_number(json, "DirVariation", dirVartiation);

	//Area of Spawn
	json_object_dotset_number(json, "AreaType", area.type);

	SaveFloat3(json, "AABBSize", area.aabb.Size());
	json_object_dotset_number(json, "SphereSize", area.sphere.r);

	billboard->Save(json);

}

void ComponentParticleEmitter::CreateParticle()
{

	//Pick a random direction from the base direction with a the angle vartiation
	float3 randomInSphere = float3::RandomSphere(lcg, { 0,0,0 }, 1);
	float3 vartiation = randomInSphere.Normalized();
	vartiation.x = vartiation.x * dirVartiation * DEGTORAD;
	vartiation.y = vartiation.y * dirVartiation * DEGTORAD;
	vartiation.z = vartiation.z * dirVartiation * DEGTORAD;

	float3 dir = float3::zero;
	if (transform_mode == GLOBAL)
		dir = direction + vartiation;
	else
		dir = (transform->global->getRotation() * direction) + vartiation;

	//Create New Particle
	Particle* newParticle = new Particle();
	newParticle->info.Set(GetRandom(startSize), GetRandom(endSize), GetRandom(startSpin), GetRandom(endSpin), GetRandom(speed), GetRandom(particleLifetime), GetRandomPosition(), dir, gravity, GetRandom(startColor), GetRandom(endColor));
	newParticle->Reset();
	particles.push_back(newParticle);
	currentParticles++;


}

void ComponentParticleEmitter::SpawnParticles(float dt)
{
	if (script_controlled)
		return;
	uint particlesToSpawn = 1;

	if (period < dt)
	{
		particlesToSpawn = dt / period;
	}

	if (emisionTimer.Read() * dt >= period)
	{
		for (uint i = 0; i < particlesToSpawn; i++)
		{
			CreateParticle();
		}

		emisionTimer.Start();
	}
}

void ComponentParticleEmitter::UpdateParticles(float dt)
{

	for (std::list<Particle*>::iterator item = particles.begin(); item != particles.end();)
	{
		if (!(*item)->Delete())
		{
			(*item)->Update(dt);
			item++;
		}
		else
		{
			currentParticles--;
			if (*item != nullptr)
			{
				delete *item;
				*item = nullptr;
			}

			item = particles.erase(item);
		}
	}

}

void ComponentParticleEmitter::Draw() const
{
	std::priority_queue<Particle*, std::vector<Particle*>, ParticlePriority> orderedParticles;

	for (std::list<Particle*>::const_iterator item = particles.cbegin(); item != particles.cend(); item++)
		orderedParticles.push(*item);

	while (orderedParticles.size() != 0)
	{
		Particle* first = orderedParticles.top();

		float distance = first->DistanceToCamera();

		billboard->UpdateFromParticle(*first);
		billboard->Draw();

		orderedParticles.pop();
	}

}

void ComponentParticleEmitter::DrawInspector(int id)
{
	
	if (ImGui::CollapsingHeader("Particle emitter"))
	{
		//Emitter Lifetime
		ImGui::Text("Set to -1 for infinite lifetime");
		if (ImGui::SliderFloat("LifeTime", &emitterLifetime, -1, 100))
			time = 0;

		if (ImGui::Button("Reset"))
			time = 0;

		ImGui::Text("LifeTime: %.2f", emitterLifetime - time);

		ImGui::NewLine();
		int particles = maxParticles;

		ImGui::Text("Set to 0 for uncapped particles");
		if (ImGui::SliderInt("Max particles", &particles, 0, 1000))
			maxParticles = particles;


		ImGui::SliderFloat("Period", &period, MINSPAWNRATE, 10);
		ImGui::Checkbox("Script Controlled", &script_controlled);


		//Area of spawn

		if (ImGui::CollapsingHeader("Spawn Area"))
		{

			if (ImGui::Selectable("AABB", area.type == AAB))
			{
				area.type = AAB;
			}
			if (ImGui::Selectable("Sphere", area.type == SPHERE))
			{
				area.type = SPHERE;
			}
			if (ImGui::Selectable("Point", area.type == AREA_NONE))
			{
				area.type = AREA_NONE;
			}


			switch (area.type)
			{
			case SPHERE:

				ImGui::DragFloat("Radius", &area.sphere.r, 0.1f);
				break;
			case AAB:
			{
				float3 size = area.aabb.Size();
				if (ImGui::DragFloat3("Size", (float*)&size, 0.1f))
				{
					area.aabb.SetFromCenterAndSize(area.aabb.CenterPoint(), size);
				}
			}
			break;
			case AREA_NONE:
			default:
				break;
			}

			ImGui::Separator();
		}

		if (ImGui::CollapsingHeader("Particle"))
		{

			//Direction
			ImGui::DragFloat3("Direction", (float*)&direction, 0.1f);
			ImGui::SliderFloat("Direction Variation", &dirVartiation, 0, 180);
			ImGui::DragFloat3("Gravity", (float*)&gravity, 0.1f);

			static bool local_particles = false;
			local_particles = transform_mode == LOCAL;

			if (ImGui::Checkbox("Local", &local_particles))
				transform_mode = local_particles ? LOCAL : GLOBAL;

			//LifeTime
			float minlife = particleLifetime.min;
			float maxlife = particleLifetime.max;

			ImGui::PushID("LT");

			ImGui::Text("Particle Life Time");
			ImGui::SliderFloat("Min", &particleLifetime.min, 0, particleLifetime.max);
			ImGui::SliderFloat("Max", &particleLifetime.max, particleLifetime.min, 100);


			ImGui::PopID();

			//Speed
			ImGui::PushID("Speed Variation");

			ImGui::Text("Speed");
			ImGui::SliderFloat("Min", &speed.min, 0, speed.max);
			ImGui::SliderFloat("Max", &speed.max, speed.min, 100);

			ImGui::PopID();

			//Start Size
			ImGui::PushID("SSize");

			ImGui::Text("Start Size");
			ImGui::SliderFloat("Min", &startSize.min, 0, startSize.max);
			ImGui::SliderFloat("Max", &startSize.max, startSize.min, 100);

			ImGui::PopID();

			//End Size
			ImGui::PushID("ESize");

			ImGui::Text("End Size");
			ImGui::SliderFloat("Min", &endSize.min, 0, endSize.max);
			ImGui::SliderFloat("Max", &endSize.max, endSize.min, 100);

			ImGui::PopID();

			////Start Spin
			//ImGui::PushID("SSpin");

			//ImGui::Text("Start Spin");
			//ImGui::SliderFloat("Min", &startSpin.min, 0, startSpin.max);
			//ImGui::SliderFloat("Max", &startSpin.max, startSpin.min, 100);

			//ImGui::PopID();

			////End Spin
			//ImGui::PushID("ESpin");

			//ImGui::Text("End Spin");
			//ImGui::SliderFloat("Min", &endSpin.min, 0, endSpin.max);
			//ImGui::SliderFloat("Max", &endSpin.max, endSpin.min, 100);

			//ImGui::PopID();

			//Start Color
			ImGui::PushID("SColor");

			ImGui::Text("StartColor");

			ImGui::ColorEdit4("Min", (float*)&startColor.min);
			ImGui::ColorEdit4("Max", (float*)&startColor.max);

			ImGui::PopID();

			//End Color
			ImGui::PushID("EColor");

			ImGui::Text("EndColor");

			ImGui::ColorEdit4("Min", (float*)&endColor.min);
			ImGui::ColorEdit4("Max", (float*)&endColor.max);

			ImGui::PopID();
		}



		if (ImGui::CollapsingHeader("Billboard"))
		{
			if (Material* material = billboard->getMaterial())
			{
				static int preview_size = 128;
				ImGui::Text("Id: %d", material->getId());
				ImGui::SameLine();
				if (ImGui::Button("remove material"))
				{
					delete billboard->getMaterial();
					billboard->setMaterial(nullptr);
					ImGui::TreePop();
				}

				ImGui::Text("Preview size");
				ImGui::SameLine();
				if (ImGui::Button("64")) preview_size = 64;
				ImGui::SameLine();
				if (ImGui::Button("128")) preview_size = 128;
				ImGui::SameLine();
				if (ImGui::Button("256")) preview_size = 256;

				Texture* texture = nullptr;
				if (ResourceTexture* tex_res = (ResourceTexture*)App->resources->getResource(material->getTextureResource(DIFFUSE)))
					texture = tex_res->texture;


				ImGui::Image(texture ? (void*)texture->getGLid() : (void*)App->gui->ui_textures[NO_TEXTURE]->getGLid(), ImVec2(preview_size, preview_size));
				ImGui::SameLine();

				int w = 0; int h = 0;
				if (texture)
					texture->getSize(w, h);

				ImGui::Text("texture data: \n x: %d\n y: %d", w, h);

				//if (ImGui::Button("Load checkered##Dif: Load checkered"))
				//	material->setCheckeredTexture(DIFFUSE);
				//ImGui::SameLine()
			}
			else
				ImGui::TextWrapped("No material assigned");

			if (ImGui::Button("Load material(from asset folder)##Billboard: Load"))
			{
				std::string texture_path = openFileWID();
				uint new_resource = App->resources->getResourceUuid(texture_path.c_str());
				if (new_resource != 0) {
					App->resources->assignResource(new_resource);

					if (Material* material = billboard->getMaterial())
						App->resources->deasignResource(material->getTextureResource(DIFFUSE));
					else
						billboard->setMaterial(new Material());

					billboard->getMaterial()->setTextureResource(DIFFUSE, new_resource);
				}
			}

		}
		//if (ImGui::Button("Remove##Remove particle emitter"))
		//	return false;
	}
}

void ComponentParticleEmitter::Set(float minSpeed, float maxSpeed, float minLife, float maxLife, float minSSize, float maxSSize, float minESize, float maxESize, float minSSpin, float maxSSpin, float minESpin, float maxESpin, Color sColorMin, Color sColorMax, Color eColorMin, Color eColorMax, float variation, float3 direction, float3 gravity)
{
	speed.min = minSpeed;
	speed.max = maxSpeed;
	particleLifetime.min = minLife;
	particleLifetime.max = maxLife;
	startSize.min = minSSize;
	startSize.max = maxSSize;
	endSize.min = minESize;
	endSize.max = maxESize;
	startSpin.min = minSSpin;
	startSpin.max = maxSSpin;
	endSpin.min = minESpin;
	endSpin.max = maxESpin;

	startColor.min = sColorMin;
	startColor.max = sColorMax;
	endColor.min = eColorMin;
	endColor.max = eColorMax;

	gravity;
	direction;
	dirVartiation = dirVartiation;
}

float ComponentParticleEmitter::GetRandom(range<float> r)
{
	return (ldexp(pcg32_random(), -32) * (r.max - r.min)) + r.min;
}

uint ComponentParticleEmitter::GetRandom(range<uint> r)
{
	return (ldexp(pcg32_random(), -32) * (r.max - r.min)) + r.min;
}

Color ComponentParticleEmitter::GetRandom(range<Color> r)
{
	Color c;
	c.r = (ldexp(pcg32_random(), -32) * (r.max.r - r.min.r)) + r.min.r;
	c.g = (ldexp(pcg32_random(), -32) * (r.max.g - r.min.g)) + r.min.g;
	c.b = (ldexp(pcg32_random(), -32) * (r.max.b - r.min.b)) + r.min.b;
	c.a = (ldexp(pcg32_random(), -32) * (r.max.a - r.min.a)) + r.min.a;

	return c;
}

float3 ComponentParticleEmitter::GetRandomPosition()
{
	float3 ret = float3::zero;

	switch (area.type)
	{
	case SPHERE:
		ret = area.sphere.RandomPointInside(lcg);
		break;
	case AAB:
		ret = area.aabb.RandomPointInside(lcg);
		break;
	case AREA_NONE:
		ret = transform->global->getPosition();
		break;
	default:
		break;
	}

	return ret;
}

void ComponentParticleEmitter::DrawSpawnArea()
{/*
	switch (area.type)
	{
	case SPHERE:
		App->renderer3D->DrawSphere(area.sphere);
		break;
	case AAB:
		App->renderer3D->DrawAABB(area.aabb);
		break;
	case AREA_NONE:
	default:
		break;
	}*/
}

void ComponentParticleEmitter::UpdateSpawnAreaPos()
{
	switch (area.type)
	{
	case SPHERE:
		area.sphere.pos = transform->global->getPosition();
		break;
	case AAB:
		area.aabb.SetFromCenterAndSize(transform->global->getPosition(), area.aabb.Size());
		break;
	case AREA_NONE:
	default:
		break;
	}

}

void ComponentParticleEmitter::SetArea(AreaType type)
{
	area.type = type;
}


std::string ComponentParticleEmitter::EvTypetoString(int evt)
{
	switch (evt)
	{
	case ParticleAnimEvents::PARTICLE_CREATE:
		return "CREATE";
	}
	return "ERROR";
}

int ComponentParticleEmitter::getEvAmount()
{
	return ParticleAnimEvents::PARTICLE_AMOUNT_OF_EVENTS;
}

void ComponentParticleEmitter::ProcessAnimationEvents(std::map<int, void*>& evts)
{
	for (auto it_evt = evts.begin(); it_evt != evts.end(); ++it_evt)
	{
		switch (it_evt->first)
		{
		case ParticleAnimEvents::PARTICLE_CREATE:
			CreateParticle();
			break;
		default:
			break;
		}
	}
}