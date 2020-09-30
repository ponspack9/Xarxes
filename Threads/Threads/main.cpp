#include <stdio.h>
#include <stdlib.h>
#include <thread>



//void function(int arg1, int arg2, int arg3)
//{
//	printf("This is the secondary thread with arguments");
//
//}

int globalFoo = 0;

void function()
{
	printf("This is the secondary thread\n");
	for (int i = 0; i < 100000; i++, globalFoo++);

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