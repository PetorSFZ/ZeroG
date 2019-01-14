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

#include "ZeroG/BackendInterface.hpp"
#include "ZeroG/CpuAllocation.hpp"

#ifdef _WIN32
#include "ZeroG/d3d12/D3D12Backend.hpp"
#endif

// Version information
// ------------------------------------------------------------------------------------------------

ZG_DLL_API uint32_t zgApiVersion(void)
{
	return ZG_COMPILED_API_VERSION;
}

// Compiled features
// ------------------------------------------------------------------------------------------------

ZG_DLL_API ZgFeatureBits zgCompiledFeatures(void)
{
	return
		uint64_t(ZG_FEATURE_BIT_BACKEND_D3D12);
}

// Context
// ------------------------------------------------------------------------------------------------

struct ZgContext final {
	ZgAllocator allocator = {};
	zg::IContext* context = nullptr;
};

ZG_DLL_API ZgErrorCode zgContextCreate(
	ZgContext** contextOut, const ZgContextInitSettings* initSettings)
{
	// Set default allocator if none is specified
	ZgContextInitSettings settings = *initSettings;
	if (settings.allocator.allocate == nullptr || settings.allocator.deallocate == nullptr) {
		settings.allocator = zg::getDefaultAllocator();
	}

	// Allocate context
	ZgContext* context = zg::zgNew<ZgContext>(settings.allocator, "ZeroG Context");
	if (context == nullptr) return ZG_ERROR_CPU_OUT_OF_MEMORY;

	// Set context's allocator
	context->allocator = settings.allocator;

	// Create and allocate requested backend api
	switch (initSettings->backend) {

	case ZG_BACKEND_NONE:
		// TODO: Implement null backend
		zg::zgDelete(settings.allocator, context);
		return ZG_ERROR_UNIMPLEMENTED;

	case ZG_BACKEND_D3D12:
		{
		ZgErrorCode res = zg::createD3D12Backend(&context->context, settings);
			if (res != ZG_SUCCESS) {
				zg::zgDelete(settings.allocator, context);
				return res;
			}
		}
		break;

	default:
		zg::zgDelete(settings.allocator, context);
		return ZG_ERROR_GENERIC;
	}

	// Return context
	*contextOut = context;
	return ZG_SUCCESS;
}

ZG_DLL_API ZgErrorCode zgContextDestroy(ZgContext* context)
{
	if (context == nullptr) return ZG_SUCCESS;

	// Delete API
	zg::zgDelete<zg::IContext>(context->allocator, context->context);

	// Delete context
	ZgAllocator allocator = context->allocator;
	zg::zgDelete<ZgContext>(allocator, context);

	return ZG_SUCCESS;
}

ZG_DLL_API ZgErrorCode zgContextResize(ZgContext* context, uint32_t width, uint32_t height)
{
	return context->context->resize(width, height);
}

// Pipeline
// ------------------------------------------------------------------------------------------------

// Note: A ZgPipeline struct does not really exist. It's just an alias for the internal
// zg::IPipeline currently. This may (or may not) change in the future.

ZG_DLL_API ZgErrorCode zgPipelineRenderingCreate(
	ZgContext* context,
	ZgPipelineRendering** pipelineOut,
	const ZgPipelineRenderingCreateInfo* createInfo)
{
	// Check arguments
	if (createInfo == nullptr) return ZG_ERROR_INVALID_ARGUMENT;
	if (createInfo->vertexShaderPath == nullptr) return ZG_ERROR_INVALID_ARGUMENT;
	if (createInfo->vertexShaderEntry == nullptr) return ZG_ERROR_INVALID_ARGUMENT;
	if (createInfo->pixelShaderPath == nullptr) return ZG_ERROR_INVALID_ARGUMENT;
	if (createInfo->pixelShaderEntry == nullptr) return ZG_ERROR_INVALID_ARGUMENT;
	if (createInfo->shaderVersion == ZG_SHADER_MODEL_UNDEFINED) return ZG_ERROR_INVALID_ARGUMENT;

	zg::IPipelineRendering* pipeline = nullptr;
	ZgErrorCode res = context->context->pipelineCreate(&pipeline, *createInfo);
	if (res != ZG_SUCCESS) return res;
	*pipelineOut = reinterpret_cast<ZgPipelineRendering*>(pipeline);
	return ZG_SUCCESS;
}

ZG_DLL_API ZgErrorCode zgPipelineRenderingRelease(
	ZgContext* context,
	ZgPipelineRendering* pipeline)
{
	return context->context->pipelineRelease(reinterpret_cast<zg::IPipelineRendering*>(pipeline));
}

// Memory
// ------------------------------------------------------------------------------------------------

ZG_DLL_API ZgErrorCode zgMemoryHeapCreate(
	ZgContext* context,
	ZgMemoryHeap** memoryHeapOut,
	const ZgMemoryHeapCreateInfo* createInfo)
{
	if (createInfo == nullptr) return ZG_ERROR_INVALID_ARGUMENT;
	if (createInfo->sizeInBytes == 0) return ZG_ERROR_INVALID_ARGUMENT;

	zg::IMemoryHeap* memoryHeap = nullptr;
	ZgErrorCode res = context->context->memoryHeapCreate(&memoryHeap, *createInfo);
	if (res != ZG_SUCCESS) return res;
	*memoryHeapOut = reinterpret_cast<ZgMemoryHeap*>(memoryHeap);
	return ZG_SUCCESS;
}

ZG_DLL_API ZgErrorCode zgMemoryHeapRelease(
	ZgContext* context,
	ZgMemoryHeap* memoryHeap)
{
	return context->context->memoryHeapRelease(reinterpret_cast<zg::IMemoryHeap*>(memoryHeap));
}

ZG_DLL_API ZgErrorCode zgBufferCreate(
	ZgContext* context,
	ZgMemoryHeap* memoryHeap,
	ZgBuffer** bufferOut,
	const ZgBufferCreateInfo* createInfo)
{
	// TODO: Implement
	return ZG_ERROR_UNIMPLEMENTED;
}

ZG_DLL_API ZgErrorCode zgBufferRelease(
	ZgContext* context,
	ZgMemoryHeap* memoryHeap,
	ZgBuffer* bufferOut)
{
	// TODO: Implement
	return ZG_ERROR_UNIMPLEMENTED;
}

// Experimental
// ------------------------------------------------------------------------------------------------

ZG_DLL_API ZgErrorCode zgRenderExperiment(ZgContext* context)
{
	return context->context->renderExperiment();
}
