#pragma once

#include "../../core/renderer/vulkanApp/vulkanApp.h"

class ShadowMappingScene : public IVulkanApp
{
	private:
	std::vector<VkImage> shadowMapImages;
	std::vector<VkImage> offScreenImages;
	VkFormat swapChainImageFormat;
	std::vector<VkImageView> shadowMapImageViews;
	std::vector<VkImageView> offScreenImageViews;
	std::vector<VkDeviceMemory> offScreenImageMemories;
	std::vector<VkDeviceMemory> shadowMapImageMemories;
	VkImage textureImage;
	VkImage shadowMapImage;
	VkDeviceMemory textureImageMemory;
	VkDeviceMemory cubemapImageMemory;
	VkSampler shadowSampler;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSetLayout primitiveDescriptorSetLayout;
	VkDescriptorSetLayout stencilDescriptorSetLayout;
	VkDescriptorSetLayout modelDescriptorSetLayout;
	VkDescriptorSetLayout computeDescriptorSetLayout;
	VkDescriptorSetLayout lightDescriptorSetLayout;
	VkDescriptorSetLayout postProcessingDescriptorSetLayout;
	VkDescriptorSetLayout screenSpaceDescriptorSetLayout;
	VkDescriptorSetLayout cubemapDescriptorSetLayout;

	VkDescriptorSetLayoutBinding samplerUniformLayoutBinding{};
	VkDescriptorSetLayoutBinding specularUniformLayoutBinding{};
	VkDescriptorSetLayoutBinding vertexLayoutBinding{};
	VkDescriptorSetLayoutBinding fragmentLayoutBinding{};
	VkDescriptorSetLayoutBinding allStagesUniformLayoutBinding{};

	VkPipelineLayout computePipelineLayout;

	VkPipeline computePipeline;

	VkImage cubemapImage;
	VkImageView cubemapImageView;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	VkImage shadowMapDepthImage;
	VkImageView shadowMapDepthImageView;
	VkDeviceMemory shadowMapDepthImageMemory;

	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	VkImage specularImage;
	VkDeviceMemory specularImageMemory;
	VkImageView specularImageView;
	VkSampler specularSampler;
	
	std::vector<VkBuffer> indexModelBuffers;
	std::vector<VkDeviceMemory> indexModelBufferMemories;
	
	std::vector<VkImage> modelImages;
	std::vector<VkDeviceMemory> modelImageMemories;
	std::vector<VkImageView> modelImageViews;
	std::vector<VkSampler> modelSamplers;
	std::vector<VkFramebuffer> shadowMapFramebuffers;
	std::vector<VkFramebuffer> offScreenFramebuffers;

	VkBuffer vertexCubemapBuffer;
	VkDeviceMemory vertexCubemapBufferMemory;

	std::vector<VkBuffer> cubemapUniformBuffers;
	std::vector<std::vector<VkBuffer>> modelUniformBuffers;
	std::vector<std::vector<VkBuffer>> primitiveUniformBuffers;
	std::vector<std::vector<VkBuffer>> shadowMapUniformBuffers;
	std::vector<std::vector<VkBuffer>> stencilUniformBuffers;
	std::vector<std::vector<VkBuffer>> materialUniformBuffers;
	std::vector<std::vector<VkBuffer>> lightUniformBuffers;
	std::vector<std::vector<VkBuffer>> modelLightUniformBuffers;
	std::vector<std::vector<VkBuffer>> lightObjectUniformBuffers;

	std::vector<VkDeviceMemory> cubemapUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> modelUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> materialUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> primitiveUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> shadowMapUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> stencilUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> lightUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> modelLightUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> lightObjectUniformBuffersMemory;

	std::vector<void*> cubemapUniformBuffersMapped;
	std::vector<std::vector<void*>> modelUniformBuffersMapped;
	std::vector<std::vector<void*>> materialUniformBuffersMapped;
	std::vector<std::vector<void*>> primitiveUniformBuffersMapped;
	std::vector<std::vector<void*>> shadowMapUniformBuffersMapped;
	std::vector<std::vector<void*>> stencilUniformBuffersMapped;
	std::vector<std::vector<void*>> modelLightUniformBuffersMapped;
	std::vector<std::vector<void*>> lightUniformBuffersMapped;
	std::vector<std::vector<void*>> lightObjectUniformBuffersMapped;

	VkDescriptorPool computeDescriptorPool;

	std::vector<VkDescriptorSet> postProcessingDescriptorSets;
	std::vector<VkDescriptorSet> screenSpaceDescriptorSets;
	std::vector<VkDescriptorSet> cubemapDescriptorSets;
	std::vector<VkDescriptorSet> computeDescriptorSets;
	std::vector<std::vector<VkDescriptorSet>> modelDescriptorSets;
	std::vector<std::vector<VkDescriptorSet>> primitiveDescriptorSets;
	std::vector<std::vector<VkDescriptorSet>> shadowMapDescriptorSets;
	std::vector<std::vector<VkDescriptorSet>> stencilDescriptorSets;
	std::vector<std::vector<VkDescriptorSet>> lightDescriptorSets;

	Model * model;

	std::vector<VkBuffer> vertexBuffers;
	std::vector<VkDeviceMemory> vertexBufferMemories;

	std::vector<VkBuffer> shaderStorageBuffers;
	std::vector<VkDeviceMemory> shaderStorageBuffersMemory;
	
	VkBuffer quadIndexBuffer;
	VkDeviceMemory quadIndexBufferMemory;

	std::vector<Vertex> modelVertices;
	std::vector<uint32_t> indices;

	std::vector<VkCommandBuffer> computeCommandBuffers;
	
	std::vector<VkFence> computeInFlightFences;
	std::vector<VkSemaphore> computeFinishedSemaphores;

	PointLight pointLights[4];

	Lights lights;

	const uint32_t PARTICLE_COUNT = 8192;

	void createImageViews();
	void createRenderPass();
	void createShadowMapRenderPass();
	void createPostProcessingRenderPass();
	void createDescriptorSetLayouts();	
	void createComputeDescriptorSetLayout();
	void createPipelines();
	void createComputePipeline();
	void createOffscreenResources();
	void createShadowMapResources();
	void createColorResources();
	void createDepthResources();
	void createFramebuffers();
	void createModel();
	void createTextureImageView(VkImage& image, VkImageView& imageView, VkFormat format, VkImageAspectFlagBits flags);
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void createCubeTextureImage(const std::string imagePath, VkImage& image, VkDeviceMemory& imageMemory);
	void createCubeMapResources();
	void createTextureSampler(VkSampler& sampler);
	void createTextureImages(std::vector<VkImage>& images, std::vector<VkDeviceMemory>& imageMemories);
	void createTextureImageViews(std::vector<VkImage>& images, std::vector<VkImageView>& imageViews);
	void createTextureSamplers(std::vector<VkSampler>& samplers);
	void createShaderStorageBuffers();
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void createVertexBuffers();
	void createQuadIndexBuffer();
	void createModelIndexBuffers();
	void createModelIndexBuffer(std::vector<uint32_t> m_Indices, VkBuffer& modelBuffer,VkDeviceMemory& modelMemory);
	void createUniformBuffers();
	void createGraphicsUniformBuffers();
	void createPrimitiveUniformBuffers();
	void createShadowMapUniformBuffers();
	void createCubemapUniformBuffers();
	void createStencilUniformBuffers();
	void createMaterialUniformBuffers();
	void createLightUniformBuffers();
	void createModelLightUniformBuffers();
	void createLightObjectUniformBuffers();
	void createModelUniformBuffers();
	void createComputeDescriptorPool();
	void createDescriptorSets();
	void createDescriptorPools();
	void createGraphicsDescriptorSets();
	void createPrimitiveDescriptorSets();
	void createShadowMapDescriptorSets();
	void createShadowMapScreenSpaceQuadDescriptorSets();
	void createStencilDescriptorSets();
	void createModelDescriptorSets();
	void createPostProcessingDescriptorSets();
	void createCubemapDescriptorSets();
	void createLightDescriptorSets();
	void createComputeDescriptorSets();
	void createComputeCommandBuffers();
	void createSyncObjects();
	void createSwapChain(GLFWwindow * window);
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void updateUniformBuffer(uint32_t currentImage);
	void recordComputeCommandBuffer(VkCommandBuffer commandBuffer);
	void recreateSwapChain(GLFWwindow * window);
	void cleanupSwapChain();

	template <typename T = Vertex>
	void createVertexBuffer(std::vector<T> vertices, VkBuffer& buffer, VkDeviceMemory& memory)
	{
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		Buffer::create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device, physicalDevice);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		Buffer::create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, memory, device, physicalDevice);

		copyBuffer(stagingBuffer, buffer, bufferSize);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);

	};	

	public:
	ShadowMappingScene() = default;

	void init(GLFWwindow * window);
	void drawFrame(GLFWwindow * window);
	void processInput(GLFWwindow * window);
	void cleanup(GLFWwindow * window);
	void deviceWaitIdle();
	void moveCamera(double xpos, double ypos);

	VkDevice* getDevice();
};
