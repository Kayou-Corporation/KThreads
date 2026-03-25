#include <iostream>
#include <syncstream>
#include <thread>
#include <chrono>

#include "ThreadPool.hpp"

int main()
{
	Kayou::ThreadPool threadPool;
	threadPool.InitQueue("Worker", 8, 5);
	threadPool.InitQueue("Kayou", 8, 1);

	for (int i = 0; i < 100; ++i)
	{
		for (int i1 = 0; i1 < 100; ++i1)
		{
			Kayou::Priority priority;
			
			if (i1 % 2 == 0)
				priority = Kayou::Priority::High;
			else
				priority = Kayou::Priority::Low;
		
			threadPool.EnqueueTask("Worker", [i1]() {
				{
					std::osyncstream(std::cout) << "Worker task " << i1 << " is running on thread " << std::this_thread::get_id() << '\n';
				}
				}, priority);
		}
		threadPool.WaitUntilQueueFinished("Worker");
		for (int i1 = 0; i1 < 100; ++i1)
		{
			Kayou::Priority priority;
			
			if (i1 % 2 == 0)
				priority = Kayou::Priority::High;
			else
				priority = Kayou::Priority::Low;

			threadPool.EnqueueTask("Kayou", [i1]() {
				{
					std::osyncstream(std::cout) << "Kayou task " << i1 << " is running on thread " << std::this_thread::get_id() << '\n';
				}
				}, priority);
		}
		threadPool.WaitUntilQueueFinished("Kayou");
	}
	threadPool.WaitUntilAllFinished();

	return 0;
}