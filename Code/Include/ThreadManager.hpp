#pragma once
#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <condition_variable>
#include <cmath>

#if defined(_WIN32)
#include <windows.h>

inline void SetThreadName(std::thread& t, const std::string& name) 
{
	SetThreadDescription(t.native_handle(), std::wstring(name.begin(), name.end()).c_str());
}
#endif
#if defined(__linux__)
#include <pthread.h>

inline void SetThreadName(std::thread& t, const std::string& name) 
{
	pthread_setname_np(t.native_handle(), name.c_str());
}
#endif

namespace Kayou
{
	enum class Priority : uint8_t
	{
		High = 0u,
		Low
	};

	class ThreadManager
	{
	public:
		ThreadManager(const char* name, uint8_t numThreads, uint8_t highPriorityNumber = 0);
		~ThreadManager();

		void Enqueue(std::function<void()> const& task, Priority priority = Priority::High);
		void WaitUntilFinished();

	private:
		std::vector<std::thread> m_threads;

		std::queue<std::function<void()>> m_highPriorityTaskQueue;
		std::queue<std::function<void()>> m_lowPriorityTaskQueue;

		std::mutex m_highPriorityQueueMutex;
		std::mutex m_lowPriorityQueueMutex;
		std::mutex m_finishMutex;

		std::condition_variable m_highPriorityWaitCondition, m_lowPriorityWaitCondition, m_finishCondition;
		std::atomic<uint32_t> m_tasksRemaining{ 0 };
		std::atomic<bool> m_stop{ false };
		uint8_t m_numThreads;
		bool m_hasOnePriority = false;

		void CheckQueue(Priority priority);
		bool ProcessHighPriority(std::function<void()>& task);
		bool ProcessLowPriority(std::function<void()>& task);
	};
}
