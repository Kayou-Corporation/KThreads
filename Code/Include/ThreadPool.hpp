#pragma once
#include <functional>
#include <memory>
#include <unordered_map>

#include "ThreadManager.hpp"

namespace Kayou
{
	class ThreadPool
	{
	public:
		ThreadPool() = default;
		~ThreadPool();

		void InitQueue(const char* queueName, uint8_t numThreads, uint8_t highPriorityNumber = 0);
		void EnqueueTask(const char* queueName, std::function<void()> task, Priority priority = Priority::High) const;
		void WaitUntilQueueFinished(const char* queueName) const;
		void WaitUntilAllFinished() const;
		void ReleaseQueue(const char* queueName);

	private:
		std::unordered_map<const char*, std::unique_ptr<ThreadManager>> m_threadManagers;
	};
}