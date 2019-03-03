#include "Skybox.h"
#include "Material.h"
#include "Mesh.h"

#include "glew-2.1.0\include\GL\glew.h"
#include "MathGeoLib\Math\TransformOps.h"

#define SKYBOX_OVERLAP_VALUE 0.1f  // negative offset of planes to avoid edges being visible for the user

Skybox::Skybox(float distance) : distance(distance)
{
	uint num_vertices = 4;
	uint num_tris = 2;

	for (int i = 0; i < 6; i++)
	{
		float3* vertices = new float3[num_vertices];	float3* normals = new float3[num_vertices];
		float3* colors = new float3[num_vertices];		float2* tex_coords = new float2[num_vertices];
		Tri* indices = new Tri[num_tris];

		for (int i = 0; i < num_vertices; i++)	colors[i] = { color.r, color.g, color.b };

		if (i % 2)  { indices[0] = { 2,1,0 }; indices[1] = { 1,2,3 }; }
		else		{ indices[0] = { 0,1,2 }; indices[1] = { 3,2,1 }; }

		switch (i)
		{
		case LEFT:		// left face
			vertices[0] = { -distance + SKYBOX_OVERLAP_VALUE, distance, distance };	
			vertices[1] = { -distance + SKYBOX_OVERLAP_VALUE, -distance, distance };
			vertices[2] = { -distance + SKYBOX_OVERLAP_VALUE, distance, -distance };	
			vertices[3] = { -distance + SKYBOX_OVERLAP_VALUE, -distance, -distance };

			tex_coords[0] = { 0.0f, 0.0f };   tex_coords[1] = { 0.0f, 1.0f };
			tex_coords[2] = { 1.0f, 0.0f };   tex_coords[3] = { 1.0f, 1.0f };
			break;
		case RIGHT:		// right face
			vertices[0] = { distance - SKYBOX_OVERLAP_VALUE, distance, distance };	
			vertices[1] = { distance - SKYBOX_OVERLAP_VALUE, -distance, distance };
			vertices[2] = { distance - SKYBOX_OVERLAP_VALUE, distance, -distance };	
			vertices[3] = { distance - SKYBOX_OVERLAP_VALUE, -distance, -distance };

			tex_coords[0] = { 1.0f, 0.0f };   tex_coords[1] = { 1.0f, 1.0f };
			tex_coords[2] = { 0.0f, 0.0f };   tex_coords[3] = { 0.0f, 1.0f };
			break;
		case UP:		// up face
			vertices[0] = { distance, distance  - SKYBOX_OVERLAP_VALUE, distance };	
			vertices[1] = { -distance, distance -  SKYBOX_OVERLAP_VALUE, distance };
			vertices[2] = { distance, distance  - SKYBOX_OVERLAP_VALUE, -distance };	
			vertices[3] = { -distance, distance -  SKYBOX_OVERLAP_VALUE, -distance };

			tex_coords[0] = { 1.0f, 1.0f };   tex_coords[1] = { 1.0f, 0.0f };
			tex_coords[2] = { 0.0f, 1.0f };   tex_coords[3] = { 0.0f, 0.0f };
			break;
		case DOWN:		// down face
			vertices[0] = { distance, -distance  + SKYBOX_OVERLAP_VALUE, distance };		
			vertices[1] = { -distance, -distance +  SKYBOX_OVERLAP_VALUE, distance };
			vertices[2] = { distance, -distance  + SKYBOX_OVERLAP_VALUE, -distance };	
			vertices[3] = { -distance, -distance +  SKYBOX_OVERLAP_VALUE, -distance };

			tex_coords[0] = { 1.0f, 0.0f };   tex_coords[1] = { 0.0f, 0.0f };
			tex_coords[2] = { 0.0f, 0.0f };   tex_coords[3] = { 0.0f, 1.0f };
			break;
		case BACK:		// back face
			vertices[0] = { distance, distance, -distance + SKYBOX_OVERLAP_VALUE };	
			vertices[1] = { -distance, distance, -distance + SKYBOX_OVERLAP_VALUE };
			vertices[2] = { distance, -distance, -distance + SKYBOX_OVERLAP_VALUE };	
			vertices[3] = { -distance, -distance, -distance + SKYBOX_OVERLAP_VALUE };

			tex_coords[0] = { 1.0f, 0.0f };   tex_coords[1] = { 0.0f, 0.0f };
			tex_coords[2] = { 1.0f, 1.0f };   tex_coords[3] = { 0.0f, 1.0f };
			break;
		case FRONT:		// front face
			vertices[0] = { distance, distance, distance - SKYBOX_OVERLAP_VALUE };		
			vertices[1] = { -distance, distance, distance - SKYBOX_OVERLAP_VALUE };
			vertices[2] = { distance, -distance, distance - SKYBOX_OVERLAP_VALUE };	
			vertices[3] = { -distance, -distance, distance - SKYBOX_OVERLAP_VALUE };

			tex_coords[0] = { 0.0f, 0.0f };						tex_coords[1] = { 1.0f, 0.0f };
			tex_coords[2] = { 0.0f, 1.0f };						tex_coords[3] = { 1.0f, 1.0f };
			break;
		}

		normals[0] = vertices[0].Neg().Normalized();	normals[1] = vertices[1].Neg().Normalized();
		normals[2] = vertices[2].Neg().Normalized();	normals[3] = vertices[3].Neg().Normalized();

		planes[i] = new Mesh(vertices, indices, normals, colors, tex_coords, num_vertices, num_tris);
		textures[i] = nullptr;   // initialize to nullptr because start values are undefined
	}
}

Skybox::~Skybox()
{
	for (int i = 0; i < 6; i++)
	{
		if (planes[i]) delete planes[i];
		if (textures[i]) delete textures[i];
	}
}

void Skybox::setAllTextures(std::array<Texture*, 6>& texs)
{ 
	for (int i = 0; i < 6; i++)
	{
		if (textures[i]) 
			delete textures[i];

		textures[i] = texs[i];
	}
}


void Skybox::setTexture(Texture* tex, Direction index)
{ 
	if (index >= 0 && index < 6)
	{
		if (textures[index])
			delete textures[index];

		textures[index] = tex;
	}
}

void Skybox::removeTexture(Direction index)
{
	if (index >= 0 && index < 6)
	{
		delete textures[index];
		textures[index] = nullptr;
	}
}

void Skybox::setDistance(float dist)
{
	transform_mat = transform_mat * transform_mat.Scale(float3(distance / dist, distance / dist, distance / dist));
	distance = dist;
}


void Skybox::Draw() const
{
	if (active)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if (planes.size() == 6 && (textures.size() == 6 || color_mode))
		{
			float4x4 view_mat = float4x4::identity;

			GLfloat matrix[16];
			glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
			view_mat.Set((float*)matrix);

			glMatrixMode(GL_MODELVIEW_MATRIX);
			glLoadMatrixf((GLfloat*)(transform_mat.Transposed() * view_mat).v);

			for (int i = 0; i < 6; i++)
			{
				Texture* diffuse_tex = textures[i];

				if (diffuse_tex && !color_mode)		glEnable(GL_TEXTURE_2D);
				else								glColor3f(color.r, color.g, color.b);

				// bind VBOs before drawing
				glBindBuffer(GL_ARRAY_BUFFER, planes[i]->vboId);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planes[i]->iboId);

				bool depth_test = glIsEnabled(GL_DEPTH_TEST);
				glDisable(GL_DEPTH_TEST);

				// enable vertex arrays
				glEnableClientState(GL_VERTEX_ARRAY);
				glEnableClientState(GL_NORMAL_ARRAY);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);

				if (diffuse_tex)		glBindTexture(GL_TEXTURE_2D, diffuse_tex->getGLid());

				size_t Offset = sizeof(float3) * planes[i]->num_vertices;

				// specify vertex arrays with their offsets
				glVertexPointer(3, GL_FLOAT, 0, (void*)0);
				glNormalPointer(GL_FLOAT, 0, (void*)Offset);
				glColorPointer(3, GL_FLOAT, 0, (void*)(Offset * 2));
				glTexCoordPointer(2, GL_FLOAT, 0, (void*)(Offset * 3));
				glDrawElements(GL_TRIANGLES, planes[i]->num_tris * 3, GL_UNSIGNED_INT, NULL);

				if (diffuse_tex)		glBindTexture(GL_TEXTURE_2D, 0);
				else					glDisableClientState(GL_COLOR_ARRAY);

				glDisableClientState(GL_VERTEX_ARRAY);
				glDisableClientState(GL_NORMAL_ARRAY);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);

				if (depth_test)
					glEnable(GL_DEPTH_TEST);

				glColor3f(1.0f, 1.0f, 1.0f);

				// unbind VBOs
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

				if (diffuse_tex)		glDisable(GL_TEXTURE_2D);
			}
			glLoadMatrixf((GLfloat*)view_mat.v);
		}
	}
}
