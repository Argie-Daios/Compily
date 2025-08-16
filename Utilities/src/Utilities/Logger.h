#pragma once

#include <spdlog/spdlog.h>
#include <memory>

namespace Utilities
{
	class Logger
	{
	public:
		Logger(const std::string& name, const std::string& format = "%^[%n]: %v%$");

		template<typename ... Args>
		void Trace(Args&& ... args)
		{
			m_Logger->trace(std::forward<Args>(args)...);
		}

		template<typename ... Args>
		void Info(Args&& ... args)
		{
			m_Logger->info(std::forward<Args>(args)...);
		}

		template<typename ... Args>
		void Warn(Args&& ... args)
		{
			m_Logger->warn(std::forward<Args>(args)...);
		}

		template<typename ... Args>
		void Error(Args&& ... args)
		{
			m_Logger->error(std::forward<Args>(args)...);
		}

		template<typename ... Args>
		void Fatal(Args&& ... args)
		{
			m_Logger->critical(std::forward<Args>(args)...);
		}
	private:
		std::shared_ptr<spdlog::logger> m_Logger;
	};
}