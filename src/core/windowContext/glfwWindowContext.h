#pragma once
#include "../renderer/vulkanApp/vulkanApp.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>

class IVulkanApp;

class GLFWWindowContext
{
	public:
	GLFWWindowContext(std::shared_ptr<IVulkanApp> vkApp);
	static inline bool framebufferResized = false;
	static inline uint32_t WIDTH = 800;
	static inline uint32_t HEIGHT = 600;
	GLFWwindow* window;
	std::shared_ptr<IVulkanApp> vkApp;
	void initWindow();
	void* getWindow();
	static uint32_t getWindowWidth(){return WIDTH;};
	static uint32_t getWindowHeight(){return HEIGHT;};
	void static framebufferResizeCallback(GLFWwindow* window, int width, int height);
	void static mouse_callback(GLFWwindow * window, double xpos, double ypos);
};
