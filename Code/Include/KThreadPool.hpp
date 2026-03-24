#pragma once
#include <functional>
#include <memory>
#include <unordered_map>

#include "KThreadManager.hpp"

namespace Kayou
{
	class KThreadPool
	{
	public:
		KThreadPool() = default;
		~KThreadPool();

		void InitQueue(const char* queueName, uint8_t numThreads);
		void EnqueueTask(const char* queueName, std::function<void()> const& task) const;
		void WaitUntilQueueFinished(const char* queueName) const;
		void WaitUntilAllFinished() const;
		void ReleaseQueue(const char* queueName);

	private:
		std::unordered_map<const char*, std::unique_ptr<KThreadManager>> m_threadManagers;
	};
}