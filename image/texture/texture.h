#pragma once
#include "../image.h"
#include <string>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <stdexcept>
#include <iostream>

namespace Image::Texture
{
	void create(const std::string imagePath, VkImage& image, VkDeviceMemory& imageMemory, VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsAndComputeQueue);
	void loadTexture(const std::string imagePath, int& width, int& height, int& channels, int layers,stbi_uc*& texture);
};
