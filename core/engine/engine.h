#pragma once
#ifndef ENGINE_H
#define ENGINE_H
#include "../renderer/vulkanRenderer.h"
//#include "../renderer/vulkanApp/shadowMappingScene/shadowMappingScene.h"
//#include "../renderer/vulkanApp/omniDirectionalShadowMappingScene/omniDirectionalShadowMappingScene.h"
//#include "../renderer/vulkanApp/grassScene/grassScene.h"
#include "../windowContext/glfwWindowContext.h"
#include <memory>

template <typename App, typename Window, typename Renderer>
class Engine
{
	public:
		std::unique_ptr<Window> windowContext;
		std::unique_ptr<Renderer> renderer;
		std::shared_ptr<App> app;

		Engine<App, Window, Renderer>()
		{
			app = std::make_shared<App>();
			windowContext = std::make_unique<Window>(app);
			renderer = std::make_unique<Renderer>(static_cast<GLFWwindow*>(windowContext->getWindow()), app);	
		};

		void start()
		{
			renderer->render(static_cast<GLFWwindow*>(windowContext->getWindow()));
		}


};

#endif
