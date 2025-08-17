#pragma once

#include <Utilities.h>

#define PARSY_LOG_TRACE(...) Utilities::Logger::Trace("Parsy", __VA_ARGS__)
#define PARSY_LOG_INFO(...) Utilities::Logger::Info("Parsy", __VA_ARGS__)
#define PARSY_LOG_WARN(...) Utilities::Logger::Warn("Parsy", __VA_ARGS__)
#define PARSY_LOG_ERROR(...) Utilities::Logger::Error("Parsy", __VA_ARGS__)
#define PARSY_LOG_FATAL(...) Utilities::Logger::Fatal("Parsy", __VA_ARGS__)

#ifdef COMPILY_DEBUG
	#define PARSY_LOG_TRACE_DEBUG(...) Utilities::Logger::Trace("Parsy", __VA_ARGS__)
	#define PARSY_LOG_INFO_DEBUG(...) Utilities::Logger::Info("Parsy", __VA_ARGS__)
	#define PARSY_LOG_WARN_DEBUG(...) Utilities::Logger::Warn("Parsy", __VA_ARGS__)
	#define PARSY_LOG_ERROR_DEBUG(...) Utilities::Logger::Error("Parsy", __VA_ARGS__)
	#define PARSY_LOG_FATAL_DEBUG(...) Utilities::Logger::Fatal("Parsy", __VA_ARGS__)
#else
	#define PARSY_LOG_TRACE_DEBUG(...)
	#define PARSY_LOG_INFO_DEBUG(...)
	#define PARSY_LOG_WARN_DEBUG(...)
	#define PARSY_LOG_ERROR_DEBUG(...)
	#define PARSY_LOG_FATAL_DEBUG(...)
#endif