#include "ThreadManager.hpp"

#include <string>
#include <iostream>
#include<syncstream>

namespace Kayou
{
	ThreadManager::ThreadManager(const char* name, const uint8_t numThreads, uint8_t highPriorityNumber) : m_numThreads(numThreads)
	{
		uint8_t numHighPriority = highPriorityNumber;

		bool isHighPriorityNumValid = (numHighPriority <= numThreads) && (numHighPriority > 0);

		numHighPriority = isHighPriorityNumValid ? numHighPriority : numThreads;

		if (!isHighPriorityNumValid || highPriorityNumber == numThreads)
			m_hasOnePriority = true;

		for (uint8_t i = 0u; i < m_numThreads; ++i)
		{
			Priority priority;

			if (numHighPriority > 0)
			{
				priority = Priority::High;
				numHighPriority--;
			}
			else
				priority = Priority::Low;

			m_threads.emplace_back([this, priority]() { CheckQueue(priority); });
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
		m_stop.store(true, std::memory_order_release);
		m_highPriorityWaitCondition.notify_all();
		m_lowPriorityWaitCondition.notify_all();

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

	void ThreadManager::Enqueue(std::function<void()> const& task, const Priority priority)
	{
		m_tasksRemaining.fetch_add(1u, std::memory_order_acq_rel);

		if (priority == Priority::High || m_hasOnePriority)
		{
			std::lock_guard<std::mutex> lock(m_highPriorityQueueMutex);
			m_highPriorityTaskQueue.emplace(task);
			m_highPriorityWaitCondition.notify_one();
		}
		else
		{
			std::lock_guard<std::mutex> lock(m_lowPriorityQueueMutex);
			m_lowPriorityTaskQueue.emplace(task);
			m_lowPriorityWaitCondition.notify_one();
		}
	}

	void ThreadManager::WaitUntilFinished()
	{
		std::unique_lock<std::mutex> lock(m_finishMutex);
		m_finishCondition.wait(lock, [this]() { return m_tasksRemaining.load(std::memory_order_acquire) == 0u; });
	}

	void ThreadManager::CheckQueue(Priority priority)
	{
		while (true)
		{
			std::function<void()> task;

			if (priority == Priority::High)
			{
				if (!ProcessHighPriority(task))
					return;
			}
			else
			{
				if (!ProcessLowPriority(task))
					return;
			}

			try
			{
				task();
			}
			catch (const std::exception& e)
			{
				std::osyncstream(std::cerr) << e.what() << '\n';
			}

			const uint32_t remaining = m_tasksRemaining.fetch_sub(1u, std::memory_order_acq_rel) - 1u;
			if (remaining == 0)
			{
				std::lock_guard<std::mutex> lock(m_finishMutex);
				m_finishCondition.notify_all();
			}
		}
	}

	bool ThreadManager::ProcessHighPriority(std::function<void()>& task)
	{
		std::unique_lock<std::mutex> lock(m_highPriorityQueueMutex);
		m_highPriorityWaitCondition.wait(lock, [this]() { return m_stop.load(std::memory_order_acquire) || !m_highPriorityTaskQueue.empty(); });

		if (m_stop.load(std::memory_order_acquire))
		{
			return false;
		}

		task = std::move(m_highPriorityTaskQueue.front());
		m_highPriorityTaskQueue.pop();
		return true;
	}

	bool ThreadManager::ProcessLowPriority(std::function<void()>& task)
	{
		std::unique_lock<std::mutex> lock(m_lowPriorityQueueMutex);

		if (m_lowPriorityTaskQueue.empty() && m_tasksRemaining.load(std::memory_order_acquire) != 0u)
		{
			lock.unlock();

			return ProcessHighPriority(task);
		}

		m_lowPriorityWaitCondition.wait(lock, [this]() { return m_stop.load(std::memory_order_acquire) || !m_lowPriorityTaskQueue.empty(); });

		if (m_stop.load(std::memory_order_acquire))
		{
			return false;
		}

		task = std::move(m_lowPriorityTaskQueue.front());
		m_lowPriorityTaskQueue.pop();
		return true;
	}
}
