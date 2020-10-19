#pragma once
#include <Macros.h>

// Module declarations
class Module;
class ModuleUDP;
class ModuleTCP;
//class ModuleTaskManager;

enum class update_status
{
	UPDATE_CONTINUE,
	UPDATE_ERROR,
	UPDATE_STOP
};

class Application
{
public:

	// Constructor and destructor

	Application();

	~Application();

	// Application lifetime methods

	bool init();

	update_status update();

	bool cleanUp();


public:

	// Modules
	ModuleUDP* modUDP = nullptr;
	ModuleTCP* modTCP = nullptr;
	//ModuleTaskManager* modTaskManager = nullptr;


private:

	// All modules
	Module* modules[1] = {};
	int numModules = 0;
};

extern Application* App;
