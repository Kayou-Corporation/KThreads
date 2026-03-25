#include "ThreadPool.hpp"

#include <ranges>

namespace Kayou
{
	ThreadPool::~ThreadPool()
	{
		m_threadManagers.clear();
	}

	void ThreadPool::InitQueue(const char* queueName, uint8_t numThreads, uint8_t highPriorityNumber)
	{
		m_threadManagers.try_emplace(queueName, std::make_unique<ThreadManager>(queueName, numThreads, highPriorityNumber));
	}

	void ThreadPool::EnqueueTask(const char* queueName, std::function<void()> task, Priority priority) const
	{
		m_threadManagers.at(queueName)->Enqueue(task, priority);
	}

	void ThreadPool::WaitUntilQueueFinished(const char* queueName) const
	{
		m_threadManagers.at(queueName)->WaitUntilFinished();
	}

	void ThreadPool::WaitUntilAllFinished() const
	{
		for (const std::unique_ptr<ThreadManager>& manager : m_threadManagers | std::views::values)
		{
			manager->WaitUntilFinished();
		}
	}

	void ThreadPool::ReleaseQueue(const char* queueName)
	{
		m_threadManagers.erase(queueName);
	}
}
