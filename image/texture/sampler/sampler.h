#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Image::Texture::Sampler
{
	void createTextureSampler(VkSampler& sampler, VkDevice device, VkPhysicalDevice physicalDevice);
};
