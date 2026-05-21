#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H
#include "./renderer.h"
#include "./vulkanApp/vulkanApp.h"

class VulkanRenderer : public IRenderer
{


	private:
	IVulkanApp * vkApp;
	void drawFrame(GLFWwindow * window);
	
	public:
	VulkanRenderer(GLFWwindow * window, IVulkanApp * vkApp);
	void render(GLFWwindow * window);
	void processInput(GLFWwindow * window);
	void cleanup(GLFWwindow * window);
};

#endif
