#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H
#include "./renderer.h"
#include "./vulkanApp/vulkanApp.h"
#include <memory>

class VulkanRenderer : public IRenderer
{


	private:
	std::shared_ptr<IVulkanApp> vkApp;
	void drawFrame(GLFWwindow * window);
	
	public:
	VulkanRenderer(GLFWwindow * window, std::shared_ptr<IVulkanApp> vkApp);
	void render(GLFWwindow * window);
	void processInput(GLFWwindow * window);
	void cleanup(GLFWwindow * window);
};

#endif
