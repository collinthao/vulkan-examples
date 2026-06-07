#pragma once
#include <string>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "../../buffer/buffer.h"

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
#include "../../../model/model.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../../../camera/camera.h"
#include "../../pipeline/pipelineBuilder.h"
#include "../../vulkanImage/vulkanImageBuilder.h"
#include "../../pipeline/pipelineManager.h"
#include "../../queueFamily/queueFamily.h"
#include "../../../config/vulkanConfig.h"
#include "../../../core/image/image.h"
#include "../../../core/image/texture/texture.h"
#include "../../../core/image/texture/sampler/sampler.h"
#include "../../../core/commandBuffer/commandBuffer.h"
#include "../../../core/buffer/buffer.h"
#include "../../../core/windowContext/glfwWindowContext.h"

namespace fs = std::filesystem;

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

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

struct UniformInstance
{
	alignas(16) glm::vec3 offset;
};

struct UniformBufferObjectModel
{
	 alignas(16) glm::mat4 model;
	 alignas(16) glm::mat4 view;
	 alignas(16) glm::mat4 proj;
	 alignas(16) glm::mat4 lightSpaceMatrix;
	 alignas(16) glm::vec3 fragColor;
	 alignas(16) glm::vec3 cameraPos;
	 alignas(4)  float deltaTime;
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

class IVulkanApp
{
	public:
	VkSurfaceKHR surface;
	VkInstance instance;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkQueue graphicsAndComputeQueue;
	VkQueue presentQueue;
	VkSwapchainKHR swapChain;
	VkFormat swapChainImageFormat;
	VkBuffer vertexCubeBuffer;
	VkDeviceMemory vertexCubeBufferMemory;
	static Camera camera;
	const float FAR_PLANE = 400.f;
	static glm::vec3 cameraPos;
	static glm::vec3 cameraFront;
	static glm::vec3 cameraUp; 
	float lastFrameTime = 0.f;
	double lastTime = 0.f;

	std::vector<VkFramebuffer> swapChainFramebuffers;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	void drawFrame(GLFWwindow * window);

	VkSampler textureSampler;
	VkImageView textureImageView;

	std::vector<VkDescriptorSet> descriptorSets;

	struct
	{
		VkRenderPass shadowMapRenderPass;
		VkRenderPass renderPass;
		VkRenderPass postProcessingRenderPass;
	} renderPasses{};

	Pipeline basePipeline; 
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;

	const std::string ROOT_DIR = PROJECT_ROOT_DIR;
	const std::string MODEL_PATH = ROOT_DIR + "/resource/models/Sponza-master/sponza.obj";
	const std::string MODEL_TEXTURE_DIRECTORY = ROOT_DIR + "/resource/models/Sponza-master/";
	const std::string TEXTURE_PATH = ROOT_DIR + "/resource/textures/container.png";
	const std::string CUBEMAP_PATH = ROOT_DIR + "/resource/textures/skybox/";
	const std::string SPECULAR_PATH = ROOT_DIR + "/resource/textures/container_specular.png";
	uint32_t currentFrame = 0;

	// Virtuals
	virtual void createInstance();
	virtual void createSurface(GLFWwindow * window);
	virtual SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	virtual void createLogicalDevice();
	virtual void pickPhysicalDevice();
	virtual bool isDeviceSuitable(VkPhysicalDevice device);
	virtual int rateDeviceSuitability(VkPhysicalDevice device);
	virtual VkSampleCountFlagBits getMaxUsableSampleCount();
	virtual bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	virtual void setupDebugMessenger();
	virtual void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	virtual VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	virtual VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	virtual	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow * window);
	virtual void createSwapChain(GLFWwindow * window);
	virtual void createPipelines();
	virtual std::vector<const char*>getRequiredExtensions();
	virtual void recreateSwapChain(GLFWwindow * window);
	virtual void createImageViews();
	virtual void createRenderPass();
	virtual void createFramebuffers();
	virtual void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	virtual void createVertexBuffers();
	virtual void createUniformBuffers();
	virtual void createGraphicsUniformBuffers();
	virtual void createGraphicsDescriptorSets();
	virtual VkDevice* getDevice();
	virtual void createSyncObjects();
	virtual void cleanupSwapChain();
	virtual void updateUniformBuffer(uint32_t currentImage);
	virtual void createDescriptorSets();
	virtual void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	virtual void init(GLFWwindow * window);
	virtual void moveCamera(double xpos, double ypos);
	virtual void deviceWaitIdle();

	// Pure Virtuals
	IVulkanApp()=default;
	virtual ~IVulkanApp(){};

	virtual void processInput(GLFWwindow * window) = 0;
	virtual void cleanup(GLFWwindow * window) = 0;

	VkDevice device;

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

	static inline const std::vector<Vertex> triangleVertices = {
		{{ -0.5f, -0.5f, -0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f,  0.0f, -1.0f}, {.0, .0}}
	};

	static inline const std::vector<uint32_t> cubeIndices = {
	    0, 2, 1,
	    2, 0, 3,

	    5, 6, 4,
	    7, 4, 6,

	    9,10, 8,
	   11, 8,10,

	   12,14,13,
	   14,12,15,

	   17,18,16,
	   19,16,18,

	   20,22,21,
	   22,20,23
	};

	static inline const std::vector<uint32_t> quadIndices = {
	    4, 5, 6,
	    6, 7, 4,
	};
};
