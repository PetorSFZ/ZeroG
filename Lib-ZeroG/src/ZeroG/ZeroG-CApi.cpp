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

#define ZG_DLL_EXPORT
#include "ZeroG/ZeroG-CApi.h"

#include "ZeroG/Api.hpp"
#include "ZeroG/CpuAllocation.hpp"

#ifdef _WIN32
#include "ZeroG/d3d12/D3D12Api.hpp"
#endif

// Version information
// ------------------------------------------------------------------------------------------------

ZG_DLL_API uint32_t zgApiVersion(void)
{
	return ZG_COMPILED_API_VERSION;
}

// Backends enums and queries
// ------------------------------------------------------------------------------------------------

ZG_DLL_API ZG_BOOL zgBackendCompiled(ZgBackendType backendType)
{
	return ZG_TRUE;
}

// Context
// ------------------------------------------------------------------------------------------------

struct ZgContext {

};

ZG_DLL_API ZgErrorCode zgCreateContext(
	ZgContext** contextOut, const ZgContextInitSettings* settings)
{
	return ZG_SUCCESS;
}

ZG_DLL_API ZgErrorCode zgDestroyContext(ZgContext* context)
{
	if (context == nullptr) return ZG_SUCCESS;



	return ZG_SUCCESS;
}
