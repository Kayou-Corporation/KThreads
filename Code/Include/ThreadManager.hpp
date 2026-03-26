#pragma once

#include "KThreadsUtils.hpp"

namespace Kayou
{
	class ThreadManager
	{
	public:
		ThreadManager(const char* name, uint8_t numThreads);
		~ThreadManager();

		void Enqueue(std::move_only_function<void()> task, Priority priority = Priority::High);
		void WaitUntilFinished();

	private:
		std::vector<std::thread> m_threads;

		std::queue<std::move_only_function<void()>> m_highPriorityTaskQueue;
		std::queue<std::move_only_function<void()>> m_lowPriorityTaskQueue;

		std::mutex m_mutex;
		std::mutex m_finishMutex;

		std::condition_variable m_waitCondition, m_finishCondition;
		std::atomic<uint32_t> m_tasksRemaining{ 0 };
		bool m_stop = false;
		uint8_t m_numThreads;

		void CheckQueue();
	};
}
