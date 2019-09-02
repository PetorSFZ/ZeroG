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
#include <utility>

namespace zg {

// Context: State methods
// ------------------------------------------------------------------------------------------------

ErrorCode Context::init(const ZgContextInitSettings& settings) noexcept
{
	this->deinit();
	ZgErrorCode res = zgContextInit(&settings);
	mInitialized = res == ZG_SUCCESS;
	return (ErrorCode)res;
}

void Context::deinit() noexcept
{
	if (mInitialized) zgContextDeinit();
	mInitialized = false;
}

void Context::swap(Context& other) noexcept
{
	std::swap(mInitialized, other.mInitialized);
}

// Context: Version methods
// ------------------------------------------------------------------------------------------------

uint32_t Context::linkedApiVersion() noexcept
{
	return zgApiLinkedVersion();
}

// Context: Context methods
// ------------------------------------------------------------------------------------------------

bool Context::alreadyInitialized() noexcept
{
	return zgContextAlreadyInitialized();
}

ErrorCode Context::swapchainResize(uint32_t width, uint32_t height) noexcept
{
	return (ErrorCode)zgContextSwapchainResize(width, height);
}

ErrorCode Context::swapchainBeginFrame(Framebuffer& framebufferOut) noexcept
{
	if (framebufferOut.valid()) return ErrorCode::INVALID_ARGUMENT;
	ErrorCode res = (ErrorCode)zgContextSwapchainBeginFrame(&framebufferOut.framebuffer);
	if (!isSuccess(res)) return res;
	return (ErrorCode)zgFramebufferGetResolution(
		framebufferOut.framebuffer, &framebufferOut.width, &framebufferOut.height);
}

ErrorCode Context::swapchainFinishFrame() noexcept
{
	return (ErrorCode)zgContextSwapchainFinishFrame();
}

ErrorCode Context::getStats(ZgStats& statsOut) noexcept
{
	return (ErrorCode)zgContextGetStats(&statsOut);
}


// PipelineRenderingBuilder: Methods
// ------------------------------------------------------------------------------------------------

PipelineRenderingBuilder& PipelineRenderingBuilder::addVertexAttribute(
	ZgVertexAttribute attribute) noexcept
{
	assert(commonInfo.numVertexAttributes < ZG_MAX_NUM_VERTEX_ATTRIBUTES);
	commonInfo.vertexAttributes[commonInfo.numVertexAttributes] = attribute;
	commonInfo.numVertexAttributes += 1;
	return *this;
}

PipelineRenderingBuilder& PipelineRenderingBuilder::addVertexAttribute(
	uint32_t location,
	uint32_t vertexBufferSlot,
	ZgVertexAttributeType type,
	uint32_t offsetInBuffer) noexcept
{
	ZgVertexAttribute attribute = {};
	attribute.location = location;
	attribute.vertexBufferSlot = vertexBufferSlot;
	attribute.type = type;
	attribute.offsetToFirstElementInBytes = offsetInBuffer;
	return addVertexAttribute(attribute);
}

PipelineRenderingBuilder& PipelineRenderingBuilder::addVertexBufferInfo(
	uint32_t slot, uint32_t vertexBufferStrideBytes) noexcept
{
	assert(slot == commonInfo.numVertexBufferSlots);
	assert(commonInfo.numVertexBufferSlots < ZG_MAX_NUM_VERTEX_ATTRIBUTES);
	commonInfo.vertexBufferStridesBytes[slot] = vertexBufferStrideBytes;
	commonInfo.numVertexBufferSlots += 1;
	return *this;
}

PipelineRenderingBuilder& PipelineRenderingBuilder::addPushConstant(
	uint32_t constantBufferRegister) noexcept
{
	assert(commonInfo.numPushConstants < ZG_MAX_NUM_CONSTANT_BUFFERS);
	commonInfo.pushConstantRegisters[commonInfo.numPushConstants] = constantBufferRegister;
	commonInfo.numPushConstants += 1;
	return *this;
}

PipelineRenderingBuilder& PipelineRenderingBuilder::addSampler(
	uint32_t samplerRegister, ZgSampler sampler) noexcept
{
	assert(samplerRegister == commonInfo.numSamplers);
	assert(commonInfo.numSamplers < ZG_MAX_NUM_SAMPLERS);
	commonInfo.samplers[samplerRegister] = sampler;
	commonInfo.numSamplers += 1;
	return *this;
}

PipelineRenderingBuilder& PipelineRenderingBuilder::addSampler(
	uint32_t samplerRegister,
	ZgSamplingMode samplingMode,
	ZgWrappingMode wrappingModeU,
	ZgWrappingMode wrappingModeV,
	float mipLodBias) noexcept
{
	ZgSampler sampler = {};
	sampler.samplingMode = samplingMode;
	sampler.wrappingModeU = wrappingModeU;
	sampler.wrappingModeV = wrappingModeV;
	sampler.mipLodBias = mipLodBias;
	return addSampler(samplerRegister, sampler);
}

PipelineRenderingBuilder& PipelineRenderingBuilder::addVertexShaderPath(
	const char* entry, const char* path) noexcept
{
	commonInfo.vertexShaderEntry = entry;
	vertexShaderPath = path;
	return *this;
}

PipelineRenderingBuilder& PipelineRenderingBuilder::addPixelShaderPath(
	const char* entry, const char* path) noexcept
{
	commonInfo.pixelShaderEntry = entry;
	pixelShaderPath = path;
	return *this;
}

PipelineRenderingBuilder& PipelineRenderingBuilder::addVertexShaderSource(
	const char* entry, const char* src) noexcept
{
	commonInfo.vertexShaderEntry = entry;
	vertexShaderSrc = src;
	return *this;
}

PipelineRenderingBuilder& PipelineRenderingBuilder::addPixelShaderSource(
	const char* entry, const char* src) noexcept
{
	commonInfo.pixelShaderEntry = entry;
	pixelShaderSrc = src;
	return *this;
}

PipelineRenderingBuilder& PipelineRenderingBuilder::setWireframeRendering(
	bool wireframeEnabled) noexcept
{
	commonInfo.rasterizer.wireframeMode = wireframeEnabled ? ZG_TRUE : ZG_FALSE;
	return *this;
}

PipelineRenderingBuilder& PipelineRenderingBuilder::setCullingEnabled(bool cullingEnabled) noexcept
{
	commonInfo.rasterizer.cullingEnabled = cullingEnabled ? ZG_TRUE : ZG_FALSE;
	return *this;
}

PipelineRenderingBuilder& PipelineRenderingBuilder::setCullMode(
	bool cullFrontFacing, bool fontFacingIsCounterClockwise) noexcept
{
	commonInfo.rasterizer.cullFrontFacing = cullFrontFacing ? ZG_TRUE : ZG_FALSE;
	commonInfo.rasterizer.frontFacingIsCounterClockwise =
		fontFacingIsCounterClockwise ? ZG_TRUE : ZG_FALSE;
	return *this;
}

PipelineRenderingBuilder& PipelineRenderingBuilder::setBlendingEnabled(bool blendingEnabled) noexcept
{
	commonInfo.blending.blendingEnabled = blendingEnabled ? ZG_TRUE : ZG_FALSE;
	return *this;
}

PipelineRenderingBuilder& PipelineRenderingBuilder::setBlendFuncColor(
	ZgBlendFunc func, ZgBlendValue srcFactor, ZgBlendValue dstFactor) noexcept
{
	commonInfo.blending.blendFuncColor = func;
	commonInfo.blending.srcValColor = srcFactor;
	commonInfo.blending.dstValColor = dstFactor;
	return *this;
}

PipelineRenderingBuilder& PipelineRenderingBuilder::setBlendFuncAlpha(
	ZgBlendFunc func, ZgBlendValue srcFactor, ZgBlendValue dstFactor) noexcept
{
	commonInfo.blending.blendFuncAlpha = func;
	commonInfo.blending.srcValAlpha = srcFactor;
	commonInfo.blending.dstValAlpha = dstFactor;
	return *this;
}

PipelineRenderingBuilder& PipelineRenderingBuilder::setDepthTestEnabled(
	bool depthTestEnabled) noexcept
{
	commonInfo.depthTest.depthTestEnabled = depthTestEnabled ? ZG_TRUE : ZG_FALSE;
	return *this;
}

PipelineRenderingBuilder& PipelineRenderingBuilder::setDepthFunc(ZgDepthFunc depthFunc) noexcept
{
	commonInfo.depthTest.depthFunc = depthFunc;
	return *this;
}

ErrorCode PipelineRenderingBuilder::buildFromFileSPIRV(
	PipelineRendering& pipelineOut) const noexcept
{
	// Build create info
	ZgPipelineRenderingCreateInfoFileSPIRV createInfo = {};
	createInfo.common = this->commonInfo;
	createInfo.vertexShaderPath = this->vertexShaderPath;
	createInfo.pixelShaderPath = this->pixelShaderPath;

	// Build pipeline
	return pipelineOut.createFromFileSPIRV(createInfo);
}

ErrorCode PipelineRenderingBuilder::buildFromFileHLSL(
	PipelineRendering& pipelineOut, ZgShaderModel model) const noexcept
{
	// Build create info
	ZgPipelineRenderingCreateInfoFileHLSL createInfo = {};
	createInfo.common = this->commonInfo;
	createInfo.vertexShaderPath = this->vertexShaderPath;
	createInfo.pixelShaderPath = this->pixelShaderPath;
	createInfo.shaderModel = model;
	createInfo.dxcCompilerFlags[0] = "-Zi";
	createInfo.dxcCompilerFlags[1] = "-O3";

	// Build pipeline
	return pipelineOut.createFromFileHLSL(createInfo);
}

ErrorCode PipelineRenderingBuilder::buildFromSourceHLSL(
	PipelineRendering& pipelineOut, ZgShaderModel model) const noexcept
{
	// Build create info
	ZgPipelineRenderingCreateInfoSourceHLSL createInfo = {};
	createInfo.common = this->commonInfo;
	createInfo.vertexShaderSrc = this->vertexShaderSrc;
	createInfo.pixelShaderSrc= this->pixelShaderSrc;
	createInfo.shaderModel = model;
	createInfo.dxcCompilerFlags[0] = "-Zi";
	createInfo.dxcCompilerFlags[1] = "-O3";

	// Build pipeline
	return pipelineOut.createFromSourceHLSL(createInfo);
}


// PipelineRendering: State methods
// ------------------------------------------------------------------------------------------------

ErrorCode PipelineRendering::createFromFileSPIRV(
	const ZgPipelineRenderingCreateInfoFileSPIRV& createInfo) noexcept
{
	this->release();
	return (ErrorCode)zgPipelineRenderingCreateFromFileSPIRV(
		&this->pipeline, &this->signature, &createInfo);
}

ErrorCode PipelineRendering::createFromFileHLSL(
	const ZgPipelineRenderingCreateInfoFileHLSL& createInfo) noexcept
{
	this->release();
	return (ErrorCode)zgPipelineRenderingCreateFromFileHLSL(
		&this->pipeline, &this->signature, &createInfo);
}

ErrorCode PipelineRendering::createFromSourceHLSL(
	const ZgPipelineRenderingCreateInfoSourceHLSL& createInfo) noexcept
{
	this->release();
	return (ErrorCode)zgPipelineRenderingCreateFromSourceHLSL(
		&this->pipeline, &this->signature, &createInfo);
}

void PipelineRendering::swap(PipelineRendering& other) noexcept
{
	std::swap(this->pipeline, other.pipeline);
	std::swap(this->signature, other.signature);
}

void PipelineRendering::release() noexcept
{
	if (this->pipeline != nullptr) zgPipelineRenderingRelease(this->pipeline);
	this->pipeline = nullptr;
	this->signature = {};
}


// MemoryHeap: State methods
// ------------------------------------------------------------------------------------------------

ErrorCode MemoryHeap::create(const ZgMemoryHeapCreateInfo& createInfo) noexcept
{
	this->release();
	return (ErrorCode)zgMemoryHeapCreate(&this->memoryHeap, &createInfo);
}

ErrorCode MemoryHeap::create(uint64_t sizeInBytes, ZgMemoryType memoryType) noexcept
{
	ZgMemoryHeapCreateInfo createInfo = {};
	createInfo.sizeInBytes = sizeInBytes;
	createInfo.memoryType = memoryType;
	return this->create(createInfo);
}

void MemoryHeap::swap(MemoryHeap& other) noexcept
{
	std::swap(this->memoryHeap, other.memoryHeap);
}

void MemoryHeap::release() noexcept
{
	if (this->memoryHeap != nullptr) zgMemoryHeapRelease(this->memoryHeap);
	this->memoryHeap = nullptr;
}

// MemoryHeap: MemoryHeap methods
// ------------------------------------------------------------------------------------------------

ErrorCode MemoryHeap::bufferCreate(zg::Buffer& bufferOut, const ZgBufferCreateInfo& createInfo) noexcept
{
	bufferOut.release();
	return (ErrorCode)zgMemoryHeapBufferCreate(this->memoryHeap, &bufferOut.buffer, &createInfo);
}

ErrorCode MemoryHeap::bufferCreate(Buffer& bufferOut, uint64_t offset, uint64_t size) noexcept
{
	ZgBufferCreateInfo createInfo = {};
	createInfo.offsetInBytes = offset;
	createInfo.sizeInBytes = size;
	return this->bufferCreate(bufferOut, createInfo);
}

ErrorCode MemoryHeap::texture2DCreate(
	Texture2D& textureOut, const ZgTexture2DCreateInfo& createInfo) noexcept
{
	textureOut.release();
	return (ErrorCode)zgMemoryHeapTexture2DCreate(
		this->memoryHeap, &textureOut.texture, &createInfo);
}


// Buffer: State methods
// --------------------------------------------------------------------------------------------

void Buffer::swap(Buffer& other) noexcept
{
	std::swap(this->buffer, other.buffer);
}

void Buffer::release() noexcept
{
	if (this->buffer != nullptr) {
		zgBufferRelease(this->buffer);
	}
	this->buffer = nullptr;
}

// Buffer: Buffer methods
// --------------------------------------------------------------------------------------------

ErrorCode Buffer::memcpyTo(uint64_t bufferOffsetBytes, const void* srcMemory, uint64_t numBytes)
{
	return (ErrorCode)zgBufferMemcpyTo(this->buffer, bufferOffsetBytes, srcMemory, numBytes);
}

ErrorCode Buffer::setDebugName(const char* name) noexcept
{
	return (ErrorCode)zgBufferSetDebugName(this->buffer, name);
}


// Texture2D: State methods
// ------------------------------------------------------------------------------------------------

void Texture2D::swap(Texture2D& other) noexcept
{
	std::swap(this->texture, other.texture);
}

void Texture2D::release() noexcept
{
	if (this->texture != nullptr) zgTexture2DRelease(this->texture);
	this->texture = nullptr;
}

// Texture2D: Methods
// ------------------------------------------------------------------------------------------------

// See zgTexture2DGetAllocationInfo
ErrorCode Texture2D::getAllocationInfo(
	ZgTexture2DAllocationInfo& allocationInfoOut,
	const ZgTexture2DCreateInfo& createInfo) noexcept
{
	return (ErrorCode)zgTexture2DGetAllocationInfo(&allocationInfoOut, &createInfo);
}

ErrorCode Texture2D::setDebugName(const char* name) noexcept
{
	return (ErrorCode)zgTexture2DSetDebugName(this->texture, name);
}


// FramebufferBuilder: Methods
// ------------------------------------------------------------------------------------------------

FramebufferBuilder& FramebufferBuilder::addRenderTarget(Texture2D& renderTarget) noexcept
{
	assert(createInfo.numRenderTargets < ZG_FRAMEBUFFER_MAX_NUM_RENDER_TARGETS);
	uint32_t idx = createInfo.numRenderTargets;
	createInfo.numRenderTargets += 1;
	createInfo.renderTargets[idx] = renderTarget.texture;
	return *this;
}

FramebufferBuilder& FramebufferBuilder::setDepthBuffer(Texture2D& depthBuffer) noexcept
{
	createInfo.depthBuffer = depthBuffer.texture;
	return *this;
}

ErrorCode FramebufferBuilder::build(Framebuffer& framebufferOut) noexcept
{
	return framebufferOut.create(this->createInfo);
}


// Framebuffer: State methods
// ------------------------------------------------------------------------------------------------

ErrorCode Framebuffer::create(const ZgFramebufferCreateInfo& createInfo) noexcept
{
	this->release();
	ErrorCode res = (ErrorCode)zgFramebufferCreate(&this->framebuffer, &createInfo);
	if (!isSuccess(res)) return res;
	return (ErrorCode)zgFramebufferGetResolution(this->framebuffer, &this->width, &this->height);
}

void Framebuffer::swap(Framebuffer& other) noexcept
{
	std::swap(this->framebuffer, other.framebuffer);
	std::swap(this->width, other.width);
	std::swap(this->height, other.height);
}

void Framebuffer::release() noexcept
{
	if (this->framebuffer != nullptr) zgFramebufferRelease(this->framebuffer);
	this->framebuffer = nullptr;
	this->width = 0;
	this->height = 0;
}


// Fence: State methods
// ------------------------------------------------------------------------------------------------

ErrorCode Fence::create() noexcept
{
	this->release();
	return (ErrorCode)zgFenceCreate(&this->fence);
}

void Fence::swap(Fence& other) noexcept
{
	std::swap(this->fence, other.fence);
}

void Fence::release() noexcept
{
	if (this->fence != nullptr) zgFenceRelease(this->fence);
	this->fence = nullptr;
}

// Fence: Fence methods
// ------------------------------------------------------------------------------------------------

ErrorCode Fence::reset() noexcept
{
	return (ErrorCode)zgFenceReset(this->fence);
}

ErrorCode Fence::checkIfSignaled(bool& fenceSignaledOut) const noexcept
{
	ZgBool signaled = ZG_FALSE;
	ErrorCode res = (ErrorCode)zgFenceCheckIfSignaled(this->fence, &signaled);
	fenceSignaledOut = signaled == ZG_FALSE ? false : true;
	return res;
}

bool Fence::checkIfSignaled() const noexcept
{
	bool signaled = false;
	[[maybe_unused]] ErrorCode res = this->checkIfSignaled(signaled);
	return signaled;
}

ErrorCode Fence::waitOnCpuBlocking() const noexcept
{
	return (ErrorCode)zgFenceWaitOnCpuBlocking(this->fence);
}


// CommandQueue: Constructors & destructors
// ------------------------------------------------------------------------------------------------

ErrorCode CommandQueue::getPresentQueue(CommandQueue& presentQueueOut) noexcept
{
	if (presentQueueOut.commandQueue != nullptr) return ErrorCode::INVALID_ARGUMENT;
	return (ErrorCode)zgCommandQueueGetPresentQueue(&presentQueueOut.commandQueue);
}

ErrorCode CommandQueue::getCopyQueue(CommandQueue& copyQueueOut) noexcept
{
	if (copyQueueOut.commandQueue != nullptr) return ErrorCode::INVALID_ARGUMENT;
	return (ErrorCode)zgCommandQueueGetCopyQueue(&copyQueueOut.commandQueue);
}

// CommandQueue: State methods
// ------------------------------------------------------------------------------------------------

void CommandQueue::swap(CommandQueue& other) noexcept
{
	std::swap(this->commandQueue, other.commandQueue);
}
void CommandQueue::release() noexcept
{
	// TODO: Currently there is no destruction of command queues as there is only one
	this->commandQueue = nullptr;
}

// CommandQueue: CommandQueue methods
// ------------------------------------------------------------------------------------------------

ErrorCode CommandQueue::signalOnGpu(Fence& fenceToSignal) noexcept
{
	return (ErrorCode)zgCommandQueueSignalOnGpu(this->commandQueue, fenceToSignal.fence);
}

ErrorCode CommandQueue::waitOnGpu(const Fence& fence) noexcept
{
	return (ErrorCode)zgCommandQueueWaitOnGpu(this->commandQueue, fence.fence);
}

ErrorCode CommandQueue::flush() noexcept
{
	return (ErrorCode)zgCommandQueueFlush(this->commandQueue);
}

ErrorCode CommandQueue::beginCommandListRecording(CommandList& commandListOut) noexcept
{
	if (commandListOut.commandList != nullptr) return ErrorCode::INVALID_ARGUMENT;
	return (ErrorCode)zgCommandQueueBeginCommandListRecording(
		this->commandQueue, &commandListOut.commandList);
}

ErrorCode CommandQueue::executeCommandList(CommandList& commandList) noexcept
{
	ZgErrorCode res = zgCommandQueueExecuteCommandList(this->commandQueue, commandList.commandList);
	commandList.commandList = nullptr;
	return (ErrorCode)res;
}


// PipelineBindings: Methods
// ------------------------------------------------------------------------------------------------

PipelineBindings& PipelineBindings::addConstantBuffer(ConstantBufferBinding binding) noexcept
{
	assert(numConstantBuffers < ZG_MAX_NUM_CONSTANT_BUFFERS);
	constantBuffers[numConstantBuffers] = binding;
	numConstantBuffers += 1;
	return *this;
}

PipelineBindings& PipelineBindings::addConstantBuffer(
	uint32_t shaderRegister, Buffer& buffer) noexcept
{
	ConstantBufferBinding binding;
	binding.shaderRegister = shaderRegister;
	binding.buffer = &buffer;
	return this->addConstantBuffer(binding);
}

PipelineBindings& PipelineBindings::addTexture(TextureBinding binding) noexcept
{
	assert(numTextures < ZG_MAX_NUM_TEXTURES);
	textures[numTextures] = binding;
	numTextures += 1;
	return *this;
}

PipelineBindings& PipelineBindings::addTexture(
	uint32_t textureRegister, Texture2D& texture) noexcept
{
	TextureBinding binding;
	binding.textureRegister = textureRegister;
	binding.texture = &texture;
	return this->addTexture(binding);
}

ZgPipelineBindings PipelineBindings::toCApi() const noexcept
{
	assert(numConstantBuffers < ZG_MAX_NUM_CONSTANT_BUFFERS);
	assert(numTextures < ZG_MAX_NUM_TEXTURES);

	// Constant buffers
	ZgPipelineBindings cBindings = {};
	cBindings.numConstantBuffers = this->numConstantBuffers;
	for (uint32_t i = 0; i < this->numConstantBuffers; i++) {
		cBindings.constantBuffers[i].shaderRegister = this->constantBuffers[i].shaderRegister;
		cBindings.constantBuffers[i].buffer = this->constantBuffers[i].buffer->buffer;
	}

	// Textures
	cBindings.numTextures = this->numTextures;
	for (uint32_t i = 0; i < this->numTextures; i++) {
		cBindings.textures[i].textureRegister = this->textures[i].textureRegister;
		cBindings.textures[i].texture = this->textures[i].texture->texture;
	}

	return cBindings;
}


// CommandList: State methods
// ------------------------------------------------------------------------------------------------

void CommandList::swap(CommandList& other) noexcept
{
	std::swap(this->commandList, other.commandList);
}

void CommandList::release() noexcept
{
	// TODO: Currently there is no destruction of command lists as they are owned by the
	//       CommandQueue.
	this->commandList = nullptr;
}

// CommandList: CommandList methods
// ------------------------------------------------------------------------------------------------

ErrorCode CommandList::memcpyBufferToBuffer(
	Buffer& dstBuffer,
	uint64_t dstBufferOffsetBytes,
	Buffer& srcBuffer,
	uint64_t srcBufferOffsetBytes,
	uint64_t numBytes) noexcept
{
	return (ErrorCode)zgCommandListMemcpyBufferToBuffer(
		this->commandList,
		dstBuffer.buffer,
		dstBufferOffsetBytes,
		srcBuffer.buffer,
		srcBufferOffsetBytes,
		numBytes);
}

ErrorCode CommandList::memcpyToTexture(
	Texture2D& dstTexture,
	uint32_t dstTextureMipLevel,
	const ZgImageViewConstCpu& srcImageCpu,
	Buffer& tempUploadBuffer) noexcept
{
	return (ErrorCode)zgCommandListMemcpyToTexture(
		this->commandList,
		dstTexture.texture,
		dstTextureMipLevel,
		&srcImageCpu,
		tempUploadBuffer.buffer);
}

ErrorCode CommandList::enableQueueTransition(Buffer& buffer) noexcept
{
	return (ErrorCode)zgCommandListEnableQueueTransitionBuffer(this->commandList, buffer.buffer);
}

ErrorCode CommandList::enableQueueTransition(Texture2D& texture) noexcept
{
	return (ErrorCode)zgCommandListEnableQueueTransitionTexture(this->commandList, texture.texture);
}

ErrorCode CommandList::setPushConstant(
	uint32_t shaderRegister, const void* data, uint32_t dataSizeInBytes) noexcept
{
	return (ErrorCode)zgCommandListSetPushConstant(
		this->commandList, shaderRegister, data, dataSizeInBytes);
}

ErrorCode CommandList::setPipelineBindings(const PipelineBindings& bindings) noexcept
{
	ZgPipelineBindings cBindings = bindings.toCApi();
	return (ErrorCode)zgCommandListSetPipelineBindings(this->commandList, &cBindings);
}

ErrorCode CommandList::setPipeline(PipelineRendering& pipeline) noexcept
{
	return (ErrorCode)zgCommandListSetPipelineRendering(this->commandList, pipeline.pipeline);
}

ErrorCode CommandList::setFramebuffer(
	Framebuffer& framebuffer,
	const ZgFramebufferRect* optionalViewport,
	const ZgFramebufferRect* optionalScissor) noexcept
{
	return (ErrorCode)zgCommandListSetFramebuffer(
		this->commandList, framebuffer.framebuffer, optionalViewport, optionalScissor);
}

ErrorCode CommandList::setFramebufferViewport(
	const ZgFramebufferRect& viewport) noexcept
{
	return (ErrorCode)zgCommandListSetFramebufferViewport(this->commandList, &viewport);
}

ErrorCode CommandList::setFramebufferScissor(
	const ZgFramebufferRect& scissor) noexcept
{
	return (ErrorCode)zgCommandListSetFramebufferScissor(this->commandList, &scissor);
}

ErrorCode CommandList::clearFramebuffer(float red, float green, float blue, float alpha) noexcept
{
	return (ErrorCode)zgCommandListClearFramebuffer(this->commandList, red, green, blue, alpha);
}

ErrorCode CommandList::clearDepthBuffer(float depth) noexcept
{
	return (ErrorCode)zgCommandListClearDepthBuffer(this->commandList, depth);
}

ErrorCode CommandList::setIndexBuffer(Buffer& indexBuffer, ZgIndexBufferType type) noexcept
{
	return (ErrorCode)zgCommandListSetIndexBuffer(this->commandList, indexBuffer.buffer, type);
}

ErrorCode CommandList::setVertexBuffer(uint32_t vertexBufferSlot, Buffer& vertexBuffer) noexcept
{
	return (ErrorCode)zgCommandListSetVertexBuffer(
		this->commandList, vertexBufferSlot, vertexBuffer.buffer);
}

ErrorCode CommandList::drawTriangles(uint32_t startVertexIndex, uint32_t numVertices) noexcept
{
	return (ErrorCode)zgCommandListDrawTriangles(this->commandList, startVertexIndex, numVertices);
}

ErrorCode CommandList::drawTrianglesIndexed(uint32_t startIndex, uint32_t numTriangles) noexcept
{
	return (ErrorCode)zgCommandListDrawTrianglesIndexed(
		this->commandList, startIndex, numTriangles);
}

} // namespace zg
