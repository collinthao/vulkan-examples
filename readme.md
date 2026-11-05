Graphic examples using Vulkan API and C++23 I've made to use for learning purposes/reference in other projects.

To run, go to the /build directory in your cli of choice and run:
```
cmake CMakeLists.txt ..
```
```
cmake --build .
```

***TODO***

**Deferred Renderer**
- Fix issue with deferred buffer memory alignment. There are values that should not be there and are incorrect

**SSAO**
- This is all fucked up. Look into how to fix

**MipMaps and Image Transitions**
- Figure out why the transitions and pipeline barriers aren't syncing correctly. You're getting this error:
	validation layer: vkCmdBlitImage(): dstImage Cannot use VkImage 0x1040000000104[textures.hdr.image] (layer 0, mip 0) with specific layout VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL that doesn't match the previous known layout VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL.
	The Vulkan spec states: dstImageLayout must specify the layout of the image subresources of dstImage specified in pRegions at the time this command is executed on a VkDevice (https://docs.vulkan.org/spec/latest/chapters/copies.html#VUID-vkCmdBlitImage-dstImageLayout-00226)
	validation layer: vkCmdPipelineBarrier(): pImageMemoryBarriers[0].image (VkImage 0x1040000000104[textures.hdr.image]) cannot transition the layout of aspect=1, level=0, layer=0 from VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL when the previous known layout is VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL.
	The Vulkan spec states: If layouts are not ignored, oldLayout must be VK_IMAGE_LAYOUT_UNDEFINED or the current layout of the image subresources affected by the barrier (https://docs.vulkan.org/spec/latest/chapters/synchronization.html#VUID-VkImageMemoryBarrier-oldLayout-01197)
	Mipwidth: 800
	validation layer: vkCmdBlitImage(): dstImage Cannot use VkImage 0x1040000000104[textures.hdr.image] (layer 0, mip 1) with specific layout VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL that doesn't match the previous known layout VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL.
	The Vulkan spec states: dstImageLayout must specify the layout of the image subresources of dstImage specified in pRegions at the time this command is executed on a VkDevice (https://docs.vulkan.org/spec/latest/chapters/copies.html#VUID-vkCmdBlitImage-dstImageLayout-00226)
	validation layer: vkCmdPipelineBarrier(): pImageMemoryBarriers[0].subresourceRange.baseMipLevel (2) + levelCount (10) is 12, which is greater than the image mipLevels (11).
	The Vulkan spec states: If subresourceRange.levelCount is not VK_REMAINING_MIP_LEVELS, subresourceRange.baseMipLevel + subresourceRange.levelCount must be less than or equal to the mipLevels specified in VkImageCreateInfo when image was created (https://docs.vulkan.org/spec/latest/chapters/synchronization.html#VUID-VkImageMemoryBarrier-subresourceRange-01724)
	Mipwidth: 400
	validation layer: vkCmdBlitImage(): srcImage Cannot use VkImage 0x1040000000104[textures.hdr.image] (layer 0, mip 1) with specific layout VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL that doesn't match the previous known layout VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL.
	T

XDG_SESSION_TYPE=x11


		VkCommandBuffer commandBufferIT = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);		
		
		VkImageMemoryBarrier barrier
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = swapChainImages[imageIndex]
		};

		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; 
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags srcStage;
		VkPipelineStageFlags dstStage;
		
		srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;		

		vkCmdPipelineBarrier
		(
			commandBufferIT,
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);	
		
		CommandBuffer::endSingleTimeCommands(commandBufferIT, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);



		commandBufferIT = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);		
		
		barrier = 
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.dstAccessMask = 0,
			.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = swapChainImages[imageIndex]
		};

		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; 
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;		
		dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

		vkCmdPipelineBarrier
		(
			commandBufferIT,
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);	
		
		CommandBuffer::endSingleTimeCommands(commandBufferIT, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);


