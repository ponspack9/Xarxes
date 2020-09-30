#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <mutex>


//void function(int arg1, int arg2, int arg3)
//{
//	printf("This is the secondary thread with arguments");
//
//}

int globalFoo = 0;

std::mutex mtx;

void function()
{
	printf("This is the secondary thread\n");
	for (int i = 0; i < 100000; i++)
	{
		// this line lock anything below and inside the scope
		// this must be used EVERYWHERE the shared variable is used
		std::unique_lock<std::mutex> lock(mtx);

		globalFoo++;
	}

	printf("GlobalFoo %i \n", globalFoo);
}


int main()
{

	//std::thread t = std::thread(function, arg1, arg2, arg3);

	std::thread t[] = {
		std::thread(function),
		std::thread(function)
	};
	printf("This is the main thread\n");
	printf("GlobalFoo %i\n", globalFoo);

	t[0].join();
	t[1].join();
	system("pause");
	return 0;
}