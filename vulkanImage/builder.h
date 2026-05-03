#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "./vulkanImage.h"

namespace Image 
{	
	class Builder
	{
		private:
	virtual uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice& physicalDevice) = 0;

		public:
			Builder();
			virtual ~Builder(){};	
			
			virtual Builder& setMipLevels(uint32_t mipLevels) = 0;
			virtual Builder& setArrayLayers(uint32_t arrayLayers) = 0;
			virtual Builder& setImageFlags(VkImageCreateFlags imageFlags) = 0;
			virtual Builder& setImageType(VkImageType imageType) = 0;
			virtual Builder& setSampleCount(VkSampleCountFlagBits numSamples) = 0;
			virtual Builder& setFormat(VkFormat imageFormat) = 0;
			virtual Builder& setTiling(VkImageTiling imageTiling) = 0;
			virtual Builder& setUsageFlags(VkImageUsageFlags imageUsageFlags) = 0;
			virtual Builder& setMemoryPropertyFlags(VkMemoryPropertyFlags memoryProperties) = 0;

			virtual Builder& setLayout(VkImageLayout imageLayout) = 0;
			virtual VulkanImage build(VkDevice& device, VkPhysicalDevice& physicalDevice) = 0;
	};
};
