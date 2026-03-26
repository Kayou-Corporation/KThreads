#pragma once

#include "KThreadsUtils.hpp"

namespace Kayou
{
	class Thread
	{
	public:
		Thread(const char* name);
		~Thread();

		void Enqueue(std::move_only_function<void()> task);
		void WaitUntilFinished();

	private:
		std::thread m_thread;

		std::queue<std::move_only_function<void()>> m_taskQueue;

		std::mutex m_mutex;
		std::mutex m_finishMutex;

		std::condition_variable m_waitCondition, m_finishCondition;
		std::atomic<uint32_t> m_tasksRemaining{ 0 };
		bool m_stop = false;

		void CheckQueue();
	};
}