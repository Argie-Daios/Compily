#pragma once

#include <Utilities.h>

#define LEXY_LOG_TRACE(...) Utilities::Logger::Trace("Lexy", __VA_ARGS__)
#define LEXY_LOG_INFO(...) Utilities::Logger::Info("Lexy", __VA_ARGS__)
#define LEXY_LOG_WARN(...) Utilities::Logger::Warn("Lexy", __VA_ARGS__)
#define LEXY_LOG_ERROR(...) Utilities::Logger::Error("Lexy", __VA_ARGS__)
#define LEXY_LOG_FATAL(...) Utilities::Logger::Fatal("Lexy", __VA_ARGS__)

#ifdef COMPILY_DEBUG
	#define LEXY_LOG_TRACE_DEBUG(...) Utilities::Logger::Trace("Lexy", __VA_ARGS__)
	#define LEXY_LOG_INFO_DEBUG(...) Utilities::Logger::Info("Lexy", __VA_ARGS__)
	#define LEXY_LOG_WARN_DEBUG(...) Utilities::Logger::Warn("Lexy", __VA_ARGS__)
	#define LEXY_LOG_ERROR_DEBUG(...) Utilities::Logger::Error("Lexy", __VA_ARGS__)
	#define LEXY_LOG_FATAL_DEBUG(...) Utilities::Logger::Fatal("Lexy", __VA_ARGS__)
#else
	#define LEXY_LOG_TRACE_DEBUG(...)
	#define LEXY_LOG_INFO_DEBUG(...)
	#define LEXY_LOG_WARN_DEBUG(...)
	#define LEXY_LOG_ERROR_DEBUG(...)
	#define LEXY_LOG_FATAL_DEBUG(...)
#endif