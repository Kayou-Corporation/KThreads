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
	threadPool.InitQueue("Worker", 8);
	threadPool.InitQueue("Kayou", 8);
	threadPool.InitQueue("Enqueue", 8);

    constexpr uint32_t numTasks = 1'000'000;
    constexpr uint32_t workPerTask = 100;

    auto work = [=]()
        {
            DoWork(workPerTask);
        };

    std::cout << "<------------KThreads------------>\n";
    std::cout << "Number of tasks: " << numTasks << '\n';
    std::cout << "Work per task: " << workPerTask << "\n\n";

    std::cout << "<---------Time test : no priority--------->\n";

    auto start = std::chrono::high_resolution_clock::now();

    for (uint32_t i = 0; i < numTasks; ++i)
    {
        threadPool.EnqueueTask("Worker", std::move(work));
    }

    threadPool.WaitUntilAllFinished();

    auto end = std::chrono::high_resolution_clock::now();

    double time = std::chrono::duration<double>(end - start).count();

    std::cout << "Time: " << time << " s\n";
    std::cout << "Tasks/sec: " << (numTasks / time) << "\n\n";

    std::cout << "<---------Time test : low priority starvation--------->\n";

    Kayou::Priority priority;

    auto start1 = std::chrono::high_resolution_clock::now();

    for (uint32_t i = 0; i < numTasks; ++i)
    {
        priority = i % 10 == 0 ? Kayou::Priority::Low : Kayou::Priority::High;

        threadPool.EnqueueTask("Kayou", std::move(work), priority);
    }

    threadPool.WaitUntilAllFinished();

    auto end1 = std::chrono::high_resolution_clock::now();

    time = std::chrono::duration<double>(end1 - start1).count();

    std::cout << "Time: " << time << " s\n";
    std::cout << "Tasks/sec: " << (numTasks / time) << "\n\n";

    std::cout << "<---------Time test : enqueue latency--------->\n";

    auto task = [] {};

    auto start2 = std::chrono::high_resolution_clock::now();

    for (uint32_t i = 0; i < numTasks; ++i)
    {
        threadPool.EnqueueTask("Enqueue", std::move(task));
    }

    auto end2 = std::chrono::high_resolution_clock::now();

    time = std::chrono::duration<double>(end2 - start2).count();
    std::cout << "Time: " << time << " s\n";
    std::cout << "Avg enqueue time: " << (time / numTasks) * 1e+9 << " ns\n";

	return 0;
}