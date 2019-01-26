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

#include "ZeroG/d3d12/D3D12CommandList.hpp"

#include <algorithm>

namespace zg {

// D3D12CommandList: Constructors & destructors
// ------------------------------------------------------------------------------------------------

D3D12CommandList::~D3D12CommandList() noexcept
{

}

// State methods
// ------------------------------------------------------------------------------------------------

void D3D12CommandList::swap(D3D12CommandList& other) noexcept
{
	std::swap(this->commandAllocator, other.commandAllocator);
	std::swap(this->commandList, other.commandList);
	std::swap(this->fenceValue, other.fenceValue);
}

// D3D12CommandList: Virtual methods
// ------------------------------------------------------------------------------------------------

ZgErrorCode D3D12CommandList::beginRecording() noexcept
{
	return ZG_ERROR_UNIMPLEMENTED;
}

ZgErrorCode D3D12CommandList::finishRecording() noexcept
{
	return ZG_ERROR_UNIMPLEMENTED;
}

} // namespace zg
