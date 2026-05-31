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
#include <stb_image.h>
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

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class IVulkanApp
{
	public:
	const std::string ROOT_DIR = PROJECT_ROOT_DIR;
	const std::string MODEL_PATH = ROOT_DIR + "/resource/models/Sponza-master/sponza.obj";
	const std::string MODEL_TEXTURE_DIRECTORY = ROOT_DIR + "/resource/models/Sponza-master/";
	const std::string TEXTURE_PATH = ROOT_DIR + "/resource/textures/container.png";
	const std::string CUBEMAP_PATH = ROOT_DIR + "/resource/textures/skybox/";
	const std::string SPECULAR_PATH = ROOT_DIR + "/resource/textures/container_specular.png";

	virtual void createInstance()=0;
	virtual void createSurface(GLFWwindow * window)=0;
	virtual void setupDebugMessenger()=0;
	virtual void pickPhysicalDevice()=0;
	virtual void createLogicalDevice()=0;
	virtual void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)=0;
	virtual bool isDeviceSuitable(VkPhysicalDevice device)=0;
	virtual int rateDeviceSuitability(VkPhysicalDevice device)=0;
	virtual bool checkDeviceExtensionSupport(VkPhysicalDevice device)=0;
	virtual void createSwapChain(GLFWwindow * window)=0;
	virtual void createImageViews()=0;
	virtual void createRenderPass()=0;
	virtual void createDescriptorSetLayouts()=0;	
	virtual void createComputeDescriptorSetLayout()=0;
	virtual void createPipelines()=0;
	virtual void createFramebuffers()=0;
	virtual void createTextureImageView(VkImage& image, VkImageView& imageView, VkFormat format, VkImageAspectFlagBits flags)=0;
	virtual VkShaderModule createShaderModule(const std::vector<char>& code)=0;
	virtual VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)=0;
	virtual VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)=0;
	virtual	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow * window)=0;
	virtual SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device)=0;
	virtual VkSampleCountFlagBits getMaxUsableSampleCount()=0;
	virtual std::vector<const char*>getRequiredExtensions()=0;
	virtual void createTextureSampler(VkSampler& sampler)=0;
	virtual void createTextureImages(std::vector<VkImage>& images, std::vector<VkDeviceMemory>& imageMemories)=0;
	virtual void createTextureImageViews(std::vector<VkImage>& images, std::vector<VkImageView>& imageViews)=0;
	virtual void createTextureSamplers(std::vector<VkSampler>& samplers)=0;
	virtual void createShaderStorageBuffers()=0;
	virtual void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)=0;
	virtual void createVertexBuffers()=0;
	virtual void createIndexBuffer()=0;
	virtual void createQuadIndexBuffer()=0;
	virtual void createModelIndexBuffers()=0;
	virtual void createModelIndexBuffer(std::vector<uint32_t> m_Indices, VkBuffer& modelBuffer,VkDeviceMemory& modelMemory)=0;
	virtual void createUniformBuffers()=0;
	virtual void createModelUniformBuffers()=0;
	virtual void createDescriptorPools()=0;
	virtual void createComputeDescriptorPool()=0;
	virtual void createDescriptorSets()=0;
	virtual void createComputeCommandBuffers()=0;
	virtual void createSyncObjects()=0;
	virtual void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)=0;
	virtual void updateUniformBuffer(uint32_t currentImage)=0;
	virtual void recordComputeCommandBuffer(VkCommandBuffer commandBuffer)=0;
	virtual void recreateSwapChain(GLFWwindow * window)=0;
	virtual void cleanupSwapChain()=0;
	virtual void setDescriptorSetLayoutBindings()=0;
	IVulkanApp()=default;
	virtual ~IVulkanApp(){};

	virtual void init(GLFWwindow * window)=0;
	virtual void drawFrame(GLFWwindow * window)=0;
	virtual void processInput(GLFWwindow * window)=0;
	virtual void cleanup(GLFWwindow * window)=0;
	virtual void deviceWaitIdle()=0;

	virtual void moveCamera(double xpos, double ypos)=0;
	virtual VkDevice* getDevice()=0;
	VkDevice device;
};
