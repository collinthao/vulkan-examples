#include <iostream>
#include "./engine.h"
#include "../renderer/vulkanApp/vulkanApp.h"

Engine::Engine(){};

void Engine::start()
{
	renderer->render(static_cast<GLFWwindow*>(windowContext->getWindow()));
}
