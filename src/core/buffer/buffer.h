#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Buffer
{
	void create(VkDeviceSize size, VkBufferUsageFlags usage , VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkDevice device, VkPhysicalDevice physicalDevice);

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
}
