#include <iostream>
#include <thread>

#include "ThreadPool.hpp"

static std::mutex COUT_MUTEX;

int main()
{
	for (int i = 0; i < 100; ++i)
	{
		Kayou::ThreadPool threadPool;
		threadPool.InitQueue("Worker", 8);
		for (int i = 0; i < 10; ++i)
		{
			threadPool.EnqueueTask("Worker", [i]() {
				{
					std::lock_guard<std::mutex> lock(COUT_MUTEX);
					std::cout << "Task " << i << " is running on thread " << std::this_thread::get_id() << '\n';
				}
				});
		}
		threadPool.InitQueue("Kayou", 8);
		for (int i = 0; i < 10; ++i)
		{
			threadPool.EnqueueTask("Kayou", [i]() {
				{
					std::lock_guard<std::mutex> lock(COUT_MUTEX);
					std::cout << "Task " << i << " is running on thread " << std::this_thread::get_id() << '\n';
				}
				});
		}
		threadPool.WaitUntilAllFinished();
	}

	return 0;
}