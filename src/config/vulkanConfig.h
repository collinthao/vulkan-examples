#ifndef VULKANCONFIG_H
#define VULKANCONFIG_H
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace VulkanConfig
{
	extern VkExtent2D swapChainExtent;
	extern const int MAX_FRAMES_IN_FLIGHT;
	extern const uint32_t MAX_POINT_LIGHTS;
	extern const uint32_t OBJECT_COUNT;
	extern VkSampleCountFlagBits msaaSamples;
	extern VkDevice device;
	extern VkPhysicalDevice physicalDevice;
	extern size_t MESH_COUNT;
	extern VkQueue graphicsAndComputeQueue;
};

#endif
