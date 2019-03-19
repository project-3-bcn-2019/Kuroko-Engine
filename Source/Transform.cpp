#include "Transform.h"
#include "MathGeoLib\Math\TransformOps.h"
#include "Application.h"
#include "ModuleCamera3D.h"
#include "Camera.h"
#include "glew-2.1.0\include\GL\glew.h"


Transform::Transform(const Transform& transform)
{
	setPosition(transform.position);
	setRotation(transform.rotation);
	setScale(transform.scale);
	CalculateMatrix();
}

Transform::Transform(JSON_Object* deff) {

	setPosition(float3(json_object_get_number(deff, "px"), json_object_get_number(deff, "py"), json_object_get_number(deff, "pz")));
	setScale(float3(json_object_get_number(deff, "sx"), json_object_get_number(deff, "sy"), json_object_get_number(deff, "sz")));
	Quat rotation;
	rotation.x = json_object_get_number(deff, "qx");
	rotation.y = json_object_get_number(deff, "qy");
	rotation.z = json_object_get_number(deff, "qz");
	rotation.w = json_object_get_number(deff, "qw");
	setRotation(rotation);
}

void Transform::setRotationEuler(const float3& euler)
{
	rotation = Quat::identity;
	rotation = rotation * rotation.RotateX(DEGTORAD * (euler.x));
	rotation = rotation * rotation.RotateY(DEGTORAD * (euler.y));
	rotation = rotation * rotation.RotateZ(DEGTORAD * (euler.z));
	euler_angles = euler;
}

void Transform::LookAt(const float3& pos, const float3& target)
{
	rotation = Quat::RotateFromTo(Forward().Normalized(), (target - pos).Normalized()) * rotation;
}

float4x4 Transform::CalculateMatrix()
{
	mat = float4x4::identity;
	mat = mat * rotation;
	mat = mat * mat.Scale(scale);
	mat.SetTranslatePart(position);

	return mat;
}

float3 Transform::Right() const
{
	return rotation * float3::unitX;;
}

float3 Transform::Forward() const
{
	return rotation * float3::unitZ;
}

float3 Transform::Up() const
{
	return rotation * float3::unitY;
}

void Transform::Save(JSON_Object* config) {

	// Position
	json_object_set_number(config, "px", position.x);
	json_object_set_number(config, "py", position.y);
	json_object_set_number(config, "pz", position.z);
	// Scale
	json_object_set_number(config, "sx", scale.x);
	json_object_set_number(config, "sy", scale.y);
	json_object_set_number(config, "sz", scale.z);
	// Rotation
	json_object_set_number(config, "qx", rotation.x);
	json_object_set_number(config, "qy", rotation.y);
	json_object_set_number(config, "qz", rotation.z);
	json_object_set_number(config, "qw", rotation.w);
}

float3 Transform::getRotationEuler(){
	euler_angles = rotation.ToEulerXYZ() * RADTODEG; 
	return euler_angles; 
}

void Transform::setPosition(const float3& pos) { position = pos; };
void Transform::Translate(const float3& dir) { position += dir; };
void Transform::setScale(const float3& scl) { scale = scl; };
void Transform::Scale(const float3& scl) { scale.x *= scl.x; scale.y *= scl.y; scale.z *= scl.z; };