#include "./glfwWindowContext.h"
#include <iostream>
//#include "../renderer/vulkanApp/vulkanApp.h"
#include "../renderer/vulkanApp/shadowMappingScene/shadowMappingScene.h"
//#include "../renderer/vulkanApp/omniDirectionalShadowMappingScene/omniDirectionalShadowMappingScene.h"
//#include "../renderer/vulkanApp/grassScene/grassScene.h"

GLFWWindowContext::GLFWWindowContext()
{
	initWindow();
};

void GLFWWindowContext::initWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
};

void GLFWWindowContext::mouse_callback(GLFWwindow * window, double xpos, double ypos)
{
	//Eventually, we want to have either the camera be static or have a class pointer to the move camera method
	// Edit:Use glfwGetCursorPos
	//IVulkanApp::moveCamera(xpos, ypos);
	//GrassScene::moveCamera(xpos, ypos);
	ShadowMappingScene::moveCamera(xpos, ypos);
	//OmniDirectionalShadowMappingScene::moveCamera(xpos, ypos);
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

