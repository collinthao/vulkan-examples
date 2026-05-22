#pragma once
#ifndef ENGINE_H
#define ENGINE_H
#include "../renderer/vulkanRenderer.h"
//#include "../renderer/vulkanApp/shadowMappingScene/shadowMappingScene.h"
//#include "../renderer/vulkanApp/omniDirectionalShadowMappingScene/omniDirectionalShadowMappingScene.h"
#include "../renderer/vulkanApp/grassScene/grassScene.h"
#include "../windowContext/glfwWindowContext.h"
#include <memory>

class Engine
{
	public:
	Engine();
	std::unique_ptr<IWindowContext> windowContext = std::make_unique<GLFWWindowContext>();
	IVulkanApp * vkApp = new GrassScene();
	std::unique_ptr<IRenderer> renderer = std::make_unique<VulkanRenderer>(static_cast<GLFWwindow*>(windowContext->getWindow()), vkApp);
	
	void start();
};

#endif
