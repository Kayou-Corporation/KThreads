#include "KThreadManager.hpp"

#include <string>

namespace Kayou
{
	KThreadManager::KThreadManager(const char* name, uint8_t numThreads) : m_numThreads(numThreads)
	{
		for (uint8_t i = 0u; i < m_numThreads; ++i)
		{
			m_threads.emplace_back(&KThreadManager::CheckQueue, this);
			std::string threadName = std::string(name) + "Thread";

			if (m_numThreads > 1u)
			{
				threadName += std::to_string(i);
			}

			SetThreadName(m_threads[i], std::wstring(threadName.begin(), threadName.end()));
		}
	}

	KThreadManager::~KThreadManager()
	{
		m_stop.store(true, std::memory_order_release);
		m_waitCondition.notify_all();

		for (uint8_t i = 0u; i < m_numThreads; ++i)
		{
			if (m_threads[i].joinable())
			{
				m_threads[i].join();
			}
		}
		m_threads.clear();
	}

	void KThreadManager::Enqueue(std::function<void()> const& task)
	{
		m_tasksRemaining.fetch_add(1u, std::memory_order_acq_rel);
		{
			std::lock_guard<std::mutex> lock(m_queueMutex);
			m_taskQueue.emplace(task);
		}
	}

	void KThreadManager::WaitUntilFinished()
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		m_finishCondition.wait(lock, [this]() { return m_tasksRemaining.load(std::memory_order_acquire) == 0u; });
	}

	void KThreadManager::CheckQueue()
	{
		while (true)
		{
			std::function<void()> task;

			{
				std::unique_lock<std::mutex> lock(m_queueMutex);
				m_waitCondition.wait(lock, [this]() { return m_stop.load(std::memory_order_acquire) || !m_taskQueue.empty(); });

				if (m_stop.load(std::memory_order_acquire) && m_taskQueue.empty())
				{
					return;
				}

				task = std::move(m_taskQueue.front());
				m_taskQueue.pop();
			}

			task();
			const uint32_t remaining = m_tasksRemaining.fetch_sub(1u, std::memory_order_acq_rel) - 1u;
			if (remaining == 0)
			{
				std::lock_guard<std::mutex> lock(m_queueMutex);
				m_finishCondition.notify_all();
			}
		}
	}
}
