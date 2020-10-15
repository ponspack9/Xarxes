#include "Application.h"
#include "ModuleUDP.h"
#include "ModuleTaskManager.h"


#define ADD_MODULE(ModuleClass, module) \
	module = new ModuleClass(); \
	modules[numModules++] = module;

Application::Application()
{
	// Create modules
	ADD_MODULE(ModuleTaskManager, modTaskManager);
	ADD_MODULE(ModuleUDP, modUDP);
}


Application::~Application()
{
	// Destroy modules
	for (auto module : modules)
	{
		if (module == nullptr) continue;

		delete module;
	}
}


bool Application::init()
{
	for (auto module : modules)
	{
		if (module == nullptr) continue;

		if (module->init() == false)
		{
			return false;
		}
	}
	return true;
}

update_status Application::update()
{
	for (auto module : modules)
	{
		if (module == nullptr) continue;

		update_status status = module->update();

		if (status != update_status::UPDATE_CONTINUE) return status;
	}

	return update_status::UPDATE_CONTINUE;
}

bool Application::cleanUp()
{
	for (int i = numModules; i > 0; --i)
	{
		Module* module = modules[i - 1];

		if (module == nullptr) continue;

		if (module->cleanUp() == false)
		{
			return false;
		}
	}

	return true;
}
