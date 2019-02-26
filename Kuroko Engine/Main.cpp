#include <stdlib.h>
#include "Application.h"
#include "Globals.h"
#include "Applog.h"
#include "Profiler.h"

#include "SDL/include/SDL.h"
#pragma comment( lib, "SDL/lib/SDL2.lib" )
#pragma comment( lib, "SDL/lib/SDL2main.lib" )

#include <stdlib.h>  
//#include <vld.h>

enum main_states
{
	MAIN_CREATION,
	MAIN_START,
	MAIN_UPDATE,
	MAIN_FINISH,
	MAIN_EXIT
};

Application* App;
AppLog* app_log;
Profiler* prof;

int main(int argc, char ** argv)
{
	app_log = new AppLog();
	app_log->AddLog("Starting engine '%s'...", TITLE);
	prof = new Profiler();

	int main_return = EXIT_FAILURE;
	main_states state = MAIN_CREATION;

	while (state != MAIN_EXIT)
	{
		APP_CYCLE_START;

		switch (state)
		{
		case MAIN_CREATION:

			PROFILE_SCOPE_START("START");

			app_log->AddLog("-------------- Application Creation --------------");
			App = new Application();
			state = MAIN_START;

			PROFILE_SCOPE_END;

			break;

		case MAIN_START:

			PROFILE_SCOPE_START("INIT");

			app_log->AddLog("-------------- Application Init --------------\n");
			if (App->Init() == false)
			{
				app_log->AddLog("Application Init exits with ERROR\n");
				state = MAIN_EXIT;
			}
			else
			{
				state = MAIN_UPDATE;
				app_log->AddLog("\n -------------- Application Update --------------\n");
			}

			PROFILE_SCOPE_END;

			break;

		case MAIN_UPDATE:
		{
			PROFILE_SCOPE_START("INIT");

			int update_return = App->Update();

			if (update_return == UPDATE_ERROR)
			{
				app_log->AddLog("Application Update exits with ERROR\n");
				state = MAIN_EXIT;
			}

			if (update_return == UPDATE_STOP)
				state = MAIN_FINISH;

			PROFILE_SCOPE_END;
		}
			break;

		case MAIN_FINISH:
			PROFILE_SCOPE_START("FINISH");

			app_log->AddLog("-------------- Application CleanUp --------------\n");
			if (App->CleanUp() == false)
			{
				app_log->AddLog("Application CleanUp exits with ERROR\n");
			}
			else
				main_return = EXIT_SUCCESS;

			state = MAIN_EXIT;

			PROFILE_SCOPE_END;

			break;

		}

		APP_CYCLE_END;
	}

	delete App;
	app_log->AddLog("Exiting game '%s'...\n", TITLE);
	delete app_log;

	return main_return;
}