#include "ModuleTaskManager.h"

// OSCAR PONS GALLART & DAVID TELLO PANEA

void ModuleTaskManager::threadMain()
{
	while (!exitFlag)
	{
		std::unique_lock<std::mutex> lock(mtx);

		if (scheduledTasks.size() > 0)
		{
			Task* task = scheduledTasks.front();
			task->execute();
			finishedTasks.push(task);
			scheduledTasks.pop();
		}
		else
		{
			conditionEvent.wait(lock);
		}
	}
}

bool ModuleTaskManager::init()
{
	for (int i = 0; i < MAX_THREADS; i++)
		threads[i] = std::thread(&ModuleTaskManager::threadMain, this);

	return true;
}

update_status ModuleTaskManager::update()
{
	while (!finishedTasks.empty())
	{
		Task* task = finishedTasks.front();
		task->owner->onTaskFinished(task);

		finishedTasks.pop();
	}
	return update_status::UPDATE_CONTINUE;
}

bool ModuleTaskManager::cleanUp()
{
	exitFlag = true;
	conditionEvent.notify_all();

	for (int i = 0; i < MAX_THREADS; i++)
		threads[i].join();

	return true;
}

void ModuleTaskManager::scheduleTask(Task* task, Module* owner)
{
	task->owner = owner;

	scheduledTasks.push(task);
	conditionEvent.notify_one();
}
