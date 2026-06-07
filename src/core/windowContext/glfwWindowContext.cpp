#include "./glfwWindowContext.h"
#include <iostream>
//#include "../renderer/vulkanApp/vulkanApp.h"
//#include "../renderer/vulkanApp/shadowMappingScene/shadowMappingScene.h"
//#include "../renderer/vulkanApp/omniDirectionalShadowMappingScene/omniDirectionalShadowMappingScene.h"
//#include "../renderer/vulkanApp/grassScene/grassScene.h"

GLFWWindowContext::GLFWWindowContext(std::shared_ptr<IVulkanApp> vkApp)
: vkApp(vkApp)
{
	initWindow();
};

void GLFWWindowContext::initWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);

	window = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
};

void GLFWWindowContext::mouse_callback(GLFWwindow * window, double xpos, double ypos)
{
	auto windowContext = reinterpret_cast<GLFWWindowContext*>(glfwGetWindowUserPointer(window));
	windowContext->vkApp->moveCamera(xpos, ypos);
}

void GLFWWindowContext::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto windowContext = reinterpret_cast<GLFWWindowContext*>(glfwGetWindowUserPointer(window));
	windowContext->framebufferResized = true;
}

void* GLFWWindowContext::getWindow()
{
	return window;
}

