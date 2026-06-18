#pragma once
#include "../../core/renderer/vulkanApp/vulkanApp.h"

class GrassScene : public IVulkanApp
{
	private:
	const int MAX_INSTANCE_COUNT = 100000;

	std::vector<VkImage> shadowMapImages;
	std::vector<VkImage> offScreenImages;
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
	VkDescriptorSetLayout grassDescriptorSetLayout;
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

	VkBuffer instanceBuffer;
	VkDeviceMemory instanceBufferMemory;

	VkBuffer vertexTriangleBuffer;
	VkBuffer vertexCubemapBuffer;
	VkDeviceMemory vertexTriangleBufferMemory;
	VkDeviceMemory vertexCubemapBufferMemory;

	std::vector<VkBuffer> cubemapUniformBuffers;
	std::vector<VkBuffer> instanceUniformBuffers;
	std::vector<std::vector<VkBuffer>> modelUniformBuffers;
	std::vector<std::vector<VkBuffer>> grassUniformBuffers;
	std::vector<std::vector<VkBuffer>> shadowMapUniformBuffers;
	std::vector<std::vector<VkBuffer>> stencilUniformBuffers;
	std::vector<std::vector<VkBuffer>> materialUniformBuffers;
	std::vector<std::vector<VkBuffer>> lightUniformBuffers;
	std::vector<std::vector<VkBuffer>> modelLightUniformBuffers;
	std::vector<std::vector<VkBuffer>> lightObjectUniformBuffers;

	std::vector<VkDeviceMemory> cubemapUniformBuffersMemory;
	std::vector<VkDeviceMemory> instanceUniformBuffersMemory;

	std::vector<std::vector<VkDeviceMemory>> modelUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> materialUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> grassUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> shadowMapUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> stencilUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> lightUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> modelLightUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> lightObjectUniformBuffersMemory;

	std::vector<void*> cubemapUniformBuffersMapped;
	std::vector<void*> instanceUniformBuffersMapped;
	std::vector<std::vector<void*>> modelUniformBuffersMapped;
	std::vector<std::vector<void*>> materialUniformBuffersMapped;
	std::vector<std::vector<void*>> grassUniformBuffersMapped;
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
	std::vector<std::vector<VkDescriptorSet>> grassDescriptorSets;
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
	std::vector<InstanceData> instanceData;
	std::vector<uint32_t> indices;

	std::vector<VkCommandBuffer> computeCommandBuffers;
	
	
	std::vector<VkFence> computeInFlightFences;
	std::vector<VkSemaphore> computeFinishedSemaphores;

	PointLight pointLights[4];

	Lights lights;

	const uint32_t PARTICLE_COUNT = 8196;
	const float FAR_PLANE = 400.f;

	void createImageViews();
	void createRenderPass();
	void createShadowMapRenderPass();
	void createPostProcessingRenderPass();
	void createDescriptorSetLayouts();	
	void createComputeDescriptorSetLayout();
	void createComputePipeline();
	void createOffscreenResources();
	void createShadowMapResources();
	void createColorResources();
	void createSwapChain(GLFWwindow * window);
	void createDepthResources();
	void createFramebuffers();
	void createModel();
	void createPipelines();
	void createTextureImageView(VkImage& image, VkImageView& imageView, VkFormat format, VkImageAspectFlagBits flags);
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void createCubeTextureImage(const std::string imagePath, VkImage& image, VkDeviceMemory& imageMemory);
	void createCubeMapResources();
	void createTextureSampler(VkSampler& sampler);
	void createTextureImages(std::vector<VkImage>& images, std::vector<VkDeviceMemory>& imageMemories);
	void createTextureImageViews(std::vector<VkImage>& images, std::vector<VkImageView>& imageViews);
	void createTextureSamplers(std::vector<VkSampler>& samplers);
	void createShaderStorageBuffers();
	void createVertexBuffers();
	void createInstanceBuffers();
	void createQuadIndexBuffer();
	void createModelIndexBuffers();
	void createModelIndexBuffer(std::vector<uint32_t> m_Indices, VkBuffer& modelBuffer,VkDeviceMemory& modelMemory);
	void createUniformBuffers();
	void createGrassUniformBuffers();
	void createShadowMapUniformBuffers();
	void createCubemapUniformBuffers();
	void createStencilUniformBuffers();
	void createInstanceUniformBuffers();
	void createMaterialUniformBuffers();
	void createLightUniformBuffers();
	void createModelLightUniformBuffers();
	void createLightObjectUniformBuffers();
	void createModelUniformBuffers();
	void createComputeDescriptorPool();
	void createDescriptorSets();
	void createDescriptorPools();
	void createGraphicsDescriptorSets();
	void createGrassDescriptorSets();
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
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void updateUniformBuffer(uint32_t currentImage);
	void recordComputeCommandBuffer(VkCommandBuffer commandBuffer);
	void recreateSwapChain(GLFWwindow * window);
	void cleanupSwapChain();
	void setDescriptorSetLayoutBindings();
	
	public:
	GrassScene() = default;

	void init(GLFWwindow * window);
//      Can always fix later...
//	void drawFrame(GLFWwindow * window);
	void processInput(GLFWwindow * window);
	void cleanup(GLFWwindow * window);

	static VkExtent2D swapChainExtent;
};
