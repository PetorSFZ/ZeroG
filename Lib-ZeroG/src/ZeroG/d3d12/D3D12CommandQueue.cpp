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

#include "ZeroG/d3d12/D3D12CommandQueue.hpp"

#include <cassert>

namespace zg {

// D3D12CommandQueue: Constructors & destructors
// ------------------------------------------------------------------------------------------------

D3D12CommandQueue::~D3D12CommandQueue() noexcept
{
	// Flush queue
	this->flush();

	// Check that all command lists have been returned
	assert(mCommandListStorage.size() == mCommandListQueue.size());

	// Destroy fence event
	CloseHandle(mCommandQueueFenceEvent);
}

// D3D12CommandQueue: State methods
// ------------------------------------------------------------------------------------------------

ZgErrorCode D3D12CommandQueue::init(
	ComPtr<ID3D12Device3>& device,
	uint32_t maxNumCommandLists,
	ZgAllocator allocator) noexcept
{
	mDevice = device;

	// Create command queue
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; // TODO: D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT
	desc.NodeMask = 0;

	if (!CHECK_D3D12_SUCCEEDED(
		device->CreateCommandQueue(&desc, IID_PPV_ARGS(&mCommandQueue)))) {
		return ZG_ERROR_NO_SUITABLE_DEVICE;
	}

	// Create command queue fence
	if (!CHECK_D3D12_SUCCEEDED(device->CreateFence(
		mCommandQueueFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mCommandQueueFence)))) {
		return ZG_ERROR_GENERIC;
	}

	// Create command queue fence event
	mCommandQueueFenceEvent = ::CreateEvent(NULL, false, false, NULL);

	// Allocate memory for command lists
	mCommandListStorage.create(
		maxNumCommandLists, allocator, "ZeroG - D3D12CommandQueue - CommandListStorage");
	mCommandListQueue.create(
		maxNumCommandLists, allocator, "ZeroG - D3D12CommandQueue - CommandListQueue");

	return ZG_SUCCESS;
}

// D3D12CommandQueue: Virtual methods
// ------------------------------------------------------------------------------------------------

ZgErrorCode D3D12CommandQueue::flush() noexcept
{
	uint64_t fenceValue = this->signalOnGpu();
	this->waitOnCpu(fenceValue);
	return ZG_SUCCESS;
}

ZgErrorCode D3D12CommandQueue::beginCommandListRecording(ICommandList** commandListOut) noexcept
{
	std::lock_guard<std::mutex> lock(mQueueMutex);

	D3D12CommandList* commandList;
	bool commandListFound = false;

	// If command lists available in queue, attempt to get one of them
	uint64_t queueSize = mCommandListQueue.size();
	if (queueSize != 0) {
		if (isFenceValueDone(mCommandListQueue.first()->fenceValue)) {
			mCommandListQueue.pop(commandList);
			commandListFound = true;
		}
	}

	// If no command list found, create new one
	if (!commandListFound) {
		ZgErrorCode res = createCommandList(commandList);
		if (res != ZG_SUCCESS) return res;
		commandListFound = true;
	}

	// Reset command list and allocator
	if (!CHECK_D3D12_SUCCEEDED(commandList->commandAllocator->Reset())) {
		return ZG_ERROR_GENERIC;
	}
	if (!CHECK_D3D12_SUCCEEDED(
		commandList->commandList->Reset(commandList->commandAllocator.Get(), nullptr))) {
		return ZG_ERROR_GENERIC;
	}

	// Return command list
	*commandListOut = commandList;
	return ZG_SUCCESS;
}

ZgErrorCode D3D12CommandQueue::executeCommandList(ICommandList* commandListIn) noexcept
{
	std::lock_guard<std::mutex> lock(mQueueMutex);
	D3D12CommandList& commandList = *reinterpret_cast<D3D12CommandList*>(commandListIn);

	// Close command list
	if (!CHECK_D3D12_SUCCEEDED(commandList.commandList->Close())) {
		return ZG_ERROR_GENERIC;
	}

	// Execute command list
	ID3D12CommandList* commandListPtr = commandList.commandList.Get();
	mCommandQueue->ExecuteCommandLists(1, &commandListPtr);

	// Signal
	commandList.fenceValue = this->signalOnGpuUnmutexed();

	// Add command list to queue
	mCommandListQueue.add(&commandList);
	return ZG_SUCCESS;
}

// D3D12CommandQueue: Synchronization methods
// ------------------------------------------------------------------------------------------------

uint64_t D3D12CommandQueue::signalOnGpu() noexcept
{
	std::lock_guard<std::mutex> lock(mQueueMutex);
	return signalOnGpuUnmutexed();
}

void D3D12CommandQueue::waitOnCpu(uint64_t fenceValue) noexcept
{
	// TODO: Kind of bad to only have one event, must have mutex here because of that.
	std::lock_guard<std::mutex> lock(mQueueMutex);

	if (!isFenceValueDone(fenceValue)) {
		CHECK_D3D12 mCommandQueueFence->SetEventOnCompletion(
			fenceValue, mCommandQueueFenceEvent);
		// TODO: Don't wait forever
		::WaitForSingleObject(mCommandQueueFenceEvent, INFINITE);
	}
}

bool D3D12CommandQueue::isFenceValueDone(uint64_t fenceValue) noexcept
{
	return mCommandQueueFence->GetCompletedValue() >= fenceValue;
}

// D3D12CommandQueue: Private  methods
// ------------------------------------------------------------------------------------------------

uint64_t D3D12CommandQueue::signalOnGpuUnmutexed() noexcept
{
	CHECK_D3D12 mCommandQueue->Signal(mCommandQueueFence.Get(), mCommandQueueFenceValue);
	return mCommandQueueFenceValue++;
}

ZgErrorCode D3D12CommandQueue::createCommandList(D3D12CommandList*& commandListOut) noexcept
{
	// Create a new command list in storage, return error if full
	bool addSuccesful = mCommandListStorage.add(D3D12CommandList());
	if (!addSuccesful) return ZG_ERROR_OUT_OF_COMMAND_LISTS;

	D3D12CommandList& commandList = mCommandListStorage.last();

	// Create command allocator
	// TODO: Type
	if (!CHECK_D3D12_SUCCEEDED(mDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandList.commandAllocator)))) {
		mCommandListStorage.pop();
		return ZG_ERROR_GENERIC;
	}

	// Create command list
	// TODO: Type
	if (!CHECK_D3D12_SUCCEEDED(mDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		commandList.commandAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(&commandList.commandList)))) {
		mCommandListStorage.pop();
		return ZG_ERROR_GENERIC;
	}

	// Ensure command list is in closed state
	if (!CHECK_D3D12_SUCCEEDED(commandList.commandList->Close())) {
		return ZG_ERROR_GENERIC;
	}

	commandListOut = &commandList;
	return ZG_SUCCESS;
}

} // namespace zg