
#include <iostream>

template <typename T>
VertexBuffer createTempBuffer(DevicePart gpu, const T *data, uint32_t size) {

	const std::array<uint32_t, 1> qIds{{gpu.queueId}};
	uint32_t elements = size;
	size *= sizeof(T);
	auto tempBuffer =
	    gpu.device.createBufferUnique({{},
	                                   size,
	                                   vk::BufferUsageFlagBits::eTransferSrc,
	                                   vk::SharingMode::eExclusive,
	                                   qIds.size(),
	                                   qIds.data()});
	const auto tempBufferReq =
	    gpu.device.getBufferMemoryRequirements(tempBuffer.get());
	auto tempMemory =
	    allocateMemory(gpu, tempBufferReq,
	                   vk::MemoryPropertyFlagBits::eHostVisible |
	                       vk::MemoryPropertyFlagBits::eHostCoherent);
	gpu.device.bindBufferMemory(tempBuffer.get(), tempMemory.get(), 0);
	void *const localMemory = gpu.device.mapMemory(tempMemory.get(), 0, size);
	std::copy(data, data + elements, reinterpret_cast<T *>(localMemory));
	gpu.device.unmapMemory(tempMemory.get());
	std::cout << "Temp buffer created " << size << std::endl;
	return {std::move(tempBuffer), std::move(tempMemory), size};
}

template <typename T>
VertexBuffer createBuffer(DevicePart gpu, const T *data, uint32_t size,
                          vk::BufferUsageFlags usage) {

	const std::array<uint32_t, 1> qIds{{gpu.queueId}};
	uint32_t elements = size;
	size *= sizeof(T);
	auto buffer = gpu.device.createBufferUnique(
	    {{},
	     size,
	     usage | vk::BufferUsageFlagBits::eTransferDst,
	     vk::SharingMode::eExclusive,
	     qIds.size(),
	     qIds.data()});
	const auto bufferReq = gpu.device.getBufferMemoryRequirements(buffer.get());
	auto memory =
	    allocateMemory(gpu, bufferReq, vk::MemoryPropertyFlagBits::eDeviceLocal);
	gpu.device.bindBufferMemory(buffer.get(), memory.get(), 0);

	auto tempBuffer = createTempBuffer(gpu, data, elements);

	auto tempCommandBuffers = gpu.device.allocateCommandBuffersUnique(
	    {gpu.commandPool, vk::CommandBufferLevel::ePrimary, 1});
	tempCommandBuffers[0].get().begin(
	    {vk::CommandBufferUsageFlagBits::eOneTimeSubmit, nullptr});
	tempCommandBuffers[0].get().copyBuffer(tempBuffer.buffer.get(), buffer.get(),
	                                       {{0, 0, size}});
	tempCommandBuffers[0].get().end();

	auto comandBuffer = tempCommandBuffers[0].get();
	auto q = gpu.device.getQueue(gpu.queueFamily, gpu.queueId);
	q.submit({{0, nullptr, nullptr, 1, &comandBuffer, 0, nullptr}}, {});
	q.waitIdle();

	std::cout << "Buffer created " << size << std::endl;

	return {std::move(buffer), std::move(memory), size};
}
