#include <iostream>
#include <thread>

#include "ThreadPool.hpp"

static std::mutex COUT_MUTEX;

int main()
{
	for (int i = 0; i < 10; ++i)
	{
		Kayou::ThreadPool threadPool;
		threadPool.InitQueue("Worker", 8);
		for (int i1 = 0; i1 < 10; ++i1)
		{
			threadPool.EnqueueTask("Worker", [i1]() {
				{
					std::lock_guard<std::mutex> lock(COUT_MUTEX);
					std::cout << "Worker task " << i1 << " is running on thread " << std::this_thread::get_id() << '\n';
				}
				});
		}
		threadPool.InitQueue("Kayou", 8);
		for (int i1 = 0; i1 < 10; ++i1)
		{
			threadPool.EnqueueTask("Kayou", [i1]() {
				{
					std::lock_guard<std::mutex> lock(COUT_MUTEX);
					std::cout << "Kayou task " << i1 << " is running on thread " << std::this_thread::get_id() << '\n';
				}
				});
		}
		threadPool.WaitUntilAllFinished();
	}

	return 0;
}