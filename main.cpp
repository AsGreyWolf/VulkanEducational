#define GLM_FORCE_RADIANS
#define GLM_DEPTH_CLIP_SPACE GLM_DEPTH_ZERO_TO_ONE
#include "glfw.hpp"
#include "loaders.hpp"
#include "pipeline.hpp"
#include "utilities.hpp"
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <vulkan/vulkan.hpp>

using namespace std;

struct vertex {
	glm::vec3 pos;
	glm::vec2 UV;
	glm::vec3 norm;
};

struct ubo {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

vector<const char *> layers = {"VK_LAYER_LUNARG_standard_validation"};
vector<const char *> extensions = {"VK_EXT_debug_report"}; //, "VK_KHR_display"
vector<const char *> ld_extensions = {"VK_KHR_swapchain"};

int main() {
	GlfwInstance glfw{};
	auto window = glfw.createWindow(1920ul, 1080ul, "VK");

	vk::ApplicationInfo appInfo{"testname", 1, "testengine", 1};
	auto exts = glfw.getRequiredInstanceExtensions();
	copy(extensions.begin(), extensions.end(), back_inserter(exts));
	auto instance = vk::createInstanceUnique({{},
	                                          &appInfo,
	                                          static_cast<uint32_t>(layers.size()),
	                                          layers.data(),
	                                          static_cast<uint32_t>(exts.size()),
	                                          exts.data()});

	auto debugCallback = createDebugCallback(instance.get());

	auto surface = window.createSurfaceUnique(instance.get());
	printInfo(instance.get(), surface.get());
	auto devices = instance.get().enumeratePhysicalDevices();
	auto deviceIt =
	    find_if(devices.begin(), devices.end(), [&](const vk::PhysicalDevice &d) {
		    return d.getSurfaceSupportKHR(0, surface.get());
		   });
	assert(deviceIt != devices.end());
	vector<float> priorities{1.0f};
	auto qProps = deviceIt->getQueueFamilyProperties();
	auto qFamily = static_cast<uint32_t>(
	    distance(qProps.begin(), find_if(qProps.begin(), qProps.end(),
	                                     [&](const vk::QueueFamilyProperties &qp) {
		                                     return qp.queueFlags &
		                                            vk::QueueFlagBits::eGraphics;
		                                    })));
	vector<vk::DeviceQueueCreateInfo> qcis = {
	    {{},
	     qFamily,
	     static_cast<uint32_t>(priorities.size()),
	     priorities.data()}};
	vk::PhysicalDeviceFeatures features{};
	features.samplerAnisotropy = true;
	features.tessellationShader = true;
	auto ldevice =
	    deviceIt->createDeviceUnique({{},
	                                  static_cast<uint32_t>(qcis.size()),
	                                  qcis.data(),
	                                  0,
	                                  nullptr,
	                                  static_cast<uint32_t>(ld_extensions.size()),
	                                  ld_extensions.data(),
	                                  &features});
	auto q = ldevice.get().getQueue(qFamily, 0);
	auto commandPool = ldevice.get().createCommandPoolUnique({{}, qFamily});
	DevicePart gpu{*deviceIt, ldevice.get(), commandPool.get(), qFamily, 0};
	array<uint32_t, 1> qs{{qFamily}};

	auto extent = deviceIt->getSurfaceCapabilitiesKHR(surface.get()).currentExtent;

	array<vk::AttachmentDescription, 2> attDescr{
	    {{{},
	      vk::Format::eB8G8R8A8Unorm,
	      vk::SampleCountFlagBits::e1,
	      vk::AttachmentLoadOp::eClear,
	      vk::AttachmentStoreOp::eStore,
	      vk::AttachmentLoadOp::eDontCare,
	      vk::AttachmentStoreOp::eDontCare,
	      vk::ImageLayout::eUndefined,
	      vk::ImageLayout::ePresentSrcKHR},
	     {{},
	      vk::Format::eD32Sfloat,
	      vk::SampleCountFlagBits::e1,
	      vk::AttachmentLoadOp::eClear,
	      vk::AttachmentStoreOp::eDontCare,
	      vk::AttachmentLoadOp::eDontCare,
	      vk::AttachmentStoreOp::eDontCare,
	      vk::ImageLayout::eUndefined,
	      vk::ImageLayout::eDepthStencilAttachmentOptimal}}};
	array<vk::AttachmentReference, 2> attRef{
	    {{0, vk::ImageLayout::eColorAttachmentOptimal},
	     {1, vk::ImageLayout::eDepthStencilAttachmentOptimal}}};
	vk::SubpassDescription subpassDescr{{},      vk::PipelineBindPoint::eGraphics,
	                                    0,       nullptr,
	                                    1,       attRef.data(),
	                                    nullptr, attRef.data() + 1,
	                                    0,       nullptr};
	vk::SubpassDependency dep{VK_SUBPASS_EXTERNAL,
	                          0,
	                          vk::PipelineStageFlagBits::eColorAttachmentOutput,
	                          vk::PipelineStageFlagBits::eColorAttachmentOutput,
	                          {},
	                          vk::AccessFlagBits::eColorAttachmentRead |
	                              vk::AccessFlagBits::eColorAttachmentWrite,
	                          {}};
	auto renderPass = ldevice.get().createRenderPassUnique(
	    {{},
	     static_cast<uint32_t>(attDescr.size()),
	     attDescr.data(),
	     1,
	     &subpassDescr,
	     1,
	     &dep});

	std::vector<vk::VertexInputBindingDescription> vertexBindings = {
	    {0, sizeof(vertex), vk::VertexInputRate::eVertex}};
	std::vector<vk::VertexInputAttributeDescription> vertexAttribs = {
	    {0, 0, vk::Format::eR32G32B32Sfloat, 0},
	    {1, 0, vk::Format::eR32G32Sfloat, 12},
	    {2, 0, vk::Format::eR32G32B32Sfloat, 20}};
	array<vk::DescriptorSetLayoutBinding, 1> matrixLayouts = {
	    {{0, vk::DescriptorType::eUniformBuffer, 1,
	      vk::ShaderStageFlagBits::eVertex |
	          vk::ShaderStageFlagBits::eTessellationEvaluation,
	      nullptr}}};
	auto matrixDescriptorSetLayout = ldevice.get().createDescriptorSetLayoutUnique(
	    {{}, static_cast<uint32_t>(matrixLayouts.size()), matrixLayouts.data()});
	array<vk::DescriptorSetLayoutBinding, 2> textureLayouts = {
	    {{0, vk::DescriptorType::eCombinedImageSampler, 1,
	      vk::ShaderStageFlagBits::eFragment, nullptr},
	     {1, vk::DescriptorType::eCombinedImageSampler, 1,
	      vk::ShaderStageFlagBits::eTessellationEvaluation, nullptr}}};
	auto textureDescriptorSetLayout =
	    ldevice.get().createDescriptorSetLayoutUnique(
	        {{},
	         static_cast<uint32_t>(textureLayouts.size()),
	         textureLayouts.data()});
	vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
	    {matrixDescriptorSetLayout.get(), textureDescriptorSetLayout.get()}};
	auto[pipeline, pipelineLayout] =
	    createPipeline(gpu, extent, vertexBindings, vertexAttribs,
	                   descriptorSetLayouts, renderPass.get(), 0);

	auto swapchain = ldevice.get().createSwapchainKHRUnique(
	    {{},
	     surface.get(),
	     3,
	     vk::Format::eB8G8R8A8Unorm,
	     vk::ColorSpaceKHR::eSrgbNonlinear,
	     extent,
	     1,
	     vk::ImageUsageFlagBits::eColorAttachment,
	     vk::SharingMode::eExclusive,
	     qs.size(),
	     qs.data(),
	     vk::SurfaceTransformFlagBitsKHR::eIdentity,
	     vk::CompositeAlphaFlagBitsKHR::eOpaque,
	     vk::PresentModeKHR::eMailbox,
	     true,
	     {}});
	auto images = ldevice.get().getSwapchainImagesKHR(swapchain.get());
	vector<vk::UniqueImageView> imageViews;
	transform(images.begin(), images.end(), back_inserter(imageViews),
	          [&](const vk::Image &img) {
		          return ldevice.get().createImageViewUnique(
		              {{},
		               img,
		               vk::ImageViewType::e2D,
		               vk::Format::eB8G8R8A8Unorm,
		               {},
		               {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}});
		         });
	vector<vk::UniqueImage> depthTexs;
	transform(imageViews.begin(), imageViews.end(), back_inserter(depthTexs),
	          [&](const vk::UniqueImageView &v) {
		          return ldevice.get().createImageUnique(
		              {{},
		               vk::ImageType::e2D,
		               vk::Format::eD32Sfloat,
		               {extent.width, extent.height, 1},
		               1,
		               1,
		               vk::SampleCountFlagBits::e1,
		               vk::ImageTiling::eOptimal,
		               vk::ImageUsageFlagBits::eDepthStencilAttachment,
		               vk::SharingMode::eExclusive,
		               static_cast<uint32_t>(qs.size()),
		               qs.data(),
		               vk::ImageLayout::eUndefined});
		         });
	vector<vk::UniqueDeviceMemory> depthMemories;
	transform(depthTexs.begin(), depthTexs.end(), back_inserter(depthMemories),
	          [&](const vk::UniqueImage &v) {
		          auto reqs = ldevice.get().getImageMemoryRequirements(v.get());
		          auto memory = allocateMemory(
		              gpu, reqs, vk::MemoryPropertyFlagBits::eDeviceLocal);
		          ldevice.get().bindImageMemory(v.get(), memory.get(), 0);
		          return memory;
		         });
	vector<vk::UniqueImageView> depthTexViews;
	transform(depthTexs.begin(), depthTexs.end(), back_inserter(depthTexViews),
	          [&](const vk::UniqueImage &v) {
		          return ldevice.get().createImageViewUnique(
		              {{},
		               v.get(),
		               vk::ImageViewType::e2D,
		               vk::Format::eD32Sfloat,
		               {},
		               {vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1}});
		         });
	vector<vk::UniqueFramebuffer> framebuffers;
	framebuffers.reserve(imageViews.size());
	for (size_t i = 0; i < imageViews.size(); i++) {
		array<vk::ImageView, 2> iv = {
		    {{imageViews[i].get()}, {depthTexViews[i].get()}}};
		framebuffers.push_back(
		    ldevice.get().createFramebufferUnique({{},
		                                           renderPass.get(),
		                                           static_cast<uint32_t>(iv.size()),
		                                           iv.data(),
		                                           extent.width,
		                                           extent.height,
		                                           1}));
	}

	vector<vertex> verts = {
	    {{1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},
	    {{0.5f, 0.0f, 0.0f}, {0.5f, 0.5f}, {0.0f, 0.0f, -1.0f}},
	    {{0.0f, 1.0f, 0.0f}, {0.0f, 0.5f}, {0.0f, 0.0f, -1.0f}},

	    {{0.5f, 0.0f, 0.0f}, {0.5f, 0.5f}, {0.0f, 0.0f, -1.0f}},
	    {{0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
	    {{-0.5f, 0.0f, 0.0f}, {0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}},

	    {{0.0f, 1.0f, 0.0f}, {0.0f, 0.5f}, {0.0f, 0.0f, -1.0f}},
	    {{-0.5f, 0.0f, 0.0f}, {0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}},
	    {{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},

	    {{0.0f, 1.0f, 0.0f}, {0.0f, 0.5f}, {0.0f, 0.0f, -1.0f}},
	    {{0.5f, 0.0f, 0.0f}, {0.5f, 0.5f}, {0.0f, 0.0f, -1.0f}},
	    {{-0.5f, 0.0f, 0.0f}, {0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}}};

	auto vertexBuffer = createBuffer(gpu, verts.data(), verts.size());
	auto texture = loadTexture(gpu, "cobblestone.jpg", 10);
	auto textureSampler =
	    ldevice.get().createSamplerUnique({{},
	                                       vk::Filter::eLinear,
	                                       vk::Filter::eLinear,
	                                       vk::SamplerMipmapMode::eLinear,
	                                       vk::SamplerAddressMode::eRepeat,
	                                       vk::SamplerAddressMode::eRepeat,
	                                       vk::SamplerAddressMode::eRepeat,
	                                       0,
	                                       true,
	                                       16,
	                                       false,
	                                       vk::CompareOp::eNever,
	                                       0,
	                                       texture.mipmaps - 1.0f,
	                                       vk::BorderColor::eFloatTransparentBlack,
	                                       false});
	auto textureDisp = loadTexture(gpu, "cobblestone_disp.jpg", 10);
	auto textureDispSampler =
	    ldevice.get().createSamplerUnique({{},
	                                       vk::Filter::eLinear,
	                                       vk::Filter::eLinear,
	                                       vk::SamplerMipmapMode::eLinear,
	                                       vk::SamplerAddressMode::eRepeat,
	                                       vk::SamplerAddressMode::eRepeat,
	                                       vk::SamplerAddressMode::eRepeat,
	                                       0,
	                                       true,
	                                       16,
	                                       false,
	                                       vk::CompareOp::eNever,
	                                       0,
	                                       textureDisp.mipmaps - 1.0f,
	                                       vk::BorderColor::eFloatTransparentBlack,
	                                       false});

	ubo uniform{glm::mat4(1.0f), glm::lookAt(glm::vec3{0.0f, 0.0f, -1.0f},
	                                         glm::vec3{0.0f, 0.0f, 1.0f},
	                                         glm::vec3{0.0f, 1.0f, 0.0f}),
	            // glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f)
	            glm::perspectiveFov(2.0, extent.width * 1.0, extent.height * 1.0,
	                                0.1, 2.0)};
	auto uniformBuffer =
	    ldevice.get().createBufferUnique({{},
	                                      sizeof(ubo),
	                                      vk::BufferUsageFlagBits::eUniformBuffer,
	                                      vk::SharingMode::eExclusive,
	                                      qs.size(),
	                                      qs.data()});
	auto uniformBufferReq =
	    ldevice.get().getBufferMemoryRequirements(uniformBuffer.get());
	auto uniformMemory =
	    allocateMemory(gpu, uniformBufferReq,
	                   vk::MemoryPropertyFlagBits::eHostVisible |
	                       vk::MemoryPropertyFlagBits::eHostCoherent);
	ldevice.get().bindBufferMemory(uniformBuffer.get(), uniformMemory.get(), 0);
	void *localMemory =
	    ldevice.get().mapMemory(uniformMemory.get(), 0, uniformBufferReq.size);
	*reinterpret_cast<ubo *>(localMemory) = uniform;
	ldevice.get().unmapMemory(uniformMemory.get());

	vector<vk::DescriptorPoolSize> descriptorSizes = {
	    {vk::DescriptorType::eUniformBuffer, 1},
	    {vk::DescriptorType::eCombinedImageSampler, 2}};
	auto descriptorPool = ldevice.get().createDescriptorPoolUnique(
	    {{},
	     2,
	     static_cast<uint32_t>(descriptorSizes.size()),
	     descriptorSizes.data()});
	auto descriptorSets = ldevice.get().allocateDescriptorSetsUnique(
	    {descriptorPool.get(), static_cast<uint32_t>(descriptorSetLayouts.size()),
	     descriptorSetLayouts.data()});
	vk::DescriptorBufferInfo descriptorBufferInfo{uniformBuffer.get(), 0,
	                                              sizeof(ubo)};
	vk::DescriptorImageInfo descriptorImageInfo{
	    textureSampler.get(), texture.wholeImageView.get(),
	    vk::ImageLayout::eShaderReadOnlyOptimal};
	vk::DescriptorImageInfo descriptorDispImageInfo{
	    textureDispSampler.get(), textureDisp.wholeImageView.get(),
	    vk::ImageLayout::eShaderReadOnlyOptimal};
	ldevice.get().updateDescriptorSets(
	    {{descriptorSets[0].get(), 0, 0, 1, vk::DescriptorType::eUniformBuffer,
	      nullptr, &descriptorBufferInfo, nullptr},
	     {descriptorSets[1].get(), 0, 0, 1,
	      vk::DescriptorType::eCombinedImageSampler, &descriptorImageInfo, nullptr,
	      nullptr},
	     {descriptorSets[1].get(), 1, 0, 1,
	      vk::DescriptorType::eCombinedImageSampler, &descriptorDispImageInfo,
	      nullptr, nullptr}},
	    {});

	auto commandBuffers = ldevice.get().allocateCommandBuffersUnique(
	    {commandPool.get(), vk::CommandBufferLevel::ePrimary,
	     static_cast<uint32_t>(framebuffers.size())});
	std::array<vk::ClearValue, 2> clearColor{{
	    {array<float, 4>{{0.0f, 0.0f, 0.0f, 1.0f}}},
	    vk::ClearDepthStencilValue{1.0f, 0},
	}};
	for (size_t i = 0; i < commandBuffers.size(); i++) {
		commandBuffers[i].get().begin(
		    {vk::CommandBufferUsageFlagBits::eSimultaneousUse, nullptr});
		commandBuffers[i].get().beginRenderPass(
		    {renderPass.get(),
		     framebuffers[i].get(),
		     {{0, 0}, extent},
		     static_cast<uint32_t>(clearColor.size()),
		     clearColor.data()},
		    vk::SubpassContents::eInline);
		commandBuffers[i].get().bindPipeline(vk::PipelineBindPoint::eGraphics,
		                                     pipeline.get());
		commandBuffers[i].get().bindVertexBuffers(0, {vertexBuffer.buffer.get()},
		                                          array<vk::DeviceSize, 1>{{0}});
		commandBuffers[i].get().bindDescriptorSets(
		    vk::PipelineBindPoint::eGraphics, pipelineLayout.get(), 0,
		    {descriptorSets[0].get(), descriptorSets[1].get()}, {});
		commandBuffers[i].get().draw(verts.size(), 1, 0, 0);
		commandBuffers[i].get().endRenderPass();
		commandBuffers[i].get().end();
	}
	vk::UniqueSemaphore imageAvailableSemaphore =
	    ldevice.get().createSemaphoreUnique({});
	vk::UniqueSemaphore renderFinishedSemaphore =
	    ldevice.get().createSemaphoreUnique({});
	auto start_time = chrono::high_resolution_clock::now();
	auto prev_time = start_time;
	size_t frames = 0;
	auto swapc = swapchain.get();
	auto beginSemaphore = imageAvailableSemaphore.get();
	vector<vk::PipelineStageFlags> semaphoreStages = {
	    vk::PipelineStageFlagBits::eColorAttachmentOutput};
	auto endSemaphore = renderFinishedSemaphore.get();

	while (!window.shouldClose()) {
		localMemory =
		    ldevice.get().mapMemory(uniformMemory.get(), 0, uniformBufferReq.size);
		*reinterpret_cast<ubo *>(localMemory) = uniform;
		ldevice.get().unmapMemory(uniformMemory.get());
		auto img =
		    ldevice.get()
		        .acquireNextImageKHR(swapchain.get(), numeric_limits<uint64_t>::max(),
		                             imageAvailableSemaphore.get(), {})
		        .value;
		auto buffer = commandBuffers[img].get();
		q.submit({{static_cast<uint32_t>(semaphoreStages.size()), &beginSemaphore,
		           semaphoreStages.data(), 1, &buffer, 1, &endSemaphore}},
		         {});
		q.presentKHR({1, &endSemaphore, 1, &swapc, &img, nullptr});
		glfw.pollEvents();
		auto cur_time = chrono::high_resolution_clock::now();
		frames++;
		if (cur_time - prev_time >= 1s) {
			cout << "[FPS] " << frames << endl;
			frames = 0;
			prev_time = cur_time;
		}
		uniform.model = glm::rotate(
		    glm::mat4{},
		    chrono::duration_cast<chrono::milliseconds>(cur_time - start_time)
		            .count() /
		        1000.0f,
		    glm::vec3{0.0f, 1.0f, 0.0f});
	}
	q.waitIdle();
}
