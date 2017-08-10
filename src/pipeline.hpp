#pragma once

#include "loaders.hpp"
#include <glm/glm.hpp>
#include <tuple>
#include <vector>

std::tuple<vk::UniquePipeline, vk::UniquePipelineLayout> createPipeline(
    DevicePart gpu, vk::Extent2D extent,
    const std::vector<vk::VertexInputBindingDescription> &vertexBindings,
    const std::vector<vk::VertexInputAttributeDescription> &vertexAttribs,
    const std::vector<vk::DescriptorSetLayout> &descriptorSetLayouts,
    vk::RenderPass renderpass, uint32_t subpass);
