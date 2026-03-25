#include <iostream>
#include <syncstream>
#include <thread>
#include <chrono>

#include "ThreadPool.hpp"

inline void DoWork(uint32_t n)
{
	volatile uint64_t x = 0;
	for (uint32_t i = 0; i < n; ++i)
		x += i;
}

int main()
{
	Kayou::ThreadPool threadPool;
	threadPool.InitQueue("Worker", 16, 0); // no priority
	threadPool.InitQueue("Kayou", 16, 10);

    constexpr uint32_t numTasks = 1'000'000;
    constexpr uint32_t workPerTask = 100;

    std::cout << "<---------Time test : no priority--------->\n";

    auto start = std::chrono::high_resolution_clock::now();

    for (uint32_t i = 0; i < numTasks; ++i)
    {
        threadPool.EnqueueTask("Worker", [=]() 
            {
                DoWork(workPerTask);
            });
    }

    threadPool.WaitUntilAllFinished();

    auto end = std::chrono::high_resolution_clock::now();

    double time = std::chrono::duration<double>(end - start).count();

    std::cout << "Time: " << time << " s\n";
    std::cout << "Tasks/sec: " << (numTasks / time) << "\n";

    std::cout << "<---------Time test : low priority starvation--------->\n";

    Kayou::Priority priority;

    start = std::chrono::high_resolution_clock::now();

    for (uint32_t i = 0; i < numTasks; ++i)
    {
        priority = i % 10 == 0 ? Kayou::Priority::Low : Kayou::Priority::High;

        threadPool.EnqueueTask("Kayou", [=]() 
            {
                DoWork(workPerTask);
            }, priority);
    }

    threadPool.WaitUntilAllFinished();

    end = std::chrono::high_resolution_clock::now();

    time = std::chrono::duration<double>(end - start).count();

    std::cout << "Time: " << time << " s\n";
    std::cout << "Tasks/sec: " << (numTasks / time) << "\n";

    std::cout << "<---------Time test : enqueue latency--------->\n";

    auto task = [] {};

    constexpr uint32_t N = 1'000'000;

    start = std::chrono::high_resolution_clock::now();

    for (uint32_t i = 0; i < N; ++i)
    {
        threadPool.EnqueueTask("Worker", task);
    }

    end = std::chrono::high_resolution_clock::now();

    time = std::chrono::duration<double>(end - start).count();
    std::cout << "Time: " << time << " s\n";
    std::cout << "Avg enqueue time: " << (time / N) * 1e+9 << " ns\n";

	return 0;
}