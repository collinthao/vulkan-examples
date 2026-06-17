#pragma once
#include "../../core/renderer/vulkanApp/vulkanApp.h"

class Minecraft : public IVulkanApp
{
	public:
	Minecraft() = default;
	~Minecraft(){};

	int GRASS_BLOCK_COUNT = 0;
	std::array<std::array<int, 100>, 100> randomTerrainPositions;

	std::vector<InstanceData> instanceData;
	VkBuffer instanceBuffer;
	VkDeviceMemory instanceBufferMemory;

	VkDeviceMemory cubemapImageMemory;
	VkDescriptorSetLayout cubemapDescriptorSetLayout;
	VkImage cubemapImage;
	VkImageView cubemapImageView;
	std::vector<VkBuffer> cubemapUniformBuffers;
	std::vector<VkDeviceMemory> cubemapUniformBuffersMemory;
	std::vector<void*> cubemapUniformBuffersMapped;
	std::vector<VkDescriptorSet> cubemapDescriptorSets;
	VkBuffer vertexCubemapBuffer;
	VkDeviceMemory vertexCubemapBufferMemory;

	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	void init(GLFWwindow* window);
	void processInput(GLFWwindow * window);
	void cleanup(GLFWwindow * window);
	void createVertexBuffers();
	void createCubeTextureImage(const std::string imagePath, VkImage& image, VkDeviceMemory& imageMemory);
	void createCubeMapResources();
	void createCubemapUniformBuffers();
	void createCubemapDescriptorSets();
	void createGraphicsDescriptorSets();
	void createDescriptorSets();
	void createUniformBuffers();
	void createColorResources();
	void createDepthResources();
	void createRenderPass();
	void createFramebuffers();
	void updateUniformBuffer(uint32_t currentImage);
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void recreateSwapChain(GLFWwindow * window);
	void createInstanceBuffers();
};
