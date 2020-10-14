#include <stdio.h>
#include <stdlib.h>

#include "Application.h"


// CLIENT

enum main_states {
	MAIN_INIT,
	MAIN_UPDATE,
	MAIN_FINISH,
	MAIN_EXIT
};

int main(int argc, char** argv)
{
	Application* app = new Application();

	int state = MAIN_INIT;
	int update_return = UPDATE_STOP;
	int ret = EXIT_FAILURE;

	while (state != MAIN_EXIT) {
		switch (state) {

		case MAIN_INIT:

			printf("Client Initialization --------------\n");

			if (app->Init() == false) {
				printf("Client Init exits with ERROR\n");
				state = MAIN_EXIT;
			}
			else {
				state = MAIN_UPDATE;
			}

			break;

		case MAIN_UPDATE:

			update_return = app->Update();

			if (update_return == update_status::UPDATE_ERROR) {
				printf("Client Update exits with ERROR\n");
				state = MAIN_EXIT;
			}

			if (update_return == update_status::UPDATE_STOP)
				state = MAIN_FINISH;

			break;


		case MAIN_FINISH:

			printf("Client Clean up --------------\n");

			if (app->CleanUp() == false)
				printf("Client Cleaning exits with ERROR\n");

			else
				ret = EXIT_SUCCESS;

			state = MAIN_EXIT;

			break;

		}
	}

	delete app;

	return ret;
}

