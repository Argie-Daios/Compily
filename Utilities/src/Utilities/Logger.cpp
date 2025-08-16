#include "Logger.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Utilities
{
	Logger::Logger(const std::string& name, const std::string& format)
	{
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console_sink->set_pattern(format);
		m_Logger = std::make_shared<spdlog::logger>(name, console_sink);
	}
}