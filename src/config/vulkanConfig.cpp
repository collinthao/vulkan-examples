#include "./vulkanConfig.h"

namespace VulkanConfig
{
	VkExtent2D swapChainExtent{};
	const int MAX_FRAMES_IN_FLIGHT = 2;
	const uint32_t MAX_POINT_LIGHTS = 1;
	const uint32_t OBJECT_COUNT = 1;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_8_BIT;
	VkDevice device = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	size_t MESH_COUNT = 1;
	VkQueue graphicsAndComputeQueue;
};
