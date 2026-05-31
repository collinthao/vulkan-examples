#pragma once
#include "./builder.h"
#include "../../../vulkanConfig.h"

class DescriptorBuilder : protected Descriptors::Builder
{
	public:
	DescriptorBuilder(){};
	
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	std::vector<VkDescriptorType> types;
	uint32_t count;
		
	DescriptorBuilder& setCount(uint32_t poolCount)
	{
		count = poolCount;
		return *this;	
	}

	DescriptorBuilder& setBindings(std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings)
	{

		for (size_t i = 0; i < setLayoutBindings.size(); i++)
		{
		}

		bindings = setLayoutBindings; 

		for (size_t i = 0; i < bindings.size(); i++)
		{
		}
		return *this;
	};

	DescriptorBuilder& setTypes(std::vector<VkDescriptorType> descriptorTypes)
	{
		types = descriptorTypes;	
		return *this;
	}

	Descriptor build(VkDevice& device)
	{
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorPool      descriptorPool;

		for (size_t i = 0; i < bindings.size(); i++)
		{
			bindings[i].binding = i;
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		{
			std::cout << "Layout info: \n";
			throw std::runtime_error("Failed to create descriptor set layout!");
		}
		
		std::vector<VkDescriptorPoolSize> poolSizes{types.size()};

		for (size_t i = 0; i < types.size(); i++)
		{
			poolSizes[i].type = types[i];
			poolSizes[i].descriptorCount = static_cast<uint32_t>(count * VulkanConfig::MAX_FRAMES_IN_FLIGHT);
		}

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(count * VulkanConfig::MAX_FRAMES_IN_FLIGHT);

		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");
		}

		return Descriptor(descriptorSetLayout, descriptorPool);
	}
};
