#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Image
{
	extern uint32_t mipLevels;

	void create(uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t arrayLayers, VkImageCreateFlags flags, VkImageType imageType,VkSampleCountFlagBits numSamples,VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, VkImageLayout imageLayout, VkDevice& device, VkPhysicalDevice& physicalDevice);

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice& physicalDevice);

	VkImageView createView(VkImage image, VkImageView imageView, VkImageViewType viewType, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, uint32_t layerCount, VkDevice& device, VkQueue graphicsAndComputeQueue);

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, uint32_t layerCount, VkDevice& device, VkQueue graphicsAndComputeQueue);

	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount, VkDevice& device, VkQueue graphicsAndComputeQueue);

	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels, uint32_t layerCount, VkDevice& device, VkPhysicalDevice physicalDevice, VkQueue graphicsAndComputeQueue);

	bool hasStencilComponent(VkFormat format);
};
