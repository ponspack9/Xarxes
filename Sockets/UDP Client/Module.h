#pragma once
#include "Application.h"

class Task;

class Module
{
public:

	// Constructor and destructor

	Module() { }

	virtual ~Module() { }


	// Virtual functions

	virtual bool init() { return true; }

	virtual update_status update() { return update_status::UPDATE_CONTINUE; }

	virtual bool cleanUp() { return true; }


	// For tasks

	virtual void onTaskFinished(Task*) { }
};

