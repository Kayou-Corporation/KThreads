#pragma once
#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>

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
