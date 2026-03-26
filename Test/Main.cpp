#include <iostream>
#include <syncstream>
#include <thread>
#include <chrono>

#include "ThreadPool.hpp"
#include "Thread.hpp"

inline void DoWork(uint32_t n)
{
	volatile uint64_t x = 0;
	for (uint32_t i = 0; i < n; ++i)
		x += i;
}

int main()
{
	Kayou::ThreadPool threadPool;
	threadPool.InitQueue("Worker", std::thread::hardware_concurrency());
	threadPool.InitQueue("Kayou", std::thread::hardware_concurrency());
	threadPool.InitQueue("Enqueue", 8);

    Kayou::Thread thread("Single");

    constexpr uint32_t numTasks = 1'000'000;
    constexpr uint32_t workPerTask = 10000;

    auto work = [=]()
        {
            DoWork(workPerTask);
        };

    auto task = [] {};

    std::cout << "\033[1;31m<------------KThreads------------>\033[0m\n";
    std::cout << "Number of tasks: " << numTasks << '\n';
    std::cout << "Work per task: " << workPerTask << "\n\n";

    std::cout << "\033[1;32m<---------Thread pool time test : no priority--------->\033[0m\n";

    auto start = std::chrono::high_resolution_clock::now();

    for (uint32_t i = 0; i < numTasks; ++i)
    {
        threadPool.EnqueueTask("Worker", std::move(work));
    }

    threadPool.WaitUntilQueueFinished("Worker");

    auto end = std::chrono::high_resolution_clock::now();

    double time = std::chrono::duration<double>(end - start).count();

    std::cout << "Time: " << time << " s\n";
    std::cout << "Tasks/sec: " << (numTasks / time) << "\n\n";

    std::cout << "\033[1;32m<---------Thread pool time test : low priority starvation--------->\033[0m\n";

    Kayou::Priority priority;

    auto start1 = std::chrono::high_resolution_clock::now();

    for (uint32_t i = 0; i < numTasks; ++i)
    {
        priority = i % 10 == 0 ? Kayou::Priority::Low : Kayou::Priority::High;

        threadPool.EnqueueTask("Kayou", std::move(work), priority);
    }

    threadPool.WaitUntilQueueFinished("Kayou");

    auto end1 = std::chrono::high_resolution_clock::now();

    time = std::chrono::duration<double>(end1 - start1).count();

    std::cout << "Time: " << time << " s\n";
    std::cout << "Tasks/sec: " << (numTasks / time) << "\n\n";

    std::cout << "\033[1;32m<---------Thread pool time test : enqueue latency--------->\033[0m\n";

    auto start2 = std::chrono::high_resolution_clock::now();

    for (uint32_t i = 0; i < numTasks; ++i)
    {
        threadPool.EnqueueTask("Enqueue", std::move(task));
    }

    auto end2 = std::chrono::high_resolution_clock::now();

    threadPool.WaitUntilAllFinished();

    time = std::chrono::duration<double>(end2 - start2).count();
    std::cout << "Time: " << time << " s\n";
    std::cout << "Avg enqueue time: " << (time / numTasks) * 1e+9 << " ns\n\n";

    std::cout << "\033[1;33m<---------Single thread time test : execution time--------->\033[0m\n";

    auto start3 = std::chrono::high_resolution_clock::now();

    for (uint32_t i = 0; i < numTasks; ++i)
    {
        thread.Enqueue(std::move(work));
    }

    thread.WaitUntilFinished();

    auto end3 = std::chrono::high_resolution_clock::now();

    time = std::chrono::duration<double>(end3 - start3).count();

    std::cout << "Time: " << time << " s\n";
    std::cout << "Tasks/sec: " << (numTasks / time) << "\n\n";

    std::cout << "\033[1;33m<---------Single thread time test : enqueue latency--------->\033[0m\n";

    auto start4 = std::chrono::high_resolution_clock::now();

    for (uint32_t i = 0; i < numTasks; ++i)
    {
        thread.Enqueue(std::move(task));
    }

    auto end4 = std::chrono::high_resolution_clock::now();

    thread.WaitUntilFinished();

    time = std::chrono::duration<double>(end4 - start4).count();
    std::cout << "Time: " << time << " s\n";
    std::cout << "Avg enqueue time: " << (time / numTasks) * 1e+9 << " ns\n";

	return 0;
}