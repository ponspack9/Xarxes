#include "ModuleTaskManager.h"

// OSCAR PONS GALLART & DAVID TELLO PANEA

void ModuleTaskManager::threadMain()
{
	while (!exitFlag)
	{
		// TODO 3:
		// - Wait for new tasks to arrive
		// - Retrieve a task from scheduledTasks
		// - Execute it
		// - Insert it into finishedTasks
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
	// TODO 1: Create threads (they have to execute threadMain())

	for (int i = 0; i < MAX_THREADS; i++)
		threads[i] = std::thread(&ModuleTaskManager::threadMain, this);

	return true;
}

bool ModuleTaskManager::update()
{
	// TODO 4: Dispatch all finished tasks to their owner module (use Module::onTaskFinished() callback)
	while (!finishedTasks.empty())
	{
		Task* task = finishedTasks.front();
		task->owner->onTaskFinished(task);

		finishedTasks.pop();
	}
	return true;
}

bool ModuleTaskManager::cleanUp()
{
	// TODO 5: Notify all threads to finish and join them
	exitFlag = true;
	conditionEvent.notify_all();

	for (int i = 0; i < MAX_THREADS; i++)
		threads[i].join();

	return true;
}

void ModuleTaskManager::scheduleTask(Task* task, Module* owner)
{
	task->owner = owner;

	// TODO 2: Insert the task into scheduledTasks so it is executed by some thread
	scheduledTasks.push(task);
	conditionEvent.notify_one();
}
