#pragma once

#include "loaders.hpp"
#include <glm/glm.hpp>
#include <vector>

vk::UniquePipeline createPipeline(
    DevicePart gpu, vk::Extent2D extent,
    const std::vector<vk::VertexInputBindingDescription> &vertexBindings,
    const std::vector<vk::VertexInputAttributeDescription> &vertexAttribs,
    vk::PipelineLayout pipelineLayout, vk::RenderPass renderpass,
    uint32_t subpass);
