#include "./vulkanImage.h"

VulkanImage::VulkanImage(VkImage image, VkDeviceMemory imageMemory)
: image(image), memory(imageMemory)
{}
