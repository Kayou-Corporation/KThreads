#include "Thread.hpp"

#include <string>
#include <iostream>
#include <syncstream>

namespace Kayou
{
	Thread::Thread(const char* name)
	{
		m_thread = std::thread(&Thread::CheckQueue, this);

		std::string threadName = std::string(name) + "Thread";
		SetThreadName(m_thread, threadName);
	}

	Thread::~Thread()
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_stop = true;
		}

		m_waitCondition.notify_all();

		if (m_thread.joinable())
		{
			m_thread.join();
		}

		while (!m_taskQueue.empty())
			m_taskQueue.pop();
	}

	void Thread::Enqueue(std::move_only_function<void()> task)
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			m_taskQueue.emplace(std::move(task));

			m_waitCondition.notify_one();
		}

		m_tasksRemaining.fetch_add(1u, std::memory_order_release);
	}

	void Thread::WaitUntilFinished()
	{
		std::unique_lock<std::mutex> lock(m_finishMutex);
		m_finishCondition.wait(lock, [this]() { return m_tasksRemaining.load(std::memory_order_acquire) == 0u; });
	}

	void Thread::CheckQueue()
	{
		while (true)
		{
			std::move_only_function<void()> task;

			{
				std::unique_lock<std::mutex> lock(m_mutex);
				m_waitCondition.wait(lock, [this] { return m_stop || !m_taskQueue.empty(); });

				if (m_stop && m_taskQueue.empty())
					return;

				task = std::move(m_taskQueue.front());
				m_taskQueue.pop();
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
}