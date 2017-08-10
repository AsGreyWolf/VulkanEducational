#pragma once

#include <vulkan/vulkan.hpp>

vk::UniqueDebugReportCallbackEXT createDebugCallback(vk::Instance instance);
void printInfo(vk::Instance instance, vk::SurfaceKHR surface);
