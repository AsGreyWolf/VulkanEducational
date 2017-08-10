#include "pipeline.hpp"

std::tuple<vk::UniquePipeline, vk::UniquePipelineLayout> createPipeline(
    DevicePart gpu, vk::Extent2D extent,
    const std::vector<vk::VertexInputBindingDescription> &vertexBindings,
    const std::vector<vk::VertexInputAttributeDescription> &vertexAttribs,
    const std::vector<vk::DescriptorSetLayout> &descriptorSetLayouts,
    vk::RenderPass renderpass, uint32_t subpass) {
	auto vert = loadShader(gpu, "shaders/vert.spv");
	auto frag = loadShader(gpu, "shaders/frag.spv");
	auto tesc = loadShader(gpu, "shaders/tesc.spv");
	auto tese = loadShader(gpu, "shaders/tese.spv");
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStage = {
	    {{}, vk::ShaderStageFlagBits::eVertex, vert.get(), "main", nullptr},
	    {{}, vk::ShaderStageFlagBits::eFragment, frag.get(), "main", nullptr},
	    {{},
	     vk::ShaderStageFlagBits::eTessellationControl,
	     tesc.get(),
	     "main",
	     nullptr},
	    {{},
	     vk::ShaderStageFlagBits::eTessellationEvaluation,
	     tese.get(),
	     "main",
	     nullptr}};
	vk::PipelineVertexInputStateCreateInfo vertexInputStage{
	    {},
	    static_cast<uint32_t>(vertexBindings.size()),
	    vertexBindings.data(),
	    static_cast<uint32_t>(vertexAttribs.size()),
	    vertexAttribs.data()};
	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStage{
	    {}, vk::PrimitiveTopology::ePatchList, false};
	vk::PipelineTessellationStateCreateInfo tesselationStage{{}, 3};
	vk::Viewport viewport{0,    0,   extent.width * 1.0f, extent.height * 1.0f,
	                      0.0f, 1.0f};
	vk::Rect2D scissor{{0, 0}, extent};
	vk::PipelineViewportStateCreateInfo clippingStage{
	    {}, 1, &viewport, 1, &scissor};
	vk::PipelineRasterizationStateCreateInfo rasterStage{
	    {},
	    false,
	    false,
	    vk::PolygonMode::eFill,
	    vk::CullModeFlagBits::eNone,
	    vk::FrontFace::eCounterClockwise,
	    false,
	    0.0f,
	    0.0f,
	    0.0f,
	    1.0f};
	vk::PipelineMultisampleStateCreateInfo multisampleStage{
	    {}, vk::SampleCountFlagBits::e1, false, 1.0f, nullptr, false, false};
	vk::PipelineDepthStencilStateCreateInfo depthStencilStage{
	    {}, true, true, vk::CompareOp::eLessOrEqual, false, false, {},
	    {}, 0.0f, 1.0f};
	std::vector<vk::PipelineColorBlendAttachmentState> blendAttachments = {
	    {false, vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
	     vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
	     vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
	         vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA}};
	vk::PipelineColorBlendStateCreateInfo blendStage{
	    {},
	    false,
	    vk::LogicOp::eCopy,
	    static_cast<uint32_t>(blendAttachments.size()),
	    blendAttachments.data(),
	    {{0.0f, 0.0f, 0.0f, 0.0f}}};
	auto pipelineLayout = gpu.device.createPipelineLayoutUnique(
	    {{},
	     static_cast<uint32_t>(descriptorSetLayouts.size()),
	     descriptorSetLayouts.data(),
	     0,
	     nullptr});
	auto pipeline = gpu.device.createGraphicsPipelineUnique(
	    vk::PipelineCache{},
	    {{},
	     static_cast<uint32_t>(shaderStage.size()),
	     shaderStage.data(),
	     &vertexInputStage,
	     &inputAssemblyStage,
	     &tesselationStage,
	     &clippingStage,
	     &rasterStage,
	     &multisampleStage,
	     &depthStencilStage,
	     &blendStage,
	     nullptr,
	     pipelineLayout.get(),
	     renderpass,
	     subpass,
	     vk::Pipeline{},
	     -1});
	return {std::move(pipeline), std::move(pipelineLayout)};
}
