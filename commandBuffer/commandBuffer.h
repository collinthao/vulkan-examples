#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

namespace CommandBuffer
{
	extern VkCommandPool commandPool;
	extern std::vector<VkCommandBuffer> commandBuffers;

	extern VkCommandBuffer beginSingleTimeCommands(VkDevice& device);
	extern void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue graphicsAndComputeQueue, VkDevice& device);
	extern void createCommandBuffers(VkDevice& device);
	extern void createCommandPool(VkPhysicalDevice physicalDevice, VkDevice& device, VkSurfaceKHR& surface);
};
