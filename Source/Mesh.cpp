#include "Mesh.h"
#include "Color.h"
#include "Applog.h"
#include "Material.h"
#include "ModuleScene.h"
#include "Application.h"
#include "MathGeoLib\MathGeoLib.h"
#include "glew-2.1.0\include\GL\glew.h"
#include "ModuleResourcesManager.h"
#include "ResourceTexture.h"
#include "ModuleShaders.h"
#include "ModuleCamera3D.h"
#include "Camera.h"
#include "ComponentAnimation.h"

#include "Assimp\include\scene.h"

Mesh::Mesh(const aiMesh& imported_mesh, const aiScene& scene,const char* file_name) : id(App->scene->last_mesh_id++)
{
	if (LoadFromAssimpMesh(imported_mesh, scene))	LoadDataToVRAM();
	else											app_log->AddLog("error loading mesh for the component %s", imported_mesh.mName.C_Str());

	mesh_name = file_name;
	
}

Mesh::Mesh(float3* _vertices, Tri* _tris, float3* _normals, float3* _colors, float2* _tex_coords, uint _num_vertices, uint _num_tris, const float3& centroid): id(App->scene->last_mesh_id++) 
{
	num_vertices	= _num_vertices;
	vertices		= _vertices;
	num_tris		= _num_tris;
	tris			= _tris;

	if (_normals)
	{
		imported_normals = true;
		normals = _normals;
	}
	else 
	{
		normals = new float3[num_vertices];
		for (int i = 0; i < num_vertices; i++)	normals[i] = { 0.0f, 0.0f, 0.0f };
	}

	if (_colors)
	{
		imported_colors = true;
		colors = _colors;
	}
	else
	{
		colors = new float3[num_vertices];
		for (int i = 0; i < num_vertices; i++)	colors[i] = { 1.0f, 1.0f, 1.0f };
	}

	if (_tex_coords)
	{
		imported_tex_coords = true;
		tex_coords = _tex_coords;
	}
	else 
	{
		tex_coords = new float2[num_vertices];
		for (int i = 0; i < num_vertices; i++)
			tex_coords[i] = float2::zero;
	}

	calculateCentroidandHalfsize();
	this->centroid = centroid;
	LoadDataToVRAM();
	//Descoment to use shader render
	//LoadDataToVRAMShaders();
}

Mesh::Mesh(PrimitiveTypes primitive) : id(App->scene->last_mesh_id++)
{
	switch (primitive)
	{
	case Primitive_Cube:	 BuildCube(); break;
	case Primitive_Plane:	 BuildPlane(); break;
	case Primitive_Sphere:	 BuildSphere(); break;
	case Primitive_Cylinder: BuildCylinder(); break;
	default:
		break;
	}

	calculateCentroidandHalfsize();
	LoadDataToVRAM();
	//Descoment to use shader render
	//LoadDataToVRAMShaders();
}


Mesh::~Mesh()
{
	if(iboId != 0)
		glDeleteBuffers(1, &iboId);
	if (vboId != 0)
		glDeleteBuffers(1, &vboId);
	if (vaoId != 0)
		glDeleteVertexArrays(1, &vaoId);

	if (vertices)	delete vertices;
	if (tris)		delete tris;
	if (normals)	delete normals;
	if (colors)		delete colors;
	if (tex_coords) delete tex_coords;
	
	RELEASE(MeshGPU);
}

void Mesh::setMorphedVertices(float3 * vertices)
{
	RELEASE_ARRAY(morphed_vertices);
	morphed_vertices = vertices;
}

void Mesh::LoadDataToVRAM()
{
	// create VBOs
	glDeleteBuffers(1, &vboId);
	glDeleteBuffers(1, &iboId);
	glGenBuffers(1, &vboId);    // for vertex buffer
	glGenBuffers(1, &iboId);    // for index buffer

	size_t vSize = sizeof(float3) * num_vertices;
	size_t tSize = sizeof(float2) * num_vertices;

	// copy vertex attribs data to VBO
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, vSize + vSize + vSize + tSize, 0, GL_STATIC_DRAW); // reserve space
	if (morphed_vertices == nullptr)
		glBufferSubData(GL_ARRAY_BUFFER, 0, vSize, vertices);                  // copy verts at offset 0
	else
		glBufferSubData(GL_ARRAY_BUFFER, 0, vSize, morphed_vertices);
	glBufferSubData(GL_ARRAY_BUFFER, vSize, vSize, normals);               // copy norms after verts
	glBufferSubData(GL_ARRAY_BUFFER, vSize + vSize, vSize, colors);          // copy cols after norms
	glBufferSubData(GL_ARRAY_BUFFER, vSize + vSize + vSize, tSize, tex_coords); // copy texs after cols
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// copy index data to VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float3) * num_tris, tris, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::LoadDataToVRAMShaders()
{
	FillMeshGPU();

	//glBindTexture(GL_TEXTURE_2D, id_texture);
	glGenVertexArrays(1, &vaoId);
	glGenBuffers(1, &vboId);
	glGenBuffers(1, &iboId);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(vaoId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, (sizeof(Vertex::position) + sizeof(Vertex::tex_coords) + sizeof(Vertex::color) + sizeof(Vertex::normal))*num_vertices, MeshGPU, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) *num_tris*3, tris, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (GLvoid*)(7 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (GLvoid*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);
	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


void Mesh::Draw(Material* mat, bool draw_as_selected)  const
{
	Texture* diffuse_tex = nullptr;
	if (mat) {
		uint diffuse_resource_id = mat->getTextureResource(DIFFUSE);
		if (diffuse_resource_id != 0) {
			ResourceTexture* diffuse_resource = (ResourceTexture*)App->resources->getResource(diffuse_resource_id);
			if (diffuse_resource)
				diffuse_tex = diffuse_resource->texture;
		}
	}

	Draw(diffuse_tex, draw_as_selected);

}


void Mesh::Draw(Texture* tex, bool draw_as_selected)  const
{
	if (tex)								glEnable(GL_TEXTURE_2D);
	else if (!draw_as_selected && imported_colors)	glEnableClientState(GL_COLOR_ARRAY);

	// bind VBOs before drawing
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);

	// enable vertex arrays
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	if (draw_as_selected)
	{
		glColor3f(0.0f, 1.0f, 0.0f);
		glLineWidth(2.5f);
	}
	else
		glColor3f(tint_color.r, tint_color.b, tint_color.g);

	if (tex)		glBindTexture(GL_TEXTURE_2D, tex->getGLid());

	size_t Offset = sizeof(float3) * num_vertices;

	// specify vertex arrays with their offsets
	glVertexPointer(3, GL_FLOAT, 0, (void*)0);
	glNormalPointer(GL_FLOAT, 0, (void*)Offset);
	glColorPointer(3, GL_FLOAT, 0, (void*)(Offset * 2));
	glTexCoordPointer(2, GL_FLOAT, 0, (void*)(Offset * 3));
	glDrawElements(GL_TRIANGLES, num_tris * 3, GL_UNSIGNED_INT, NULL);

	if (tex)		glBindTexture(GL_TEXTURE_2D, 0);
	else					glDisableClientState(GL_COLOR_ARRAY);

	if (draw_as_selected)
		glLineWidth(1.0f);

	glColor3f(1.0f, 1.0f, 1.0f);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	// unbind VBOs
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	if (tex)		glDisable(GL_TEXTURE_2D);

}


void Mesh::FillMeshGPU()
{
	MeshGPU = new Vertex[num_vertices];
	float4 color = { *colors, 1.0f };
	for (int i = 0; i < num_vertices; i++)
	{
		MeshGPU[i].Assign(vertices[i], color);

		MeshGPU[i].tex_coords = tex_coords[i];

		MeshGPU[i].normal = normals[i];
		
	}
}

void Mesh::MaxDrawFunctionTest(Material* mat,ComponentAnimation* animation,float* global_transform, bool draw_as_selected) const
{
	float3 lightPosition = { 0.0f,10.0f,10.0f };
	float3 lightColors = { 1.0f,1.0f,1.0f };

	//Texture* diffuse_tex = mat ? mat->getTexture(DIFFUSE) : nullptr;
	Texture* diffuse_tex = nullptr;
	if (mat) {
		uint diffuse_resource_id = mat->getTextureResource(DIFFUSE);
		if (diffuse_resource_id != 0) {
			ResourceTexture* diffuse_resource = (ResourceTexture*)App->resources->getResource(diffuse_resource_id);
			if (diffuse_resource)
				diffuse_tex = diffuse_resource->texture;
		}
	}

	if (diffuse_tex)
		glEnable(GL_TEXTURE_2D);
	else if (!draw_as_selected /*&& imported_colors*/)	
		glEnableClientState(GL_COLOR_ARRAY);

	if (draw_as_selected)
	{
		glColor3f(0.0f, 1.0f, 0.0f);
		glLineWidth(2.5f);
	}
	else
		glColor3f(tint_color.r, tint_color.b, tint_color.g);

	if (mat)
	{
		if (diffuse_tex)
		{
			glBindTexture(GL_TEXTURE_2D, diffuse_tex->getGLid());
		}
		
		if (animation == nullptr)
		{
			glUseProgram(App->shaders->GetDefaultShaderProgram()->programID);

			GLint model_loc = glGetUniformLocation(App->shaders->GetDefaultShaderProgram()->programID, "model_matrix");
			glUniformMatrix4fv(model_loc, 1, GL_FALSE, global_transform);
			GLint proj_loc = glGetUniformLocation(App->shaders->GetDefaultShaderProgram()->programID, "projection");
			glUniformMatrix4fv(proj_loc, 1, GL_FALSE, App->camera->current_camera->GetProjectionMatrix());
			GLint view_loc = glGetUniformLocation(App->shaders->GetDefaultShaderProgram()->programID, "view");
			glUniformMatrix4fv(view_loc, 1, GL_FALSE, App->camera->current_camera->GetViewMatrix());
			GLint lightPos = glGetUniformLocation(App->shaders->GetDefaultShaderProgram()->programID, "lightPos");
			glUniform3f(lightPos,lightPosition.x, lightPosition.y, lightPosition.z);
			GLint lightColor = glGetUniformLocation(App->shaders->GetDefaultShaderProgram()->programID, "lightColor");
			glUniform3f(lightColor, lightColors.x, lightColors.y, lightColors.z);

			if (diffuse_tex)
			{
				GLint texture = glGetUniformLocation(App->shaders->GetDefaultShaderProgram()->programID, "test");
				glUniform1i(texture,1);
			}
			else
			{
				GLint texture = glGetUniformLocation(App->shaders->GetDefaultShaderProgram()->programID, "test");
				glUniform1i(texture, 0);
			}
		}
		else
		{
			glUseProgram(App->shaders->GetAnimationShaderProgram()->programID);

			/*
			TODO
			Introduce uniform variables
			*/
		}
	}
	else
	{
		glUseProgram(App->shaders->GetDefaultShaderProgram()->programID);

		GLint model_loc = glGetUniformLocation(App->shaders->GetDefaultShaderProgram()->programID, "model_matrix");
		glUniformMatrix4fv(model_loc, 1, GL_FALSE, global_transform);
		GLint proj_loc = glGetUniformLocation(App->shaders->GetDefaultShaderProgram()->programID, "projection");
		glUniformMatrix4fv(proj_loc, 1, GL_FALSE, App->camera->current_camera->GetProjectionMatrix());
		GLint view_loc = glGetUniformLocation(App->shaders->GetDefaultShaderProgram()->programID, "view");
		glUniformMatrix4fv(view_loc, 1, GL_FALSE, App->camera->current_camera->GetViewMatrix());
		GLint lightPos = glGetUniformLocation(App->shaders->GetDefaultShaderProgram()->programID, "lightPos");
		glUniform3f(lightPos, lightPosition.x, lightPosition.y, lightPosition.z);
		GLint lightColor = glGetUniformLocation(App->shaders->GetDefaultShaderProgram()->programID, "lightColor");
		glUniform3f(lightColor, lightColors.x, lightColors.y, lightColors.z);

	}

	glBindVertexArray(vaoId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);

	glDrawElements(GL_TRIANGLES, num_tris*3, GL_UNSIGNED_INT, 0);
	//Disable All The Data
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	if (diffuse_tex)		glBindTexture(GL_TEXTURE_2D, 0);
	else					glDisableClientState(GL_COLOR_ARRAY);

	if (draw_as_selected)
		glLineWidth(1.0f);

	glColor3f(1.0f, 1.0f, 1.0f);

	// unbind VBOs
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	if (diffuse_tex)		glDisable(GL_TEXTURE_2D);

}


void Mesh::DrawNormals() const
{
	glBegin(GL_LINES);

	float3 centroid = float3::zero;
	float3 h_s = float3::zero;

	for (int i = 0; i < num_vertices; i++)
	{
		glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
		glVertex3f(vertices[i].x + (normals[i].x * 0.2f), vertices[i].y + (normals[i].y * 0.2f), vertices[i].z + (normals[i].z * 0.2f));
	}

	glEnd();
}

void Mesh::updateVRAM()
{
	LoadDataToVRAM(); //Load to VRAM each frame due to skinning
}

void Mesh::BuildCube(float3& size)
{
	size.x *= 0.5f; size.y *= 0.5f; size.z *= 0.5f;

	mesh_name = "CUBE";
	num_vertices = 8;
	vertices = new float3[num_vertices];

	vertices[0] = { -size.x, -size.y, -size.z };
	vertices[1] = { size.x, -size.y, -size.z };
	vertices[2] = { -size.x, -size.y, size.z };
	vertices[3] = { size.x, -size.y, size.z };

	vertices[4] = { -size.x, size.y, -size.z };
	vertices[5] = { size.x, size.y, -size.z };
	vertices[6] = { -size.x, size.y, size.z };
	vertices[7] = { size.x, size.y, size.z };

	num_tris = 12;
	tris = new Tri[num_tris];

	tris[0].set(0, 1, 2);	tris[1].set(3, 2, 1);	//front
	tris[2].set(6, 5, 4);	tris[3].set(5, 6, 7);	//back
	tris[4].set(5, 3, 1);	tris[5].set(3, 5, 7);	//up
	tris[6].set(0, 2, 4);	tris[7].set(6, 4, 2);	//down
	tris[8].set(4, 1, 0);	tris[9].set(1, 4, 5);	//left
	tris[10].set(2, 3, 6);	tris[11].set( 7, 6, 3 );	//right  

	normals = new float3[num_vertices];
	for (int i = 0; i < num_vertices; i++)
		normals[i] = vertices[i].Normalized();

	colors = new float3[num_vertices];
	for (int i = 0; i < num_vertices; i++)
		colors[i] = { 1.0f, 1.0f, 1.0f };

	tex_coords = new float2[36];

	tex_coords[0] = { 0.0f, 0.0f }; tex_coords[1] = { 1.0f, 0.0f };
	tex_coords[2] = { 0.0f, 1.0f }; tex_coords[3] = { 1.0f, 1.0f };
	tex_coords[4] = { 0.0f, 0.0f }; tex_coords[5] = { 1.0f, 0.0f };
	tex_coords[6] = { 0.0f, 1.0f }; tex_coords[7] = { 1.0f, 1.0f };

	tint_color.setRandom();
}

void Mesh::BuildPlane(float sx, float sy)
{
	sx *= 0.5f, sy *= 0.5f;

	mesh_name = "PLANE";
	num_vertices = 8;
	vertices = new float3[num_vertices];

	vertices[0] = { -sx, -sy, 0 }; vertices[1] = { sx, -sy, 0 };
	vertices[2] = { -sx, sy, 0 };  vertices[3] = { sx, sy, 0 };
	vertices[4] = { -sx, -sy, 0 }; vertices[5] = { sx, -sy, 0 };
	vertices[6] = { -sx, sy, 0 };  vertices[7] = { sx, sy, 0 };

	num_tris = 4;
	tris = new Tri[num_tris];

	tris[0].set( 0, 1, 2);	tris[1].set( 3, 2, 1);
	tris[2].set( 6, 5, 4);	tris[3].set( 7, 5, 6);

	normals = new float3[num_vertices];
	for (int i = 0; i < num_vertices; i++)
		normals[i] = vertices[i].Normalized();

	colors = new float3[num_vertices];
	for (int i = 0; i < num_vertices; i++)
		colors[i] = { 1.0f, 1.0f, 1.0f };

	tex_coords = new float2[num_vertices];
	tex_coords[0] = { 0.0f, 0.0f };   tex_coords[1] = { 1.0f, 0.0f };
	tex_coords[2] = { 0.0f, 1.0f };   tex_coords[3] = { 1.0f, 1.0f };
	tex_coords[4] = { 0.0f, 0.0f }; tex_coords[5] = { 1.0f, 0.0f };
	tex_coords[6] = { 0.0f, 1.0f }; tex_coords[7] = { 1.0f, 1.0f };

	tint_color.setRandom();
}

void Mesh::BuildSphere(float radius, float sectorCount, float stackCount) {

	// Sphere (code from http://www.songho.ca/opengl/gl_sphere.html)

	mesh_name = "SPHERE";	
	float x, y, z, xy;                              // vertex position
	float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
	float s, t;                                     // vertex texCoord

	float sectorStep = 2 * PI / sectorCount;
	float stackStep = PI / stackCount;
	float sectorAngle, stackAngle;

	float sector_size = sectorCount + 1;
	float stack_size = stackCount + 1;

	num_vertices = sector_size * stack_size;
	num_tris = stackCount * sectorCount * 2 - (2 * sectorCount);

	vertices = new float3[num_vertices];
	normals = new float3[num_vertices];
	tex_coords = new float2[num_vertices];
	tris = new Tri[num_tris];

	int array_pos = 0;
	for (int i = 0; i <= stackCount; ++i) {
		stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
		xy = radius * cosf(stackAngle);             // r * cos(u)
		z = radius * sinf(stackAngle);              // r * sin(u)

													// add (sectorCount+1) vertices per stack
													// the first and last vertices have same position and normal, but different tex coods
		for (int j = 0; j <= sectorCount; ++j) {
			sectorAngle = j * sectorStep;

			// vertex position (x, y, z)
			x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
			y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
			vertices[array_pos] = { x, y, z };

			// vertex normal (nx, ny, nz)
			nx = x * lengthInv;
			ny = y * lengthInv;
			nz = z * lengthInv;
			normals[array_pos] = { nx, ny, nz };

			// vertex tex coord (s, t)
			s = (float)j / sectorCount;
			t = (float)i / stackCount;
			tex_coords[array_pos] = { s, t };
			array_pos++;
		}
	}

	array_pos = 0;
	int k1, k2;
	for (int i = 0; i < stackCount; ++i) {
		k1 = i * (sectorCount + 1);     // beginning of current stack
		k2 = k1 + sectorCount + 1;      // beginning of next stack

		for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
			// 2 triangles per sector excluding 1st and last stacks
			if (i != 0) {
				tris[array_pos] = { (uint)k1, (uint)k2, (uint)k1 + 1 };
				array_pos++;
			}

			if (i != (stackCount - 1)) {
				tris[array_pos] = { (uint)k1 + 1, (uint)k2, (uint)k2 + 1 };
				array_pos++;
			}
		}
	}

	colors = new float3[num_vertices];
	for (int i = 0; i < num_vertices; i++)
		colors[i] = { 1.0f, 1.0f, 1.0f };


	tint_color.setRandom();
}

void Mesh::BuildCylinder(float radius, float length, int numSteps) {


	// code belongs to Zakuayada https://www.gamedev.net/forums/topic/359467-draw-cylinder-with-triangle-strips/

	mesh_name = "CYLINDER";
	num_vertices = numSteps * 2 + 2;
	vertices = new float3[num_vertices];
	normals = new float3[num_vertices];
	tex_coords = new float2[num_vertices];
	colors = new float3[num_vertices];

	colors = new float3[num_vertices];
	for (int i = 0; i < num_vertices; i++)
		colors[i] = { 1.0f, 1.0f, 1.0f };

	float hl = length * 0.5f;
	float a = 0.0f;
	float step = PI * 2 / (float)numSteps;
	for (int i = 0; i < numSteps; ++i) {
		float x = cos(a) * radius;
		float y = sin(a) * radius;
		vertices[i] = { x, y, hl };
		tex_coords[i] = { 1.0f / numSteps * i , 1.0f };
		vertices[i + numSteps] = { x, y, -hl };
		tex_coords[i + numSteps] = { 1.0f / numSteps * i , 0.0f };

		a += step;
	}

	vertices[numSteps * 2 + 0] = {0.0f, 0.0f, +hl};
	vertices[numSteps * 2 + 1] = {0.0f, 0.0f, -hl};

	num_tris = 4 * numSteps * 3;
	tris = new Tri[num_tris];

	for (int i = 0; i < numSteps; ++i) {
		unsigned int i1 = i;
		unsigned int i2 = (i1 + 1) % numSteps;
		unsigned int i3 = i1 + numSteps;
		unsigned int i4 = i2 + numSteps;

		// Sides

		tris[i * 6 + 0] = { i1, i3, i2};
		tris[i * 6 + 3] = { i4, i2, i3};
		// Caps
		tris[numSteps * 6 + i * 6 + 0] = { (uint)numSteps * 2 + 0, i1, i2};
		tris[numSteps * 6 + i * 6 + 3] = { (uint)numSteps * 2 + 1, i4, i3};
	}

	for (int i = 0; i < num_vertices; i++)
		normals[i] = vertices[i].Normalized();

	tint_color.setRandom();
}



bool Mesh::LoadFromAssimpMesh(const aiMesh& imported_mesh, const aiScene& scene)
{
	//vertices
	if (imported_mesh.mNumVertices)
	{
		num_vertices = imported_mesh.mNumVertices;
		vertices = new float3[num_vertices];
		memcpy(vertices, imported_mesh.mVertices, sizeof(float3) * num_vertices);
	}
	else
		return false;

	// faces
	if (imported_mesh.HasFaces())
	{
		num_tris = imported_mesh.mNumFaces;
		tris = new Tri[num_tris]; // assume each face is a triangle
		for (uint i = 0; i < num_tris; ++i)
		{
			if (imported_mesh.mFaces[i].mNumIndices == 3)
				memcpy(&tris[i], imported_mesh.mFaces[i].mIndices, sizeof(Tri));
			else
				app_log->AddLog("WARNING, geometry face with != 3 indices!");
		}
	}
	else
		return false;

	// normals
	normals = new float3[num_vertices];
	if (imported_mesh.HasNormals())
	{
		imported_normals = true;
		memcpy(normals, imported_mesh.mNormals, sizeof(float3) * num_vertices);
	}
	else
	{
		for (int i = 0; i < num_vertices; i++)
			normals[i] = { 0.0f, 0.0f, 0.0f };
	}

	// colors
	colors = new float3[num_vertices];
	aiColor3D color(-1.0f, 0.0f, 0.0f);
	scene.mMaterials[imported_mesh.mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, color);

	if (color.r != -1.0f)
	{
		imported_colors = true;
		for (int i = 0; i < num_vertices; i++)
			colors[i] = { color.r, color.g, color.b };
	}
	else
	{
		if (imported_mesh.HasVertexColors(0))
		{
			imported_colors = true;
			memcpy(colors, imported_mesh.mColors[0], sizeof(float3) * num_vertices);
		}
		else
		{
			Color random_color;
			random_color.setRandom();

			for (int i = 0; i < num_vertices; i++)
				colors[i] = { random_color.r, random_color.g, random_color.b };
		}
	}

	// texture coordinates
	tex_coords = new float2[num_vertices];
	if (imported_mesh.HasTextureCoords(0))
	{
		imported_tex_coords = true;
		for (int i = 0; i < num_vertices; i++)
			tex_coords[i] = { imported_mesh.mTextureCoords[0][i].x, imported_mesh.mTextureCoords[0][i].y };
	}
	else
	{
		for (int i = 0; i < num_vertices; i++)
			tex_coords[i] = float2::zero;
	}
	calculateCentroidandHalfsize();

	for (int i = 0; i < num_vertices; i++)
		vertices[i] -= centroid;

	return true;
}

void Mesh::calculateCentroidandHalfsize()
{
	float3 lowest_p = float3::inf;
	float3 highest_p = -float3::inf;

	if (vertices)
	{
		for (int i = 0; i < num_vertices; i++)
		{
			if (lowest_p.x > vertices[i].x) lowest_p.x = vertices[i].x;
			if (lowest_p.y > vertices[i].y) lowest_p.y = vertices[i].y;
			if (lowest_p.z > vertices[i].z) lowest_p.z = vertices[i].z;

			if (highest_p.x < vertices[i].x) highest_p.x = vertices[i].x;
			if (highest_p.y < vertices[i].y) highest_p.y = vertices[i].y;
			if (highest_p.z < vertices[i].z) highest_p.z = vertices[i].z;
		}
	}

	centroid = ((lowest_p + highest_p) * 0.5f);
	half_size = highest_p - centroid;

}

void Mesh::getData(uint& vert_num, uint& poly_count, bool& has_normals, bool& has_colors, bool& has_texcoords) const
{
	vert_num = num_vertices;
	poly_count = num_tris;
	has_normals = imported_normals;
	has_colors = imported_colors;
	has_texcoords = imported_tex_coords;
}

void Mesh::ClearData()
{
	glDeleteBuffers(1, &iboId);
	glDeleteBuffers(1, &vboId);
	glDeleteVertexArrays(1, &vaoId);

	if (vertices)	delete vertices;
	if (tris)		delete tris;
	if (normals)	delete normals;
	if (colors)		delete colors;
	if (tex_coords) delete tex_coords;

	RELEASE(MeshGPU);
	
	imported_normals = imported_colors = imported_tex_coords = false;
	num_vertices = num_tris = iboId = vboId = 0;
}
