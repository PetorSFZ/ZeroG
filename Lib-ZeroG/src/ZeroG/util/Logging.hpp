// Copyright (c) Peter Hillerström (skipifzero.com, peter@hstroem.se)
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#pragma once

#include "ZeroG.h"
#include "ZeroG/Context.hpp"

namespace zg {

// Logging macros
// ------------------------------------------------------------------------------------------------

#define ZG_LOG(logger, logLevel, format, ...) zg::logWrapper( \
	(logger), __FILE__, __LINE__, (logLevel), (format), ##__VA_ARGS__)

#define ZG_INFO(logger, format, ...) ZG_LOG((logger), ZG_LOG_LEVEL_INFO, (format), ##__VA_ARGS__)

#define ZG_WARNING(logger, format, ...) ZG_LOG((logger), ZG_LOG_LEVEL_WARNING, (format), ##__VA_ARGS__)

#define ZG_ERROR(logger, format, ...) ZG_LOG((logger), ZG_LOG_LEVEL_ERROR, (format), ##__VA_ARGS__)

// Logger wrappers for logging macros
// ------------------------------------------------------------------------------------------------

void logWrapper(
	ZgContext& ctx, const char* file, int line, ZgLogLevel level, const char* fmt, ...) noexcept;

void logWrapper(
	ZgLogger& logger, const char* file, int line, ZgLogLevel level, const char* fmt, ...) noexcept;

// Default logger
// ------------------------------------------------------------------------------------------------

ZgLogger getDefaultLogger() noexcept;

} // namespace zg