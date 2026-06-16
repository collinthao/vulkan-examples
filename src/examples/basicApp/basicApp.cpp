#include "./basicApp.h"

void BasicApp::cleanup(GLFWwindow * window)
{
	cleanupSwapChain();

	vkDestroySampler(device, textureSampler, nullptr);
	vkDestroyImageView(device, textureImageView, nullptr);

	for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroyBuffer(device, uniformBuffers[i], nullptr);
		vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
	}

	vkDestroyBuffer(device, indexBuffer, nullptr);
	vkFreeMemory(device, indexBufferMemory, nullptr);
	
	vkDestroyBuffer(device, vertexCubeBuffer, nullptr);
	vkFreeMemory(device, vertexCubeBufferMemory, nullptr);

	for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(device, CommandBuffer::commandPool, nullptr);

	vkDestroyRenderPass(device, renderPasses.renderPass, nullptr);

	vkDestroyDevice(device, nullptr);

	if (enableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(instance, surface, nullptr);
	
	vkDestroyInstance(instance, nullptr);

	glfwDestroyWindow(window);

	glfwTerminate();
};

void BasicApp::processInput(GLFWwindow * window)
{
	camera.cameraSpeed = 10.f * lastFrameTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera.move(FORWARD);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.move(BACKWARD);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.move(LEFT);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.move(RIGHT);
//	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
//		steps += 0.1;
//	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
//		steps -= 0.1;
//	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
//		verticalSteps += 0.1;
//	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
//		verticalSteps -= 0.1;
};
