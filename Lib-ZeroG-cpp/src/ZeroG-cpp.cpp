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

#include "ZeroG-cpp.hpp"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <utility>

namespace zg {


// Statics
// ------------------------------------------------------------------------------------------------

static const char* stripFilePath(const char* file) noexcept
{
	const char* strippedFile1 = std::strrchr(file, '\\');
	const char* strippedFile2 = std::strrchr(file, '/');
	if (strippedFile1 == nullptr && strippedFile2 == nullptr) {
		return file;
	}
	else if (strippedFile2 == nullptr) {
		return strippedFile1 + 1;
	}
	else {
		return strippedFile2 + 1;
	}
}

// Error handling helpers
// ------------------------------------------------------------------------------------------------

ZgErrorCode CheckZgImpl::operator% (ZgErrorCode result) noexcept
{
	assert(result == ZG_SUCCESS);
	if (result == ZG_SUCCESS) return ZG_SUCCESS;
	printf("%s:%i: ZeroG error: %s\n", stripFilePath(file), line, zgErrorCodeToString(result));
	return result;
}

// Context
// ------------------------------------------------------------------------------------------------

ZgErrorCode Context::init(const ZgContextInitSettings& settings)
{
	this->destroy();
	ZgErrorCode res = zgContextInit(&settings);
	mInitialized = res == ZG_SUCCESS;
	return res;
}

void Context::swap(Context& other) noexcept
{
	std::swap(mInitialized, other.mInitialized);
}

void Context::destroy() noexcept
{
	if (mInitialized) zgContextDeinit();
	mInitialized = false;
}

} // namespace zg
