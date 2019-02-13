#include "PPrimitive.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "Module.h"
#include "SDL/include/SDL_opengl.h"
#include "glut/glut.h"

#pragma comment (lib, "glut/glut32.lib")


PPrimitive::PPrimitive() :transform(float4x4::identity), color(White), wire(false), axis(false), type(PrimitiveTypes::P_POINT)
{

}

void PPrimitive::Render() const
{
	if (has_primitive_render)
	{
		glPushMatrix();
		glMultMatrixf(transform.ptr());

		if (axis == true)
		{
			// Draw Axis Grid
			glLineWidth(2.0f);

			glBegin(GL_LINES);

			glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

			glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(1.0f, 0.0f, 0.0f);
			glVertex3f(1.0f, 0.1f, 0.0f); glVertex3f(1.1f, -0.1f, 0.0f);
			glVertex3f(1.1f, 0.1f, 0.0f); glVertex3f(1.0f, -0.1f, 0.0f);

			glColor4f(0.0f, 1.0f, 0.0f, 1.0f);

			glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 1.0f, 0.0f);
			glVertex3f(-0.05f, 1.25f, 0.0f); glVertex3f(0.0f, 1.15f, 0.0f);
			glVertex3f(0.05f, 1.25f, 0.0f); glVertex3f(0.0f, 1.15f, 0.0f);
			glVertex3f(0.0f, 1.15f, 0.0f); glVertex3f(0.0f, 1.05f, 0.0f);

			glColor4f(0.0f, 0.0f, 1.0f, 1.0f);

			glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 1.0f);
			glVertex3f(-0.05f, 0.1f, 1.05f); glVertex3f(0.05f, 0.1f, 1.05f);
			glVertex3f(0.05f, 0.1f, 1.05f); glVertex3f(-0.05f, -0.1f, 1.05f);
			glVertex3f(-0.05f, -0.1f, 1.05f); glVertex3f(0.05f, -0.1f, 1.05f);

			glEnd();

			glLineWidth(1.0f);
		}

		glColor3f(color.r, color.g, color.b);

		//if (App->renderer3D->attributes.wireframe)
		//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//else
		//	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		InnerRender();

		glLineWidth(1.0f);

		glPopMatrix();
	}
}

void PPrimitive::InnerRender() const
{
	glPointSize(5.0f);

	glBegin(GL_POINTS);

	glVertex3f(0.0f, 0.0f, 0.0f);

	glEnd();

	glPointSize(1.0f);
}



void PPrimitive::SetPos(float x, float y, float z)
{
	transform[3][0] = x;
	transform[3][1] = y;
	transform[3][2] = z;	
}

void PPrimitive::SetRotation(float angle, const float3 & u)
{
	transform.RotateAxisAngle(u,angle);
	
}

void PPrimitive::Scale(float x, float y, float z)
{
	transform.Scale(x, y, z);
}

float3 PPrimitive::GetScale()
{
	return transform.GetScale();
}

PrimitiveTypes PPrimitive::GetType() const
{
	return type;
}


//CUBE=====================================================
PCube::PCube() :PPrimitive()
{
	type = PrimitiveTypes::P_CUBE;

}

PCube::PCube(float x, float y, float z) :PPrimitive(), dimensions(x, y, z)
{
	type = PrimitiveTypes::P_CUBE;
}


void PCube::InnerRender() const
{	
	//draw direct mode cube
	float sx = dimensions.x * scale.x * 0.25f;
	float sy = dimensions.y * scale.y * 0.25f;
	float sz = dimensions.z * scale.z * 0.25f;

	glLineWidth(5.0f);
	glBegin(GL_QUADS);

	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-sx, -sy, sz);
	glVertex3f(sx, -sy, sz);
	glVertex3f(sx, sy, sz);
	glVertex3f(-sx, sy, sz);

	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(sx, -sy, -sz);
	glVertex3f(-sx, -sy, -sz);
	glVertex3f(-sx, sy, -sz);
	glVertex3f(sx, sy, -sz);

	glNormal3f(1.0f, 0.0f, 0.0f);
	glVertex3f(sx, -sy, sz);
	glVertex3f(sx, -sy, -sz);
	glVertex3f(sx, sy, -sz);
	glVertex3f(sx, sy, sz);

	glNormal3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(-sx, -sy, -sz);
	glVertex3f(-sx, -sy, sz);
	glVertex3f(-sx, sy, sz);
	glVertex3f(-sx, sy, -sz);

	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-sx, sy, sz);
	glVertex3f(sx, sy, sz);
	glVertex3f(sx, sy, -sz);
	glVertex3f(-sx, sy, -sz);

	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(-sx, -sy, -sz);
	glVertex3f(sx, -sy, -sz);
	glVertex3f(sx, -sy, sz);
	glVertex3f(-sx, -sy, sz);

	glEnd();
}



// PLANE ==================================================
PPlane::PPlane() : PPrimitive(), normal(0, 1, 0), constant(1)
{
	type = PrimitiveTypes::P_PLANE;
}

PPlane::PPlane(float x, float y, float z, float d) : PPrimitive(), normal(x, y, z), constant(d)
{
	type = PrimitiveTypes::P_PLANE;
}


void PPlane::InnerRender() const
{
	glLineWidth(1.0f);

	glBegin(GL_LINES);

	float d = 200.0f;

	for (float i = -d; i <= d; i += 1.0f)
	{
		glVertex3f(i, 0.0f, -d);
		glVertex3f(i, 0.0f, d);
		glVertex3f(-d, 0.0f, i);
		glVertex3f(d, 0.0f, i);
	}

	glEnd();
}



PSphere::PSphere() : PPrimitive(), radius(1.0f)
{
	type = PrimitiveTypes::P_SPHERE;
}

PSphere::PSphere(float radius) : PPrimitive(), radius(radius)
{
	type = PrimitiveTypes::P_SPHERE;
}

void PSphere::InnerRender() const
{
	glutSolidSphere(radius*scale.x, 25, 25);

}

// LINE ==================================================
PLine::PLine() : PPrimitive(), origin(0, 0, 0), destination(1, 1, 1)
{
	type = PrimitiveTypes::P_LINE;
}

PLine::PLine(float x, float y, float z) : PPrimitive(), origin(0, 0, 0), destination(x, y, z)
{
	type = PrimitiveTypes::P_LINE;
}

void PLine::InnerRender() const
{
	glLineWidth(2.0f);

	glBegin(GL_LINES);

	glVertex3f(origin.x, origin.y, origin.z);
	glVertex3f(destination.x, destination.y, destination.z);

	glEnd();

	glLineWidth(1.0f);
}



