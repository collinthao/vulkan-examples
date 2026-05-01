#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class VulkanImage
{
	public:
	VulkanImage(VkImage image);
	VkImage image;
};
