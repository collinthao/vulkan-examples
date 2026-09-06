#include "./buffer.h"
#include "../image/image.h"
#include <stdexcept>
#include <iostream>
#include "../../config/vulkanConfig.h"
#include "../../core/commandBuffer/commandBuffer.h"

void Buffer::create(VkDeviceSize size, VkBufferUsageFlags usage , VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkDevice device, VkPhysicalDevice physicalDevice)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create vertex buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = Image::findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, physicalDevice);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate vertex buffer memory!");
	}

	vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void Buffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);
};

