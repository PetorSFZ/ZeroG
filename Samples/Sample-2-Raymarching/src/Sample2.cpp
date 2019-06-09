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

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstdio>

#include <SDL.h>
#include <SDL_syswm.h>

#include "ZeroG-cpp.hpp"

#include "Cube.hpp"
#include "SampleCommon.hpp"

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#endif

// Settings
// ------------------------------------------------------------------------------------------------

constexpr bool DEBUG_MODE = true;

// Helpers
// ------------------------------------------------------------------------------------------------

struct Vertex {
	float position[3];
	float normal[3];
	float texcoord[2];
};
static_assert(sizeof(Vertex) == sizeof(float) * 8, "Vertex is padded");

// A simple helper function that allocates a heap and a buffer that covers the entirety of it.
// In practice you want to have multiple buffers per heap and use some sort of allocation scheme,
// but this is good enough for this sample.
static void allocateMemoryHeapAndBuffer(
	ZgContext* context,
	ZgMemoryHeap*& heapOut,
	ZgBuffer*& bufferOut,
	ZgMemoryType memoryType,
	uint64_t numBytes) noexcept
{
	// Create heap
	ZgMemoryHeapCreateInfo heapInfo = {};
	heapInfo.memoryType = memoryType;
	heapInfo.sizeInBytes = numBytes;

	ZgMemoryHeap* heap = nullptr;
	CHECK_ZG zgMemoryHeapCreate(context, &heap, &heapInfo);

	// Create buffer
	ZgBufferCreateInfo bufferInfo = {};
	bufferInfo.offsetInBytes = 0;
	bufferInfo.sizeInBytes = numBytes;
	
	ZgBuffer* buffer = nullptr;
	CHECK_ZG zgMemoryHeapBufferCreate(heap, &buffer, &bufferInfo);

	heapOut = heap;
	bufferOut = buffer;
}

// A simple helper function that allocates and copies data to a device buffer In practice you will
// likely want to do something smarter.
static void createDeviceBufferSimpleBlocking(
	ZgContext* context,
	ZgCommandQueue* queue,
	ZgBuffer*& bufferOut,
	ZgMemoryHeap*& memoryHeapOut,
	const void* data,
	uint64_t numBytes,
	uint64_t bufferSizeBytes = 0) noexcept
{
	// Create temporary upload buffer (accessible from CPU)
	ZgMemoryHeap* uploadHeap = nullptr;
	ZgBuffer* uploadBuffer = nullptr;
	allocateMemoryHeapAndBuffer(context, uploadHeap, uploadBuffer,
		ZG_MEMORY_TYPE_UPLOAD, bufferSizeBytes != 0 ? bufferSizeBytes : numBytes);

	// Copy cube vertices to upload buffer
	CHECK_ZG zgBufferMemcpyTo(context, uploadBuffer, 0, data, numBytes);

	// Create device buffer
	ZgMemoryHeap* deviceHeap = nullptr;
	ZgBuffer* deviceBuffer = nullptr;
	allocateMemoryHeapAndBuffer(context, deviceHeap, deviceBuffer,
		ZG_MEMORY_TYPE_DEVICE, bufferSizeBytes != 0 ? bufferSizeBytes : numBytes);

	// Copy to the device buffer
	ZgCommandList* commandList = nullptr;
	CHECK_ZG zgCommandQueueBeginCommandListRecording(queue, &commandList);
	CHECK_ZG zgCommandListMemcpyBufferToBuffer(
		commandList, deviceBuffer, 0, uploadBuffer, 0, numBytes);
	CHECK_ZG zgCommandQueueExecuteCommandList(queue, commandList);
	CHECK_ZG zgCommandQueueFlush(queue);

	// Release upload heap and buffer
	CHECK_ZG zgMemoryHeapBufferRelease(uploadHeap, uploadBuffer);
	CHECK_ZG zgMemoryHeapRelease(context, uploadHeap);

	bufferOut = deviceBuffer;
	memoryHeapOut = deviceHeap;
}

using time_point = std::chrono::high_resolution_clock::time_point;

// Helper functions that calculates the time in seconds since the last call
static float calculateDelta(time_point& previousTime) noexcept
{
	time_point currentTime = std::chrono::high_resolution_clock::now();

	using FloatSecond = std::chrono::duration<float>;
	float delta = std::chrono::duration_cast<FloatSecond>(currentTime - previousTime).count();

	previousTime = currentTime;
	return delta;
}

struct FullscreenVertex {
	float pos[2];
	float coord[2];
};
static_assert(sizeof(FullscreenVertex) == sizeof(float) * 4, "FullscreenVertex is padded");

static ZgPipelineRendering* compileRaymarchingPipeline(ZgContext* ctx) noexcept
{
	ZgPipelineRenderingCreateInfo pipelineInfo = {};
	pipelineInfo.vertexShaderPath = "res/Sample-2-Raymarching/raymarching.hlsl";
	pipelineInfo.vertexShaderEntry = "VSMain";

	pipelineInfo.pixelShaderPath = "res/Sample-2-Raymarching/raymarching.hlsl";
	pipelineInfo.pixelShaderEntry = "PSMain";

	pipelineInfo.shaderVersion = ZG_SHADER_MODEL_6_2;
	pipelineInfo.dxcCompilerFlags[0] = "-Zi";
	pipelineInfo.dxcCompilerFlags[1] = "-O4";

	pipelineInfo.numVertexAttributes = 2;

	// "position"
	pipelineInfo.vertexAttributes[0].location = 0;
	pipelineInfo.vertexAttributes[0].vertexBufferSlot = 0;
	pipelineInfo.vertexAttributes[0].offsetToFirstElementInBytes = offsetof(FullscreenVertex, pos);
	pipelineInfo.vertexAttributes[0].type = ZG_VERTEX_ATTRIBUTE_F32_2;

	// "coord"
	pipelineInfo.vertexAttributes[1].location = 1;
	pipelineInfo.vertexAttributes[1].vertexBufferSlot = 0;
	pipelineInfo.vertexAttributes[1].offsetToFirstElementInBytes = offsetof(FullscreenVertex, coord);
	pipelineInfo.vertexAttributes[1].type = ZG_VERTEX_ATTRIBUTE_F32_2;

	pipelineInfo.numVertexBufferSlots = 1;
	pipelineInfo.vertexBufferStridesBytes[0] = sizeof(FullscreenVertex);

	pipelineInfo.numPushConstants = 1;
	pipelineInfo.pushConstantRegisters[0] = 0;

	pipelineInfo.numSamplers = 0;

	ZgPipelineRendering* pipeline = nullptr;
	ZgPipelineRenderingSignature signature = {};
	CHECK_ZG zgPipelineRenderingCreate(ctx, &pipeline, &signature, &pipelineInfo);

	return pipeline;
}

static void attemptReloadRaymarchingPipeline(ZgContext* ctx, ZgCommandQueue* queue, ZgPipelineRendering*& pipeline) noexcept
{
	ZgPipelineRendering* newPipeline = compileRaymarchingPipeline(ctx);
	if (newPipeline == nullptr) {
		printf("\n\nFailed to compile pipeline, using old one\n\n");
		return;
	}

	CHECK_ZG zgCommandQueueFlush(queue);
	CHECK_ZG zgPipelineRenderingRelease(ctx, pipeline);
	pipeline = newPipeline;
}

// Main
// ------------------------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	// Enable hi-dpi awareness on Windows
#ifdef _WIN32
	SetProcessDPIAware();

	// Set current working directory to SDL_GetBasePath()
	char* basePath = SDL_GetBasePath();
	_chdir(basePath);
	SDL_free(basePath);
#endif

	// Initialize SDL2 and create a window
	SDL_Window* window = initializeSdl2CreateWindow("ZeroG - Sample2 - Raymarching");


	// Create ZeroG context
	ZgContextInitSettings initSettings = {};
	initSettings.backend = ZG_BACKEND_D3D12;
	initSettings.width = 512;
	initSettings.height = 512;
	initSettings.debugMode = DEBUG_MODE ? ZG_TRUE : ZG_FALSE;
	initSettings.nativeWindowHandle = getNativeWindowHandle(window);
	zg::Context ctx;
	CHECK_ZG ctx.init(initSettings);


	// Get the command queue
	ZgCommandQueue* commandQueue = nullptr;
	CHECK_ZG zgContextGeCommandQueueGraphicsPresent(ctx.mContext, &commandQueue);


	// Create a rendering pipeline
	ZgPipelineRendering* raymarchingPipeline = compileRaymarchingPipeline(ctx.mContext);
	if (raymarchingPipeline == nullptr) {
		printf("Could not compile raymarching pipeline!\n");
		return EXIT_FAILURE;
	}

	FullscreenVertex fullscreenVertices[4] = 
	{
		{ {-1.0f, -1.0f}, {-0.5f, -0.5f} }, // Bottom left
		{ {-1.0f, 1.0f}, {-0.5f, 0.5f} }, // Top left
		{ {1.0f, -1.0f}, {0.5f, -0.5f} }, // Bottom right
		{ {1.0f, 1.0f}, {0.5f, 0.5f} } // Top right
	};
	
	uint32_t fullscreenIndices[6] = {
		0, 1, 2,
		1, 3, 2
	};

	ZgBuffer* fullscreenVerticesDevice = nullptr;
	ZgMemoryHeap* fullscreenVerticesHeap = nullptr;
	createDeviceBufferSimpleBlocking(ctx.mContext, commandQueue, fullscreenVerticesDevice,
		fullscreenVerticesHeap, fullscreenVertices, sizeof(FullscreenVertex) * 4);

	ZgBuffer* fullscreenIndicesDevice = nullptr;
	ZgMemoryHeap* fullscreenIndicesHeap = nullptr;
	createDeviceBufferSimpleBlocking(ctx.mContext, commandQueue, fullscreenIndicesDevice,
		fullscreenIndicesHeap, fullscreenIndices, sizeof(uint32_t) * 6);

	
	// Run our main loop
	time_point previousTimePoint;
	calculateDelta(previousTimePoint);
	float timeSinceStart = 0.0f;
	bool running = true;
	while (running) {

		// Query SDL events, loop wrapped in a lambda so we can continue to next iteration of main
		// loop. "return false;" == continue to next iteration
		if (![&]() -> bool {
			SDL_Event event = {};
			while (SDL_PollEvent(&event) != 0) {
				switch (event.type) {

				case SDL_QUIT:
					running = false;
					return false;

				case SDL_KEYUP:
					if (event.key.keysym.sym == SDLK_ESCAPE) {
						running = false;
						return false;
					}
					else {
						attemptReloadRaymarchingPipeline(ctx.mContext, commandQueue, raymarchingPipeline);
					}
					break;
				}
			}
			return true;
		}()) continue;

		// Update time since start
		timeSinceStart += calculateDelta(previousTimePoint);

		// Query drawable width and height and update ZeroG context
		int width = 0;
		int height = 0;
		SDL_GL_GetDrawableSize(window, &width, &height);
		CHECK_ZG zgContextResize(ctx.mContext, uint32_t(width), uint32_t(height));

		// Create view and projection matrices
		float vertFovDeg = 40.0f;
		float aspectRatio = float(width) / float(height);
		Vector origin = Vector(
			std::cos(timeSinceStart) * 5.0f,
			std::sin(timeSinceStart * 0.75f) + 1.5f,
			std::sin(timeSinceStart) * 5.0f);
		Matrix viewMatrix = createViewMatrix(
			origin, -origin, Vector(0.0f, 1.0f, 0.0f));
		Matrix projMatrix = createProjectionMatrix(vertFovDeg, aspectRatio, 0.01f, 10.0f);

		// Begin frame
		ZgFramebuffer* framebuffer = nullptr;
		CHECK_ZG zgContextBeginFrame(ctx.mContext, &framebuffer);

		// Get a command list
		ZgCommandList* commandList = nullptr;
		CHECK_ZG zgCommandQueueBeginCommandListRecording(commandQueue, &commandList);

		// Set framebuffer and clear it
		ZgCommandListSetFramebufferInfo framebufferInfo = {};
		framebufferInfo.framebuffer = framebuffer;
		CHECK_ZG zgCommandListSetFramebuffer(commandList, &framebufferInfo);
		CHECK_ZG zgCommandListClearFramebuffer(commandList, 0.2f, 0.2f, 0.3f, 1.0f);
		CHECK_ZG zgCommandListClearDepthBuffer(commandList, 1.0f);

		// Set pipeline
		CHECK_ZG zgCommandListSetPipelineRendering(commandList, raymarchingPipeline);

		// Set pipeline bindings
		ZgPipelineBindings bindings = {};
		bindings.numConstantBuffers = 0;
		bindings.numTextures = 0;
		//CHECK_ZG zgCommandListSetPipelineBindings(commandList, &bindings);

		// Set Cube's vertex and index buffer
		CHECK_ZG zgCommandListSetIndexBuffer(
			commandList, fullscreenIndicesDevice, ZG_INDEX_BUFFER_TYPE_UINT32);
		CHECK_ZG zgCommandListSetVertexBuffer(commandList, 0, fullscreenVerticesDevice);
		
		struct InputData {
			float aspectRatio[4];
		};

		InputData data = {};
		data.aspectRatio[0] = aspectRatio;
		CHECK_ZG zgCommandListSetPushConstant(commandList, 0, &data, sizeof(InputData));

		CHECK_ZG zgCommandListDrawTrianglesIndexed(commandList, 0, 2);
		
		
		// Execute command list
		CHECK_ZG zgCommandQueueExecuteCommandList(commandQueue, commandList);

		// Finish frame
		CHECK_ZG zgContextFinishFrame(ctx.mContext);
	}

	// Flush command queue so nothing is running when we start releasing resources
	CHECK_ZG zgCommandQueueFlush(commandQueue);

	// Release ZeroG resources
	CHECK_ZG zgMemoryHeapBufferRelease(fullscreenVerticesHeap, fullscreenVerticesDevice);
	CHECK_ZG zgMemoryHeapRelease(ctx.mContext, fullscreenVerticesHeap);

	CHECK_ZG zgMemoryHeapBufferRelease(fullscreenIndicesHeap, fullscreenIndicesDevice);
	CHECK_ZG zgMemoryHeapRelease(ctx.mContext, fullscreenIndicesHeap);

	CHECK_ZG zgPipelineRenderingRelease(ctx.mContext, raymarchingPipeline);

	// Destroy ZeroG context
	ctx.destroy();

	// Cleanup SDL2
	cleanupSdl2(window);

	return EXIT_SUCCESS;
}
