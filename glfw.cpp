#include "glfw.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

GlfwWindow::GlfwWindow()
    : m_window{glfwCreateWindow(600, 600, "", nullptr, nullptr)} {};
GlfwWindow::GlfwWindow(const std::string &name)
    : m_window{glfwCreateWindow(800, 600, name.c_str(), nullptr, nullptr)} {};
GlfwWindow::GlfwWindow(size_t w, size_t h, const std::string &name)
    : m_window{glfwCreateWindow(w, h, name.c_str(), nullptr, nullptr)} {};
GlfwWindow::GlfwWindow(GlfwWindow &&second) noexcept {
	m_window = second.m_window;
	second.m_window = nullptr;
}
GlfwWindow &GlfwWindow::operator=(GlfwWindow &&second) noexcept {
	m_window = second.m_window;
	second.m_window = nullptr;
	return *this;
}
GlfwWindow::operator GLFWwindow *() { return m_window; }
bool GlfwWindow::shouldClose() const { return glfwWindowShouldClose(m_window); }
vk::SurfaceKHR GlfwWindow::createSurface(vk::Instance vkInstance) {
	VkSurfaceKHR_T *result;
	assert(glfwCreateWindowSurface(vkInstance, m_window, nullptr, &result) ==
	       VK_SUCCESS);
	return vk::SurfaceKHR{result};
}
vk::UniqueSurfaceKHR GlfwWindow::createSurfaceUnique(vk::Instance vkInstance) {
	VkSurfaceKHR_T *result;
	assert(glfwCreateWindowSurface(vkInstance, m_window, nullptr, &result) ==
	       VK_SUCCESS);
	return vk::UniqueSurfaceKHR{result, {vkInstance}};
}
GlfwWindow::~GlfwWindow() {
	if (m_window)
		glfwDestroyWindow(m_window);
	m_window = nullptr;
}

GlfwInstance::GlfwInstance() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}
void GlfwInstance::pollEvents() { return glfwPollEvents(); }
std::vector<const char *> GlfwInstance::getRequiredInstanceExtensions() {
	unsigned int count;
	const char **ext = glfwGetRequiredInstanceExtensions(&count);
	return std::vector<const char *>(ext, ext + count);
}
GlfwInstance::~GlfwInstance() { glfwTerminate(); }
