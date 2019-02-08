#include "Primitive.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "Module.h"
#include "SDL/include/SDL_opengl.h"
#include "glut/glut.h"

#pragma comment (lib, "glut/glut32.lib")


Primitive::Primitive() :transform(float4x4::identity), color(White), wire(false), axis(false), type(PrimitiveTypes::Primitive_Point)
{

}

void Primitive::Render() const
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

void Primitive::InnerRender() const
{
	glPointSize(5.0f);

	glBegin(GL_POINTS);

	glVertex3f(0.0f, 0.0f, 0.0f);

	glEnd();

	glPointSize(1.0f);
}



void Primitive::SetPos(float x, float y, float z)
{
	transform[3][0] = x;
	transform[3][1] = y;
	transform[3][2] = z;	
}

void Primitive::SetRotation(float angle, const float3 & u)
{
	transform.RotateAxisAngle(u,angle);
	
}

void Primitive::Scale(float x, float y, float z)
{
	transform.Scale(x, y, z);
}

float3 Primitive::GetScale()
{
	return transform.GetScale();
}

PrimitiveTypes Primitive::GetType() const
{
	return type;
}


//CUBE=====================================================
PCube::PCube() :Primitive()
{
	type = PrimitiveTypes::Primitive_Cube;

}

PCube::PCube(float x, float y, float z) :Primitive(), dimensions(x, y, z)
{
	type = PrimitiveTypes::Primitive_Cube;
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
PPlane::PPlane() : Primitive(), normal(0, 1, 0), constant(1)
{
	type = PrimitiveTypes::Primitive_Plane;
}

PPlane::PPlane(float x, float y, float z, float d) : Primitive(), normal(x, y, z), constant(d)
{
	type = PrimitiveTypes::Primitive_Plane;
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



PSphere::PSphere() : Primitive(), radius(1.0f)
{
	type = PrimitiveTypes::Primitive_Sphere;
}

PSphere::PSphere(float radius) : Primitive(), radius(radius)
{
	type = PrimitiveTypes::Primitive_Sphere;
}

void PSphere::InnerRender() const
{
	glutSolidSphere(radius*scale.x, 25, 25);

}

// LINE ==================================================
PLine::PLine() : Primitive(), origin(0, 0, 0), destination(1, 1, 1)
{
	type = PrimitiveTypes::Primitive_Line;
}

PLine::PLine(float x, float y, float z) : Primitive(), origin(0, 0, 0), destination(x, y, z)
{
	type = PrimitiveTypes::Primitive_Line;
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


// =============================================
/*
void PDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	debug_line.origin.Set(from.getX(), from.getY(), from.getZ());
	debug_line.destination.Set(to.getX(), to.getY(), to.getZ());
	debug_line.color.Set(color.getX(), color.getY(), color.getZ());
	debug_line.Render();
}

void PDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{
	point.SetPos(PointOnB.getX(), PointOnB.getY(), PointOnB.getZ());
	point.color.Set(color.getX(), color.getY(), color.getZ());
	point.Render();
}

void PDebugDrawer::reportErrorWarning(const char* warningString)
{
	LOG("Bullet warning: %s", warningString);
}

void PDebugDrawer::draw3dText(const btVector3& location, const char* textString)
{
	LOG("Bullet draw text: %s", textString);
}

void PDebugDrawer::setDebugMode(int debugMode)
{
	mode = (DebugDrawModes)debugMode;
}

int	 PDebugDrawer::getDebugMode() const
{
	return mode;
}
*/

