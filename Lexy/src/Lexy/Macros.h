#pragma once

#include <Utilities.h>

inline static Utilities::Logger s_LexyLogger("Lexy");

#define LEXY_LOG_TRACE(...) s_LexyLogger.Trace(__VA_ARGS__)
#define LEXY_LOG_INFO(...) s_LexyLogger.Info(__VA_ARGS__)
#define LEXY_LOG_WARN(...) s_LexyLogger.Warn(__VA_ARGS__)
#define LEXY_LOG_ERROR(...) s_LexyLogger.Error(__VA_ARGS__)
#define LEXY_LOG_FATAL(...) s_LexyLogger.Fatal(__VA_ARGS__)

#ifdef COMPILY_DEBUG
	#define LEXY_LOG_TRACE_DEBUG(...) s_LexyLogger.Trace(__VA_ARGS__)
	#define LEXY_LOG_INFO_DEBUG(...) s_LexyLogger.Info(__VA_ARGS__)
	#define LEXY_LOG_WARN_DEBUG(...) s_LexyLogger.Warn(__VA_ARGS__)
	#define LEXY_LOG_ERROR_DEBUG(...) s_LexyLogger.Error(__VA_ARGS__)
	#define LEXY_LOG_FATAL_DEBUG(...) s_LexyLogger.Fatal(__VA_ARGS__)
#else
	#define LEXY_LOG_TRACE_DEBUG(...)
	#define LEXY_LOG_INFO_DEBUG(...)
	#define LEXY_LOG_WARN_DEBUG(...)
	#define LEXY_LOG_ERROR_DEBUG(...)
	#define LEXY_LOG_FATAL_DEBUG(...)
#endif