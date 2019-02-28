#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include "Color.h"
#include "ComponentTransform.h"
#include "ComponentBillboard.h"
#include "Globals.h"


class ParticleInfo
{
public:

	float speed = 0.0f;

	uint lifetime = 5;

	float startSize = 1.0f;
	float endSize = 1.0f;

	float startSpin = 0.0f;
	float endSpin = 0.0f;

	float3 direction = float3::zero;
	float3 startPosition = float3::zero;
	float3 gravity = -float3::unitY;

	Color startColor = White;
	Color endColor = White;

	void Set(float sSize = 0, float eSize = 0, float sSpin = 0, float eSpin = 0, float spd = 0, uint life = 0, float3 pos = float3::zero, float3 dir = float3::zero, float3 gravity = float3::zero, Color sColor = White, Color eColor = White);
};


class Particle
{
	friend class Billboard;
	friend class ComponentParticleEmitter;
public:
	Particle(ParticleInfo i);
	Particle() {};
	~Particle() {};

	void Update(float dt);
	bool Delete();
	void Reset();

	float DistanceToCamera() const;

private:

	float Ratio(float max, float  min);
	Color Ratio(Color max, Color  min);

	ParticleInfo info;

	float speed = 0;
	float lifeTime = 0;
	float size = 0;
	float spin = 0;

	float3 direction = float3::zero;
	float3 gravity = -float3::unitY;
	float3 position = float3::zero;
	Color color = White;

	float lifeLeft = 0;
	float lifetimeRatio = 1;

	bool toDelete = false;

};

#endif // !__PARTICLE_H__