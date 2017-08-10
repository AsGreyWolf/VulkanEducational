#pragma once

#include <string>
#include <vulkan/vulkan.hpp>

/**
 * GLFW Library window class RAII wrapper
 * Should be created by GlfwInstance class
 */
class GlfwWindow {
	friend class GlfwInstance;
	class GLFWwindow *m_window = nullptr;

	GlfwWindow();
	GlfwWindow(const std::string &name);
	GlfwWindow(size_t w, size_t h, const std::string &name = "");

public:
	GlfwWindow(const GlfwWindow &) = delete;
	GlfwWindow(GlfwWindow &&second) noexcept;
	GlfwWindow &operator=(const GlfwWindow &) = delete;
	GlfwWindow &operator=(GlfwWindow &&second) noexcept;
	operator GLFWwindow *();
	bool shouldClose() const;
	/**
	 * Create non-RAII surface, which should be destroyed manually
	 * @param  vkInstance
	 * @return               new surface
	 */
	vk::SurfaceKHR createSurface(vk::Instance vkInstance);
	/**
	 * Create RAII-wrapped surface
	 * @param  vkInstance
	 * @return                     new surface
	 */
	vk::UniqueSurfaceKHR createSurfaceUnique(vk::Instance vkInstance);
	~GlfwWindow();
};
/**
 * Glfw Library RAII wrapper
 */
class GlfwInstance {

public:
	GlfwInstance();
	GlfwInstance(const GlfwInstance &) = delete;
	GlfwInstance(GlfwInstance &&second) = default;
	GlfwInstance &operator=(const GlfwInstance &) = delete;
	GlfwInstance &operator=(GlfwInstance &&second) = default;
	void pollEvents();
	/**
	 * Extensions required for vulkan presentation support
	 */
	std::vector<const char *> getRequiredInstanceExtensions();
	template <typename... Ts> auto createWindow(Ts &&... params) {
		return GlfwWindow{std::forward<Ts>(params)...};
	};
	~GlfwInstance();
};
