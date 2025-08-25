#pragma once

#include <chrono>

namespace Utilities
{
	namespace Time
	{
		struct TimerHandle
		{
			std::chrono::time_point<std::chrono::high_resolution_clock> StartTimeStamp;
			std::chrono::time_point<std::chrono::high_resolution_clock> EndTimeStamp;

			TimerHandle() = default;
			TimerHandle(const TimerHandle&) = default;
			TimerHandle(TimerHandle&&) = default;

			void Start()
			{
				StartTimeStamp = std::chrono::high_resolution_clock::now();
			}

			void End()
			{
				EndTimeStamp = std::chrono::high_resolution_clock::now();
			}
			long long GetTimeElapsed() const
			{
				long long start = std::chrono::time_point_cast<std::chrono::milliseconds>(StartTimeStamp)
					.time_since_epoch().count();
				long long end = std::chrono::time_point_cast<std::chrono::milliseconds>(EndTimeStamp)
					.time_since_epoch().count();
				return end - start;
			}

			TimerHandle& operator=(const TimerHandle& other)
			{
				if (this == &other) return *this;
				this->StartTimeStamp = StartTimeStamp;
				this->EndTimeStamp = EndTimeStamp;
				return *this;
			}

			TimerHandle& operator=(TimerHandle&& other) noexcept
			{
				if (this == &other) return *this;
				this->StartTimeStamp = std::move(StartTimeStamp);
				this->EndTimeStamp = std::move(EndTimeStamp);
				return *this;
			}
		};

		template<typename Function, typename ... Args>
		inline TimerHandle BenchmarkRoutine(Function&& function, Args&& ... args)
		{
			TimerHandle handle;
			handle.Start();
			function(std::forward<Args>(args)...);
			handle.End();
			return handle;
		}
	}
}