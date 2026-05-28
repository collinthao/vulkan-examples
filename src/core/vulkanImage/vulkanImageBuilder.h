#pragma once
#include "./builder.h"
#include "../../../vulkanConfig.h"

class VulkanImageBuilder : protected Image::Builder
{
	private:
	uint32_t levels;
	uint32_t layers;
	VkImageCreateFlags flags;
	VkImageType type;
	VkSampleCountFlagBits samples;
	VkFormat format;
	VkImageTiling tiling;
	VkImageUsageFlags usageFlags;
	VkMemoryPropertyFlags memProperties;
	VkImageLayout layout;

	
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice& physicalDevice)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		throw std::runtime_error("failed to find a suitable memory type!");
	}



	public:
	VulkanImageBuilder(){};
	~VulkanImageBuilder(){};

	VulkanImageBuilder& setMipLevels(uint32_t mipLevels)
	{
		levels = mipLevels;
		return *this;
	};

	VulkanImageBuilder& setArrayLayers(uint32_t arrayLayers)
	{
		layers = arrayLayers;
		return *this;
	};

	VulkanImageBuilder& setImageFlags(VkImageCreateFlags imageFlags)
	{
		flags = imageFlags;
		return *this;
	};

	VulkanImageBuilder& setImageType(VkImageType imageType)
	{
		type = imageType;
		return *this;
	};

	VulkanImageBuilder& setSampleCount(VkSampleCountFlagBits numSamples)
	{
		samples = numSamples;
		return *this;
	};

	VulkanImageBuilder& setFormat(VkFormat imageFormat)
	{
		format = imageFormat;
		return *this;
	};

	VulkanImageBuilder& setTiling(VkImageTiling imageTiling)
	{
		tiling = imageTiling;
		return *this;
	};

	VulkanImageBuilder& setUsageFlags(VkImageUsageFlags imageUsageFlags)
	{
		usageFlags = imageUsageFlags;
		return *this;
	};
	VulkanImageBuilder& setMemoryPropertyFlags(VkMemoryPropertyFlags memoryProperties)
	{
		memProperties = memoryProperties;
		return *this;
	};

	VulkanImageBuilder& setLayout(VkImageLayout imageLayout)
	{
		layout = imageLayout;
		return *this;
	};

	VulkanImage build(VkDevice& device, VkPhysicalDevice& physicalDevice)
	{
		VkImage image;
		VkDeviceMemory imageMemory;
		uint32_t width = VulkanConfig::swapChainExtent.width;
		uint32_t height = VulkanConfig::swapChainExtent.height;
	
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = type;
		imageInfo.extent.width = static_cast<uint32_t>(width);
		imageInfo.extent.height = static_cast<uint32_t>(height);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = levels;
		imageInfo.arrayLayers = layers;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = layout;
		imageInfo.usage = usageFlags;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = samples;
		imageInfo.flags = flags;

		if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, memProperties, physicalDevice);
		
		if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate image memory!");
		}

		return VulkanImage(image, imageMemory);
	}
};
