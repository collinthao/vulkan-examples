#pragma once

#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <stdexcept>
#include <map>
#include <set>
#include <unordered_map>
#include <optional>
#include <iostream>
#include <limits>
#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <random>
#include <stb_image.h>
#include <tiny_obj_loader.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/hash.hpp>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include "../../core/vulkanImage/vulkanImageBuilder.h"
#include "../../core/renderer/vulkanApp/vulkanApp.h"

namespace fs = std::filesystem;

struct Material
{
	alignas(16)glm::vec3 specular;
	alignas(4)float shininess;
};

struct DirectionalLight
{
	alignas(16)glm::vec3 ambient;
	alignas(16)glm::vec3 diffuse;
	alignas(16)glm::vec3 specular;
	alignas(16)glm::vec3 direction;
};

struct SpotLight
{
	alignas(16)glm::vec3 ambient;
	alignas(16)glm::vec3 diffuse;
	alignas(16)glm::vec3 specular;
	alignas(16)glm::vec3 position;
	alignas(16)glm::vec3 direction;
	alignas(4)float cutOff;
	alignas(4)float outerCutOff;
};

struct PointLight
{
	alignas(16)glm::mat4 model;
	alignas(16)glm::mat4 view;
	alignas(16)glm::mat4 projection;
	alignas(16)glm::vec3 ambient;
	alignas(16)glm::vec3 diffuse;
	alignas(16)glm::vec3 specular;
	alignas(16)glm::vec3 position;
	alignas(16)glm::vec3 color;
	alignas(4)float constant;
	alignas(4)float linear;
	alignas(4)float quadratic;
};

struct Lights
{
	alignas(16)PointLight pointLights[1];	
	alignas(16)DirectionalLight directionalLight;
	alignas(16)SpotLight spotLight;
};

struct Light 
{
	alignas(16)glm::vec3 ambient;
	alignas(16)glm::vec3 diffuse;
	alignas(16)glm::vec3 specular;
	alignas(16)glm::vec3 position;
	alignas(16)glm::vec3 direction;
	alignas(4)float constant;
	alignas(4)float linear;
	alignas(4)float quadratic;
	alignas(4)float cutOff;
	alignas(4)float outerCutOff;
};

struct UniformBufferObjectModel
{
	 alignas(16) glm::mat4 model;
	 alignas(16) glm::mat4 view;
	 alignas(16) glm::mat4 proj;
	 alignas(16) glm::mat4 lightSpaceMatrix;
	 alignas(16) glm::vec3 fragColor;
	 alignas(16) glm::vec3 cameraPos;
	 alignas(4)  int matIndex;
};

struct UniformBufferObject
{
	float deltaTime;
};

namespace std 
{
	template<> struct hash<Vertex>
	{
		size_t operator()(Vertex const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec3>()(vertex.normal) << 1) >> 1 ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif
const std::vector<const char*> deviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

const std::vector<const char*> validationLayers =
{
	"VK_LAYER_KHRONOS_validation"
};

class OmniDirectionalShadowMappingScene : public IVulkanApp
{
	private:
	VkInstance instance;
	PipelineBuilder pipelineBuilder{};
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkQueue graphicsAndComputeQueue;
	VkQueue presentQueue;
	VkSwapchainKHR swapChain;
	std::vector<VkImage> shadowMapImages;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImage> offScreenImages;
	VkFormat swapChainImageFormat;
	std::vector<VkImageView> shadowMapImageViews;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkImageView> offScreenImageViews;
	std::vector<VkDeviceMemory> offScreenImageMemories;
	std::vector<VkDeviceMemory> shadowMapImageMemories;
	struct
	{
		VkRenderPass shadowMapRenderPass;
		VkRenderPass renderPass;
		VkRenderPass postProcessingRenderPass;
	} renderPasses{};
	VkImage textureImage;
	VkImage shadowMapImage;
	VkDeviceMemory textureImageMemory;
	VkDeviceMemory cubemapImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
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
	
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
		
	std::vector<VkBuffer> indexModelBuffers;
	std::vector<VkDeviceMemory> indexModelBufferMemories;
	
	std::vector<VkImage> modelImages;
	std::vector<VkDeviceMemory> modelImageMemories;
	std::vector<VkImageView> modelImageViews;
	std::vector<VkSampler> modelSamplers;
	std::vector<VkFramebuffer> shadowMapFramebuffers;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	std::vector<VkFramebuffer> offScreenFramebuffers;

	VkBuffer vertexCubeBuffer;
	VkBuffer vertexCubemapBuffer;
	VkDeviceMemory vertexCubeBufferMemory;
	VkDeviceMemory vertexCubemapBufferMemory;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkBuffer> cubemapUniformBuffers;
	std::vector<std::vector<VkBuffer>> modelUniformBuffers;
	std::vector<std::vector<VkBuffer>> primitiveUniformBuffers;
	std::vector<std::vector<VkBuffer>> shadowMapUniformBuffers;
	std::vector<std::vector<VkBuffer>> stencilUniformBuffers;
	std::vector<std::vector<VkBuffer>> materialUniformBuffers;
	std::vector<std::vector<VkBuffer>> lightUniformBuffers;
	std::vector<std::vector<VkBuffer>> modelLightUniformBuffers;
	std::vector<std::vector<VkBuffer>> lightObjectUniformBuffers;

	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<VkDeviceMemory> cubemapUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> modelUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> materialUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> primitiveUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> shadowMapUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> stencilUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> lightUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> modelLightUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> lightObjectUniformBuffersMemory;

	std::vector<void*> uniformBuffersMapped;
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

	std::vector<VkDescriptorSet> descriptorSets;
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
	
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	
	std::vector<VkFence> computeInFlightFences;
	std::vector<VkSemaphore> computeFinishedSemaphores;

	PointLight pointLights[4];

	Lights lights;

	uint32_t currentFrame = 0;

	const std::string MODEL_PATH = "models/Sponza-master/sponza.obj";
	const std::string MODEL_TEXTURE_DIRECTORY = "models/Sponza-master/";
	const std::string TEXTURE_PATH = "textures/container.png";
	const std::string CUBEMAP_PATH = "textures/skybox/";
	const std::string SPECULAR_PATH = "textures/container_specular.png";
	const uint32_t PARTICLE_COUNT = 8192;
	const float FAR_PLANE = 400.f;
	float lastFrameTime = 0.f;
	double lastTime = 0.f;

	void createInstance();
	void createSurface(GLFWwindow * window);
	void setupDebugMessenger();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	bool isDeviceSuitable(VkPhysicalDevice device);
	int rateDeviceSuitability(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	void createSwapChain(GLFWwindow * window);
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
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow * window);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkSampleCountFlagBits getMaxUsableSampleCount();
	std::vector<const char*>getRequiredExtensions();
	void createCubeTextureImage(const std::string imagePath, VkImage& image, VkDeviceMemory& imageMemory);
	void createCubeMapResources();
	void createTextureSampler(VkSampler& sampler);
	void createTextureImages(std::vector<VkImage>& images, std::vector<VkDeviceMemory>& imageMemories);
	void createTextureImageViews(std::vector<VkImage>& images, std::vector<VkImageView>& imageViews);
	void createTextureSamplers(std::vector<VkSampler>& samplers);
	void loadModel();
	void createShaderStorageBuffers();
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void createVertexBuffers();
	void createIndexBuffer();
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
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void updateUniformBuffer(uint32_t currentImage);
	void recordComputeCommandBuffer(VkCommandBuffer commandBuffer);
	void recreateSwapChain(GLFWwindow * window);
	void cleanupSwapChain();
	void setDescriptorSetLayoutBindings();

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
	OmniDirectionalShadowMappingScene() = default;

	void init(GLFWwindow * window);
	void drawFrame(GLFWwindow * window);
	void processInput(GLFWwindow * window);
	void cleanup(GLFWwindow * window);
	void deviceWaitIdle();

	static Camera camera;
	static glm::vec3 cameraPos;
	static glm::vec3 cameraFront;
	static glm::vec3 cameraUp; 
	static VkExtent2D swapChainExtent;

	VkDevice* getDevice();
	VkDevice device;

	void moveCamera(double xpos, double ypos);

	static VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance, 
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
		const VkAllocationCallbacks* pAllocator, 
		VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}

	}

	void static getInstance();
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	};


	static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}

	static std::vector<char> readFile(const std::string& filename)
	{

		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;

	}

	static inline glm::vec3 cubePositions[] = {
		glm::vec3( 1.0f, 4.f,  0.0f),
		glm::vec3( 2.0f, 4.f, -15.0f),
		glm::vec3(-1.5f, 4.f, -2.5f),
		glm::vec3(-3.8f, 4.f, -12.3f),
		glm::vec3( 2.4f, 4.f, -3.5f),
		glm::vec3(-1.7f, 4.f, -7.5f),
		glm::vec3( 1.3f, 4.f, -2.5f),
		glm::vec3( 1.5f, 4.f, -2.5f),
		glm::vec3( 1.5f, 4.f, -1.5f),
		glm::vec3(-1.3f, 4.f, -1.5f)
	};

	static inline const std::vector<Vertex> cubemapVertices = {
	    {{-1.0f,  1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,0.0f,-1.0f},{0.0f,0.0f}},
	    {{-1.0f, -1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,0.0f,-1.0f},{1.0f,0.0f}},
	    {{ 1.0f, -1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,0.0f,-1.0f},{1.0f,1.0f}},
	    {{ 1.0f, -1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,0.0f,-1.0f},{1.0f,1.0f}},
	    {{ 1.0f,  1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,0.0f,-1.0f},{0.0f,1.0f}},
	    {{-1.0f,  1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,0.0f,-1.0f},{0.0f,0.0f}},

	    {{-1.0f, -1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,0.0f, 1.0f},{0.0f,0.0f}},
	    {{-1.0f, -1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,0.0f, 1.0f},{1.0f,0.0f}},
	    {{-1.0f,  1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,0.0f, 1.0f},{1.0f,1.0f}},
	    {{-1.0f,  1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,0.0f, 1.0f},{1.0f,1.0f}},
	    {{-1.0f,  1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,0.0f, 1.0f},{0.0f,1.0f}},
	    {{-1.0f, -1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,0.0f, 1.0f},{0.0f,0.0f}},

	    {{ 1.0f, -1.0f, -1.0f},{0.5f,0.5f,0.5f},{1.0f,0.0f,0.0f},{1.0f,0.0f}},
	    {{ 1.0f, -1.0f,  1.0f},{0.5f,0.5f,0.5f},{1.0f,0.0f,0.0f},{1.0f,1.0f}},
	    {{ 1.0f,  1.0f,  1.0f},{0.5f,0.5f,0.5f},{1.0f,0.0f,0.0f},{0.0f,1.0f}},
	    {{ 1.0f,  1.0f,  1.0f},{0.5f,0.5f,0.5f},{1.0f,0.0f,0.0f},{0.0f,1.0f}},
	    {{ 1.0f,  1.0f, -1.0f},{0.5f,0.5f,0.5f},{1.0f,0.0f,0.0f},{0.0f,0.0f}},
	    {{ 1.0f, -1.0f, -1.0f},{0.5f,0.5f,0.5f},{1.0f,0.0f,0.0f},{1.0f,0.0f}},

	    {{-1.0f, -1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,-1.0f,0.0f},{0.0f,1.0f}},
	    {{-1.0f,  1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,-1.0f,0.0f},{1.0f,1.0f}},
	    {{ 1.0f,  1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,-1.0f,0.0f},{1.0f,0.0f}},
	    {{ 1.0f,  1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,-1.0f,0.0f},{1.0f,0.0f}},
	    {{ 1.0f, -1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,-1.0f,0.0f},{0.0f,0.0f}},
	    {{-1.0f, -1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,-1.0f,0.0f},{0.0f,1.0f}},

	    {{-1.0f,  1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,1.0f,0.0f},{0.0f,1.0f}},
	    {{ 1.0f,  1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,1.0f,0.0f},{1.0f,1.0f}},
	    {{ 1.0f,  1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,1.0f,0.0f},{1.0f,0.0f}},
	    {{ 1.0f,  1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,1.0f,0.0f},{1.0f,0.0f}},
	    {{-1.0f,  1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,1.0f,0.0f},{0.0f,0.0f}},
	    {{-1.0f,  1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,1.0f,0.0f},{0.0f,1.0f}},

	    {{-1.0f, -1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,-1.0f,0.0f},{0.0f,1.0f}},
	    {{-1.0f, -1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,-1.0f,0.0f},{1.0f,1.0f}},
	    {{ 1.0f, -1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,-1.0f,0.0f},{1.0f,0.0f}},
	    {{ 1.0f, -1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,-1.0f,0.0f},{1.0f,0.0f}},
	    {{-1.0f, -1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,-1.0f,0.0f},{0.0f,0.0f}},
	    {{ 1.0f, -1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,-1.0f,0.0f},{1.0f,1.0f}},
	};

	static inline const std::vector<Vertex> cubeVertices = {
		{{ -0.5f, -0.5f, -0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f,  0.0f, -1.0f}, {.0, .0}},
		{{  0.5f, -0.5f, -0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f,  0.0f, -1.0f}, {1., 0.}}, 
		{{  0.5f,  0.5f, -0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f,  0.0f, -1.0f}, {1., 1.}}, 
		{{ -0.5f,  0.5f, -0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f,  0.0f, -1.0f}, {0., 1.}}, 

		{{ -0.5f, -0.5f,  0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f,  0.0f, 1.0f} , {.0, .0}},
		{{  0.5f, -0.5f,  0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f,  0.0f, 1.0f} , {1., .0}},
		{{  0.5f,  0.5f,  0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f,  0.0f, 1.0f} , {1., 1.}},
		{{ -0.5f,  0.5f,  0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f,  0.0f, 1.0f} , {.0, 1.}},

		{{ -0.5f,  0.5f,  0.5f},{ 0.5f, 0.5f, 0.5f},{-1.0f,  0.0f,  0.0f}, {1., 0.}},
		{{ -0.5f,  0.5f, -0.5f},{ 0.5f, 0.5f, 0.5f},{-1.0f,  0.0f,  0.0f}, {1., 1.}},
		{{ -0.5f, -0.5f, -0.5f},{ 0.5f, 0.5f, 0.5f},{-1.0f,  0.0f,  0.0f}, {.0, 1.}},
		{{ -0.5f, -0.5f,  0.5f},{ 0.5f, 0.5f, 0.5f},{-1.0f,  0.0f,  0.0f}, {.0, .0}},

		{{  0.5f,  0.5f,  0.5f},{ 0.5f, 0.5f, 0.5f},{ 1.0f,  0.0f,  0.0f}, {1., .0}},
		{{  0.5f,  0.5f, -0.5f},{ 0.5f, 0.5f, 0.5f},{ 1.0f,  0.0f,  0.0f}, {1., 1.}},
		{{  0.5f, -0.5f, -0.5f},{ 0.5f, 0.5f, 0.5f},{ 1.0f,  0.0f,  0.0f}, {.0, 1.}},
		{{  0.5f, -0.5f,  0.5f},{ 0.5f, 0.5f, 0.5f},{ 1.0f,  0.0f,  0.0f}, {0., .0}},

		{{ -0.5f, -0.5f, -0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f, -1.0f,  0.0f}, {0., 1.}},
		{{  0.5f, -0.5f, -0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f, -1.0f,  0.0f}, {1., 1.}},
		{{  0.5f, -0.5f,  0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f, -1.0f,  0.0f}, {1., 0.}},
		{{ -0.5f, -0.5f,  0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f, -1.0f,  0.0f}, {0., 0.}},

		{{ -0.5f,  0.5f, -0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f,  1.0f,  0.0f}, {.0, 1.}},
		{{  0.5f,  0.5f, -0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f,  1.0f,  0.0f}, {1., 1.}},
		{{  0.5f,  0.5f,  0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f,  1.0f,  0.0f}, {1., .0}},
		{{ -0.5f,  0.5f,  0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f,  1.0f,  0.0f}, {0., 0.}},
	};

	

	static inline const std::vector<uint32_t> cubeIndices = {
	    // BACK (-Z)
	    0, 2, 1,
	    2, 0, 3,

	    // FRONT (+Z)
	    5, 6, 4,
	    7, 4, 6,

	    // LEFT (-X)
	    9,10, 8,
	   11, 8,10,

	    // RIGHT (+X)
	   12,14,13,
	   14,12,15,

	    // BOTTOM (-Y)
	   17,18,16,
	   19,16,18,

	    // TOP (+Y)
	   20,22,21,
	   22,20,23
	};


/*
static inline const std::vector<uint32_t> cubeIndices = {
    // BACK (-Z)
    0, 2, 1,
    2, 0, 3,

    // FRONT (+Z)
    4, 6, 5,
    6, 4, 7,

    // LEFT (-X)
    8,10, 9,
   10, 8,11,

    // RIGHT (+X)
   12,14,13,
   14,12,15,

    // BOTTOM (-Y)
   16,18,17,
   18,16,19,

    // TOP (+Y)
   20,22,21,
   22,20,23
};
*/
	static inline const std::vector<uint32_t> quadIndices = {
	    4, 5, 6,
	    6, 7, 4,
	};
};
