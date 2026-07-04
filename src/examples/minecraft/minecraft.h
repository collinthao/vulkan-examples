#pragma once
#include "../../core/renderer/vulkanApp/vulkanApp.h"
#include <unordered_set>

class Minecraft : public IVulkanApp
{
	public:
	Minecraft() = default;
	~Minecraft(){};

	std::vector<int> chunks;
	

	std::vector<std::vector<InstanceData>> instanceData;
	std::vector<VkBuffer> instanceBuffer;
	std::vector<VkDeviceMemory> instanceBufferMemory;
	std::unordered_set<glm::vec3> renderedChunks;

	int instanceCount = 0;
	static constexpr int CHUNK_SIZE = 32;

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
	void createInstanceBuffers(glm::vec3 offset);
	void generateTerrain();
	void drawFrame(GLFWwindow * window);
};
