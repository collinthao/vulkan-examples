#pragma once

namespace vk
{
	struct Buffer
	{
		void * 	 	mapped;
		VkBuffer 	buffer;
		VkDeviceMemory 	memory;
	};
	
	struct Descriptor
	{
		VkDescriptorSetLayout 	layout;
		VkDescriptorSet		set;
	};
};

