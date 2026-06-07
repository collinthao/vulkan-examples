#pragma once
#include "../../core/renderer/vulkanApp/vulkanApp.h"

class BasicApp : public IVulkanApp
{
	public:
	BasicApp() = default;
	~BasicApp(){};
	void processInput(GLFWwindow * window);
	void cleanup(GLFWwindow * window);
};
