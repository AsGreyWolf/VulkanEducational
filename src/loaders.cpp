#include "loaders.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <vector>

std::vector<char> loadFile(const std::string &filename) {
	std::ifstream f{filename};
	std::vector<char> result;
	std::copy(std::istreambuf_iterator<char>{f}, std::istreambuf_iterator<char>{},
	          std::back_inserter(result));
	return result;
}

vk::UniqueDeviceMemory allocateMemory(DevicePart gpu,
                                      vk::MemoryRequirements reqs,
                                      vk::MemoryPropertyFlags prefered) {
	const auto memProps = gpu.pDevice.getMemoryProperties();
	const auto memTypes = memProps.memoryTypes;
	uint32_t memTypeId = std::numeric_limits<uint32_t>::max();
	for (size_t i = 0; i < 16; i++)
		if ((1 << i) & reqs.memoryTypeBits &&
		    ((memTypes[i].propertyFlags & prefered) == prefered))
			memTypeId = i;
	if (memTypeId == std::numeric_limits<uint32_t>::max())
		for (size_t i = 0; i < 16; i++)
			if ((1 << i) & reqs.memoryTypeBits)
				memTypeId = i;
	auto result = gpu.device.allocateMemoryUnique({reqs.size, memTypeId});
	std::cout << "Memory allocated " << reqs.size << std::endl;
	return result;
}

Texture loadTexture(DevicePart gpu, const std::string &path, uint32_t mipmaps,
                    vk::ImageUsageFlags usage,
                    vk::PipelineStageFlags stageAfter,
                    vk::AccessFlags accessAfter, vk::ImageLayout layoutAfter) {

	const std::array<uint32_t, 1> qIds{{gpu.queueId}};
	const auto[textureW, textureH, textureData] = g_ImageLoader()->Load(path);
	auto textureImage = gpu.device.createImageUnique(
	    {{},
	     vk::ImageType::e2D,
	     vk::Format::eR8G8B8A8Unorm,
	     {textureW, textureH, 1},
	     mipmaps,
	     1,
	     vk::SampleCountFlagBits::e1,
	     vk::ImageTiling::eOptimal,
	     usage | vk::ImageUsageFlagBits::eTransferDst |
	         vk::ImageUsageFlagBits::eTransferSrc,
	     vk::SharingMode::eExclusive,
	     qIds.size(),
	     qIds.data(),
	     vk::ImageLayout::ePreinitialized});
	auto reqs = gpu.device.getImageMemoryRequirements(textureImage.get());
	auto textureMemory =
	    allocateMemory(gpu, reqs, vk::MemoryPropertyFlagBits::eDeviceLocal);
	gpu.device.bindImageMemory(textureImage.get(), textureMemory.get(), 0);

	auto tempBuffer =
	    createTempBuffer(gpu, textureData.data(), textureW * textureH * 4);

	const vk::ImageSubresourceRange baseImageRange{vk::ImageAspectFlagBits::eColor,
	                                               0, 1, 0, 1};
	const vk::ImageSubresourceLayers baseImage{vk::ImageAspectFlagBits::eColor, 0,
	                                           0, 1};
	const vk::ImageSubresourceRange mipmapTail{vk::ImageAspectFlagBits::eColor, 1,
	                                           mipmaps - 1, 0, 1};
	std::vector<vk::ImageBlit> blits;
	blits.reserve(mipmaps - 1);
	for (uint32_t mipmap_lvl = 1, w = textureW / 2, h = textureH / 2;
	     mipmap_lvl < mipmaps; mipmap_lvl++, w /= 2, h /= 2) {
		blits.push_back(
		    {baseImage,
		     {{{},
		       {static_cast<int32_t>(textureW), static_cast<int32_t>(textureH), 1}}},
		     {vk::ImageAspectFlagBits::eColor, mipmap_lvl, 0, 1},
		     {{{}, {static_cast<int32_t>(w), static_cast<int32_t>(h), 1}}}});
	}

	auto tempCommandBuffers = gpu.device.allocateCommandBuffersUnique(
	    {gpu.commandPool, vk::CommandBufferLevel::ePrimary, 1});
	tempCommandBuffers[0].get().begin(
	    {vk::CommandBufferUsageFlagBits::eOneTimeSubmit, nullptr});
	tempCommandBuffers[0].get().pipelineBarrier(
	    vk::PipelineStageFlagBits::eTopOfPipe,
	    vk::PipelineStageFlagBits::eTransfer, {}, {}, {},
	    {{vk::AccessFlagBits::eHostWrite, vk::AccessFlagBits::eTransferWrite,
	      vk::ImageLayout::ePreinitialized, vk::ImageLayout::eTransferDstOptimal,
	      VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, textureImage.get(),
	      baseImageRange}});
	tempCommandBuffers[0].get().copyBufferToImage(
	    tempBuffer.buffer.get(), textureImage.get(),
	    vk::ImageLayout::eTransferDstOptimal,
	    {{0, 0, 0, baseImage, {0, 0, 0}, {textureW, textureH, 1}}});
	tempCommandBuffers[0].get().pipelineBarrier(
	    vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer,
	    {}, {}, {},
	    {{vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eTransferRead,
	      vk::ImageLayout::eTransferDstOptimal,
	      vk::ImageLayout::eTransferSrcOptimal, VK_QUEUE_FAMILY_IGNORED,
	      VK_QUEUE_FAMILY_IGNORED, textureImage.get(), baseImageRange},
	     {vk::AccessFlagBits::eHostWrite, vk::AccessFlagBits::eTransferWrite,
	      vk::ImageLayout::ePreinitialized, vk::ImageLayout::eTransferDstOptimal,
	      VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, textureImage.get(),
	      mipmapTail}});
	tempCommandBuffers[0].get().blitImage(
	    textureImage.get(), vk::ImageLayout::eTransferSrcOptimal,
	    textureImage.get(), vk::ImageLayout::eTransferDstOptimal, blits,
	    vk::Filter::eLinear);
	tempCommandBuffers[0].get().pipelineBarrier(
	    vk::PipelineStageFlagBits::eTransfer, stageAfter, {}, {}, {},
	    {{vk::AccessFlagBits::eTransferRead, accessAfter,
	      vk::ImageLayout::eTransferSrcOptimal, layoutAfter,
	      VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, textureImage.get(),
	      baseImageRange},
	     {vk::AccessFlagBits::eTransferWrite, accessAfter,
	      vk::ImageLayout::eTransferDstOptimal, layoutAfter,
	      VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, textureImage.get(),
	      mipmapTail}});
	tempCommandBuffers[0].get().end();

	auto buffer = tempCommandBuffers[0].get();
	auto q = gpu.device.getQueue(gpu.queueFamily, gpu.queueId);
	q.submit({{0, nullptr, nullptr, 1, &buffer, 0, nullptr}}, {});
	q.waitIdle();

	auto imageView = gpu.device.createImageViewUnique(
	    {{},
	     textureImage.get(),
	     vk::ImageViewType::e2D,
	     vk::Format::eR8G8B8A8Unorm,
	     {},
	     {vk::ImageAspectFlagBits::eColor, 0, mipmaps, 0, 1}});

	std::cout << "Texture created " << path << std::endl;
	return {std::move(textureImage),
	        std::move(textureMemory),
	        std::move(imageView),
	        textureW,
	        textureH,
	        mipmaps};
}

#include <iostream>
vk::UniqueShaderModule loadShader(DevicePart gpu, const std::string &path) {
	auto code = loadFile(path);
	auto result = gpu.device.createShaderModuleUnique(
	    {{}, code.size(), reinterpret_cast<const uint32_t *>(code.data())});
	std::cout << "Shader loaded " << path << std::endl;
	return result;
}
