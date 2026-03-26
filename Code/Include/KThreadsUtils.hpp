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
}