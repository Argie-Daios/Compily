#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>

namespace Utilities
{
	class Logger
	{
	public:
		inline static const std::shared_ptr<spdlog::logger>& Register(const std::string& name, const std::string& label,
			const std::string format = "%^[%n](%l): %v%$")
		{
			auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
			console_sink->set_pattern(format);
			s_Loggers[name] = std::make_shared<spdlog::logger>(label, console_sink);
			return s_Loggers[name];
		}

		template<typename ... Args>
		inline static void Trace(const std::string& name, Args&& ... args)
		{
			const auto& logger = GetLogger(name);
			logger->trace(std::forward<Args>(args)...);
		}

		template<typename ... Args>
		inline static void Info(const std::string& name, Args&& ... args)
		{
			const auto& logger = GetLogger(name);
			logger->info(std::forward<Args>(args)...);
		}

		template<typename ... Args>
		inline static void Warn(const std::string& name, Args&& ... args)
		{
			const auto& logger = GetLogger(name);
			logger->warn(std::forward<Args>(args)...);
		}

		template<typename ... Args>
		inline static void Error(const std::string& name, Args&& ... args)
		{
			const auto& logger = GetLogger(name);
			logger->error(std::forward<Args>(args)...);
		}

		template<typename ... Args>
		inline static void Fatal(const std::string& name, Args&& ... args)
		{
			const auto& logger = GetLogger(name);
			logger->critical(std::forward<Args>(args)...);
		}
	private:
		inline static const std::shared_ptr<spdlog::logger>& GetLogger(const std::string& name)
		{
			auto& it = s_Loggers.find(name);
			if (it != s_Loggers.end())
				return it->second;
			return Register(name, name);
		}
	private:
		inline static std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> s_Loggers;
	};
}