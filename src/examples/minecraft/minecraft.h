#pragma once
#include "../../core/renderer/vulkanApp/vulkanApp.h"
#include <unordered_set>

enum BlockType
{
	DIRT = 0,
	GRASS,
	WATER
};

class Minecraft : public IVulkanApp
{
	public:
	Minecraft() = default;
	~Minecraft(){};

	const std::string DIRT_BLOCK_PATH = ROOT_DIR + "/resource/textures/dirtBlock/";
	const std::string WATER_BLOCK_PATH = ROOT_DIR + "/resource/textures/waterBlock/";

	std::vector<int> chunks;

	std::vector<std::vector<InstanceData>> instanceData;
	std::vector<VkBuffer> instanceBuffer;
	std::vector<VkDeviceMemory> instanceBufferMemory;
	std::unordered_set<glm::vec3> renderedChunks;

	int instanceCount = 0;
	static constexpr int CHUNK_SIZE = 32;

	VkDeviceMemory cubemapImageMemory;
	VkDeviceMemory cubemapDirtImageMemory;
	VkDeviceMemory cubemapWaterImageMemory;
	VkDeviceMemory cubemapSkyBoxImageMemory;

	VkImage cubemapImage;
	VkImage cubemapDirtImage;
	VkImage cubemapWaterImage;
	VkImage cubemapSkyBoxImage;

	VkImageView cubemapImageView;
	VkImageView cubemapDirtImageView;
	VkImageView cubemapWaterImageView;
	VkImageView cubemapSkyBoxImageView;

	std::vector<VkBuffer> cubemapUniformBuffers;
	std::vector<VkBuffer> cubemapSkyBoxUniformBuffers;

	std::vector<VkDeviceMemory> cubemapUniformBuffersMemory;
	std::vector<VkDeviceMemory> cubemapSkyBoxUniformBuffersMemory;
	std::vector<void*> cubemapUniformBuffersMapped;
	std::vector<void*> cubemapSkyBoxUniformBuffersMapped;

	std::vector<VkDescriptorSet> cubemapDescriptorSets;
	std::vector<VkDescriptorSet> cubemapSkyBoxDescriptorSets;

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
	void createCubemapDescriptorSets(std::vector<VkBuffer>& buffers, VkImageView& imageView, std::vector<VkDescriptorSet>& descriptorSets, Pipeline& pipeline);
	void createCubemapDescriptorSets(std::vector<VkBuffer>& buffers, std::vector<VkImageView> imageView, std::vector<VkDescriptorSet>& descriptorSets, Pipeline& pipeline);
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
