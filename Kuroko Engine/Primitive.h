#ifndef __Primitive_H__
#define __Primitive_H__

#include "MathGeoLib/MathGeoLib.h"
#include "Color.h"

enum PrimitiveTypes
{
	Primitive_Point,
	Primitive_Line,
	Primitive_Plane,
	Primitive_Cube,
	Primitive_Sphere,
	Primitive_Cylinder
};
class Primitive
{
public:

	Primitive();

	virtual void	Render() const;
	virtual void	InnerRender() const;
	void			SetPos(float x, float y, float z);
	void			SetRotation(float angle, const float3 &u);	
	void			Scale(float x, float y, float z);
	float3			GetScale();
	PrimitiveTypes	GetType() const;

public:

	Color color;
	float4x4 transform;
	bool axis, wire;
	float mass = 1;
	float3 scale = { 1,1,1 };
	bool has_primitive_render = true;

protected:
	PrimitiveTypes type;
};

class PLine : public Primitive
{
public:
	PLine();
	PLine(float x, float y, float z);
	void InnerRender() const;
public:
	float3 origin;
	float3 destination;
};

class PCube : public Primitive {
public:
	PCube();
	PCube(float x, float y, float z);
	void InnerRender() const override;
public:
	float3 dimensions = { 5,5,5 };
};

class PPlane : public Primitive
{
public:
	PPlane();
	PPlane(float x, float y, float z, float d);
	void InnerRender() const override;
public:
	float3 normal;
	float constant;
};

class PSphere : public Primitive
{
public:
	PSphere();
	PSphere(float radius);
	void InnerRender() const;
public:
	float radius = 1;

	bool dead = false;
	bool isCollider = true;
};
#endif


