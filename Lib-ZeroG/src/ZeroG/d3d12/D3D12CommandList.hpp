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

#include "ZeroG/d3d12/D3D12Common.hpp"
#include "ZeroG/BackendInterface.hpp"
#include "ZeroG/ZeroG-CApi.h"

namespace zg {

// D3D12CommandList
// ------------------------------------------------------------------------------------------------

class D3D12CommandList final : public ICommandList {
public:
	// Constructors & destructors
	// --------------------------------------------------------------------------------------------

	D3D12CommandList() noexcept = default;
	D3D12CommandList(const D3D12CommandList&) = delete;
	D3D12CommandList& operator= (const D3D12CommandList&) = delete;
	D3D12CommandList(D3D12CommandList&& other) noexcept { swap(other); }
	D3D12CommandList& operator= (D3D12CommandList&& other) noexcept { swap(other); return *this; }
	~D3D12CommandList() noexcept;

	// State methods
	// --------------------------------------------------------------------------------------------

	void swap(D3D12CommandList& other) noexcept;

	// Virtual methods
	// --------------------------------------------------------------------------------------------

	ZgErrorCode beginRecording() noexcept override final;
	ZgErrorCode finishRecording() noexcept override final;

	// Members
	// --------------------------------------------------------------------------------------------

	ComPtr<ID3D12CommandAllocator> commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> commandList;
	uint64_t fenceValue = 0;
};

} // namespace zg