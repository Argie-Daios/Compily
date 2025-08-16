#pragma once

#include <Utilities.h>

inline static Utilities::Logger s_ParsyLogger("Parsy");

#define PARSY_LOG_TRACE(...) s_ParsyLogger.Trace(__VA_ARGS__)
#define PARSY_LOG_INFO(...) s_ParsyLogger.Info(__VA_ARGS__)
#define PARSY_LOG_WARN(...) s_ParsyLogger.Warn(__VA_ARGS__)
#define PARSY_LOG_ERROR(...) s_ParsyLogger.Error(__VA_ARGS__)
#define PARSY_LOG_FATAL(...) s_ParsyLogger.Fatal(__VA_ARGS__)

#ifdef COMPILY_DEBUG
	#define PARSY_LOG_TRACE_DEBUG(...) s_ParsyLogger.Trace(__VA_ARGS__)
	#define PARSY_LOG_INFO_DEBUG(...) s_ParsyLogger.Info(__VA_ARGS__)
	#define PARSY_LOG_WARN_DEBUG(...) s_ParsyLogger.Warn(__VA_ARGS__)
	#define PARSY_LOG_ERROR_DEBUG(...) s_ParsyLogger.Error(__VA_ARGS__)
	#define PARSY_LOG_FATAL_DEBUG(...) s_ParsyLogger.Fatal(__VA_ARGS__)
#else
	#define PARSY_LOG_TRACE_DEBUG(...)
	#define PARSY_LOG_INFO_DEBUG(...)
	#define PARSY_LOG_WARN_DEBUG(...)
	#define PARSY_LOG_ERROR_DEBUG(...)
	#define PARSY_LOG_FATAL_DEBUG(...)
#endif