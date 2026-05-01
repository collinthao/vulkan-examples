#pragma once
#include "./builder.h"

class VulkanImageBuilder : public Image::Builder
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

};
