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
