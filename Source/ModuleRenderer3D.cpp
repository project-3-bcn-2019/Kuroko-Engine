#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleWindow.h"
#include "ModuleUI.h"
#include "Globals.h"
#include "Applog.h"
#include "ModuleImporter.h"
#include "ModuleCamera3D.h"
#include "Camera.h"
#include "ModuleScene.h"
#include "ModuleShaders.h"

#include "glew-2.1.0\include\GL\glew.h"
#include "SDL\include\SDL_opengl.h"
#include <gl/GL.h>
#include <gl/GLU.h>

#pragma comment (lib, "opengl32.lib") /* link Microsoft OpenGL lib   */

#pragma comment( lib, "glew-2.1.0/lib/glew32.lib")
#pragma comment( lib, "glew-2.1.0/lib/glew32s.lib")



ModuleRenderer3D::ModuleRenderer3D(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	name = "renderer3d";
}

// Called before render is available
bool ModuleRenderer3D::Init(const JSON_Object* config)
{
	app_log->AddLog("Creating 3D Renderer context...\n");
	bool ret = true;
	
	//Create context
	context = SDL_GL_CreateContext(App->window->main_window->window);
	if(context == NULL)
	{
		app_log->AddLog("OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	
	if(ret == true)
	{
		GLenum err = glewInit();

		//Use Vsync
		if(VSYNC && SDL_GL_SetSwapInterval(1) < 0)
			app_log->AddLog("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());

		//Initialize Projection Matrix
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		//Check for error
		GLenum error = glGetError();
		if(error != GL_NO_ERROR)
		{
			ret = false;
		}

		//Initialize Modelview Matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		//Check for error
		error = glGetError();
		if(error != GL_NO_ERROR)
		{
			ret = false;
		}
		
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glClearDepth(1.0f);
		
		//Initialize clear color
		glClearColor(0.f, 0.f, 0.f, 1.f);

		//Check for error
		error = glGetError();
		if(error != GL_NO_ERROR)
		{
			ret = false;
		}
		
		GLfloat LightModelAmbient[] = {0.95f, 0.95f, 0.95f, 1.0f};
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LightModelAmbient);
		
		lights[0].ref = GL_LIGHT0;
		lights[0].ambient.Set(0.0f, 0.0f, 0.0f, 1.0f);
		lights[0].diffuse.Set(0.1f, 0.1f, 0.1f, 1.0f);
		lights[0].SetPos(0.0f, 2.5f, 0.0f);
		lights[0].Init();
		
		GLfloat MaterialAmbient[] = {1.0f, 1.0f, 1.0f, 1.0f};
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, MaterialAmbient);

		GLfloat MaterialDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialDiffuse);
		
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glClearDepth(1.0f);
		glClearColor(0.8f, 0.8f, 0.8f, 1.f);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE); 
		glEnable(GL_BLEND);
		lights[0].Active(true);
		glEnable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_TEXTURE_2D);

	}
	// Projection matrix for
	OnResize(App->window->main_window->width, App->window->main_window->height);


	//Initializing Shaders
	app_log->AddLog("Compiling Default Shaders...");
	App->shaders->InitializeDefaulShaders();

	return ret;
}

bool ModuleRenderer3D::Start()
{
	return true;
}

// PreUpdate: clear buffer
update_status ModuleRenderer3D::PreUpdate(float dt)
{
	for (auto cam = App->camera->game_cameras.rbegin(); cam != App->camera->game_cameras.rend(); ++cam)
	{	

		if (!(*cam)->active)
		{
			if(((*cam)->IsViewport() || (*cam) == App->camera->editor_camera) && !(*cam)->draw_in_UI)
				continue;
		}

		App->camera->current_camera = *cam;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf((GLfloat*)(*cam)->getFrustum()->ProjectionMatrix().v);

		glMatrixMode(GL_MODELVIEW);
		float4x4 mat((*cam)->getFrustum()->ViewMatrix());
		glLoadMatrixf((GLfloat*)mat.Transposed().v);

		lights[0].SetPos((*cam)->getFrustum()->pos.x, (*cam)->getFrustum()->pos.y, (*cam)->getFrustum()->pos.z);

		for (uint i = 0; i < MAX_LIGHTS; ++i)
			lights[i].Render();

		App->scene->DrawScene((*cam)->getFrustum()->pos);

		if (*cam != App->camera->background_camera && (*cam)->getFrameBuffer())
		{
			glReadBuffer(GL_BACK); // Ensure we are reading from the back buffer.
			if ((*cam)->draw_depth)  
			{
				glBindTexture(GL_TEXTURE_2D, (*cam)->getFrameBuffer()->depth_tex->gl_id);
				glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 0, 0, (*cam)->getFrameBuffer()->size_x, (*cam)->getFrameBuffer()->size_y, 0);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, (*cam)->getFrameBuffer()->tex->gl_id);
				glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, (*cam)->getFrameBuffer()->size_x, (*cam)->getFrameBuffer()->size_y, 0);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}

		
	}

	return UPDATE_CONTINUE;
}

// PostUpdate present buffer to screen
update_status ModuleRenderer3D::PostUpdate(float dt)
{
	SDL_GL_SwapWindow(App->window->main_window->window);
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleRenderer3D::CleanUp()
{
	app_log->AddLog("Destroying 3D Renderer");

	SDL_GL_DeleteContext(context);

	return true;
}


void ModuleRenderer3D::OnResize(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf((GLfloat*)App->camera->editor_camera->getFrustum()->ProjectionMatrix().v);
}



void ModuleRenderer3D::DirectDrawCube(float3& size, float3& pos) const
{
	glLineWidth(2.0f);

	size.x *= 0.5f; size.y *= 0.5f; size.z *= 0.5f;
	glBegin(GL_QUADS);

	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-size.x + pos.x, -size.y + pos.y, size.z + pos.z);
	glVertex3f(size.x + pos.x, -size.y + pos.y, size.z + pos.z);
	glVertex3f(size.x + pos.x, size.y + pos.y, size.z + pos.z);
	glVertex3f(-size.x + pos.x, size.y + pos.y, size.z + pos.z);

	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(size.x + pos.x, -size.y + pos.y, -size.z + pos.z);
	glVertex3f(-size.x + pos.x, -size.y + pos.y, -size.z + pos.z);
	glVertex3f(-size.x + pos.x, size.y + pos.y, -size.z + pos.z);
	glVertex3f(size.x + pos.x, size.y + pos.y, -size.z + pos.z);

	glNormal3f(1.0f, 0.0f, 0.0f);
	glVertex3f(size.x + pos.x, -size.y + pos.y, size.z + pos.z);
	glVertex3f(size.x + pos.x, -size.y + pos.y, -size.z + pos.z);
	glVertex3f(size.x + pos.x, size.y + pos.y, -size.z + pos.z);
	glVertex3f(size.x + pos.x, size.y + pos.y, size.z + pos.z);

	glNormal3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(-size.x + pos.x, -size.y + pos.y, -size.z + pos.z);
	glVertex3f(-size.x + pos.x, -size.y + pos.y, size.z + pos.z);
	glVertex3f(-size.x + pos.x, size.y + pos.y, size.z + pos.z);
	glVertex3f(-size.x + pos.x, size.y + pos.y, -size.z + pos.z);

	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-size.x + pos.x, size.y + pos.y, size.z + pos.z);
	glVertex3f(size.x + pos.x, size.y + pos.y, size.z + pos.z);
	glVertex3f(size.x + pos.x, size.y + pos.y, -size.z + pos.z);
	glVertex3f(-size.x + pos.x, size.y + pos.y, -size.z + pos.z);

	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(-size.x + pos.x, -size.y + pos.y, -size.z + pos.z);
	glVertex3f(size.x + pos.x, -size.y + pos.y, -size.z + pos.z);
	glVertex3f(size.x + pos.x, -size.y + pos.y, size.z + pos.z);
	glVertex3f(-size.x + pos.x, -size.y + pos.y, size.z + pos.z);

	glEnd();

}

void ModuleRenderer3D::DrawDirectAABB(AABB aabb) const {

	float3 lowest_p = { aabb.minPoint.x, aabb.minPoint.y,aabb.minPoint.z };
	float3 highest_p = { aabb.maxPoint.x, aabb.maxPoint.y,aabb.maxPoint.z };

	glLineWidth(1.5f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_LINES);

	glVertex3f(lowest_p.x, lowest_p.y, lowest_p.z);		glVertex3f(highest_p.x, lowest_p.y, lowest_p.z);
	glVertex3f(lowest_p.x, lowest_p.y, lowest_p.z);		glVertex3f(lowest_p.x, highest_p.y, lowest_p.z);
	glVertex3f(highest_p.x, lowest_p.y, lowest_p.z);	glVertex3f(highest_p.x, highest_p.y, lowest_p.z);
	glVertex3f(lowest_p.x, highest_p.y, lowest_p.z);	glVertex3f(highest_p.x, highest_p.y, lowest_p.z);

	glVertex3f(highest_p.x, highest_p.y, highest_p.z);  glVertex3f(lowest_p.x, highest_p.y, highest_p.z);
	glVertex3f(highest_p.x, highest_p.y, highest_p.z);  glVertex3f(highest_p.x, lowest_p.y, highest_p.z);
	glVertex3f(lowest_p.x, highest_p.y, highest_p.z);	glVertex3f(lowest_p.x, lowest_p.y, highest_p.z);
	glVertex3f(highest_p.x, lowest_p.y, highest_p.z);	glVertex3f(lowest_p.x, lowest_p.y, highest_p.z);

	glVertex3f(lowest_p.x, lowest_p.y, lowest_p.z);		glVertex3f(lowest_p.x, lowest_p.y, highest_p.z);
	glVertex3f(highest_p.x, highest_p.y, lowest_p.z);	glVertex3f(highest_p.x, highest_p.y, highest_p.z);
	glVertex3f(highest_p.x, lowest_p.y, lowest_p.z);	glVertex3f(highest_p.x, lowest_p.y, highest_p.z);
	glVertex3f(lowest_p.x, highest_p.y, lowest_p.z);	glVertex3f(lowest_p.x, highest_p.y, highest_p.z);

	glEnd();
	glColor3f(1.0f, 1.0f, 1.0f);

	glLineWidth(1.0f);
}

