#include "ThreadManager.hpp"

#include <string>
#include <iostream>
#include <syncstream>

namespace Kayou
{
	ThreadManager::ThreadManager(const char* name, const uint8_t numThreads) : m_numThreads(numThreads)
	{
		for (uint8_t i = 0u; i < m_numThreads; ++i)
		{
			m_threads.emplace_back(&ThreadManager::CheckQueue, this);
			std::string threadName = std::string(name) + "Thread";

			if (m_numThreads > 1u)
			{
				threadName += std::to_string(i);
			}

			SetThreadName(m_threads[i], threadName);
		}
	}

	ThreadManager::~ThreadManager()
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_stop = true;
		}

		m_waitCondition.notify_all();

		for (uint8_t i = 0u; i < m_numThreads; ++i)
		{
			if (m_threads[i].joinable())
			{
				m_threads[i].join();
			}
		}
		m_threads.clear();

		while (!m_highPriorityTaskQueue.empty())
			m_highPriorityTaskQueue.pop();

		while (!m_lowPriorityTaskQueue.empty())
			m_lowPriorityTaskQueue.pop();
	}

	void ThreadManager::Enqueue(std::move_only_function<void()> task, const Priority priority)
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			if (priority == Priority::High)
			{
				m_highPriorityTaskQueue.emplace(std::move(task));
			}
			else
			{
				m_lowPriorityTaskQueue.emplace(std::move(task));
			}

			m_waitCondition.notify_one();
		}

		m_tasksRemaining.fetch_add(1u, std::memory_order_release);
	}

	void ThreadManager::WaitUntilFinished()
	{
		std::unique_lock<std::mutex> lock(m_finishMutex);
		m_finishCondition.wait(lock, [this]() { return m_tasksRemaining.load(std::memory_order_acquire) == 0u; });
	}

	void ThreadManager::CheckQueue()
	{
		while (true)
		{
			std::move_only_function<void()> task;

			{
				std::unique_lock<std::mutex> lock(m_mutex);
				m_waitCondition.wait(lock, [this] { return m_stop || !m_highPriorityTaskQueue.empty() || !m_lowPriorityTaskQueue.empty(); });

				if (m_stop && m_highPriorityTaskQueue.empty() && m_lowPriorityTaskQueue.empty())
					return;

				if (!m_highPriorityTaskQueue.empty())
				{
					task = std::move(m_highPriorityTaskQueue.front());
					m_highPriorityTaskQueue.pop();
				}
				else
				{
					task = std::move(m_lowPriorityTaskQueue.front());
					m_lowPriorityTaskQueue.pop();
				}
			}

			try
			{
				task();
			}
			catch (const std::exception& e)
			{
				std::osyncstream(std::cerr) << e.what() << '\n';
			}

			//const uint32_t remaining = m_tasksRemaining.fetch_sub(1u, std::memory_order_acq_rel) - 1u;
			if (--m_tasksRemaining == 0)
			{
				std::lock_guard<std::mutex> lock(m_finishMutex);
				m_finishCondition.notify_all();
			}
		}
	}
}