#pragma once
#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

#if defined(_WIN32)
#include <windows.h>

inline void SetThreadName(std::thread& t, const std::wstring& name) {
	SetThreadDescription(t.native_handle(), name.c_str());
}
#endif
#if defined(__linux__)
#include <pthread.h>

inline void SetThreadName(std::thread& t, const std::string& name) {
	pthread_setname_np(t.native_handle(), name.c_str());
}
#endif

namespace Kayou
{
	class KThreadManager
	{
	public:
		KThreadManager(const char* name, uint8_t numThreads);
		~KThreadManager();

		void Enqueue(std::function<void()> const& task);
		void WaitUntilFinished();

	private:
		std::vector<std::thread> m_threads;
		std::queue<std::function<void()>> m_taskQueue;
		std::mutex m_queueMutex;
		std::condition_variable m_waitCondition, m_finishCondition;
		std::atomic<uint32_t> m_tasksRemaining{ 0 };
		std::atomic<bool> m_stop{ false };
		uint8_t m_numThreads;

		void CheckQueue();
	};
}
