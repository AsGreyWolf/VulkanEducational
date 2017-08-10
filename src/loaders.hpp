#pragma once

#include "image_loader.hpp"
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

struct DevicePart {
	vk::PhysicalDevice pDevice;
	vk::Device device;
	vk::CommandPool commandPool;
	uint32_t queueFamily;
	uint32_t queueId;
};

struct Texture {
	vk::UniqueImage image;
	vk::UniqueDeviceMemory memory;
	vk::UniqueImageView wholeImageView;
	uint32_t w;
	uint32_t h;
	uint32_t mipmaps;
};

struct VertexBuffer {
	vk::UniqueBuffer buffer;
	vk::UniqueDeviceMemory memory;
	uint32_t size;
};

std::vector<char> loadFile(const std::string &filename);
vk::UniqueDeviceMemory allocateMemory(DevicePart gpu,
                                      vk::MemoryRequirements reqs,
                                      vk::MemoryPropertyFlags prefered);
template <typename T>
VertexBuffer createTempBuffer(DevicePart gpu, const T *data, uint32_t size);
template <typename T>
VertexBuffer createBuffer(
    DevicePart gpu, const T *data, uint32_t size,
    vk::BufferUsageFlags usage = vk::BufferUsageFlagBits::eVertexBuffer);
Texture loadTexture(
    DevicePart gpu, const std::string &path, uint32_t mipmaps = 1,
    vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eSampled,
    vk::PipelineStageFlags stageAfter = vk::PipelineStageFlagBits::eAllGraphics,
    vk::AccessFlags accessAfter = vk::AccessFlagBits::eShaderRead,
    vk::ImageLayout layoutAfter = vk::ImageLayout::eShaderReadOnlyOptimal);
vk::UniqueShaderModule loadShader(DevicePart gpu, const std::string &path);

#include "loaders.inl"
