#include "Particle.h"
#include "Application.h"
#include "Camera.h"
#include "ModuleCamera3D.h"

#include "glew-2.1.0\include\GL\glew.h"
#include "SDL\include\SDL_opengl.h"
#include <gl/GL.h>
#include <gl/GLU.h>

Particle::Particle(ParticleInfo i)
{
	info = i;
	Reset();

}


void Particle::Reset()
{
	position = info.startPosition;
	lifeTime = info.lifetime;
	size = info.startSize;
	spin = info.startSpin;
	speed = info.speed;
	lifeLeft = lifeTime;
	color = info.startColor;
	direction = info.direction;
	gravity = info.gravity;
}


void Particle::Update(float dt)
{
	lifeLeft -= dt;

	if (lifeLeft <= 0)
		toDelete = true;

	lifetimeRatio = lifeLeft / lifeTime;
	if (lifetimeRatio < 0)
		lifetimeRatio = 0;

	size = Ratio(info.endSize, info.startSize);
	spin = Ratio(info.endSpin, info.startSpin);
	color = Ratio(info.endColor, info.startColor);

	float3 displacement = (direction + gravity) * speed;

	gravity += gravity * dt;
	position += displacement * dt;

}


bool Particle::Delete()
{
	return toDelete;
}

float Particle::DistanceToCamera() const
{
	return position.DistanceSq(App->camera->current_camera->getFrustum()->pos);
}


float Particle::Ratio(float max, float min)
{
	return (max - min) * (1 - lifetimeRatio) + min;
}

Color Particle::Ratio(Color max, Color min)
{
	Color c;
	c.r = Ratio(info.endColor.r, info.startColor.r);
	c.g = Ratio(info.endColor.g, info.startColor.g);
	c.b = Ratio(info.endColor.b, info.startColor.b);
	c.a = Ratio(info.endColor.a, info.startColor.a);

	return c;
}

void ParticleInfo::Set(float sSize, float eSize, float sSpin, float eSpin, float spd, uint life, float3 pos, float3 dir, float3 grav, Color sColor, Color eColor)
{
	startSize = sSize;
	endSize = eSize;

	startSpin = sSpin;
	endSpin = eSpin;

	speed = spd;
	lifetime = life;

	startPosition = pos;
	direction = dir;
	gravity = grav;

	startColor = sColor;
	endColor = eColor;

}
