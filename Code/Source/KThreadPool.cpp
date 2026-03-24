#include "KThreadPool.hpp"

#include <ranges>

namespace Kayou
{
	KThreadPool::~KThreadPool()
	{
		m_threadManagers.clear();
	}

	void KThreadPool::InitQueue(const char* queueName, uint8_t numThreads)
	{
		m_threadManagers.try_emplace(queueName, std::make_unique<KThreadManager>(queueName, numThreads));
	}

	void KThreadPool::EnqueueTask(const char* queueName, std::function<void()> const& task) const
	{
		m_threadManagers.at(queueName)->Enqueue(task);
	}

	void KThreadPool::WaitUntilQueueFinished(const char* queueName) const
	{
		m_threadManagers.at(queueName)->WaitUntilFinished();
	}

	void KThreadPool::WaitUntilAllFinished() const
	{
		for (const std::unique_ptr<KThreadManager>& manager : m_threadManagers | std::views::values)
		{
			manager->WaitUntilFinished();
		}
	}

	void KThreadPool::ReleaseQueue(const char* queueName)
	{
		m_threadManagers.erase(queueName);
	}
}
